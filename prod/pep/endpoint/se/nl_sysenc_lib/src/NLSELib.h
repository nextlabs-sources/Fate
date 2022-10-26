#ifndef __NLSE_LIB_H__
#define __NLSE_LIB_H__

#define CELOG_CUR_MODULE L"NLSELib"

extern int ensureInited(void);
extern bool get_next_path_component(_In_z_ const wchar_t* in_path,
                                    _Inout_ size_t *index,
                                    _Out_ wchar_t** component);
extern bool is_matching_wildcard_path(_In_z_ const wchar_t* in_path,
                                      _In_ const wchar_t* wildcard_path);
extern bool can_encrypt( _In_z_ const wchar_t* in_path );
extern bool is_NLSE_service_running(void);
extern nlse_enc_error_t checkPassword(_In_z_ const wchar_t *password);
extern nlse_enc_error_t initCryptoContext(HCRYPTPROV *phProv);
extern nlse_enc_error_t freeCryptoContext(HCRYPTPROV hProv);
extern nlse_enc_error_t initCryptoKey(HCRYPTPROV hProv,
                                      const BYTE key[NLE_KEY_LENGTH_IN_BYTES],
                                      ULONGLONG initVector, HCRYPTKEY *phKey);
extern nlse_enc_error_t freeCryptoKey(HCRYPTKEY hKey);
extern nlse_enc_error_t getPCKey(const char *pcKeyRingName,
                                NLSE_KEY_ID *pcKeyId,
                                unsigned char pcKey[NLE_KEY_LENGTH_IN_BYTES]);

VOID WINAPI KeepProcessTrusted() ;

#endif /* __NLSE_LIB_H__ */
