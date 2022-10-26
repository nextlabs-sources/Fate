

#ifndef _GPG_UTILITY_H_
#define _GPG_UTILITY_H_

#include <locale.h>
#include <string>
#include "gpg/gpgme.h"

class GpgUtility
{
public:
    GpgUtility()
    {
        gpgme_error_t err;
        m_valid = FALSE;
        gpgme_check_version (NULL);
        setlocale (LC_ALL, "");
        gpgme_set_locale (NULL, LC_CTYPE, setlocale (LC_CTYPE, NULL));
        err = gpgme_engine_check_version (GPGME_PROTOCOL_OpenPGP);
        if (0 == err)
        {
            err = gpgme_new (&m_ctx);
            if(err) return;
            err = gpgme_set_protocol (m_ctx, GPGME_PROTOCOL_OpenPGP);
            if(err)
            {
                gpgme_release (m_ctx);
                m_ctx = 0;
                return;
            }
            gpgme_set_armor (m_ctx, 1);
            m_valid = TRUE;
        }
    }
    virtual ~GpgUtility()
    {
        if(m_ctx) gpgme_release (m_ctx); m_ctx = 0; 
        m_valid = FALSE;
    }

    std::string import_key_from_file(const char* key_file)
    {
        gpgme_error_t           err;
        gpgme_data_t            in;
        gpgme_import_result_t   result;
        std::string             strFpr = "";
        gpgme_import_status_t pImport = 0;

        if(!m_valid) return strFpr;

        err = gpgme_data_new_from_file (&in, key_file, 1);
        if(err) goto _exit;

        err = gpgme_op_import (m_ctx, in);
        if(err) goto _exit;
        result = gpgme_op_import_result (m_ctx);

        pImport = result->imports;
        while (pImport)
        {
            if (pImport->fpr)
            {
                strFpr = pImport->fpr;
                goto _exit;
            }
            pImport = pImport->next;
        }
_exit:
        if(in) gpgme_data_release (in);
        return strFpr;
    }

    unsigned int encrypt_file_by_keyfile(const char* src, const char* key_file, const char* dest=0)
    {
        if(!m_valid) return 1001;
        const std::string& strFpr = import_key_from_file(key_file);
        if(strFpr.length()<=0)
            return 1000;
        return encrypt_file_by_keyfpr(src, strFpr.c_str(), dest);
    }

    unsigned int encrypt_file_by_keyfpr(const char* src, const char* key_fpr, const char* dest=0)
    {
        gpgme_error_t   err = 0;
        gpgme_data_t    in = NULL, out = NULL;
        gpgme_encrypt_result_t result;
        gpgme_key_t     key[2] = {NULL, NULL};
        std::string strOut = "";

        if(!m_valid) return 1000;

        HANDLE hFile = CreateFileA(src, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
        if(INVALID_HANDLE_VALUE == hFile)
            return 1;
        DWORD dwSize = GetFileSize(hFile, NULL);
        CloseHandle(hFile);
        if(0==dwSize) return 2;

        err = gpgme_get_key (m_ctx, key_fpr, &key[0], 0);
        if(err) goto _exit;
        err = gpgme_data_new_from_file(&in, src, dwSize);
        if(err) goto _exit;
        err = gpgme_data_new (&out);
        if(err) goto _exit;

        err = gpgme_op_encrypt (m_ctx, key, GPGME_ENCRYPT_ALWAYS_TRUST, in, out);
        result = gpgme_op_encrypt_result (m_ctx);
        if (result->invalid_recipients)
        {
            fprintf (stderr, "Invalid recipient encountered: %s\n", result->invalid_recipients->fpr);
            goto _exit;
        }

        if(dest) strOut = dest;
        else
        {
            strOut = src;
            strOut += ".gpg";
        }
        err = export_cipher_2_file(out, strOut.c_str());

_exit:
        if(key[0]) gpgme_key_unref (key[0]);
        if(in)     gpgme_data_release (in);
        if(out)    gpgme_data_release (out);
        return err;
    }

protected:
    unsigned int export_cipher_2_file(gpgme_data_t dh, const char* file)
    {
#define EXPORT_BUF_SIZE 512
        unsigned int  ret = 1;
        char buf[EXPORT_BUF_SIZE + 1];
        memset(buf, 0, sizeof(buf));

        ret = gpgme_data_seek (dh, 0, SEEK_SET);
        if (ret)
            goto _exit;

        FILE* fd = NULL;
		fopen_s(&fd, file, "w");
        if(NULL==fd)
            goto _exit;

        while ((ret = gpgme_data_read (dh, buf, EXPORT_BUF_SIZE)) > 0)
            fwrite(buf, ret, 1, fd);
        ret = 0;

_exit:
        if(fd) fclose(fd);
        fd = 0;
        return ret;
    }

private:
    BOOL            m_valid;
    gpgme_ctx_t     m_ctx;
};

#endif