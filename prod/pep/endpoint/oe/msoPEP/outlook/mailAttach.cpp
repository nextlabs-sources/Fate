#include "stdafx.h"
#include "ItemEventDisp.h"
#include "outlookUtilities.h"
#include "../common/policy.h"
#include <time.h>
#include "../outlook/outlookUtilities.h"
#include "..\outlook\MailItemUtility.h"
#include "../outlook/ParameterForMulQuery.h"

#include <algorithm> //add by jjm
#include "mailAttach.h"
#include "eframework/platform/cesdk_loader.hpp"

extern nextlabs::cesdk_loader cesdk;
extern std::wstring g_strOETempFolder;

int GetCurrTimeStr(wchar_t *sStr)
{
	time_t t = time(0); 
    wcsftime( sStr, 15, L"%Y%m%d%H%M%S",localtime(&t) ); 
	
	return 1;

}
BOOL IsMsgFile(LPCWSTR pwzFile)
{
	LPCWSTR pSuffix = wcsrchr(pwzFile, L'.');
	if(NULL == pSuffix) return FALSE;

    if(0 == _wcsicmp(pSuffix, L".msg") || 0 == _wcsicmp(pSuffix, L".oft"))
		return TRUE;

	return FALSE;
}
BOOL IsOftFile(LPCWSTR pwzFile)
{
	LPCWSTR pSuffix = wcsrchr(pwzFile, L'.');
	if(NULL == pSuffix) return FALSE;

    if(0 == _wcsicmp(pSuffix, L".oft"))
		return TRUE;

	return FALSE;
}
HRESULT GetAttachObjFromMailItem(CComPtr<IDispatch> pMailItem,int iIndex,Outlook::Attachment **pAttach)
{
	CComPtr<Outlook::Attachments> pAtts=NULL;
	CComPtr<Outlook::Attachment>  pAtt=NULL;
	int	nAttchmentCount=0;
	HRESULT hr=S_OK;
	if(iIndex<0||pMailItem==NULL||pAttach==NULL)
		return S_FALSE;

	hr=MailItemUtility::get_Attachments(pMailItem,&pAtts,TRUE);
	if(SUCCEEDED(hr)&&pAtts)
	{
		hr=pAtts->get_Count((long*)&nAttchmentCount);
		if(SUCCEEDED(hr))
		{
			if(nAttchmentCount<iIndex)
				return S_FALSE;

			VARIANT vi; vi.vt = VT_INT; vi.intVal = iIndex;

            hr = pAtts->Item(vi, &pAtt);
            if(SUCCEEDED(hr))
			{
				*pAttach=pAtt;
			}
		}
	}
	return hr;
}

