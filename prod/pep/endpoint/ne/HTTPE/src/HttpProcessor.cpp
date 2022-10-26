#include "StdAfx.h"
#include "HttpProcessor.h"
#include "MapperMngr.h"
#include "httpcollector.h"

#pragma warning( push )
#pragma warning( disable : 6011 )
#  include <boost/algorithm/string.hpp>
#pragma warning( pop )

#include <map>
using namespace std;

#include "timeout_list.hpp"

#include "eframework/platform/cesdk_loader.hpp"
extern nextlabs::cesdk_loader cesdkLoader;

#define HTTP_HEADER_INJECTION_NAME	L"HTTP_HEADER_INJECTION"
#define OB_LOGID_NAME					L"LogId"

#define EVAL_TIMEOUT_VALUE 2000
#define TEMP_DWNLD_FILE_TIMEOUT_VALUE (60 * 60 * 1000)

static CTimeoutList g_EvalList(EVAL_TIMEOUT_VALUE);

CTimeoutList g_Temp_DownloadURIList(TEMP_DWNLD_FILE_TIMEOUT_VALUE);

CTimeoutList CHttpProcessor::m_listEvalCache(2 * 1000);

pair<wstring, wstring> g_mapURI[] = {pair<wstring, wstring>(L"|", L"%7C")};
static void URIConvert(wstring& strURI)
{
	for(int i = 0; i < _countof(g_mapURI); i++)
	{
		boost::replace_all(strURI, g_mapURI[i].first, g_mapURI[i].second);
	//	g_log.Log(CELOG_DEBUG, L"HTTPE::Replace \"%s\" with \"%s\"", g_mapURI[i].first.c_str(), g_mapURI[i].second.c_str());
	}
}

static BOOL GetInjectionItems( IN CObligation& obligation, IN const wstring& strOBName, OUT map<wstring, vector<wstring>>& mapItems)
{
	vector<ATTRIBUTELIST*> vAttrs;
	if(obligation.GetAttributeListByName2(strOBName, vAttrs))
	{
		for(vector<ATTRIBUTELIST* >::iterator itr = vAttrs.begin(); itr != vAttrs.end(); itr++)
		{//every item of vAttrs is an entire obligation, including all attributes
			if(*itr && !(*itr)->empty())
			{
				wstring strLogId = obligation.GetAttrValueByName(*(*itr), OB_LOGID_NAME );
				wstring strField_Key = obligation.GetAttrValueByName(*(*itr), HTTP_HEADER_INJECTION_KEY );
				wstring strField_Value = obligation.GetAttrValueByName(*(*itr), HTTP_HEADER_INJECTION_VALUE );

				if(strField_Key.empty() || strLogId.empty())//empty key is not allowed
				{
					continue;
				}

				map<wstring, vector<wstring>>::iterator itr2;
				BOOL bFind = FALSE;
				wstring strText = strField_Key + L": " + strField_Value;//Combine the value, like: "NE4.6: HTTPE"
				for(itr2 = mapItems.begin(); itr2 != mapItems.end(); itr2++)
				{
					if((*itr2).first == strLogId)
					{//If the logid exists already, update the value.
						if(find((*itr2).second.begin(), (*itr2).second.end(), strText) != (*itr2).second.end())
						{
							break;
						}
						else
						{
							(*itr2).second.push_back(strText);
						
						}
						bFind = TRUE;
					}
				}
				if(!bFind)
				{
					vector<wstring> vText;
					vText.push_back(strText);
					mapItems[strLogId] = vText;
				}
			}
			
		}
		return TRUE;
	}

	return FALSE;
}
/*
for the http processor
*/
CHttpProcessor::CHttpProcessor(void)
{
}

CHttpProcessor::~CHttpProcessor(void)
{
}

