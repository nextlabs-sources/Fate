/*
*\brief:	This SDK is not thread safe, so we don't be able to maintain one connection for multi-thread.
			we keep a short connection only for evaluation one times.
*/

#include "stdafx.h"
#include "utils.h"
#include "dllmain.h"



////////////////////////////CELog2//////////////////////////////////////////////
// for CELog2 we should define: CELOG_CUR_FILE
#define CELOG_CUR_FILE static_cast<celog_filepathint_t>(CELOG_FILEPATH_OFFICEPEP_MIN + EMNLFILEINTEGER_POLICY)
//////////////////////////////////////////////////////////////////////////


static const wchar_t* WDE_PLATFORM_RESOURCE_DOCUMENT = L"fso";     // document resource

time_t CNxtSDK::WinTime2JavaTime( SYSTEMTIME* pSysTime )
{
	time_t rtTime = 0;
	tm       rtTM   = { 0 };

	rtTM.tm_year   = pSysTime->wYear - 1900;
	rtTM.tm_mon   = pSysTime->wMonth - 1;
	rtTM.tm_mday = pSysTime->wDay;
	rtTM.tm_hour   = pSysTime->wHour;
	rtTM.tm_min    = pSysTime->wMinute;
	rtTM.tm_sec     = pSysTime->wSecond;
	rtTM.tm_wday = pSysTime->wDayOfWeek;
	rtTM.tm_isdst  = -1;     // Let CRT Lib compute whether DST is in effect,
	
	// assuming US rules for DST.
	rtTime = mktime( &rtTM ); // get the second from Jan. 1, 1970

	if ( rtTime == (time_t) -1 )
	{
		if ( pSysTime->wYear <= 1970 )
		{
			rtTime = (time_t) 0;		// Underflow.  Return the lowest number possible.
		}
		else
		{
			rtTime = (time_t) _I64_MAX;	// Overflow.  Return the highest number possible.
		}
	}
	else
	{
		rtTime*= 1000;          // get millisecond
	}
	return rtTime;
}

wstring CNxtSDK::GetCurrentStringTime()
{
	WCHAR wzTm[MAX_PATH+1] = {0};
	SYSTEMTIME sysTm = {0};
	GetSystemTime( &sysTm );
	time_t javaTime = WinTime2JavaTime( &sysTm );
	StringCchPrintfW( wzTm,  MAX_PATH,  L"%I64d",  javaTime );
	return wzTm;
}

EMNLOFFICE_RESULT CNxtSDK::DoEvaluation( const WCHAR* pwchSource, const EMNLOFFICE_ACTION& emAction, nextlabs::Obligations& obs, const WCHAR* pwchDest,
									  vector< pair<wstring,wstring> >* pvecSrcAttrs, 
										CENoiseLevel_t noise_level, const wstring wstrapp_attr_value, bool bForceNoCache )
{
	wstring wstrAction = NLACTIONTYPEINSTANCE.NLGetStringActionTypeByEnum( emAction );
	return CNxtSDK::DoEvaluation( pwchSource, wstrAction.c_str(), obs, pwchDest, pvecSrcAttrs, noise_level, wstrapp_attr_value, bForceNoCache );
}

