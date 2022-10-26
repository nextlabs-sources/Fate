#ifndef __ADAPTERMANAGER_H__
#define __ADAPTERMANAGER_H__
#include "adaptercomm.h"

//#define ADAPTERMGR_INI_FILENAME		L"adaptermgr.ini"
namespace{
class AdapterManager
{
public:
	AdapterManager(CComPtr<IDispatch> pItem,AdapterCommon::Attachments*pAtts):m_pItem(pItem),m_pAttachments(pAtts),m_bIsRTF(false){vecAdapters.clear();};
	~AdapterManager(){vecAdapters.clear();};
	BOOL Do()
	{
		if(m_pAttachments->Count()<=0)
		{
			return TRUE;
		}
		size_t nCount=vecAdapters.size();
		if(nCount<=0)
		{
			return TRUE;
		}
		size_t iIndex=0;
		for(iIndex=0;iIndex<nCount;iIndex++)
		{
			if(vecAdapters[iIndex].Upload(m_pItem,m_pAttachments)==FALSE)
			{
				return FALSE;
			}
		}
		return true;
	}
	
	BOOL DoEx(wchar_t** pwchObligationTopMessage,int &iTopMsgLength,wchar_t** pwchObligationBottomMessage,int &iBottomMsgLength)
	{
		UNREFERENCED_PARAMETER(iTopMsgLength);
		UNREFERENCED_PARAMETER(iBottomMsgLength);
		if(m_pAttachments->Count()<=0)
		{
			return TRUE;
		}
		size_t nCount=vecAdapters.size();
		if(nCount<=0)
		{
			return TRUE;
		}
		size_t iIndex=0;

		std::wstring strTempTopMsg;
		std::wstring strTempBottomMsg;

		for(iIndex=0;iIndex<nCount;iIndex++)
		{
			wchar_t* pwchTempTopMsg=NULL;
			wchar_t* pwchTempBottomMsg=NULL;
			int iTempTogMsgLength=0;
			int iTempBottomMsgLength=0;
			
			if(vecAdapters[iIndex].UploadEx(m_pItem,m_pAttachments,&pwchTempTopMsg,iTempTogMsgLength,&pwchTempBottomMsg,iTempBottomMsgLength)==FALSE)
			{
				if(iTempTogMsgLength>0&&iTempBottomMsgLength>0)
				{
					if(pwchTempTopMsg!=NULL)
					{
						strTempTopMsg.append(pwchTempTopMsg);
					}
					if(pwchTempBottomMsg!=NULL)
					{
						strTempBottomMsg.append(pwchTempBottomMsg);
					}
				}
				if(pwchTempTopMsg!=NULL)
				{
					vecAdapters[iIndex].RleaseUploadObligationPWCH(pwchTempTopMsg,true);
				}
				if(pwchTempBottomMsg!=NULL)
				{
					vecAdapters[iIndex].RleaseUploadObligationPWCH(pwchTempBottomMsg,true);
				}
				continue;
			}
			else
			{
				if(iTempTogMsgLength>0&&iTempBottomMsgLength>0)
				{
					if(pwchTempTopMsg!=NULL)
					{
						strTempTopMsg.append(pwchTempTopMsg);
					}
					if(pwchTempBottomMsg!=NULL)
					{
						strTempBottomMsg.append(pwchTempBottomMsg);
					}	
				}

				if(pwchTempTopMsg!=NULL)
				{
					vecAdapters[iIndex].RleaseUploadObligationPWCH(pwchTempTopMsg,true);
				}
				if(pwchTempBottomMsg!=NULL)
				{
					vecAdapters[iIndex].RleaseUploadObligationPWCH(pwchTempBottomMsg,true);
				}
				
				for(size_t i=0;i<m_pAttachments->Count();i++)
				{
					AdapterCommon::Attachment& pAtt=(m_pAttachments->Item(i));
					pAtt.SetRemovedFlag(true);
				}
			}
		}

		*pwchObligationTopMessage=new wchar_t[strTempTopMsg.length()+1];
		*pwchObligationBottomMessage=new wchar_t[strTempBottomMsg.length()+1];
		wmemset(*pwchObligationTopMessage,0,strTempTopMsg.length()+1);
		wmemset(*pwchObligationBottomMessage,0,strTempBottomMsg.length()+1);


		swprintf_s(*pwchObligationTopMessage,   strTempTopMsg.length()+1,      L"%s", strTempTopMsg.c_str());
		swprintf_s(*pwchObligationBottomMessage,strTempBottomMsg.length()+1,L"%s", strTempBottomMsg.c_str());

iTopMsgLength=(int)strTempTopMsg.length();
		return true;
	}
	bool IsRTFBody(){return m_bIsRTF;}
	BOOL StripAttachments()
	{
		CComVariant varAttachments;
		HRESULT hr=AdapterCommon::AutoWrap(DISPATCH_PROPERTYGET,&varAttachments,m_pItem,L"Attachments",0);
		if(FAILED(hr)||varAttachments.pdispVal==NULL)
			return FALSE;
		CComVariant varAttsCount=0;
		hr=AdapterCommon::AutoWrap(DISPATCH_PROPERTYGET,&varAttsCount,varAttachments.pdispVal,L"Count",0);
		if(FAILED(hr)||varAttsCount.intVal<=0)
			return FALSE;

		int i=varAttsCount.intVal;
		for(i=varAttsCount.intVal;i>0;i--)
		{
			CComVariant varItem(i),varAttachment;
			hr=AdapterCommon::AutoWrap(DISPATCH_PROPERTYGET,&varAttachment,varAttachments.pdispVal,L"Item",1,varItem);
			if(FAILED(hr)||varAttachment.pdispVal==NULL)
				return FALSE;
			/*CComVariant varFileName;
			hr=AdapterCommon::AutoWrap(DISPATCH_PROPERTYGET,&varFileName,varAttachment.pdispVal,L"FileName",0);
			if(FAILED(hr)||varFileName.bstrVal==NULL)
				return FALSE;
			CComVariant varPathName;
			hr=AdapterCommon::AutoWrap(DISPATCH_PROPERTYGET,&varPathName,varAttachment.pdispVal,L"DisplayName",0);*/
			
			std::wstring wstrAttTempPath;
			if(GetTempPathFromAttachmentObject(varAttachment.pdispVal,wstrAttTempPath)==FALSE)
			{
				continue;
			}

			size_t iIndex=0,nCount=m_pAttachments->Count();
			for(iIndex=0;iIndex<nCount;iIndex++)
			{
				AdapterCommon::Attachment& pAtt=(m_pAttachments->Item(iIndex));
				std::wstring wstrTempPath=pAtt.GetTempPath();
				if(pAtt.GetStripFlag()==true&&wstrTempPath==wstrAttTempPath)
				{
					CComVariant varResult;
					hr=AdapterCommon::AutoWrap(DISPATCH_METHOD,&varResult,varAttachments.pdispVal,L"Remove",1,varItem);
					if(FAILED(hr))
						return FALSE;
					else
					{
						if(IsRTFBody())
						{
							varResult.Clear();
							hr=AdapterCommon::AutoWrap(DISPATCH_METHOD,&varResult,m_pItem,L"Save",0);
						}
						pAtt.SetRemovedFlag(true);
					}

				}
			}
		}
		return TRUE;
	}
	void Init(LPCWSTR wzEnforcerName)
	{
		HKEY    hKeyReposAdapters     = NULL;
		WCHAR   wzKeyName[MAX_PATH];
		_snwprintf_s(wzKeyName,MAX_PATH, _TRUNCATE, L"%s\\%s\\%s",
										AdapterCommon::Adapter::KEYNAME_COMPLIANT_ENTERPRISE,
										wzEnforcerName,
										AdapterCommon::Adapter::KEYNAME_REPOSITORY_ADAPTER);
		RegOpenKeyEx(HKEY_LOCAL_MACHINE, wzKeyName,0, KEY_READ,&hKeyReposAdapters);
		
		int i=0;
		WCHAR wzFullSubKey[MAX_PATH];
		WCHAR wzSubKeyName[MAX_PATH+1];memset(wzSubKeyName, 0, sizeof(wzSubKeyName));
		WCHAR wzImage[MAX_PATH+1];memset(wzImage, 0, sizeof(wzImage));
		WCHAR wzObligationName[MAX_PATH+1];memset(wzObligationName, 0, sizeof(wzObligationName));
		DWORD	dwData=0;
		long lSubResult=0;
		HKEY    hSubKey     = NULL;
		for(i=0;i<256;i++)
		{
			long lResult=RegEnumKey(hKeyReposAdapters,i,wzSubKeyName,MAX_PATH);
			
			if(lResult != ERROR_SUCCESS)
				break;
			if(lResult == ERROR_MORE_DATA)
				break;
			if(lResult == ERROR_NO_MORE_ITEMS)
				break;
			
			_snwprintf_s(wzFullSubKey,MAX_PATH, _TRUNCATE,L"%s\\%s",wzKeyName,wzSubKeyName);
			lSubResult=RegOpenKeyEx(HKEY_LOCAL_MACHINE, wzFullSubKey,0, KEY_READ,&hSubKey);
			if(lSubResult !=ERROR_SUCCESS)
				continue;
			dwData=sizeof(wzImage);
			lSubResult = RegQueryValueEx(hSubKey,AdapterCommon::Adapter::KEYVALUE_IMAGE,NULL,NULL,(LPBYTE)wzImage,&dwData);
			if(lSubResult != ERROR_SUCCESS)
			{
				RegCloseKey(hSubKey);
				continue;
			}
			dwData=sizeof(wzObligationName);
			lSubResult = RegQueryValueEx(hSubKey,AdapterCommon::Adapter::KEYVALUE_OBLIGATIONNAME,NULL,NULL,(LPBYTE)wzObligationName,&dwData);
			if(lSubResult != ERROR_SUCCESS)
			{
				RegCloseKey(hSubKey);
				continue;
			}
			AdapterCommon::Adapter adapter(wzImage,wzObligationName);
			adapter.SetName(wzSubKeyName);
			AddAdapter(adapter);
		}
		CComVariant varResult;
		HRESULT hr=AdapterCommon::AutoWrap(DISPATCH_PROPERTYGET,&varResult,m_pItem,L"BodyFormat",0);
		if(SUCCEEDED(hr)&&varResult.intVal==3)
			m_bIsRTF=true;
	}
private:
	AdapterManager():m_pItem(NULL),m_pAttachments(NULL){vecAdapters.clear();};
	void AddAdapter(AdapterCommon::Adapter& adapter){vecAdapters.push_back(adapter);};
	