int CHttpProcessor::ParseObligation( CEAttributes *obligation, CObligation& Ob)
{
	if( NULL == obligation || NULL == obligation->attrs || 0 == obligation->count )
	{
		return 0;
	}

	// The first key must be the count of obligation
	if( NULL == ( cesdkLoader.fns.CEM_GetString(obligation->attrs[0].key) ) || NULL == ( cesdkLoader.fns.CEM_GetString(obligation->attrs[0].value) ) )
	{
		return 0;
	}

	for( int i = 1; i < obligation->count; i++ )
	{
		OBLIGATION obInfo ;
		if(NULL==(cesdkLoader.fns.CEM_GetString(obligation->attrs[i].key) )|| NULL==(cesdkLoader.fns.CEM_GetString(obligation->attrs[i].value)))
			continue;

		std::wstring name(cesdkLoader.fns.CEM_GetString(obligation->attrs[i].key));
		std::wstring value(cesdkLoader.fns.CEM_GetString(obligation->attrs[i].value));
		if(0==wcsncmp(OBLIGATION_NAME_ID, name.c_str(), wcslen(OBLIGATION_NAME_ID)))
		{	
	//		g_log.Log(CELOG_DEBUG, L"\r\nObligation Name: [%s]",value.c_str());
			obInfo.strOBName =  value.c_str();	
			int j = i  ;
			while( j<obligation->count-1 )
			{
				j = i+1  ;
				if(NULL==(cesdkLoader.fns.CEM_GetString(obligation->attrs[j].key)) )
				{
					i++ ;
					j = i ;
					continue;
				}
				if(  NULL==(cesdkLoader.fns.CEM_GetString(obligation->attrs[j].value)) )
				{
					std::wstring ValueIDName(cesdkLoader.fns.CEM_GetString(obligation->attrs[j].key));
					if(0==wcsncmp(OBLIGATION_VALUE_ID, ValueIDName.c_str(), wcslen(OBLIGATION_VALUE_ID)))
					{
						ATTRIBUTE attrInfo ;
						obInfo.attrList.push_back( attrInfo ) ;
						i++ ;
						j = i ;
						continue ;
					}
				}
				std::wstring attrname(cesdkLoader.fns.CEM_GetString(obligation->attrs[j].key));
				std::wstring attrvalue(cesdkLoader.fns.CEM_GetString(obligation->attrs[j].value));
		//		g_log.Log(CELOG_DEBUG, L"\r\n Name: [%s],Value:[%s]",attrname.c_str(),attrvalue.c_str());
				if(0==wcsncmp(OBLIGATION_NAME_ID, attrname.c_str(), wcslen(OBLIGATION_NAME_ID)))
				{
					break ;
				}
				i++ ;
				if(0==wcsncmp(OBLIGATION_NUMVALUES_ID, attrname.c_str(), wcslen(OBLIGATION_NUMVALUES_ID)))
				{
					j = i ;
					continue ;
				}
				if(0==wcsncmp(OBLIGATION_VALUE_ID, attrname.c_str(), wcslen(OBLIGATION_VALUE_ID)))
				{
					ATTRIBUTE attrInfo ;
					attrInfo.strValue = attrvalue.c_str() ;
					obInfo.attrList.push_back( attrInfo ) ;
					j = i ;
					continue ;
				}
				else
				{
					j = i ;
					continue ;
				}
			}
			Ob.PushObligationItem(obInfo ) ;
		}
	}
	return  Ob.GetObligationCount();
}
int CHttpProcessor::DoEvaluation(const wstring & strAction, const wstring & strSrc, const wstring & strDest)//return value: 0 means allow
{
	CObligation obligation;
	
	return DoEvaluation(strAction, strSrc, strDest, obligation);
}

int CHttpProcessor::DoEvaluation(const std::wstring & strAction, const std::wstring & strSrc, const std::wstring & strDest, std::map<std::wstring,std::wstring> & mapAttributes)
{
	CObligation obligation;
	return DoEvaluation(strAction, strSrc, strDest, mapAttributes, obligation);
}