static int GetTempAttachFileName(_In_ const wchar_t *path, _In_ wchar_t *name, _Out_cap_(validNameSize) wchar_t *validName, _In_ int validNameSize)
{
	int	ret=0;
	wchar_t pathTmp[MAX_PATH+1] = {0} ;
	wchar_t fullName[MAX_PATH+1] = {0};
	wchar_t nameTmp[MAX_PATH+1] = {0};

	wcsncpy_s(pathTmp, MAX_PATH,path, _TRUNCATE);
	size_t	len = 0 ;
	if(name[0]==0)
	{
		len=wcslen(pathTmp);
		if(pathTmp[len-1]==L'\\'||pathTmp[len-1]==L'/')
			pathTmp[len-1]=0;
	}
	INT iPathLen = (INT)wcslen( pathTmp ) ;
	INT iNameLen = (INT)wcslen(name) ; 
	if(	 iPathLen >= MAX_PATH  )
	{
		return 0 ;
	}
	if( (iNameLen+iPathLen) > MAX_PATH )
	{
	   wcsncpy_s(nameTmp,MAX_PATH,&name[iNameLen+iPathLen-MAX_PATH],_TRUNCATE);
	}
	else
	{
		wcsncpy_s(nameTmp,MAX_PATH,name, _TRUNCATE);
	}
	_snwprintf_s(fullName, MAX_PATH, _TRUNCATE, L"%s%s",pathTmp,nameTmp);
	DP((  L"_snwprintf_s(fullName,,pathTmp,nameTmp);;fullName:[%s];pathTmp[%s];nameTmp[%s]", fullName ,pathTmp,nameTmp) ) ;
	if(_waccess(fullName,0)==0)
	{
		wchar_t fileName[256];
		wcsncpy_s(fileName, 255, name, _TRUNCATE);
		DP((  L"wcsncpy_s(fileName, 255, name, _TRUNCATE);fileName[%s];name[%s]", fileName,name) ) ;
		wchar_t* pPostfix=wcsrchr(nameTmp,L'.');
		if(pPostfix)
			fileName[pPostfix-nameTmp]=0;
		
		for(int i=0;i<100;i++)
		{
			if(pPostfix)
				_snwprintf_s(fullName, MAX_PATH, _TRUNCATE, L"%s%s(%d)%s",pathTmp,fileName,i,pPostfix);	
			else
				_snwprintf_s(fullName, MAX_PATH, _TRUNCATE, L"%s%s(%d)",pathTmp,fileName,i);	
			if(_waccess(fullName,0)==0)
				continue;
			else
			{
				wcsncpy_s(validName, validNameSize-1, fullName, _TRUNCATE);
				ret=1;
				break;
			}
		}
	}
	else
	{
        // Normally, it should be (validNameSize-1), but we may need to append another L'\\' later
        // So the max chars to be copied is (validNameSize-1)
        // we keep two chars empty: one for L'\\', one for L'\0'
		wcsncpy_s(validName, validNameSize-1, fullName, _TRUNCATE);
		ret=1;
	}
	if ((name[0]==0) && (ret == 1))
	{
		len=wcslen(validName);
		if(len>0&&len<(size_t)validNameSize-1&&validName[len-1]!=L'\\')
		{
			validName[len]=L'\\';
			validName[len+1]=0;
		}
	}
	return ret;

}
//Extract/Pack .msg file must be done on temp one. This function implement to create a temp one for an msg file.
//False:failed
//True:Succeeded
BOOL CreateTempMsgFile(const wchar_t* pTopDir,LpAttachmentData attachMail)
{
	BOOL ret=TRUE;
	if(FALSE==IsMsgFile(attachMail->src))
		return ret;
	if(_waccess(attachMail->src,0)==0)
	{
		if(_wcsicmp(attachMail->src,attachMail->temp) == 0||attachMail->temp[0]==0)
		{
			wchar_t* pName = wcsrchr(attachMail->src, L'\\');
			if(pName==NULL)
				pName= wcsrchr(attachMail->src, L'/');
			if(pName==NULL)
				ret=FALSE;
			else
			{
				wchar_t wszBuf[1024];
				if(GetTempAttachFileName(pTopDir,pName+1,wszBuf,1024))
				{
					if(CopyFile(attachMail->src,wszBuf,TRUE))
						wcsncpy_s(attachMail->temp,MAX_SRC_PATH_LENGTH,wszBuf, _TRUNCATE);
					else
						ret=FALSE;
				}
				else
					ret=FALSE;
			}
		}
	}
	return ret;
}



