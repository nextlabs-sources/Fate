#include <string>
#include <iostream>
#include <errno.h>
#include "resattrlib.h"
#include "pdfattrs.h"

// PoDoFo pdf library
#pragma warning(push)
#pragma warning(disable: 4244 4512)
#include "podofo.h"
#include "PdfError.h"
#include "PdfString.h"
#include "PdfDictionary.h"
#include "PdfMemDocument.h"
#pragma warning(pop)
#include "nlconfig.hpp"

// for CELog2
#include "NLCELog.h"
#define  CELOG_CUR_FILE static_cast<celog_filepathint_t>(EMNLFILEINTEGER_PDFATTRSCPP)

#ifdef TAGLIB_WINDOWS
#define CHARTYPE wchar_t
#include "stdafx.h"
#include "FileAttributeReaderWriter.h"
#else
#define CHARTYPE char
#endif

extern wchar_t g_szCurrentDir[MAX_PATH + 1];

using namespace PoDoFo;

#ifdef TAGLIB_WINDOWS

struct TAG 
{
	char szTagName[1025];
	char szTagValue[4097];
};

HMODULE g_hModPDFLib = NULL;
typedef int (*PDF_Func)(const char* pszFileName, void* pParam, int nCount);
typedef int (*PDF_ReadFunc)(const char* pszFileName, void** pParam, int* nCount);
typedef bool (*PDF_IsPDFFileType)(const char* pszFileName);
typedef int (*PDF_ReadSummaryFunc)(const char* pszFileName, void** pParam, int* nCount);

PDF_ReadFunc lfPDF_ReadTags = NULL;
PDF_Func lfPDF_WriteTags = NULL;
PDF_Func lfPDF_DeleteTags = NULL;
PDF_IsPDFFileType lfPDF_IsPDFFile = NULL;
PDF_ReadSummaryFunc lfPDF_ReadSummaryTags = NULL;


static std::wstring GetCommonComponentsDir()
{
	wchar_t szDir[MAX_PATH] = {0};
	if(NLConfig::ReadKey(L"SOFTWARE\\NextLabs\\CommonLibraries\\InstallDir", szDir, MAX_PATH))
	{
#ifdef _WIN64
		wcsncat_s(szDir, MAX_PATH, L"\\bin64\\", _TRUNCATE);
#else
		wcsncat_s(szDir, MAX_PATH, L"\\bin32\\", _TRUNCATE);
#endif
		return szDir;
	}

	return L"";
}

std::string MyWideCharToMultipleByte(const std::wstring & strValue)
{
	int nLen = WideCharToMultiByte(CP_ACP, 0, strValue.c_str(), (int)strValue.length(), NULL, 0, NULL, NULL); 

	char* pBuf = new char[nLen + 1];
	if(!pBuf)
		return "";
	memset(pBuf, 0, nLen +1);
	nLen = WideCharToMultiByte(CP_ACP, 0, strValue.c_str(), (int)strValue.length(), pBuf, nLen, NULL, NULL); 

	std::string strResult(pBuf);

	if(pBuf)
	{
		delete []pBuf;
		pBuf = NULL;
	}
	return strResult;
}