int CHttpProcessor::DoEvaluation(const wstring & strAction, const wstring & strSrc, const wstring & strDest, CObligation& obligation)
{
	std::map<std::wstring, std::wstring> mapAttributes;

	CHttpCollector& collector = CHttpCollector::CreateInstance();
	smartHttpMsg spMsg;
	if(collector.GetHttpMsgByRmtPath_RecvList( string( strSrc.begin(), strSrc.end() ), spMsg ) )
	{
		spMsg->GetHeaderItems(mapAttributes);
	}
	return DoEvaluation(strAction, strSrc, strDest, mapAttributes, obligation);
}

int CHttpProcessor::DoEvaluation(const wstring & strAction, const wstring & strSrc, const wstring & strDest, std::map<std::wstring, std::wstring> & mapAttributes, CObligation& obligation)//return value: 0 means allow
{
	//Try to get the result from cache
	wstring strResult;
	if(m_listEvalCache.FindItem(strSrc + strDest + strAction, strResult))
	{
	//	g_log.Log(CELOG_DEBUG, L"HTTPE::Use cache evaluation result, %s, \r\nresult: %s\r\n", (strSrc + strDest + strAction).c_str(), strResult.c_str());
		return (strResult == L"Allow")?0:1;
	}

	BOOL bAllow = TRUE;
	CPolicy* pPolicy = CPolicy::CreateInstance();
	if(pPolicy && CPolicy::m_bSDK)
	{
		EVALDATA data;
		data.strSrc = strSrc;

		if(_wcsicmp(strAction.c_str(), HTTP_OPEN) == 0)
		{
			data.strResourceType_Src = L"server";
			URIConvert(data.strSrc );
			ConvertUNCPath(data.strDest);
		}
		else if((_wcsicmp(strAction.c_str(), HTTP_UPLOAD) == 0))
		{
			data.strResourceType_Src = L"fso";
			ConvertUNCPath(data.strSrc);
		}
		

		data.strDest = strDest;
		if(_wcsicmp(strAction.c_str(), HTTP_OPEN) == 0)
		{
			data.strResourceType_Dest = L"fso";
		}
		else if((_wcsicmp(strAction.c_str(), HTTP_UPLOAD) == 0))
		{
			data.strResourceType_Dest = L"server";
			URIConvert(data.strDest);
		}
		

		CEEnforcement_t enforcement;
		memset(&enforcement, 0, sizeof(CEEnforcement_t));

		DWORD dwStart = GetTickCount();
		if(pPolicy->QueryPolicy(strAction, &data, mapAttributes, enforcement))
		{
			if(enforcement.result == CEAllow)
			{
				;
			}
			else
			{
				;
				bAllow = FALSE;
			}
		}
		/*
			Parse the obligation
		*/   
		ParseObligation( enforcement.obligation, obligation ) ;
		cesdkLoader.fns.CEEVALUATE_FreeEnforcement(enforcement);

		pPolicy->Release();

		g_log.Log(CELOG_DEBUG, L"\r\n----------------------HTTPE::Do Evaluation:\r\n Action: %s, \r\n src: %s, \r\n dest: %s\r\n result: %s\r\n Elapsed: %d ms \r\n", strAction.c_str(), data.strSrc.c_str(), data.strDest.c_str(), bAllow?L"Allow": L"Denied", GetTickCount() - dwStart);

		//Add the evaluation result to cache
		m_listEvalCache.AddItem(strSrc + strDest + strAction, bAllow? L"Allow": L"Deny");
	}
	return bAllow?0: 1;
}