	BOOL GetTempPathFromAttachmentObject(CComPtr<IDispatch> pAttachment,std::wstring& wstrTempPath)
	{
		CComVariant varDisplayName;
		HRESULT hr=AdapterCommon::AutoWrap(DISPATCH_PROPERTYGET,&varDisplayName,pAttachment,L"DisplayName",0);
		if(FAILED(hr)||varDisplayName.bstrVal==NULL)
			return FALSE;

		std::wstring wstrTemp=varDisplayName.bstrVal;
		std::wstring::size_type pos=wstrTemp.find(MAGIC_SEPARATOR);
		if(pos != std::wstring::npos)
			wstrTempPath=wstrTemp.substr(0,pos);
		else
			return FALSE;
		return TRUE;
	};
public:
	static WCHAR MAGIC_SEPARATOR[]; 
private:
	bool						m_bIsRTF;
	std::vector<AdapterCommon::Adapter> vecAdapters;
	CComPtr<IDispatch>					m_pItem;
	AdapterCommon::Attachments *m_pAttachments;
};
WCHAR AdapterManager::MAGIC_SEPARATOR[]=L"#TEMP&REAL#";
}
STDAPI RepositoryAdaptersManager(WCHAR* wzEnforcerKeyName,CComPtr<IDispatch> pItem,AdapterCommon::Attachments* pAtts);
STDAPI RepositoryAdaptersManagerEx(WCHAR* wzEnforcerKeyName,CComPtr<IDispatch> pItem,AdapterCommon::Attachments* pAtts,wchar_t** pwchObligationTopMessage,int &iTopMsgLength,wchar_t** pwchObligationBottomMessage,int &iBottomMsgLength);
STDAPI ReleaseRepositoryAdaptersResource(wchar_t * pwch,bool bIsArray);
#endif //__ADAPTERMANAGER_H__

