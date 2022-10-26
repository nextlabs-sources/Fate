#include <windows.h>
#include <iostream>
#include <filter.h>
#include <ntquery.h>
#include <filterr.h>
#include <propidl.h>
#include <cierror.h>
#include <ktmw32.h>
#include "nl_content_cracker.hpp"
#include "nl_content_misc.hpp"
#include <atlcomcli.h>

/***********************************************************************************************
 * NLContentCracker_IFilter
 *
 * This class implements the IFilter cracker.  It wraps the Microsoft IFilter interface and
 * permits text/value extraction via the NLCA::ContentCracker::GetText method.
 *
 * Pre-condition:
 *
 *  COM must be initialized before an instance of NLContentCracker_IFilter can be constrcted.
 *  See MSDN docs for CoInitialize and CoInitializeEx.
 *
 *  When a PDF file is searched using NLContentCracker_IFilter other instances PDF instances
 *  NLContentCracker will be syncrhonized.  The effect will be that search of PDF files in the
 *  same process will be serialized.
 *
 **********************************************************************************************/
class NLContentCracker_IFilter : public NLCA::ContentCracker
{
public:
    /** NLContentCracker_IFilter
     *
     *  \brief Create an IFilter instance for the given file.  Throws HRESULT
     *         if an IFilter object cannot be constructed.
     *
     *  \param file (in) File to construct iFilter for.
     */
    NLContentCracker_IFilter( const wchar_t* file ) :
        pFilter(NULL),            // no open IFilter
        chunk_empty(true),        // no existing chunk
        force_eos(false),         // cancel state is false
        file_name(),              // empty default file name
        file_name_is_temp(false), // no temp file by default
        pdf_mutex(NULL)           // PDF IFilter serialization mutex
    {
        assert( file != NULL );
        memset(&stat,0x00,sizeof(stat));

        file_name = file;
    
        if( file_name.find(L".pdf") != std::wstring::npos )
        {
            pdf_mutex = CreateMutexA(NULL,FALSE,"NLCA_PDFMutex");
            if( pdf_mutex == NULL )
            {
                throw E_FAIL;  // cannot open mutex

            }
            if( WaitForSingleObject(pdf_mutex,INFINITE) != WAIT_OBJECT_0 )
           {
                Cleanup();
                throw E_FAIL;  // cannot acquire object
            }
        }

        HRESULT hr;

        if (nextlabs::windows_fse::is_wfse_installed() && NLCA::CopyFileToTemp(file_name))
        {
            file_name_is_temp = true;
        }

#pragma warning(push)
#pragma warning(disable:6309 6387)
        hr = LoadIFilter(file_name.c_str(),0,(void**)&pFilter);
#pragma warning(pop)

        /* If the file is in use by an application (i.e. MS Word) it may produce a sharing violation.
         * Another problem is that Adobe PDF IFilter (tested with 8.0 or 9.0) fails to access PDF in a remote share because Adobe's IFilter breaks user impersonation.  This results in E_FAIL. 
         * In these cases, the file will be copied to avoid sharing violation and deleted by the destuctor.
         */
        if( (hr == STG_E_SHAREVIOLATION || hr == E_FAIL) && !file_name_is_temp )
        {
            if (NLCA::CopyFileToTemp(file_name))
            {
                file_name_is_temp = true;
#pragma warning(push)
#pragma warning(disable:6309)
                hr = LoadIFilter(file_name.c_str(),0,(void**)&pFilter); // try open of new file
#pragma warning(pop)
            }
        }/* hr == STG_E_SHAREVIOLATION || hr == E_FAIL */

        if( !SUCCEEDED(hr) )
        {
            Cleanup();
            throw hr;
        }

        DWORD flags = 0;
        hr = pFilter->Init(IFILTER_INIT_INDEXING_ONLY |
                           IFILTER_INIT_APPLY_INDEX_ATTRIBUTES |
                           IFILTER_INIT_APPLY_OTHER_ATTRIBUTES,
                           0,0,&flags);
        if( FAILED(hr) )
        {
            Cleanup();
            throw hr;
        }
    }/* NLContentCracker_IFilter */

