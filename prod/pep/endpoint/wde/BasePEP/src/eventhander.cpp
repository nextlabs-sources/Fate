#include "fileoperationdlg.h"
#include "contentstorage.h"
#include <wininet.h>
#pragma warning(push)
#pragma warning(disable: 4244 4819 4512)
#include <boost/function.hpp> 
#pragma warning(pop)
#include "commonutils.hpp"

#include "eventhander.h"
namespace
{
    const unsigned int kMaxFileCount = 50; 
	const wchar_t* const kNew_Document = L"NEW_DOCUMENT"; 
} // ns anonymous

namespace nextlabs
{
	

	EventResult CEventHander::HandleNewFileAction(const std::wstring& fileName, PVOID pUserData)
	{
		policyengine::PolicyResult pr = policyengine::PolicyResultAllow;

		if (pUserData == NULL)
		{
			pr = nextlabs::policyengine::DoEvaluation(nextlabs::policyengine::WdeActionWrite, fileName);
		}
		else
		{
			pr = nextlabs::policyengine::DoEvaluation(nextlabs::policyengine::WdeActionWrite, fileName, *(nextlabs::Obligations*)pUserData);
		}

        if ( pr != nextlabs::policyengine::PolicyResultAllow)
        {  // denied by policy, can't create new file.
           return kEventReturnDirectly;   
        }
		return kEventAllow;
	}

	EventResult CEventHander::HandleEditFileAction(const std::wstring& fileName, PVOID pUserData)
	{
        if (nextlabs::utils::CanIgnoreFile(fileName))
        {
            return kEventAllow;
        }

		policyengine::PolicyResult pr = policyengine::PolicyResultAllow;

		if (pUserData == NULL)
		{
			pr = nextlabs::policyengine::DoEvaluation(nextlabs::policyengine::WdeActionWrite, fileName);
		}
		else
		{
			pr = nextlabs::policyengine::DoEvaluation(nextlabs::policyengine::WdeActionWrite, fileName, *(nextlabs::Obligations*)pUserData);
		}

		if ( pr != nextlabs::policyengine::PolicyResultAllow)
		{
			SetLastError(ERROR_ACCESS_DENIED);
			return kEventDeny;   
		}
		return kEventAllow;
	}

    EventResult CEventHander::HandleNewDirectoryAction(const std::wstring& dirName, PVOID pUserData)
    {
		policyengine::PolicyResult pr = policyengine::PolicyResultAllow;

		if (pUserData == NULL)
		{
			pr = nextlabs::policyengine::DoEvaluation(nextlabs::policyengine::WdeActionWrite, dirName);
		}
		else
		{
			pr = nextlabs::policyengine::DoEvaluation(nextlabs::policyengine::WdeActionWrite, dirName, *(nextlabs::Obligations*)pUserData);
		}

        if (pr != nextlabs::policyengine::PolicyResultAllow)
        {   // denied by policy, can't create new directory.
            return kEventReturnDirectly;   
        }
        return kEventAllow;
    }

    EventResult CEventHander::HandleRenameAction(const std::vector<std::pair<std::wstring, nextlabs::Obligations>>& vecFiles, const std::wstring& strNewName)
    {
        std::vector<std::pair<std::wstring, nextlabs::Obligations>> vecItems = vecFiles;
        for(std::vector<std::pair<std::wstring, nextlabs::Obligations>>::iterator itr = vecItems.begin(); itr != vecItems.end();)
        {
            if (nextlabs::policyengine::DoEvaluation(nextlabs::policyengine::WdeActionRename, itr->first, strNewName,itr->second) != nextlabs::policyengine::PolicyResultAllow)
            {  // denied by policy, can't be renamed.
                itr = vecItems.erase(itr);
                continue;
            }

            ++itr;
        }

        if (vecItems.empty())  // all files can't be renames (blocked by policy), so don't need to do anything.
        {
            return kEventReturnDirectly;
        }

        if (vecItems.size() != vecFiles.size())  // that means there are some files can't be renamed, then try to call IFileOperation to do rename items action
        {
            CoInitialize(NULL);  

            CComPtr<IFileOperation> spFO;  
            if (SUCCEEDED(spFO.CoCreateInstance(CLSID_FileOperation, NULL, CLSCTX_INPROC)))  
            {
                CComPtr<IShellItemArray> spSIA;
                nextlabs::comhelper::CreateShellItemArrayFromPaths(static_cast<UINT>(vecItems.size()), vecItems, &spSIA);

                spFO->SetOperationFlags(FOF_RENAMEONCOLLISION);
                spFO->RenameItems(spSIA, strNewName.c_str());
                spFO->PerformOperations();
            }

            CoUninitialize();

            return kEventReturnDirectly;
        }

        return kEventAllow;
    }