bool LoadPDFLib()
{
	std::wstring strCommonPath;
	if(wcslen(g_szCurrentDir) > 3)//try to find pdflib in the same folder with resattrmgr.dll first.
		strCommonPath = g_szCurrentDir;
	else
		strCommonPath = GetCommonComponentsDir();

#ifdef _WIN64
	std::wstring strPDFLib = strCommonPath + L"\\pdflib.dll";
#else
	std::wstring strPDFLib = strCommonPath + L"\\pdflib32.dll";
#endif
	SetDllDirectoryW(strCommonPath.c_str());
	g_hModPDFLib = LoadLibraryW(strPDFLib.c_str());
	SetDllDirectoryW(L"");	// remove the current set one

	if(g_hModPDFLib == NULL)
	{
		return false;
	}

	lfPDF_ReadTags = (PDF_ReadFunc)GetProcAddress(g_hModPDFLib, "PDF_ReadTags");
	lfPDF_WriteTags = (PDF_Func)GetProcAddress(g_hModPDFLib, "PDF_AddTags");
	lfPDF_DeleteTags = (PDF_Func)GetProcAddress(g_hModPDFLib, "PDF_DeleteTags");
	lfPDF_IsPDFFile = (PDF_IsPDFFileType)GetProcAddress(g_hModPDFLib, "PDF_IsPDFFile");
	lfPDF_ReadSummaryTags = (PDF_ReadFunc)GetProcAddress(g_hModPDFLib, "PDF_ReadSummaryTags");
	

	return g_hModPDFLib && lfPDF_ReadTags && lfPDF_WriteTags && lfPDF_DeleteTags && lfPDF_IsPDFFile&&lfPDF_ReadSummaryTags?true: false;
}
#endif

BOOL IsPDFFile(const CHARTYPE *pwzFile)
{
	if (pwzFile == NULL)
	{
		return FALSE;
	}

#ifdef TAGLIB_WINDOWS
	static bool bInit = false;
	if(!bInit)
	{
		bInit = LoadPDFLib();
	}

	if(bInit && lfPDF_IsPDFFile)
	{
		bool bRet = FALSE;
		std::string strPath = MyWideCharToMultipleByte(std::wstring(pwzFile));
		//lfPDF_IsPDFFile(strPath.c_str());
		if (strPath.length() < 4)
		{
			return FALSE;
		}
		
		if(0 == _stricmp(strPath.c_str()+strPath.length()-4,".pdf"))
		{	
			bRet = TRUE;
		}

		return bRet;
	}
#else
	const char* pSuffix = strrchr(pwzFile, L'.');

	if(NULL == pSuffix) 
		return FALSE;
	if(0 == strcasecmp(pSuffix, ".pdf"))
		return TRUE;
#endif
    
    return FALSE;
}

#ifdef TAGLIB_WINDOWS
static std::string GetPDFTempPath(LPCSTR pszFileName)
{
    if(!pszFileName)
        return ".";

    char szPathTemp[MAX_PATH * 2 + 1] =  {0};
    strncpy_s(szPathTemp, MAX_PATH * 2, pszFileName, _TRUNCATE);

    char* p = strrchr(szPathTemp, '\\');
    if(p)
    {
        *p = '\0';
    }
    return szPathTemp;
}
#endif

BOOL FindTagNameWithoutCaseSensitive(PdfDictionary& dic, const char *pszTagName, std::string& strTagName)
{
    if(!pszTagName)
        return FALSE;

    BOOL bSuccess = FALSE;
    try {
        for (TKeyMap::const_iterator iter = dic.GetKeys().begin();
             iter != dic.GetKeys().end();
             ++iter)
        {
            PdfObject *obj = iter->second;

            if (obj && obj->IsString())
            {
                std::string key = (iter->first).GetName();

                if(key.length() > 15)
                    continue;

#ifdef TAGLIB_WINDOWS
                if(0 == _memicmp(key.c_str(), pszTagName, key.length()))
#else
                    if(0 == strncasecmp(key.c_str(), pszTagName, key.length()))
#endif
                    {
                        strTagName = key;
                        bSuccess = TRUE;
                        break;
                    }
            }
        }

  

    } catch (PdfError& err) {
        std::cout << "Caught an error!" << std::endl;
        err.PrintErrorMsg();
        bSuccess = FALSE;
    }
    return bSuccess;
}


