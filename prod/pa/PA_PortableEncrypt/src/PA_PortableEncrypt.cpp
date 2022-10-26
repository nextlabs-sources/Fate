// PortableEncryptPA.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

#include "PA_PortableEncrypt.h"
#include "NextLabsEncryption_Types.h"
#include "nl_sysenc_lib.h"
#include "resattrmgr.h"

 PA_STATUS ParseObligationList(OBJECTINFO &file, PE_Obligation &obligation)
{
	OBLIGATIONLIST::iterator iterObligation;

	POBLIGATION lpOligation = NULL;
	for (iterObligation = file.obList.begin(); iterObligation != file.obList.end(); iterObligation++)
	{
		if (!_wcsicmp((*iterObligation).strOBName.c_str(), L"AUTOWRAP"))
		{
			lpOligation = (POBLIGATION)&(*iterObligation);
		}
	}

	if (lpOligation)
	{
		ATTRIBUTELIST::iterator iterAttribute;

		obligation.obType = PE_AUTOWRAP ;
		for (iterAttribute = lpOligation->attrList.begin();
			iterAttribute != lpOligation->attrList.end(); iterAttribute++)
		{
			if (!_wcsicmp((*iterAttribute).strValue.c_str(), L"LogId"))
			{
				iterAttribute++;
				obligation.wstrLogId = (*iterAttribute).strValue.c_str();
			}
		}
	}

	return PA_SUCCESS;
}
 BOOL WINAPI SaveDeleteFile(const wchar_t * filePath )
 {
	 BOOL bRet = ::DeleteFile( filePath ) ;
	if( bRet == FALSE )
	{
		DWORD derror = ::GetLastError() ;
		if( derror == ERROR_FILE_NOT_FOUND )
		{
			bRet = TRUE ;
		}
	}
	return bRet ;
 }

 static BOOL CopyTags(LPCWSTR wzSource, LPCWSTR wzTarget)
 {
     ResourceAttributeManager *mgr   = NULL;
     ResourceAttributes       *attrs = NULL;

     //
     // Init
     //
     AllocAttributes(&attrs);
     if(NULL == attrs) return FALSE;
     CreateAttributeManager(&mgr);
     if(NULL == mgr) {FreeAttributes(attrs); return FALSE;}

     //
     // Get attributes
     //
     if(0 == ReadResourceAttributesW(mgr, wzSource, attrs))
     {
         FreeAttributes(attrs);
         CloseAttributeManager(mgr);
         return FALSE;
     }

     //
     // Add source file tags to target NXL file.  (NXL_xxx tags will be silently ignored.)
     //
     if(0 == WriteResourceAttributesW(mgr, wzTarget, attrs))
     {
         FreeAttributes(attrs);
         CloseAttributeManager(mgr);
         return FALSE;
     }

     FreeAttributes(attrs);
     CloseAttributeManager(mgr);
     return TRUE;
 }

 PA_STATUS WINAPI DoFileTagging( std::wstring strSrc,std::wstring destpath )
 {
     //
     // Add source file's tags to NXL file
     //
     return CopyTags(strSrc.c_str(), destpath.c_str()) ? PA_SUCCESS : PA_ERROR;
 }

static
std::wstring
GetFileName(
            _In_ LPCWSTR wzPath
            )
{
    std::wstring            strName(wzPath);
    const WCHAR* pos = wcsrchr(wzPath, L'\\');

    if(NULL != pos)
        strName = ++pos;

    return strName;
}