int CHttpProcessor::ProcessMsg(smartHttpMsg& httpMsg)//return value, 0 means ALLOW
{
	if(httpMsg->GetType() == HTTP_GET)
	{
		string navigationUrl;		
		httpMsg->GetNavigationURL(navigationUrl);

		//	transfer the remote path to GB2312 encoding for there maybe Chinese words.
		string navigationUrl_GB;
		CEncoding::UTF8ToGB2312(navigationUrl, navigationUrl_GB);
		wstring strKey = MyMultipleByteToWideChar(navigationUrl_GB);
		
		wstring strEvalResult;
		if(!g_EvalList.FindItem(strKey, strEvalResult))
		{
	//		g_log.Log(CELOG_DEBUG, "\r\n HTTPE::Do evaluation for navigation %s, \r\n socket: %d,",  httpMsg->GetNavigationURL().c_str(), httpMsg->GetSock());
			CObligation obligation ;
			map<wstring, wstring> mapAttrs;
			httpMsg->GetHeaderItems(mapAttrs);
			int nRet = DoEvaluation(HTTP_OPEN, strKey, L" ", mapAttrs, obligation);
			strEvalResult = (nRet == 0? L"allow": L"deny");
			g_EvalList.AddItem(strKey, strEvalResult);
			const wstring & redirectUrl = obligation.GetRedirectURL();
			if( redirectUrl.length() != 0 )
			{
				g_log.Log(CELOG_DEBUG, L"\r\n===%s==== ",redirectUrl.c_str());
				httpMsg->SetRedirectURL(string(redirectUrl.begin(), redirectUrl.end())) ;
			}
			vector<wstring>   vInjectData ; 
			if( obligation.GetHeaderInjectionData(   vInjectData ) == TRUE )
			{  //added for the HTTP Header Injection
				//g_log.Log(CELOG_DEBUG, "\r\n Inejection Data %s,", strInjectData.c_str());
				httpMsg->SetHeaderInjectionData(vInjectData );

				//Log decision
				map<wstring, vector<wstring>> mapItems;
					CPolicy* pPolicy = CPolicy::CreateInstance();
					if(pPolicy)
					{
					if(GetInjectionItems(obligation, HTTP_HEADER_INJECTION_NAME, mapItems))
					{
						map<wstring, vector<wstring>>::iterator itr;
						for(itr = mapItems.begin(); itr != mapItems.end(); itr++)//Every item is one policy.
						{
						        std::wstring::size_type uCount = (*itr).second.size();
							if(uCount == 0)
							{
								continue;
							}

							vector<wstring>::iterator it2;
							unsigned nIndex = 0;
							wstring strLogValue;
							for(it2 = (*itr).second.begin(); it2 != (*itr).second.end() && nIndex < uCount; it2++)
							{//Try to get all the "header items".
								if(!strLogValue.empty())
							{
									strLogValue += L"; ";
								}
								strLogValue += *it2;
								nIndex++;
							}

							//fix bug10652, show all the "header items" in one line. like: "NE: HTTPE; Nextlabls: NE".
							if(!strLogValue.empty())
							{
								CEAttributes Attributes ={0};
								Attributes.count = 1;
								Attributes.attrs = new CEAttribute[Attributes.count];

								Attributes.attrs[0].key = cesdkLoader.fns.CEM_AllocateString(L"");
								Attributes.attrs[0].value = cesdkLoader.fns.CEM_AllocateString(strLogValue.c_str());

							pPolicy->LogObligationData((*itr).first.c_str(), HTTP_HEADER_INJECTION_NAME, &Attributes);

							for(int i = 0; i < Attributes.count; i ++)
							{
								cesdkLoader.fns.CEM_FreeString( Attributes.attrs[i].key ) ;
								cesdkLoader.fns.CEM_FreeString( Attributes.attrs[i].value ) ;
							}
							delete [] Attributes.attrs;
						}
							
						}
					}
					pPolicy->Release();
				}
				
			}
		}

		if(strEvalResult != L"allow")
		{
			if ( !httpMsg->GetHttpsFlag() )
			{
				//	navigation is denied, and it is not HTTPS, do not return winsock error code, so here return 0,
				//	set evaluation result to this http message
				httpMsg->SetProcResult(RESULT_DENY);
				return 0;
			}
			//	for HTTPS, return winsock error code.
			return 1;
		}
	}
	else if(httpMsg->GetType() == HTTP_POST)
	{
		//	for upload
		std::vector<std::string> vFiles;
		httpMsg->GetLocalPaths(vFiles);

		std::vector<std::string>::iterator itr = vFiles.begin();
		for( ; itr != vFiles.end(); itr++)
		{
	//		g_log.Log(CELOG_DEBUG, "\r\nUpload file: Local file path: %s, remote path: %s", (*itr).c_str(), httpMsg->GetRmt().c_str());
			string rmt;
			httpMsg->GetRmt(rmt, TRUE);
			
			map<wstring, wstring> attrs;
			httpMsg->GetHeaderItems(attrs);

			string strRemotePath_GB;
			CEncoding::UTF8ToGB2312(rmt, strRemotePath_GB);

			int nRet = DoEvaluation(HTTP_UPLOAD, MyMultipleByteToWideChar(*itr), MyMultipleByteToWideChar(strRemotePath_GB), attrs);
// 			if(IsProcess(L"firefox.exe") && httpMsg->GetHttpsFlag())//We can't delete the httpmsg since it's possible there are multiple files which needs to be uploaded.
// 			{
// 				CHttpCollector& collector = CHttpCollector::CreateInstance();
// 				collector.RemoveHttpMsg(TRUE, httpMsg->GetSock());
// 			}
			if( nRet != 0)
			{
				if(IsProcess(L"opera.exe"))
				{
					return 1;
				}
				if ( !httpMsg->GetHttpsFlag() )
				{
					//	upload is denied, and it is not HTTPS, do not return winsock error code, so here return 0,
					//	set evaluation result to this http message
					httpMsg->SetProcResult(RESULT_DENY);
					return 0;
				}
				//	for HTTPS, return winsock error code.
				return 1;
			}	
			
			httpMsg->SetEvalResult((*itr), RESULT_ALLOW);	
		}
	}
	if( httpMsg->GetType() == HTTP_200_OK && httpMsg->GetReqType() == HTTP_POST )//download
	{
		list<HTTP_MULTI_FILEDATA> listFile;
		httpMsg ->GetFileList(listFile) ;

		for(  list<HTTP_MULTI_FILEDATA>::iterator itor = listFile.begin() ; itor != listFile.end() ; ++itor )
		{
			if(	 (*itor).filedata.length() >0 )
			{
				CMapperMngr& ins = CMapperMngr::Instance() ;

				//	transfer the remote path to GB2312 encoding for there maybe Chinese words.
				string strRemote_GB;
				CEncoding::UTF8ToGB2312((*itor).filename, strRemote_GB);
				wstring strRemoteFile = MyMultipleByteToWideChar(strRemote_GB);

				ins.saveRemotePathAndData2( strRemoteFile,(*itor).filedata, httpMsg->GetSock() ) ;
				
			}
		}
	}
	if( (httpMsg->GetType() == HTTP_200_OK)  && (httpMsg->GetReqType() == HTTP_GET ))//download
	{
		list<HTTP_MULTI_FILEDATA> listFile;
		httpMsg ->GetFileList(listFile);

		for(  list<HTTP_MULTI_FILEDATA>::iterator itor = listFile.begin() ; itor != listFile.end() ; ++itor )
		{
			
			if(	 (*itor).filedata.length() >0 )
			{
				CMapperMngr& ins = CMapperMngr::Instance() ;
				
				//	transfer the remote path to GB2312 encoding for there maybe Chinese words.
				string strRemote_GB;
				CEncoding::UTF8ToGB2312((*itor).filename, strRemote_GB);
				wstring strRemoteFile = MyMultipleByteToWideChar(strRemote_GB);
				ins.saveRemotePathAndData2( strRemoteFile,(*itor).filedata, httpMsg->GetSock() ) ;
		//		g_log.Log(CELOG_DEBUG, "\r\nHTTPE::Get the remote file path%s", strRemoteFile.c_str());
				
			}
		}
	}

	return 0;
}