BOOL GetPDFFileProps(const CHARTYPE *fileName, ResourceAttributes *attrs)
{NLCELOG_ENTER
#ifdef TAGLIB_WINDOWS
	if(!lfPDF_ReadTags)
		NLCELOG_RETURN_VAL( FALSE );

	std::string strFilePath = MyWideCharToMultipleByte(std::wstring(fileName));

	TAG * pTags = NULL;
	int nTagCount = 0;

	lfPDF_ReadTags(strFilePath.c_str(), (void**)&pTags, &nTagCount);

	if(pTags != NULL && nTagCount > 0)
	{
		for(int i = 0; i < nTagCount; i++)
		{
			AddAttributeA(attrs, pTags[i].szTagName, pTags[i].szTagValue);
		}

		delete [] pTags;
		pTags = NULL;
	}
	
	NLCELOG_RETURN_VAL( TRUE )
#else
    const char *fileNameIn=fileName;

    BOOL bSuccess = TRUE;
    try {
        PdfMemDocument pdfDoc(fileNameIn);

        PdfDictionary dictionary = pdfDoc.GetInfo()->GetObject()->GetDictionary();

        for (TKeyMap::const_iterator iter = dictionary.GetKeys().begin();
             iter != dictionary.GetKeys().end();
             ++iter)
        {
            PdfObject *obj = iter->second;

            if (obj && obj->IsString())
            {
                std::string key = (iter->first).GetName();
    
                if(key.length() > 15)
                    continue;

                const char *value = obj->GetString().GetString();

                AddAttributeA(attrs, key.c_str(), value);
            }
        }

        bSuccess = TRUE;

    } catch (PdfError& err) {
        std::cout << "Caught an error!" << std::endl;
        err.PrintErrorMsg();
        bSuccess = FALSE;
    }

    NLCELOG_RETURN_VAL( bSuccess )

#endif
}


BOOL GetPDFSummaryProps(const CHARTYPE *fileName, ResourceAttributes *attrs)
{NLCELOG_ENTER
#ifdef TAGLIB_WINDOWS
	if(!lfPDF_ReadSummaryTags)
	NLCELOG_RETURN_VAL( FALSE );

	std::string strFilePath = MyWideCharToMultipleByte(std::wstring(fileName));

	TAG * pTags = NULL;
	int nTagCount = 0;

	lfPDF_ReadSummaryTags(strFilePath.c_str(), (void**)&pTags, &nTagCount);

	if(pTags != NULL && nTagCount > 0)
	{
		for(int i = 0; i < nTagCount; i++)
		{
			AddAttributeA(attrs, pTags[i].szTagName, pTags[i].szTagValue);
		}

		delete [] pTags;
		pTags = NULL;
	}

NLCELOG_RETURN_VAL( TRUE )
#else
	const char *fileNameIn=fileName;

	BOOL bSuccess = TRUE;
	try {
		PdfMemDocument pdfDoc(fileNameIn);

		PdfDictionary dictionary = pdfDoc.GetInfo()->GetObject()->GetDictionary();

		for (TKeyMap::const_iterator iter = dictionary.GetKeys().begin();
			iter != dictionary.GetKeys().end();
			++iter)
		{
			PdfObject *obj = iter->second;

			if (obj && obj->IsString())
			{
				std::string key = (iter->first).GetName();

				if(key.length() > 15)
					continue;

				const char *value = obj->GetString().GetString();

				AddAttributeA(attrs, key.c_str(), value);
			}
		}

		bSuccess = TRUE;

	} catch (PdfError& err) {
		std::cout << "Caught an error!" << std::endl;
		err.PrintErrorMsg();
		bSuccess = FALSE;
	}

NLCELOG_RETURN_VAL( bSuccess )

#endif
}