static
BOOL
SetFileName(
            _In_ LPCWSTR wzFilePath,
            _In_ LPCWSTR wzFileName
            )
{
    NextLabsFile_Header_t   hdrinfo = {0};
    
    if(!SE_GetFileInfo(wzFilePath, SE_FileInfo_NextLabs, &hdrinfo))
        return FALSE;
    
    memset(hdrinfo.orig_file_name, 0, 512);
    wcsncpy_s(hdrinfo.orig_file_name, 256, wzFileName, _TRUNCATE);
    return SE_SetFileInfo(wzFilePath, SE_FileInfo_NextLabs, &hdrinfo);
}

 /*
1.	Call SE_IsEncrypted() to see if the source file is encrypted.
2.	If yes, call SE_WrapEncryptedFile().
3.	If no, call SE_WrapPlainFile().
 */
 PA_STATUS WINAPI DoAutoWrap (OBJECTINFO &file, wchar_t* strDestPath  )
 {
	 std::wstring strSrcPath ;
	 std::wstring strFileName ;

	 if( file.bFileNameChanged == TRUE )
	 {
		 strSrcPath = file.strRetName ;
		 //Modified current path is file.strRetName
	 }
	 else
	 {
		 strSrcPath = file.strSrcName ;
	 }

     strFileName = GetFileName(strSrcPath.c_str());

	 WCHAR wzTempPath[MAX_PATH + 1]; memset(wzTempPath, 0, sizeof(wzTempPath));
	 
	 BOOL bWrapRet =  TRUE ;
	 std::wstring destPath = strSrcPath + L".nxl" ;
	 if (GetTempPathW(MAX_PATH, wzTempPath) > 0)
	 {
		 // put to the temp folder first
		 WCHAR wzTempFilePath[MAX_PATH];

		 if (GetTempFileName(wzTempPath, L"SE_", 0, wzTempFilePath) > 0)
		 {
			 // Rename the file from ".TMP" to ".nxl".
			 WCHAR wzTempFilePath2[MAX_PATH];
			 
			 wcsncpy_s(wzTempFilePath2, MAX_PATH, wzTempFilePath, _TRUNCATE);
			 int len = static_cast<int> (wcslen(wzTempFilePath2));
			 wcsncpy_s(&wzTempFilePath2[len - wcslen(NL_FILE_EXT)], wcslen(NL_FILE_EXT) + 1, NL_FILE_EXT, _TRUNCATE);

			 if (MoveFile(wzTempFilePath, wzTempFilePath2))
			 {
				 destPath = wzTempFilePath2;
			 }
			 else
			 {
				 DeleteFile(wzTempFilePath);
			 }
		 }
	 }

	 /*No encrypt and the last error code is ERROR_SUCCESS*/
	 if( SE_IsEncrypted(strSrcPath.c_str()) ==TRUE)
	 {
		 std::wstring strExt = L"";
		 if (strSrcPath.find_last_of(L'.') != std::wstring::npos)
		 {
			 strExt = strSrcPath.substr(strSrcPath.find_last_of(L'.'));
		 }
		 if (strExt.compare(L".nxl") == 0)
		 {
			 ::wcsncpy_s( strDestPath,MAX_PATH,strSrcPath.c_str(), _TRUNCATE ) ;
			 return PA_SUCCESS ;
		 }
		 else
		 {
			bWrapRet = SE_WrapEncryptedFile(strSrcPath.c_str(), destPath.c_str()  ) ;	
		 }
	 }
	 else
	 {
		 
		  bWrapRet = SE_WrapPlainFile(strSrcPath.c_str(), destPath.c_str() ) ;
	 }
	 if( bWrapRet == TRUE)
	 {
         // Update file name in NXL header
         if(!SetFileName(destPath.c_str(), strFileName.c_str()))
         {
         }

		 // If the source file is un-encrypted, copy the tags from the
		 // source file to the .nxl file.
		 //
		 // If the source file is local-encrypted, we don't need to
		 // copy the tags since SE_WrapEncryptedFile() has already
		 // copied the NLT header (hence the tags inside) in the
		 // source file to the NLT header in the .nxl file.
		 if( SE_IsEncrypted(strSrcPath.c_str()) == TRUE ||
		     PA_SUCCESS == DoFileTagging(strSrcPath.c_str(), destPath.c_str()) )
		 {
			 ::wcsncpy_s( strDestPath,MAX_PATH,destPath.c_str(), _TRUNCATE ) ;
			 if( file.bFileNameChanged == TRUE )
			 {
				 SaveDeleteFile(strSrcPath.c_str() ) ;
			 }
			 return PA_SUCCESS ;
		 }
         else
		 {
			 return PA_ERROR ;
		 }
	 }
	 else
	 {
		    ::DeleteFileW(destPath.c_str() ) ;
			SE_DisplayErrorMessage(SE_GetLastError( ));
		 return PA_ERROR ;
	 }
	 //
 }

/*
Parse  the pa_parameter
Added 3rd param for auto wrap on SE files being copied or moved
*/
PA_STATUS WINAPI DoPolicyAssistant( PA_PARAM &param, const HWND _hParentWnd, bool forceObligation )
{
	PA_STATUS status = PA_SUCCESS;
	OBJECTINFOLIST::iterator iterObj;
	PE_Obligation obligation;
	/* Parse PA Parameters */
	for (iterObj = param.objList.begin(); iterObj != param.objList.end(); iterObj++)
	{
		if ((*iterObj).lPARet)
		{
			continue;
		}
		obligation.obType = DEFAULT;
		status = ParseObligationList((*iterObj), obligation);
		if (status != PA_SUCCESS && !forceObligation)
		{
			return status;
		}
		else if (forceObligation)
			 obligation.obType = PE_AUTOWRAP;
			 

		if( obligation.obType == PE_AUTOWRAP )
		{
			wchar_t szDestPath[MAX_PATH]= {0} ;
			status  =DoAutoWrap( (*iterObj),szDestPath ) ;
			if(  status == PA_SUCCESS )
			{
				/*Reattach or copy with the ret file path if file name changed....
				*/
				std::wstring strSrcPath;
				if( (*iterObj).bFileNameChanged == TRUE )
				{
					strSrcPath = (*iterObj).strRetName ;
					//Modified current path is file.strRetName
				}
				else
				{
					strSrcPath = (*iterObj).strSrcName ;
				}

				if (_wcsicmp(strSrcPath.c_str(), szDestPath) != 0)
				{
					(*iterObj).bFileNameChanged = TRUE ;
					ZeroMemory( (*iterObj).strRetName, wcslen((*iterObj).strRetName) *sizeof(wchar_t ) ) ;
					::wcsncpy_s( (*iterObj).strRetName,MAX_PATH,szDestPath, _TRUNCATE ) ;
				}
			}
			else
			{
				/*Here should add the last error code.*/
				return PA_ERROR ;
			}
		}
	}
	return status;
}
