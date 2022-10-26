// ftag.cpp : Defines the entry point for the console application.
//

#ifdef _CRT_SECURE_NO_WARNINGS 
#undef _CRT_SECURE_NO_WARNINGS 
#define _CRT_SECURE_NO_WARNINGS  1
#endif 
#include "stdafx.h"
#include <stdio.h>
#include <windows.h>
#include <string.h>
#include <string>
#include <map>
#include "hpdf.h"
#include <AtlBase.h>
#include <atlconv.h>
#include <wchar.h>
#include "pdflib.h"


#ifdef HPDF_DLL
void  __stdcall
#else
void
#endif
error_handler  (HPDF_STATUS   error_no,
                HPDF_STATUS   detail_no,
                void         *user_data)
{
    printf ("ERROR: error_no=%04X, detail_no=%u\n", (HPDF_UINT)error_no,
            (HPDF_UINT)detail_no);

}

void PDFGet_FreePropValue(WCHAR *wsPropValue)
{
    delete[] wsPropValue;
}

int PDFGet_PDFLib(const WCHAR *wsFileName, const WCHAR *wsPropName, WCHAR **wsPropValue)
{
    PDF *p;
    int ret=0;
    char *pszStr;
    char sNewFileName[256]="";
    USES_CONVERSION ;
	
    *wsPropValue = NULL;

    if ((p = PDF_new()) == (PDF *) 0)
    {
        // printf("Couldn't create PDFlib object (out of memory)!\n");
        return(2);
    }

    PDF_TRY(p) 
    {
        int pdfDoc=0;
        int page=0,pageno=1;
        char sPropPath[512];

        PDF_set_parameter(p, "errorpolicy", "return");
        pszStr=W2A(wsFileName);

        GetTempFileNameA(".", "CE", 0, sNewFileName);

        if (PDF_begin_document(p, sNewFileName, 0, "") == -1)
        {
            // printf("Error: %s\n", PDF_get_errmsg(p));
            return(2);
        }

        if ((pdfDoc=PDF_open_pdi_document(p, pszStr, 0, ""))== -1) 
        {
            // printf("Error: %s\n", PDF_get_errmsg(p));
            return(2);
        }

		
        USES_CONVERSION ;
        pszStr=W2A(wsPropName);
        _snprintf_s(sPropPath,512, _TRUNCATE, "type:/Info/%s",pszStr);
        const char* pObjtype = PDF_pcos_get_string(p,pdfDoc, sPropPath);
        if(strncmp(pObjtype,"string",6)==0)
        {
            _snprintf_s(sPropPath,512, _TRUNCATE, "/Info/%s",pszStr);
            const char *psValue = PDF_pcos_get_string(p,pdfDoc, sPropPath);
            if(psValue[0]==0)
                ret=0;
            else
            {
                WCHAR* pPropValue=A2W(psValue);
                *wsPropValue = new WCHAR[wcslen(pPropValue)+1];
                wcsncpy_s(*wsPropValue,wcslen(pPropValue)+1,pPropValue, _TRUNCATE);
                ret=1;
            }
        }

        PDF_begin_page_ext(p, a4_width, a4_height, "");
        PDF_end_page_ext(p, "");
		
        PDF_close_pdi_document(p, pdfDoc);
        PDF_end_document(p, "");

    }

    PDF_CATCH(p) 
    {
        // printf("PDFlib exception occurred in hello sample:\n");
        // printf("[%d] %s: %s\n", PDF_get_errnum(p), PDF_get_apiname(p), PDF_get_errmsg(p));
        PDF_delete(p);
        return(2);
    }

    PDF_delete(p);

    if (sNewFileName[0] != '\0')
    {
        DeleteFileA(sNewFileName);
    }
    return ret;

}