//False:means continue to do following stuff
//True: means stop to do anymore.
BOOL ExtractMsgAttachment(const wchar_t* pTopDir,
									   MAILTYPE m_mtMailType,
									   STRINGLIST& listRecipients,
									   LpAttachmentData attachMail,
									   CComPtr<IDispatch> pMail,
									   CEEnforcement_t *enforcer,
									   PolicyCommunicator* spPolicyCommunicator,
									   BOOL* bNeedHdr,
									   BOOL* bCancel,
									   AttachInMsgFormat** ppMsgAttach) 
{
#ifdef MAX_PATH
#undef MAX_PATH
#define MAX_PATH	1024
#endif
	BOOL ret=FALSE;
	STRINGLIST listDeny;
	CComPtr<Outlook::_Application> pOLApp=NULL;
	if(FALSE==IsMsgFile(attachMail->src))
		return FALSE;

	LpAttachInMsgFormat pAttInMsgFormat=new AttachInMsgFormat();
	//ZeroMemory(pAttInMsgFormat,sizeof(AttachInMsgFormat));
	*ppMsgAttach=pAttInMsgFormat;
	HRESULT hr=MailItemUtility::get_olApplication(pMail,&pOLApp);
	if(SUCCEEDED(hr)&&pOLApp)
	{
		if(_waccess(attachMail->temp,0)!=0)
		{
			if(_waccess(attachMail->src,0)!=0)
			{
				std::wstring OLTempFolder;
				wchar_t		tmpFileName[1024];
				CComPtr<Outlook::Attachment>	pAtt=NULL;
				hr=GetAttachObjFromMailItem(pMail,attachMail->iIndex,&pAtt);
				if(FAILED(hr))
					return FALSE;

				if(!OLUtilities::GetOutlookTempFolder(OLTempFolder, 12))
					if(!OLUtilities::GetOutlookTempFolder(OLTempFolder, 11))
						if(!OLUtilities::GetOutlookTempFolder(OLTempFolder, 10))
							OLUtilities::GetOutlookTempFolder(OLTempFolder, 9);
				DP((  L"Extracting msg file path:[%s]; File Name[%s]", OLTempFolder.c_str(), attachMail->dispname ) ) ;
				if(GetTempAttachFileName(OLTempFolder.c_str(),attachMail->dispname,tmpFileName,1024))
				{
					DP((  L"Extracted msg file path:[%s]; Temp File Name[%s]", OLTempFolder.c_str(), tmpFileName) ) ;
					BSTR bstrTmpAtt=::SysAllocString(tmpFileName);
					hr=pAtt->SaveAsFile(tmpFileName);
					::SysFreeString(bstrTmpAtt);
					if(SUCCEEDED(hr))
					{
						wcsncpy_s(attachMail->src,MAX_SRC_PATH_LENGTH,tmpFileName, _TRUNCATE);
						//wcscpy(attachMail->resolved,tmpFileName);
						wcsncpy_s(attachMail->temp,MAX_SRC_PATH_LENGTH,tmpFileName, _TRUNCATE);
					}
					else
						return FALSE;
				}
				else
					return FALSE;
			}
			else
				wcsncpy_s(attachMail->temp,MAX_SRC_PATH_LENGTH,attachMail->src, _TRUNCATE);
		}
		CComVariant covOptional((long)DISP_E_PARAMNOTFOUND, VT_ERROR);
		CComPtr<IDispatch> pIDisp=NULL;
		wcsncpy_s(pAttInMsgFormat->m_path,MAX_SRC_PATH_LENGTH,attachMail->temp, _TRUNCATE);
		hr=pOLApp->CreateItemFromTemplate(attachMail->temp,covOptional,&pIDisp);
		if(SUCCEEDED(hr)&&pIDisp)
		{
			/* Check which type mail item. 
			*/
			CComPtr<IDispatch> pMailItem=NULL;
			hr=MailItemUtility::QueryItemInterface(pIDisp,(void**)&pMailItem);
			if(SUCCEEDED(hr)&&pMailItem)
			{
				CComPtr<Outlook::Attachments> pAttachments=NULL;
				
				int iIdx=0,nAttach=0;
				int iFlagDirCreated=FALSE;//FALSE:indicate that the dir have not been created,TRUE indicate dir have been created.
				hr = MailItemUtility::get_Attachments(pMailItem,&pAttachments,TRUE);
				
				

				if(SUCCEEDED(hr)&&pAttachments)
				{
					hr=pAttachments->get_Count((long*)&nAttach);
					for(iIdx=0;iIdx<nAttach;iIdx++)
					{
						CComPtr<Outlook::Attachment>   pAttachment=NULL;
						wchar_t			wszName[MAX_PATH];
						wchar_t			wszPath[MAX_PATH];
						wchar_t			wszMsgFullName[1024];
						std::wstring	wstrDispName;
						VARIANT vi; vi.vt = VT_INT; vi.intVal = iIdx+1;
						if(ret==TRUE)
							break;

						hr = pAttachments->Item(vi, &pAttachment);
						if(SUCCEEDED(hr))
						{
							if(OLUtilities::IsIgnoredAttach(pAttachment))//We don't need to handle "embedded attachment", 
							{
								continue;
							}
							CComBSTR     bstrDispName;
                            std::wstring strFileName;
							LpAttachmentData pAttachData=new AttachmentData;
							
							hr = pAttachment->get_DisplayName(&bstrDispName);
                            if(SUCCEEDED(hr) && NULL!=bstrDispName.m_str)
                            {
							    wcsncpy_s(pAttachData->dispname,MAX_SRC_PATH_LENGTH,bstrDispName, _TRUNCATE);
                            }

                            strFileName = OLUtilities::GetAttachFileName(pAttachment);
                            wcsncpy_s(pAttachData->src,MAX_SRC_PATH_LENGTH,strFileName.c_str(), _TRUNCATE);
							wcsncpy_s(pAttachData->resolved,MAX_SRC_PATH_LENGTH,strFileName.c_str(), _TRUNCATE);
                            if(SUCCEEDED(hr) && !strFileName.empty())
							{
								//begin to create tmp dir for msg attachment
								wchar_t* pFileName=wcsrchr(attachMail->src,L'\\');
								wcsncpy_s(wszName, MAX_PATH, strFileName.c_str(), _TRUNCATE);
								DP((  L"wcsncpy_s(wszName, MAX_PATH, strFileName.c_str(), _TRUNCATE);wszName:[%s];bstrFileName:[%s]", wszName, strFileName.c_str() ) ) ;
								if(pTopDir[0]&&pTopDir[wcslen(pTopDir)-1]==L'\\')
								{
									if(pFileName)
										_snwprintf_s(wszPath,MAX_PATH, _TRUNCATE, L"%s%s_dir\\",pTopDir,pFileName+1);
									else
										_snwprintf_s(wszPath,MAX_PATH, _TRUNCATE, L"%s%s_dir\\",pTopDir,attachMail->src);
								}
								else
								{
									if(pFileName)
										_snwprintf_s(wszPath,MAX_PATH, _TRUNCATE, L"%s\\%s_dir\\",pTopDir,pFileName+1);
									else
										_snwprintf_s(wszPath,MAX_PATH, _TRUNCATE, L"%s\\%s_dir\\",pTopDir,attachMail->src);

								}
								if(FALSE==iFlagDirCreated)
								{
									wchar_t	wszTmpPath[1024];
									if(GetTempAttachFileName(wszPath,L"",wszTmpPath,1024))
									{
										wcsncpy_s(wszPath, MAX_PATH, wszTmpPath, _TRUNCATE);
									}
									DP((  L"GetTempAttachFileName(wszPath,,wszTmpPath);wszTmpPath:[%s]", wszTmpPath ) ) ;
									BOOL retDir=CreateDirectory(wszPath,NULL);
									if(retDir||GetLastError()==ERROR_ALREADY_EXISTS)
										iFlagDirCreated=TRUE;
								}
								//dir creating finished
								if(GetTempAttachFileName(wszPath,wszName,wszMsgFullName,1024))
								{
									DP((  L"GetTempAttachFileName(wszPath,wszName,wszMsgFullName)bstrTargetFile:[%s];wszName[%s];wszMsgFullName[%s]", wszMsgFullName,wszName,wszMsgFullName ) ) ;
									BSTR	bstrTargetFile=::SysAllocString(wszMsgFullName);
									hr=pAttachment->SaveAsFile(bstrTargetFile);
									//DP((  L"pAttachment->SaveAsFile(bstrTargetFile);;bstrTargetFile:[%s]", bstrTargetFile ) ) ;
									wcsncpy_s(pAttachData->temp,MAX_SRC_PATH_LENGTH,wszMsgFullName, _TRUNCATE);
									DP((  L"0000000000000000000000000000000000000000" ) ) ;
									wcsncpy_s(pAttachData->src,MAX_SRC_PATH_LENGTH,wszMsgFullName, _TRUNCATE);
									DP((  L"01010101010010101010101010010101010101010101010" ) ) ;
									::SysFreeString(bstrTargetFile);
								}
								else
								{
									//pAttachment->Release();
									break;
								}
								if(OLUtilities::IsWordFile(pAttachData->temp))
									pAttachData->type = attachWord;
								else if(OLUtilities::IsExcelFile(pAttachData->temp) )
									pAttachData->type = attachExcel;
								else if(OLUtilities::IsPwptFile(pAttachData->temp))
									pAttachData->type = attachPPT;
								
								if(TRUE==IsMsgFile(wszName))
								{
									LpAttachInMsgFormat pMsgAttach=NULL;
									if(ExtractMsgAttachment(wszPath,
													  m_mtMailType,
													  listRecipients,
													  pAttachData,
													  pMailItem,
													  enforcer,
													  spPolicyCommunicator,
													  bNeedHdr,
													  bCancel,
													  &pMsgAttach) )
										ret=TRUE;
									if(pMsgAttach)
										pAttInMsgFormat->m_vecChilds.push_back(pMsgAttach);
									
								}
								else
								{
									pAttInMsgFormat->m_vecAttachs.push_back(pAttachData);
									BOOL bQueryOK = spPolicyCommunicator->QueryOutlookPolicy(m_mtMailType, pAttachData->src, pAttachData->resolved, L"", L"", listRecipients, listDeny, enforcer);
#ifdef _DEBUG 
									if(!bQueryOK)
										bQueryOK=1;
#endif 
									if(!bQueryOK||CEAllow !=enforcer->result)
									{
										if(bQueryOK)
											cesdk.fns.CEEVALUATE_FreeEnforcement(*enforcer);
										if(CEAllow !=enforcer->result)
											*bCancel=TRUE;

										ret=TRUE;
									}
									else
									{
										/*
										Modified by chellee for the bug 10213
										*/
										BOOL bCheckHr = CheckHdrNeeded( pAttachData,enforcer);              
										if(*bNeedHdr==FALSE)
										{                                                                      
											wcsncpy_s(       attachMail->Origtemp, MAX_SRC_PATH_LENGTH,  attachMail->temp, _TRUNCATE ) ;                                                
											*bNeedHdr = bCheckHr ;                                  
										}

#ifdef _DEBUG 
										if(wcslen(pAttachData->hdrUrl)==0)
											wcsncpy_s(pAttachData->hdrUrl,MAX_SRC_PATH_LENGTH,L"http://www.nextlabs.com/",_TRUNCATE);
#endif
#ifdef WSO2K7
#ifdef _DEBUG
										pAttachData->hdr=TRUE;

#endif
#endif
										cesdk.fns.CEEVALUATE_FreeEnforcement(*enforcer);
									}

								}
							}
							//pAttachment->Release();
							
						}//deal one attachment
					}//for(iIdx
					//pAttachments->Release();
				}
				//pMailItem->Release();
			}
			//pIDisp->Release();
		}
	}

	return ret;
#undef MAX_PATH
#define MAX_PATH	256

}