EMNLOFFICE_RESULT CNxtSDK::DoEvaluation( const WCHAR* source, const WCHAR* action, nextlabs::Obligations& obs, const WCHAR* dest,
										vector< pair<wstring,wstring> >* psrcAttrs,
										CENoiseLevel_t noise_level, const wstring wstrapp_attr_value, bool bForceNoCache )
{NLCELOG_ENUM_ENTER(EMNLOFFICE_RESULT)
	NLPRINT_DEBUGLOG( L" The Parameters are: source=%ls, action=%ls, obs=%p, dest=%ls, psrcAttrs=%p, noise_level=%d, wstrapp_attr_value=%ls, bForceNoCache=%ls \n", print_long_string(source), print_long_string(action), &obs, print_long_string(dest),psrcAttrs,noise_level, wstrapp_attr_value.c_str(),bForceNoCache?L"TRUE":L"FALSE");
	EMNLOFFICE_RESULT emOfficeResult = emNLOfficeResultUnknown;
	static nextlabs::cesdk_context context;
	if ( NULL == context.m_pApp )
	{
		nextlabs::comm_helper::Init_Cesdk( &context );	// if no initialize, initialize it.
	}

	boost::shared_ptr<nextlabs::comm_base> ptr = nextlabs::comm_helper::CreateComm(nextlabs::type_cesdk,&context);
	nextlabs::ATTRS theAttrs;
	nextlabs::eval_parms parm;
	parm.SetNoiseLevel(noise_level);
	
	wstring strAction(action);
	wstring strSrc(source);
	wstring strDst(L"");
	wstring strSType = WDE_PLATFORM_RESOURCE_DOCUMENT;
	wstring strDType = WDE_PLATFORM_RESOURCE_DOCUMENT;

	if(dest != NULL)	strDst = dest;

	/*
	*\ for local path or unc path ,we just disable PC  reading tag.
	*	for http or ftp or other network path, we will disable pc reading tag and reading content.
	* disable read tag, just set "ce::file_custom_attributes_included = yes"
	* disable read tag/content, set "ce::filesystemcheck=no"
	*/
	static const wchar_t* cstr_disablereadtagname = L"ce::file_custom_attributes_included";
	static const wchar_t* cstr_disablereadtagcontentname = L"ce::filesystemcheck";

	pair<wstring,wstring> pairWstrExtraTag;
	if(psrcAttrs != NULL && !psrcAttrs->empty()) 	
	{
		psrcAttrs->push_back(pair<wstring,wstring>( cstr_disablereadtagcontentname, L"no" ) );
		pairWstrExtraTag.first		= cstr_disablereadtagcontentname;
		pairWstrExtraTag.second	= L"no";
	}

	parm.SetAction(strAction.c_str());
	
	//revert to the original value if the converted path is not existed.
	wstring wstrDstBak = strDst;
	ConvertURLCharacterW(strDst);
	if (!PathFileExistsW(strDst.c_str())) 
		strDst = wstrDstBak;
		
	if(dest != NULL) parm.SetTarget(strDst.c_str(),strDType.c_str());
	if(psrcAttrs != NULL && !psrcAttrs->empty())
	{
		vector<pair<wstring,wstring>> vecTag;
		vector<pair<wstring,wstring>>::const_iterator inner,it=psrcAttrs->begin();
		for(;it != psrcAttrs->end();it++)
		{
			const wstring& strname = (*it).first;
			const wstring& strvalue = (*it).second;
			
			// we should delete empty tags
			if ( strname.empty() || strvalue.empty() )
			{
				::OutputDebugStringW( L"**** policy, has empty tags **** \n" );
				continue;
			}

			bool bHas = false;
			for(inner = vecTag.begin();inner != vecTag.end();inner++)
			{
				const wstring& striname=(*inner).first;
				const wstring& strivalue = (*inner).second;
				if(_wcsicmp(strname.c_str(),striname.c_str()) == 0 &&
					_wcsicmp(strvalue.c_str(),strivalue.c_str())==0 )	
				{
					bHas = true; // delete repeat tags
					break;
				}
			}
			if(!bHas)
			{
				vecTag.push_back(pair<wstring,wstring>(strname,strvalue));
			}
		}
		theAttrs.insert(vecTag.begin(), vecTag.end());
	}
	ConvertURLCharacterW(strSrc);

	if( bForceNoCache )
	{
		wstring strTime = GetCurrentStringTime();
		theAttrs.insert(pair<wstring,wstring>(L"modified_date",strTime));	// modify the time and the file update, PC will query again and not use the policy cache.
	}
	parm.SetSrc(strSrc.c_str(),strSType.c_str(),&theAttrs);
	nextlabs::ATTRS app_attr;
	wstring app_attr_name(L"NextLabs Enforcer");
	pair<wstring,wstring> app_attr_tag(app_attr_name,wstrapp_attr_value);
	app_attr.insert(app_attr_tag);
	parm.SetApplicationAttrs(&app_attr);

	ptr->Query(&parm);
	obs = ptr->GetObligations();
	if(ptr->IsDenied())	emOfficeResult = emNLOfficeResultPolicyDeny;

	else emOfficeResult =  emNLOfficeResultPolicyAllow;

	if ( emNLOfficeResultPolicyDeny == emOfficeResult )
	{
		NLPRINT_DEBUGLOG( L"DoEvaluation: denied by the policy, source [%s], dst is [%s],thread id is [%d].\n ", strSrc.c_str(),strDst.c_str(),::GetCurrentThreadId());
	}
	else
	{
		NLPRINT_DEBUGLOG( L"DoEvaluation: allowed by the policy, source [%s], dst is [%s],thread id is [%d].\n ", strSrc.c_str(),strDst.c_str(),::GetCurrentThreadId());
	}

	if (psrcAttrs != NULL && psrcAttrs->size() > 0)
	{
		for( vector<std::pair<std::wstring,std::wstring>>::iterator it = psrcAttrs->begin(); it != psrcAttrs->end(); it++ )
		{
			NLPRINT_DEBUGLOG( L"\tTag Name: [%s], Tag Value: [%s]\n", (*it).first.c_str(), (*it).second.c_str());
		}
	}
	
	// delete the extra tag
	if ( NULL != psrcAttrs && !pairWstrExtraTag.first.empty() )
	{
		for ( vector<pair<wstring,wstring>>::iterator it = psrcAttrs->begin(); it != psrcAttrs->end(); it++ )
		{
			if ( 0 == wcscmp( it->first.c_str(),  pairWstrExtraTag.first.c_str()  ) &&
				0 == wcscmp( it->second.c_str(), pairWstrExtraTag.second.c_str() ) )
			{
				psrcAttrs->erase( it );
				break;
			}
		}
	}

	if (g_bDebugMode_L2) 
	{
		wchar_t wszlog[2048]={0};
		StringCchPrintfW(wszlog,2047,L"DoEvaluation[%s]: source [%s], dst is [%s],thread id is [%d].\n",strAction.c_str(), strSrc.c_str(),strDst.c_str(),::GetCurrentThreadId());
		
		wstring wstrTestLog = wszlog;

		if ( NULL != psrcAttrs && !pairWstrExtraTag.first.empty() )
		{
			for ( vector<pair<wstring,wstring>>::iterator it = psrcAttrs->begin(); it != psrcAttrs->end(); it++ )
			{
				// add file tags here
				wchar_t wszlogTag[1024]={0};
				StringCchPrintfW( wszlogTag, 1023, L"\n tag:[ %s = %s ] \n", it->first.c_str(), it->second.c_str() );
				wstrTestLog += wszlogTag;
			}
		}

		if(MessageBoxW(GetActiveWindow(),wstrTestLog.c_str(),L"Allow?",MB_YESNO) == IDNO)
		{
			NLCELOG_ENUM_RETURN_VAL( emNLOfficeResultPolicyDeny )
		}
	}
	NLCELOG_ENUM_RETURN_VAL( emOfficeResult )
}