static BOOL ModifyPDFFileProps(const CHARTYPE *fileName, ResourceAttributes *attrs, bool bRemove)
{	
	    NLCELOG_ENTER
		if (attrs == NULL || GetAttributeCount(attrs) <= 0 || !lfPDF_DeleteTags || !lfPDF_WriteTags)
		{
			NLCELOG_RETURN_VAL( FALSE )
		}

#ifdef TAGLIB_WINDOWS
		int count = GetAttributeCount(attrs);

		TAG* pTags = new TAG[count];
		if(!pTags) NLCELOG_RETURN_VAL( FALSE )
			int nIndex = 0;
		memset(pTags, 0, sizeof(TAG) * count);
		for (int i = 0; i < count; ++i)
		{
			const WCHAR *attrName = GetAttributeName(attrs, i);
			const WCHAR *attrValue = GetAttributeValue(attrs, i);

			if(!attrName || !attrValue)
				continue;

			std::string strName = MyWideCharToMultipleByte(std::wstring(attrName));
			std::string strValue = MyWideCharToMultipleByte(std::wstring(attrValue));

			strncpy_s(pTags[nIndex].szTagName, sizeof(pTags[nIndex].szTagName), strName.c_str(), _TRUNCATE);
			strncpy_s(pTags[nIndex].szTagValue, sizeof(pTags[nIndex].szTagValue), strValue.c_str(), _TRUNCATE);

			nIndex++;
		}

		std::string strFileName = MyWideCharToMultipleByte(std::wstring(fileName));
		int nRet = -1;
		if(bRemove)//detele tags
		{
			nRet = lfPDF_DeleteTags(strFileName.c_str(), pTags, nIndex);
		}
		else
		{
			nRet = lfPDF_WriteTags(strFileName.c_str(), pTags, nIndex);
		}


		delete []pTags;
		pTags = NULL;

		NLCELOG_RETURN_VAL( nRet == 0? TRUE: FALSE )
#else
		const char *fileNameIn=fileName;
		std::string strFullPath,strNewFileName;
		strNewFileName=fileName;
		std::string::size_type pos=strNewFileName.rfind("/");
		if(pos!=std::string::npos)
			strFullPath=strNewFileName.substr(0,pos+1);
		else
			strFullPath="";
		if(NSLinuxTagLib::Utils::GetTempFile(fileName,strNewFileName)==FALSE)
			NLCELOG_RETURN_VAL( FALSE )

			strFullPath+=strNewFileName;
		char *mbNewFileName = new char[strFullPath.length()+1];
		strncpy_s(mbNewFileName,strFullPath.length()+1,strFullPath.c_str(),_TRUNCATE);



		BOOL bSuccess = TRUE;
		try 
		{
			PdfMemDocument pdfDoc(fileNameIn);

			PdfDictionary& dictionary = pdfDoc.GetInfo()->GetObject()->GetDictionary();

			int count = GetAttributeCount(attrs);

			for (int i = 0; i < count; ++i)
			{

				const char *mbAttrName = GetAttributeName(attrs, i);

				if(bRemove)
				{
					if(bSuccess)
						bSuccess = dictionary.RemoveKey(PdfName(mbAttrName));
				}
				else
				{
					std::string strTagName;
					if(FindTagNameWithoutCaseSensitive(dictionary, mbAttrName, strTagName))
					{
						dictionary.RemoveKey(PdfName(strTagName.c_str()));
					}


					const char *mbAttrValue = GetAttributeValue(attrs, i);

					dictionary.AddKey( PdfName(mbAttrName), PdfString(mbAttrValue) );

				}

			}
			DeleteFileA(mbNewFileName);
			pdfDoc.Write(mbNewFileName);
			bSuccess = TRUE;
		}
		catch (PdfError& err) 
		{
			std::cout << "Caught an error setting PDF tags.  Error code: " << err.GetError() << std::endl;
			err.PrintErrorMsg();
			bSuccess = FALSE;
		}


		if(bSuccess)
		{
			BOOL bRet = rename(mbNewFileName,fileName);

			if(bRet)
			{
				printf("Failed to rename (Add tags for PDF files) errno=%d",errno);
				remove(mbNewFileName);
				NLCELOG_RETURN_VAL( FALSE )
			}
		}
		else
			remove(mbNewFileName);


		delete[] mbNewFileName;

		NLCELOG_RETURN_VAL( bSuccess )
#endif
}

BOOL SetPDFFileProps(const CHARTYPE *fileName, ResourceAttributes *attrs)
{
	return ModifyPDFFileProps(fileName,attrs,false);
}

BOOL RemovePDFFileProps(const CHARTYPE *fileName, ResourceAttributes *attrs)
{
    return ModifyPDFFileProps(fileName,attrs,true);
}