    ~NLContentCracker_IFilter(void)
    {
        Cleanup();
    }/* ~NLContentCracker_IFilter */

    bool GetText( wchar_t* buf , unsigned long& buf_size , bool& eos )
    {
        assert( buf_size >= MIN_BUF_SIZE );
        eos = force_eos; /* may have been cancelled */

        if( chunk_empty == true ) /* Ensure current chunk is always set */
        {
            HRESULT hr = pFilter->GetChunk(&stat);
            if( hr == FILTER_E_END_OF_CHUNKS )
            {
                eos = true;
            }
            if( hr == FILTER_E_ACCESS ||
                hr == FILTER_E_PARTIALLY_FILTERED )
            {
                return false;
            }
            chunk_empty = false;
        }

        /* Value (non-text attribute such as document name/subject).  Read value and set
         * caller's params, otherwise signal chunk_empty for next iteration.
         */
        if( (stat.flags & CHUNK_VALUE) != 0 )
        {
            PROPVARIANT* pv = NULL;
            HRESULT hr = pFilter->GetValue(&pv);
            if( SUCCEEDED(hr) )
            {
                if( pv->vt == VT_LPWSTR )
                {
                    wcsncpy_s(buf,buf_size-1,pv->pwszVal,_TRUNCATE);             // string into caller's buf
                    buf_size = (unsigned long)(wcslen(pv->pwszVal) + 1) * sizeof(wchar_t);      // set caller's buf size
                }
                else
                {
                    buf_size = 0;      // non-LPWSTR value - don't care
                }
                PropVariantClear(pv);
                CoTaskMemFree(pv);   // free value per GetValue
            }
            else
            {
                chunk_empty = true;  // Out of values from GetValue()
                buf_size = 0;        // Nothing for caller to read
            }
            return true;
        }/* if CHUNK_VALUE */

        buf_size--;  // space for termination
		
        if( ((stat.flags & CHUNK_TEXT) != 0 ) && 
            (stat.breakType == CHUNK_EOP))
        { 
          // We need to also add a newline between this buffer and the previous one. Give GetText less
          // space to work with
          buf_size--;
          buf[0] = (wchar_t)'\n';
          buf++;
        }
		
        HRESULT hr = pFilter->GetText((ULONG*)&buf_size,buf);
        switch( hr )
        {
            case FILTER_E_NO_TEXT:
            case FILTER_E_NO_MORE_TEXT:
            case FILTER_S_LAST_TEXT:
                chunk_empty = true;
                break;
        }

        if( ((stat.flags & CHUNK_TEXT) != 0 ) && 
            (stat.breakType == CHUNK_EOP))
        {
          // Set things back the way they were
          buf--;
          buf_size++;
        }

        if( SUCCEEDED(hr) && buf_size > 0 )
        {
            buf[buf_size] = (wchar_t)NULL;      /* ensure termination */
        }
        else
        {
            buf_size = 0;                       /* failure of some kind */
        }
        return true;
    }/* GetText */

    /** Cancel
     *
     *  \brief Cancel the IFilter text extraction.  This will force and end-of-stream condition
     *         which will cause GetText to issue end of stream to its caller.
     *
     *  \return true always.
     */
    bool Cancel(void)
    {
        force_eos = true;
        return false;
    }/* Cancel */

private:

    void Cleanup(void)
    {
        /* If a temp file was used remove it. */
        if( file_name_is_temp == true )
        {
            DeleteFile(file_name.c_str());
        }
        if( pdf_mutex != NULL )
        {
            ReleaseMutex(pdf_mutex);
            CloseHandle(pdf_mutex);
        }
    }

    CComPtr<IFilter> pFilter;       /* filter instance */
    STAT_CHUNK stat;        /* current chunk state */
    bool chunk_empty;       /* current chunk is empty? */
    bool force_eos;         /* force end-of-stream (eos)? */
    std::wstring file_name; /* file name */
    bool file_name_is_temp; /* file name is temporary and requires deletion? */
    HANDLE pdf_mutex;       /* PDF IFilter serialization mutex */

};/* NLContentCracker_IFilter */