    EventResult CEventHander::HandleDeleteAction(const std::vector<std::pair<std::wstring, nextlabs::Obligations>>& vecFiles)
    {
        //OutputDebugStringW(__FUNCTIONW__);
		std::vector<std::pair<std::wstring, nextlabs::Obligations>> vecItems = vecFiles;
		
		for(std::vector<std::pair<std::wstring, nextlabs::Obligations>>::const_iterator itr = vecItems.begin() ; itr != vecItems.end();)
		{
			if (nextlabs::policyengine::DoEvaluation(nextlabs::policyengine::WdeActionDelete, itr->first) != nextlabs::policyengine::PolicyResultAllow)
			{  // denied by policy, can't be deleted.		
				// call shell something has changed
				SHChangeNotify(SHCNE_UPDATEDIR, SHCNF_PATH, utils::GetFilePathFromName(itr->first).c_str(), 0);
				itr = vecItems.erase(itr);
				continue;
			}

			++itr;
		}

        if (vecItems.empty())  // all files can't be deleted (blocked by policy), so don't need to do anything.
        {
            return kEventReturnDirectly;
        }

		if (vecItems.size() != vecFiles.size())  // that means there are some files can't be deleted, then try to call SHFileOperation for files which can be deleted.
		{			
			SHFILEOPSTRUCTW theStruct;
			::ZeroMemory(&theStruct,sizeof(theStruct));

			theStruct.fFlags = FOF_MULTIDESTFILES | FOF_NOCONFIRMMKDIR; 
			theStruct.wFunc = FO_DELETE;

            std::wstring pathBuf;
			for(std::vector<std::pair<std::wstring, nextlabs::Obligations>>::const_iterator itr = vecItems.begin(); itr != vecItems.end(); ++itr)
			{
				pathBuf.append(itr->first);
                pathBuf.append(L"\0", 1);
			}
            pathBuf.append(L"\0", 1);

			theStruct.pFrom = pathBuf.c_str();
			theStruct.fFlags = FOF_SIMPLEPROGRESS;
			SHFileOperationW(&theStruct) == 0 ? true : false;
            return kEventReturnDirectly;
		}
		return kEventAllow;
    }

    EventResult CEventHander::HandleCopyAction(const std::vector<nextlabs::comhelper::FILEOP>& vecFileOp)
    {
		//OutputDebugStringW(__FUNCTIONW__);
		return _ImplHandleMoveCopyAction(policyengine::WdeActionCopy,vecFileOp);
    }

