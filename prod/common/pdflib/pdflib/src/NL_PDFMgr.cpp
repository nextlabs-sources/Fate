#include "stdafx.h"
#include "NL_PDFMgr.h"
#include "NL_IncrementalUpdate.h"
#include <Windows.h>
#include "NL_Utils.h"

#define TONNY_DEBUG    1

CPDFMgr::CPDFMgr()
{

}

CPDFMgr::~CPDFMgr(void)
{
}


bool CPDFMgr::GetTags(const char* pszFileName, map<string, string>& mapAllTags)
{
	try
	{
		CPDFParser parser( pszFileName );
        map<string, string> mapOurTags;
		if(parser.Init())
		{
			string strOurTags;
			string strInfo;
			bool bLastIU;
			DWORD dwStart = GetTickCount();
			bool bIsReadTagCall = true;
			string strMetaDataObjContent = "";
			bool bFindMetaObjContent = false;
			strOurTags = parser.GetOurTagObj(strInfo, bLastIU,bIsReadTagCall,strMetaDataObjContent,bFindMetaObjContent);
			printf("calling GetOurTagObj() used: %d ms\r\n", GetTickCount() - dwStart);

#if 1  //add by Tonny at 4/29/2016, after that, we only have pc read all tags instead of read tag by pep and pc both.
            string strVersion = parser.GetPdfVersion();
            mapAllTags["PDFVersion"] = strVersion;
            if (bFindMetaObjContent && strMetaDataObjContent.size()>1)
            {
                CUtils::AnalyzeMetaDataInfo(strMetaDataObjContent, mapAllTags);

#if TONNY_DEBUG
                OutputDebugStringW(L"+++++++++++++++++> CUtils::AnalyzeMetaDataInfo(strMetaDataObjContent, mapAllTags);.\n");

#endif
            }
#endif
			map<string, string> mapTags;
			CUtils::SplitTags(strInfo, strOurTags, mapTags, mapOurTags);

			map<string, string>::iterator itr;
			for(itr = mapTags.begin(); itr != mapTags.end(); itr++)
			{
				mapAllTags[(*itr).first] = (*itr).second;
			}

			for(itr = mapOurTags.begin(); itr != mapOurTags.end(); itr++)
			{
				mapAllTags[(*itr).first] = (*itr).second;
			}

			
		} 
        //Try to get "comment tags"
        mapOurTags.clear();
		int nLen = 0;
		parser.GetCommentTags(mapOurTags, nLen);
		map<string, string>::iterator itr;
		for(itr = mapOurTags.begin(); itr != mapOurTags.end(); itr++)
		{
			mapAllTags[(*itr).first] = (*itr).second;
		}
		
		return mapAllTags.empty()?false: true;
	}
	catch( const PoDoFo::PdfError & eCode )
	{
		printf("Exception, failed to read tag. error: %d\r\n", eCode.GetError());
	}

	return false;
}



bool CPDFMgr::GetSummaryTags(const char* pszFileName, map<string, string>& mapAllTags)
{
	try
	{
		CPDFParser parser( pszFileName );

		if(parser.Init())
		{
			string strOurTags;
			string strInfo;
			bool bLastIU;
			DWORD dwStart = GetTickCount();
			bool bIsReadTagCall = true;
			string strMetaDataObjContent = "";
			bool bFindMetaObjContent = false;
			strOurTags = parser.GetOurTagObj(strInfo, bLastIU,bIsReadTagCall,strMetaDataObjContent,bFindMetaObjContent);
			printf("calling GetOurTagObj() used: %d ms\r\n", GetTickCount() - dwStart);


			string strVersion = parser.GetPdfVersion();
			mapAllTags["PDFVersion"] = strVersion;
			if (bFindMetaObjContent)
			{
				CUtils::AnalyzeMetaDataInfo(strMetaDataObjContent,mapAllTags);
			}
		}
		return mapAllTags.empty()?false: true;
	}
	catch( const PoDoFo::PdfError & eCode )
	{
		printf("Exception, failed to read tag. error: %d\r\n", eCode.GetError());
	}

	return false;
}

/******************************************************************
tag format
itar0x01yes0x02class0x01private0x02...
******************************************************************/
bool CPDFMgr::AddTags(const char* pszFileName, std::map<string,string> &mapTags)
{
	bool bRet = false;
	try
	{
		CIncrementalUpdate update;
		bRet =  update.AddIncrementalUpdate((char*)pszFileName, &mapTags);
	}
	catch( const PoDoFo::PdfError & eCode )
	{
		printf("Exception, failed to read tag. error: %d\r\n", eCode.GetError());
	}
	return bRet;
}

bool CPDFMgr::DeleteTags(const char *pszFileName, std::map<string,string> &mapTags)
{
	bool bRet = false;
	try
	{
		CIncrementalUpdate update;
		bRet = update.AddIncrementalUpdate((char*)pszFileName, &mapTags, false);
	}
	catch( const PoDoFo::PdfError & eCode )
	{
		printf("Exception, failed to read tag. error: %d\r\n", eCode.GetError());
	}
	return bRet;
}