int PDFSet_PDFLib(const WCHAR *wsFileName,const WCHAR *wsPropName,const WCHAR* wsPropValue)
{
    USES_CONVERSION ;
    int ret=0;
    PDF *p;
    char sNewFileName[256]="";
	
    if ((p = PDF_new()) == (PDF *) 0)
    {
        // printf("Couldn't create PDFlib object (out of memory)!\n");
        return(2);
    }

    PDF_TRY(p) 
    {
        char *pszStr;
        double fPageCount=0;
        int		iPageCount=0;
        int pdfDoc=0;
        int page=0,iPageIndex;

        PDF_set_parameter(p, "errorpolicy", "return");

        pszStr=W2A(wsFileName);
        GetTempFileNameA(".", "CE", 0, sNewFileName);

        if (PDF_begin_document(p, sNewFileName, 0, "") == -1) 
        {
            // printf("Error: %s\n", PDF_get_errmsg(p));
            return(2);
        }

        if ((pdfDoc=PDF_open_pdi_document(p, pszStr, 0, ""))== -1) {
            // printf("Error: %s\n", PDF_get_errmsg(p));
            return(2);
        }
		
        fPageCount=PDF_pcos_get_number(p,pdfDoc, "length:pages");
        iPageCount=(int)(fPageCount+0.5);

        const char *pType=PDF_pcos_get_string(p,pdfDoc,"type:/Info");//dict
        double	    fInfoNum=PDF_pcos_get_number(p,pdfDoc,"length:/Info");
        int iInfoNum=(int)(fInfoNum+0.5);
        for(int iInfoIndex=0;iInfoIndex<iInfoNum;iInfoIndex++)
        {
            const char *pType=PDF_pcos_get_string(p,pdfDoc,"type:/Info[%d].val",iInfoIndex);
            const char *pKey =PDF_pcos_get_string(p,pdfDoc,"/Info[%d].key",iInfoIndex);
            const char *pVal =PDF_pcos_get_string(p,pdfDoc,"/Info[%d].val",iInfoIndex);

            if(strncmp(pKey,"CreationDate",12)&&strncmp(pKey,"Producer",8)&&strncmp(pKey,"ModDate",7))
                PDF_set_info(p,pKey,pVal);
            // printf("%15s: %s[%s]\n",pKey,pVal,pType);

        }
        char *psPropName=W2A(wsPropName);
        char *psPropVal=W2A(wsPropValue);

        if(strncmp(psPropName,"CreationDate",12)&&
           strncmp(psPropName,"Producer",8)&&
           strncmp(psPropName,"ModDate",7))
        {
            PDF_set_info(p,psPropName,psPropVal);
        }

        double fWidth=a4_width,fHeight=a4_height;
		
        iPageIndex=1;
        for(iPageIndex;iPageIndex<=iPageCount;iPageIndex++)
        {
            fWidth=a4_width,fHeight=a4_height;
            fWidth =PDF_pcos_get_number(p, pdfDoc, "pages[%d]/width",iPageIndex-1);
            fHeight=PDF_pcos_get_number(p, pdfDoc, "pages[%d]/height",iPageIndex-1);
            page = PDF_open_pdi_page(p,pdfDoc, iPageIndex, "");
            PDF_begin_page_ext(p,fWidth, fHeight, "");
            PDF_fit_pdi_page(p,page, 0, 0, "adjustpage");
            PDF_close_pdi_page(p,page);
            PDF_end_page_ext(p,"");
        }
		
        PDF_close_pdi_document(p, pdfDoc);
        PDF_end_document(p, "");

    }

    PDF_CATCH(p) 
    {
        // printf("PDFlib exception occurred in PDFSet_PDFLib:\n");
        // printf("[%d] %s: %s\n", PDF_get_errnum(p), PDF_get_apiname(p), PDF_get_errmsg(p));
        PDF_delete(p);
        return(2);
    }

    PDF_delete(p);
	
    TCHAR*pwsNewFileName=A2W(sNewFileName);
    DeleteFile(wsFileName);
    ret=MoveFile(pwsNewFileName,wsFileName);
	
    return ret;
}


BOOL IsPDFFile(LPCWSTR pwzFile)
{
    LPCWSTR pSuffix = wcsrchr(pwzFile, L'.');
    if(NULL == pSuffix) 
        return FALSE;
    
    if(0 == _wcsicmp(pSuffix, L".pdf") ||0 == _wcsicmp(pSuffix, L".PDF"))
        return TRUE;
    
    return FALSE;
}

static const WCHAR *findchar(const WCHAR *start, WCHAR sep)
{
    while (*start != L'\0' && *start != sep)
    {
        start++;
    }

    if (*start == L'\0')
    {
        return NULL;
    }

    return start;
}

typedef std::map<std::wstring, std::wstring> wstrmap;

/*
 * We need a way to get all the properties of a PDF.  PDFlib doesn't seem to give us this ability.
 * Instead, I'm putting all of the properties as carriage return separated keys and values under
 * CE_PROPERTY_SEPARATOR
 */
#define CE_PROPERTY_SEPARATOR L'\n'
#define CE_PDF_PROPERTY_NAME L"CE::Properties"
void GetPDFFileProps(const WCHAR *fileName, wstrmap &results)
{
    WCHAR *wsPropValue = NULL;
    int ret = PDFGet_PDFLib(fileName, CE_PDF_PROPERTY_NAME, &wsPropValue);

    if (wsPropValue == NULL)
    {
        return;
    }

    // Normally I'd use wcstok_s, but that doesn't exist in VC7 (I think)
    const WCHAR *startOfSubstring = wsPropValue;

    while(1)
    {
        const WCHAR *endOfSubstring = findchar(startOfSubstring, CE_PROPERTY_SEPARATOR);
        if (endOfSubstring == NULL)
        {
            break;
        }

        std::wstring key(startOfSubstring, endOfSubstring-startOfSubstring);

        startOfSubstring = endOfSubstring+1;
        endOfSubstring = findchar(startOfSubstring, CE_PROPERTY_SEPARATOR);

        if (endOfSubstring == NULL)
        {
            std::wstring value(startOfSubstring);
            results[key] = value;
            break;
        }
        else
        {
            std::wstring value(startOfSubstring, endOfSubstring-startOfSubstring);
            results[key] = value;
            startOfSubstring = endOfSubstring+1;
        }
    }


    PDFGet_FreePropValue(wsPropValue);

    return;
}

static void SetPDFFileProps(const WCHAR *fileName, wstrmap &results)
{
    std::wstring value = L"";

    BOOL needsSeparator = FALSE;

    // Convert map to string
    for (wstrmap::const_iterator iter = results.begin();
         iter != results.end();
         ++iter)
    {
        if (needsSeparator)
        {
            value += CE_PROPERTY_SEPARATOR;
        }
        else
        {
            needsSeparator = TRUE;
        }

        value += (*iter).first;
        value += CE_PROPERTY_SEPARATOR;
        value += (*iter).second;
    }

    PDFSet_PDFLib(fileName, CE_PDF_PROPERTY_NAME, value.c_str());

    return;
}

void SetPDFFileProp(const WCHAR *fileName, const WCHAR *key, const WCHAR *value)
{
    wstrmap results;

    GetPDFFileProps(fileName, results);

    results[key] = value;

    SetPDFFileProps(fileName, results);
}