	EventResult CEventHander::HandleCopyFileW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, BOOL bFailIfExists, nextlabs::Obligations& obligations)
	{
		std::wstring wstrOriginalSrcFile = lpExistingFileName;
		std::wstring wstrOriginalDstFile = lpNewFileName;

		nextlabs::policyengine::PolicyResult pr = nextlabs::policyengine::DoEvaluation(nextlabs::policyengine::WdeActionCopy, wstrOriginalSrcFile, wstrOriginalDstFile, obligations);

		std::wstring wstrSrcFile = wstrOriginalSrcFile;
		std::wstring wstrDstFile = wstrOriginalDstFile;

		bool bHasItemPathChanged = false;

		switch (pr)
		{
		case nextlabs::policyengine::PolicyResultAllow:
			if (!nextlabs::policyassistant::PAHelper(PABase::AT_COPY, obligations, wstrSrcFile, wstrDstFile, ERROR_ACCESS_DENIED, true))
			{
				return kEventReturnDirectly;
			}
			// after dong PA successfully,  path both src&dst may be changed , so write back to
			else if (!boost::algorithm::equals(wstrOriginalSrcFile, wstrSrcFile) || !boost::algorithm::equals(wstrOriginalDstFile, wstrDstFile))
			{
				wstrOriginalSrcFile = wstrSrcFile;
				wstrOriginalDstFile = wstrDstFile;
				bHasItemPathChanged = true;
			}
			SHChangeNotify(SHCNE_UPDATEDIR, SHCNF_PATH, utils::GetFilePathFromName(wstrSrcFile.c_str()).c_str(), 0);
			SHChangeNotify(SHCNE_UPDATEDIR, SHCNF_PATH, utils::GetFilePathFromName(wstrDstFile.c_str()).c_str(), 0);
			break;

		case nextlabs::policyengine::PolicyResultDeny:
			// for deny policy ,ignore result by PA
			nextlabs::policyassistant::PAHelper(PABase::AT_COPY, obligations, wstrSrcFile, wstrDstFile, ERROR_ACCESS_DENIED, true);

			SHChangeNotify(SHCNE_UPDATEDIR, SHCNF_PATH, utils::GetFilePathFromName(wstrSrcFile.c_str()).c_str(), 0);
			SHChangeNotify(SHCNE_UPDATEDIR, SHCNF_PATH, utils::GetFilePathFromName(wstrDstFile.c_str()).c_str(), 0);

			return kEventReturnDirectly;
			break;
		default:
			break;
		}

		if (bHasItemPathChanged || !obligations.GetObligations().empty())
		{
			if (CopyFileW(wstrOriginalSrcFile.c_str(), wstrOriginalDstFile.c_str(), bFailIfExists))
			{
				if(obligations.IsObligationSet(WDE_OBLIGATION_ENCRYPT_NAME))
				{
					if(!comhelper::HandleEFSObligation(wstrOriginalDstFile.c_str()))
					{
						DeleteFileW(wstrOriginalDstFile.c_str());
					}
				}
			}
			return kEventReturnDirectly;
		}

		return kEventAllow;
	}

    EventResult CEventHander::HandleMoveAction(const std::vector<nextlabs::comhelper::FILEOP>& vecFileOp)
    {
        //OutputDebugStringW(__FUNCTIONW__);
		return _ImplHandleMoveCopyAction(policyengine::WdeActionMove,vecFileOp);
    }

    EventResult CEventHander::HandleCopyContentAction(const std::wstring& wstrFilePath)
    {
        //OutputDebugStringW(__FUNCTIONW__);
        if (nextlabs::policyengine::DoEvaluation(nextlabs::policyengine::WdeActionCopyContent, wstrFilePath) != nextlabs::policyengine::PolicyResultAllow)
        {  
            OutputDebugString(L"Deny Copy Content");
            return kEventDeny;   
        }
        return kEventAllow;
    }

    EventResult CEventHander::HandleDropContentAction(const std::wstring& wstrSrcFilePath, const std::wstring& wstrDstFilePath)
    {
         if (nextlabs::policyengine::DoEvaluation(nextlabs::policyengine::WdeActionCopyContent, wstrSrcFilePath, wstrDstFilePath) != nextlabs::policyengine::PolicyResultAllow)
        {  
            OutputDebugString(L"Deny Drop Copy");
            return kEventDeny;   
        }
        return kEventAllow;
    }

    EventResult CEventHander::HandlePasteContentAction(const std::wstring& wstrSrcFilePath, const std::wstring& wstrDstFilePath)
    {
        if (nextlabs::policyengine::DoEvaluation(nextlabs::policyengine::WdeActionCopyContent, wstrSrcFilePath, wstrDstFilePath) != nextlabs::policyengine::PolicyResultAllow)
        {  
            OutputDebugString(L"Deny Copy Content");
            //for olegetclipboard.
            if (OpenClipboard(NULL))
            {
                EmptyClipboard();
                CloseClipboard();
            }
            //for GetClipboardData.
            EmptyClipboard();

            CContextStorage storage = CContextStorage();
            storage.StoreClipboardInfo(L"");
            return kEventDeny;   
        }
        return kEventAllow;
    }

	EventResult CEventHander::HandleWriteFileAction(const std::wstring& wstrFilePath)
	{
        if (nextlabs::utils::CanIgnoreFile(wstrFilePath))
        {
            return kEventAllow;
        }

		if(nextlabs::policyengine::DoEvaluation(nextlabs::policyengine::WdeActionWrite,wstrFilePath) != nextlabs::policyengine::PolicyResultAllow)
		{
			return kEventDeny;
		}
		return kEventAllow;
	}

    EventResult CEventHander::HandleOpenFileAction(const std::wstring& filePath, bool bIgnoreSomeDirectorys)
    {
		if (bIgnoreSomeDirectorys)
		{
			if (nextlabs::utils::CanIgnoreFile(filePath))
			{
				return kEventAllow;
			}
		}

        if(nextlabs::policyengine::DoEvaluation(nextlabs::policyengine::WdeActionRead,filePath) == nextlabs::policyengine::PolicyResultAllow)
        {
            return kEventAllow;
        }
        return kEventDeny;

    }

    EventResult CEventHander::HandleOpenFolderAction(const std::wstring& folderPath)
    {
        if (nextlabs::utils::CanIgnoreFile(folderPath))
        {
            return kEventAllow;
        }

        std::wstring folderListPath(folderPath);
        folderListPath.append(L"\\*.*");
        if(nextlabs::policyengine::DoEvaluation(nextlabs::policyengine::WdeActionRead,folderListPath) == nextlabs::policyengine::PolicyResultAllow)
        {
            return kEventAllow;
        }
        return kEventDeny;
    }

     EventResult CEventHander::HandleSetFileAttributeAction(const std::wstring& fileName)
     {

         if(nextlabs::policyengine::DoEvaluation(nextlabs::policyengine::WdeActionChangeAttribute,fileName) == nextlabs::policyengine::PolicyResultAllow)
         {
             return kEventAllow;
         }

         return kEventDeny;
     }

     EventResult CEventHander::HandleSetFileSecurityAttributeAction(const std::wstring& fileName)
     {
         if(nextlabs::policyengine::DoEvaluation(nextlabs::policyengine::WdeActionChangeSecurityAttribute,fileName) == nextlabs::policyengine::PolicyResultAllow)
         {
             return kEventAllow;
         }

         return kEventDeny;
     }

    EventResult CEventHander::HandleCreateProcess(const std::wstring& wstrAppPath)
    {
        //OutputDebugStringW(__FUNCTIONW__);
        if (nextlabs::policyengine::DoEvaluation(nextlabs::policyengine::WdeActionRun, wstrAppPath) != nextlabs::policyengine::PolicyResultAllow)
        {  
            return kEventReturnDirectly;   
        }
        return kEventAllow;
    }

	EventResult CEventHander::HandleSaveAsAction(LPOPENFILENAMEW lpofn, const std::wstring& strSource, SaveAsInfo::SaveAsStruct* pComSaveAsStr)
	{
		if (NULL == lpofn || NULL == lpofn->lpstrFile) 
		{
			return kEventAllow;
		}

		std::wstring strTargetFileName = lpofn->lpstrFile;

		if (!strTargetFileName.empty())
		{
			if (strSource.empty())
			{
				if(nextlabs::policyengine::DoEvaluation(nextlabs::policyengine::WdeActionCopy, kNew_Document, strTargetFileName) != nextlabs::policyengine::PolicyResultAllow)
				{
					lpofn->lpstrFile[0] = L'\0';
					return kEventDeny;
				}
			}
			else
			{
				nextlabs::Obligations obligations;

				if(nextlabs::policyengine::DoEvaluation(nextlabs::policyengine::WdeActionCopy, strSource, strTargetFileName, obligations) != nextlabs::policyengine::PolicyResultAllow)
				{
					lpofn->lpstrFile[0] = L'\0';
					return kEventDeny;
				}
				else
				{
					if (pComSaveAsStr != NULL)
					{
						pComSaveAsStr->bPolicyAllow = TRUE;
						pComSaveAsStr->strDestinationPath = strTargetFileName;
						pComSaveAsStr->obs = obligations;
					}
				}
			}
		}

		return kEventAllow;
	}

	EventResult CEventHander::HandleSaveAsAction(IFileSaveDialog* pThis, HWND hwndOwner, const std::wstring& strSource, SaveAsInfo::SaveAsStruct* pComSaveAsStr)
	{
		EventResult er = kEventAllow;

		CComPtr<IShellItem> psiResult;
		HRESULT hrTemp = pThis->GetResult(&psiResult);
		if (SUCCEEDED(hrTemp))
		{
			PWSTR pszFilePath = NULL;
			hrTemp = psiResult->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
			if (SUCCEEDED(hrTemp))
			{
				if (strSource.empty())
				{
					if(nextlabs::policyengine::DoEvaluation(nextlabs::policyengine::WdeActionCopy, kNew_Document, pszFilePath) != nextlabs::policyengine::PolicyResultAllow)
					{
						er = kEventDeny;
					}
				}
				else
				{
					nextlabs::Obligations obligations;

					if(nextlabs::policyengine::DoEvaluation(nextlabs::policyengine::WdeActionCopy, strSource, pszFilePath, obligations) != nextlabs::policyengine::PolicyResultAllow)
					{
						er = kEventDeny;
					}
					else
					{
						if (pComSaveAsStr != NULL)
						{
							pComSaveAsStr->bPolicyAllow = TRUE;
							pComSaveAsStr->strDestinationPath = pszFilePath;
							pComSaveAsStr->obs = obligations;
						}
					}
				}
				CoTaskMemFree(pszFilePath);
			}
		}
		
		return er;
	}

    EventResult CEventHander::HandleUploadAction(LPOPENFILENAMEW lpofn, const std::wstring& strDest)
    {
        if (NULL == lpofn || NULL == lpofn->lpstrFile) 
        {
            return kEventAllow;
        }
        std::wstring strSource;
        strSource.append(lpofn->lpstrFile, lpofn->nMaxFile);

        int begin = 0;
        begin = strSource.find(L'\0', begin);

        if (begin == 0)
        {
            return kEventReturnDirectly;
        }
        
        std::wstring strDirOrPath = strSource.substr(0, begin);

        strSource = strSource.substr(begin + 1, std::wstring::npos);
        begin = strSource.find(L'\0');
        if (begin == 0 || begin == -1)
        {
            if(nextlabs::policyengine::DoEvaluation(nextlabs::policyengine::WdeActionSend, strDirOrPath, strDest) != nextlabs::policyengine::PolicyResultAllow)
            {
                lpofn->lpstrFile[0] = L'\0';
                return kEventDeny;
            }
            return kEventReturnDirectly;
        }
        else
        {
            memset(lpofn->lpstrFile, 0, lpofn->nMaxFile);
            memcpy(lpofn->lpstrFile, strDirOrPath.c_str(), wcslen(strDirOrPath.c_str())*sizeof(WCHAR));
            strDirOrPath.append(L"\\");
            LPWSTR tempAddress = lpofn->lpstrFile + wcslen(lpofn->lpstrFile) + 1;
            std::wstring strSingleTemp;
            int nCount = 0;
            do 
            {
                begin = strSource.find(L'\0');
                if (begin == 0 || begin == -1)
                {
                    break;
                }

                std::wstring strEvaluation = strDirOrPath;
                if(nextlabs::policyengine::DoEvaluation(nextlabs::policyengine::WdeActionSend, strEvaluation.append(strSource.c_str()).c_str(), strDest) == nextlabs::policyengine::PolicyResultAllow)
                {
                     ++nCount;
                     strSingleTemp.append(strEvaluation);
                     memcpy(tempAddress, strSource.c_str(), wcslen(strSource.c_str()) * sizeof(WCHAR));
                     ++tempAddress;
                     tempAddress += wcslen(tempAddress) + 1;
                }
               
                strSource = strSource.substr(begin + 1, std::wstring::npos);
            } while (begin != -1);
            if (nCount == 1)
            {
                memset(lpofn->lpstrFile, 0, lpofn->nMaxFile);
                memcpy(lpofn->lpstrFile, strSingleTemp.c_str(), wcslen(strSingleTemp.c_str())*sizeof(WCHAR));
            }
        }        
        return kEventAllow;
    }

	BOOL HttpHeaderExist(_In_ HINTERNET hRes, _In_ LPCWSTR wzHD)
	{
		BOOL bExist = FALSE;
		char* raw_hds = NULL;
		WCHAR* hd = NULL;
		DWORD raw_hd_len = 0;
		DWORD dwIndex = 0;

		HttpQueryInfoW(hRes, HTTP_QUERY_RAW_HEADERS, &dwIndex, &raw_hd_len, &dwIndex);
		if (ERROR_INSUFFICIENT_BUFFER != GetLastError()
			|| raw_hd_len <= 0)
			return bExist;

		raw_hd_len += 4;
		raw_hds = new char[raw_hd_len];
		if (NULL == raw_hds)
			goto _exit;
		memset(raw_hds, 0, sizeof(raw_hds));
		if (HttpQueryInfoW(hRes, HTTP_QUERY_RAW_HEADERS, raw_hds, &raw_hd_len, &dwIndex))
		{
			hd = (WCHAR*)raw_hds;
			while (L'\0' != hd[0])
			{
				if (0 == wcscmp(hd, wzHD))
				{
					bExist = TRUE;
					goto _exit;
				}
				hd += (wcslen(hd) + 1);
			}
		}

	_exit:
		if (raw_hds) delete[] raw_hds; raw_hds = NULL;
		return bExist;
	}


	void CEventHander::HandleHttpOpen(HINTERNET hRequest, const std::wstring& serverUrl)
	{
		std::wstring key;
		std::wstring val;

		if (!HasHttpInjectHeader(serverUrl, key, val))
		{
			return;
		}
		std::wstring header = key;
		header += L": ";
		header += val;
		if (!HttpHeaderExist(hRequest, header.c_str()))
		{
			if (::HttpAddRequestHeadersW(hRequest,
				header.c_str(), (DWORD)header.length(),
				HTTP_ADDREQ_FLAG_ADD | HTTP_ADDREQ_FLAG_REPLACE))
			{
				//::OutputDebugStringW(L"OK:inject http header");
			}
			else
			{
				//::OutputDebugStringW(L"False:inject http header");
			}
		}
		
	}


	//////////////////////////////////////////////////////////////////////////
	// send the request to the Policy Framework
	//////////////////////////////////////////////////////////////////////////
	static BOOL IsDateValid(LPCWSTR wzStart, LPCWSTR wzEnd)
	{
		ULONG sy = 0, sm = 0, sd = 0;
		ULONG ey = 0, em = 0, ed = 0;
		ULONG ly = 0, lm = 0, ld = 0;
		ULONG startdate = 0, enddate = 0, localdate = 0;
		SYSTEMTIME  systime;
		GetSystemTime(&systime);
		ly = systime.wYear; lm = systime.wMonth; ld = systime.wDay;
		localdate = ((ly << 16) & 0xFFFF0000) | ((lm << 8) & 0x0000FF00) | (ld & 0x000000FF);

		if (3 != swscanf_s(wzStart, L"%d/%d/%d", &sm, &sd, &sy))
			return FALSE;
		startdate = ((sy << 16) & 0xFFFF0000) | ((sm << 8) & 0x0000FF00) | (sd & 0x000000FF);
		if (3 != swscanf_s(wzEnd, L"%d/%d/%d", &em, &ed, &ey))
			return FALSE;
		enddate = ((ey << 16) & 0xFFFF0000) | ((em << 8) & 0x0000FF00) | (ed & 0x000000FF);

		if (localdate<startdate || localdate>enddate)
			return FALSE;

		return TRUE;
	}


	BOOL CEventHander::HasHttpInjectHeader(const std::wstring& serverUrl, std::wstring& key, std::wstring& val)
	{
		static const wchar_t* WDE_OBLIGATION_HTTPHDINJECT = L"NL_HTTP_HEADER_INJECTION";
		BOOL       bNeedExtraHeader = FALSE;
		policyengine::PolicyResult bAllow = policyengine::PolicyResultDeny;
		nextlabs::Obligations obs;
		// query PC to get extra header?
		{
			nextlabs::policyengine::strCEApplication ceApplication;
			ceApplication.appName = L"URL";
			ceApplication.appPath = L"";
			ceApplication.appUrl = serverUrl.c_str();
			ATTRS sourceAttrs;
			sourceAttrs.insert(std::make_pair(L"modified_date", L"0123456789"));
			bAllow=nextlabs::policyengine::DoEvaluation(nextlabs::policyengine::WdeActionRead, 
				serverUrl, L"", obs, 
				&ceApplication, &sourceAttrs);
		}

		// check result of PC
		if (policyengine::PolicyResultAllow == bAllow
			&& obs.IsObligationSet(WDE_OBLIGATION_HTTPHDINJECT) == true)
		{
			const std::list<nextlabs::Obligation>& ob_list = obs.GetObligations();
			std::list<nextlabs::Obligation>::const_iterator it;
			nextlabs::ObligationOptions::const_iterator options_it;
			std::wstring hd_start = L"";
			std::wstring hd_end = L"";
			for (it = ob_list.begin(); it != ob_list.end(); ++it) /* Each obligation */
			{
				if (it->name == WDE_OBLIGATION_HTTPHDINJECT)     /* Obligation name */
				{
					options_it = it->options.begin();
					key = options_it->second.c_str();    ++options_it;   // Argument 1: key
					if (options_it == it->options.end()) break;
					val = options_it->second.c_str();  ++options_it;   // Argument 2: value
					if (options_it == it->options.end()) break;
					hd_start = options_it->second.c_str();  ++options_it;   // Argument 3: start date
					if (options_it == it->options.end()) break;
					hd_end = options_it->second.c_str();    ++options_it;   // Argument 4: end date
					//
					if (IsDateValid(hd_start.c_str(), hd_end.c_str()) && !key.empty() && !val.empty())
					{
						bNeedExtraHeader = TRUE;
					}
					break;
				}
			}
		}
		return bNeedExtraHeader;
	}

	nextlabs::EventResult 
	CEventHander::_ImplHandleMoveCopyAction(nextlabs::policyengine::WdeAction action, 
		const std::vector<nextlabs::comhelper::FILEOP>& vecFileOp)
	{
		if (vecFileOp.empty())
		{
			return kEventAllow;
		}

		//OutputDebugStringW(__FUNCTIONW__);
		std::vector<nextlabs::comhelper::FILEOP> vecItems = vecFileOp;

		// display file scanning and analyzing progress if file count is bigger.
		nextlabs::FILEOPERATION fileOperation = { 0 };
		if (vecFileOp.size() >= kMaxFileCount)
		{
			fileOperation.fileCOUNT = static_cast<UINT> (vecFileOp.size());
			fileOperation.hEvent = CreateEventW(NULL, TRUE, FALSE, NULL);
			if (fileOperation.hEvent != NULL)
			{
				HANDLE hThread = CreateThread(NULL, 0, nextlabs::FileOperationThread, &fileOperation, 0, NULL);
				if (hThread != NULL)
				{
					WaitForSingleObject(fileOperation.hEvent, INFINITE);
					CloseHandle(hThread);
				}
				CloseHandle(fileOperation.hEvent);
			}
		}

		UINT currentFileCount = 0;
		bool bHasItemPathChanged = false;
		BOOL bHasObligations = FALSE;

		std::vector<nextlabs::comhelper::FILEOP>::iterator iter = vecItems.begin();
		
		while (iter != vecItems.end())
		{
			// handle cancel or update the progress
			currentFileCount++;
			if (fileOperation.pFileOperationDlg != NULL)
			{
				if (fileOperation.pFileOperationDlg->bCancel)
				{
					//OutputDebugStringW(L" cancel--------------------");
					fileOperation.pFileOperationDlg->SendMessage(WM_MYCLOSE);
					vecItems.clear();
					break;
				}

				fileOperation.pFileOperationDlg->PostMessage(WM_MYSETPROGRESSPOS, 0, (LPARAM)currentFileCount);
			}

			nextlabs::comhelper::FILEOP item = *iter;
			nextlabs::policyengine::PolicyResult pr =
				nextlabs::policyengine::DoEvaluation(action,item.strSrc, item.strDst, item.obligations);
			iter->obligations = item.obligations;
			if (!iter->obligations.GetObligations().empty())
			{
				bHasObligations = TRUE;
			}


			// set PA-action
			PABase::ACTION PaAction = PABase::AT_DEFAULT;
			if (action == policyengine::WdeActionCopy)
			{
				PaAction = PABase::AT_COPY;
			}
			else if (action == policyengine::WdeActionMove)
			{
				PaAction = PABase::AT_MOVE;
			}
			else{
				//::OutputDebugStringW(L"undefined action");
			}

			bool bIterChanged = false;
			switch (pr)
			{
			case nextlabs::policyengine::PolicyResultAllow:
				//
				if (!nextlabs::policyassistant::PAHelper(PaAction, item.obligations,
					item.strSrc, item.strDst, ERROR_ACCESS_DENIED, true)){
					iter = vecItems.erase(iter);
					bIterChanged = true;
				}
				// after dong PA successfully,  path both src&dst may be changed , so write back to (*iter)
				// *** very important ***
				else if (!boost::algorithm::equals(iter->strSrc, item.strSrc) ||
					!boost::algorithm::equals(iter->strDst, item.strDst)
					)
				{
					iter->strSrc = item.strSrc;
					iter->strDst = item.strDst;
					bHasItemPathChanged = true;
				}
				// call shell something has changed
				SHChangeNotify(SHCNE_UPDATEDIR, SHCNF_PATH, utils::GetFilePathFromName(item.strSrc.c_str()).c_str(), 0);
				SHChangeNotify(SHCNE_UPDATEDIR, SHCNF_PATH, utils::GetFilePathFromName(item.strDst.c_str()).c_str(), 0);
				break;

			case nextlabs::policyengine::PolicyResultDeny:
				// for deny policy ,ignore result by PA
				nextlabs::policyassistant::PAHelper(PaAction, item.obligations,
					item.strSrc, item.strDst, ERROR_ACCESS_DENIED, true);

				iter = vecItems.erase(iter);
				// call shell something has changed
				SHChangeNotify(SHCNE_UPDATEDIR, SHCNF_PATH, utils::GetFilePathFromName(item.strSrc.c_str()).c_str(), 0);
				SHChangeNotify(SHCNE_UPDATEDIR, SHCNF_PATH, utils::GetFilePathFromName(item.strDst.c_str()).c_str(), 0);
				bIterChanged = true;
				break;
			default:
				break;
			}

			// last step , modify iter 
			if (bIterChanged){
				continue;
			}
			else{
				++iter;
			}
		}


		if (vecItems.empty())
		{
			return kEventReturnDirectly;
		}

		if (vecItems.size() != vecFileOp.size() || bHasItemPathChanged || bHasObligations) // only copy the files need to be copied.
		{
			nextlabs::comhelper::SHFileOperation(vecItems, action == policyengine::WdeActionCopy?true:false);
			return kEventReturnDirectly;
		}

		return kEventAllow;
	}


}  // ns nextlabs