BOOL CheckHdrNeeded(LpAttachmentData lpAttachmentData,CEEnforcement_t *enforcer)
{
	BOOL bNeedHdr=FALSE;
	CObligations*  obligation=new CObligations;
	obligation->GetObligations(enforcer->obligation);
	//if(!hdrInfo.valid && 0<listObligations[i]->m_Obligations.size())
	if(0<obligation->m_Obligations.size())
	{
		HIDDENDATAREMOVALINFO hdrInfo;
		obligation->CheckHiddenDataRemoval(&hdrInfo);
		if(hdrInfo.valid)
		{
			bNeedHdr=TRUE;
			lpAttachmentData->hdr = TRUE;
			wcsncpy_s(lpAttachmentData->hdrUrl, MAX_SRC_PATH_LENGTH-1, hdrInfo.url.c_str(), _TRUNCATE);
		}
	}
	delete obligation;
	return bNeedHdr;
}

int PackMsgAttach(CComPtr<IDispatch> pMail,AttachInMsgFormat *pMsgAttach)
{
	int		ret=0;
	HRESULT hr=S_OK;
	size_t	iIndex=0;
	if(pMsgAttach->m_vecChilds.size())
	{
		MSGATTACHLIST::size_type count=pMsgAttach->m_vecChilds.size();
		MSGATTACHLIST::size_type index=0;
		for(index;index<count;index++)
			PackMsgAttach(pMail,pMsgAttach->m_vecChilds[index]);
	}

	CComPtr<Outlook::_Application> pOLApp=NULL;
	hr=MailItemUtility::get_olApplication(pMail,&pOLApp);
	if(SUCCEEDED(hr)&&pOLApp)
	{
		CComVariant covOptional((long)DISP_E_PARAMNOTFOUND, VT_ERROR);
		CComPtr<IDispatch> pIDisp=NULL;
		hr=pOLApp->CreateItemFromTemplate(pMsgAttach->m_path,covOptional,&pIDisp);
		if(SUCCEEDED(hr)&&pIDisp)
		{
			CComPtr<IDispatch> pMailItem=NULL;
			hr=MailItemUtility::QueryItemInterface(pIDisp,(void**)&pMailItem);
			if(SUCCEEDED(hr)&&pMailItem)
			{
				CComPtr<Outlook::Attachments> pAttachments=NULL;
				long nCount=0;
				size_t nVecCount=0;
				hr = MailItemUtility::get_Attachments(pMailItem,&pAttachments,TRUE);
				if(SUCCEEDED(hr)&&pAttachments)
				{
					hr=pAttachments->get_Count(&nCount);
					for(int iiIndex = 0; iiIndex<(int)nCount; iiIndex++)
					{
						CComPtr<Outlook::Attachment>   pAttachment=NULL;
						VARIANT vi; 
						vi.vt = VT_INT;
						vi.intVal = iiIndex+1;
						hr = pAttachments->Item(vi, &pAttachment);
						if(SUCCEEDED(hr))
						{
							if(OLUtilities::IsIgnoredAttach(pAttachment))//We don't need to handle "embedded attachment", 
							{
								continue;
							}
							pAttachment->Delete();
						}
						//hr=pAttachments->Remove(1);
					}
					//Add .msg/.oft type attachments
					nVecCount=pMsgAttach->m_vecChilds.size();
					for(iIndex=0;iIndex<nVecCount;iIndex++)
					{
						CComPtr<Outlook::Attachment> pAttachment = NULL;
						CComVariant varSource(pMsgAttach->m_vecChilds[iIndex]->m_path);
						CComVariant varByVal(olByValue);
						//CComVariant varDispname(lpAttachmentData->dispname);
						CComVariant varDispname((long)DISP_E_PARAMNOTFOUND, VT_ERROR);
						CComVariant varPosition((long)1);
						hr=pAttachments->Add(varSource,varByVal,varPosition,varDispname,&pAttachment);
                        DP((L"  PackMsgAttach: %s to add msg attachment from temp (0x%08x): %s\n", SUCCEEDED(hr)?L"Succeed":L"Fail", hr, pMsgAttach->m_vecChilds[iIndex]->m_path));
					}
					//add another type file as attachment
					nVecCount=pMsgAttach->m_vecAttachs.size();
					for(iIndex=0;iIndex<nVecCount;iIndex++)
					{
						CComPtr<Outlook::Attachment> pAttachment = NULL;
						CComVariant varSource(pMsgAttach->m_vecAttachs[iIndex]->temp);
						CComVariant varByVal(olByValue);
						CComVariant varDispname(pMsgAttach->m_vecAttachs[iIndex]->dispname);
						//CComVariant varDispname((long)DISP_E_PARAMNOTFOUND, VT_ERROR);
						CComVariant varPosition((long)1);
						hr=pAttachments->Add(varSource,varByVal,varPosition,varDispname,&pAttachment);
                        DP((L"  PackMsgAttach: %s to add other attachment from temp (0x%08x): %s\n", SUCCEEDED(hr)?L"Succeed":L"Fail", hr, pMsgAttach->m_vecAttachs[iIndex]->temp));
					}
				}
				BSTR		bstrMailPath=::SysAllocString(pMsgAttach->m_path);
				CComVariant varType(olTemplate);
				if(IsOftFile(pMsgAttach->m_path)==TRUE)
					hr=MailItemUtility::SaveAs(pMailItem,bstrMailPath,varType);
				else
					hr=MailItemUtility::SaveAs(pMailItem,bstrMailPath);
				::SysFreeString(bstrMailPath);
			}
		}
	}
	return ret;
}

BOOL AddAttachInOneMsg4Hdr(LpAttachInMsgFormat pMsgAttach,ATTACHMENTLIST& hdrAttachList)
{
	BOOL ret=TRUE;
	size_t  iIndex=0;
	size_t	nCount=pMsgAttach->m_vecChilds.size();
	for(iIndex=0;iIndex<nCount;iIndex++)
	{
		AddAttachInOneMsg4Hdr(pMsgAttach->m_vecChilds[iIndex],hdrAttachList);
	}
	nCount=pMsgAttach->m_vecAttachs.size();
	for(iIndex=0;iIndex<nCount;iIndex++)
	{
		if(pMsgAttach->m_vecAttachs[iIndex]->hdr==TRUE)
			hdrAttachList.push_back(pMsgAttach->m_vecAttachs[iIndex]);
	}

	return ret;
}


BOOL AddAttachsInMsg4Hdr(MSGATTACHLIST & listMsgAttach,ATTACHMENTLIST& hdrAttachList)
{
	size_t  iIndex=0;
	size_t	nCount=listMsgAttach.size();
	for(iIndex=0;iIndex<nCount;iIndex++)
		AddAttachInOneMsg4Hdr(listMsgAttach[iIndex],hdrAttachList);
	return TRUE;
}


