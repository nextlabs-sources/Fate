// tag_office2k7.cpp : Defines the entry point for the DLL application.
//
#include "stdafx.h"
#include "msxml6.h"
#include "util.h"
#include <atlbase.h>
#include "tag_office2k7.h"
#include "opclib.h"
#include "TagOffice2k7.h"



#ifdef _MANAGED
#pragma managed(push, off)
#endif

HINSTANCE g_hInstance;

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch(ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		{
			g_hInstance = hModule;
			//CoInitialize(NULL);
			//
		}
		break;
	case DLL_PROCESS_DETACH:
		{
			//CoUninitialize();
			//
		}
		break;
	}
	return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif


#pragma warning(push)
#pragma warning(disable: 4273)
int AddTag(LPCWSTR pszFileName, LPCWSTR pszTagName, LPCWSTR pszTagValue, bool bRewriteIfExists)
{
	OutputDebugString(L"AddTag Start");
	std::vector<TAGPAIR> vecTag;
	TAGPAIR theTag;
	theTag.bSuccess=false;
	theTag.strTagName=pszTagName;
	theTag.strTagValue=pszTagValue;
	vecTag.push_back(theTag);
	CTagOffice2k7 aTag;
	int nRet=aTag.DoOffice2k7Tag(pszFileName,vecTag,enumAddTag,bRewriteIfExists);
	if(nRet == ERROR_NO_ERROR)
	{
		TRACE(CELOG_DEBUG, L"Add tag OK!!!!!!!!!!!!!!!!!!!!!");
	}
	else			
	{
		TRACE(CELOG_DEBUG, L"Add tag failed!!!!!!!!!!!!!!!!!!!!");
	}
	TRACE(CELOG_DEBUG, L"Add tag failed!!!!!!!!!!!!!!!!!!!!");
	return nRet;
}
int AddTags(LPCWSTR pszFileName, std::vector<TAGPAIR>& listTags, bool bRewriteIfExists)
{
	TRACE(CELOG_DEBUG, L"Add tags start");
	CTagOffice2k7 aTag;
	int nRet = aTag.DoOffice2k7Tag(pszFileName,listTags,enumAddTag,bRewriteIfExists);
	if(nRet == 0)	
	{
		TRACE(CELOG_DEBUG, L"Add tags OK!!!!!!!!!!!!!!!!!!!!!");
	}
	else
	{
		TRACE(CELOG_DEBUG, L"Add tags failed!!!!!!!!!!!!!!!!!!!!");
	}
	return nRet;
}

int GetTag(LPCWSTR pszFileName, LPCWSTR pszTagName, OUT std::wstring& strTagValue)
{
	std::vector<TAGPAIR> vecTag;
	TAGPAIR theTag;
	theTag.bSuccess=false;
	theTag.strTagName=pszTagName;
	vecTag.push_back(theTag);
	CTagOffice2k7 aTag;
	int nRet = aTag.DoOffice2k7Tag(pszFileName,vecTag,enumReadTag);
	
	if(nRet == ERROR_NO_ERROR && vecTag[0].bSuccess)
	{
		strTagValue = vecTag[0].strTagValue;
	}
	return nRet;
}
int GetTags(LPCWSTR pszFilename, OUT std::vector<TAGPAIR>& listTags)	// Fill in the tag values for the indicated tag names.
{
	TRACE(CELOG_DEBUG, L"Get tags start!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
	CTagOffice2k7 aTag;
	int nRet = aTag.DoOffice2k7Tag(pszFilename,listTags,enumReadTag);
	if(nRet == 0)
	{
		TRACE(CELOG_DEBUG, L"Get tags OK!!!!!!!!!!!!!!!!!!!!!");
	}
	else
	{
		TRACE(CELOG_DEBUG, L"Get tags failed!!!!!!!!!!!!!!!!!!!!");
	}
	return nRet;
}


int GetSummaryTags(LPCWSTR pszFilename, OUT std::vector<TAGPAIR>& listTags)	// Fill in the tag values for the indicated tag names.
{
	TRACE(CELOG_DEBUG, L"Get tags start!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
	CTagOffice2k7 aTag;
	int nRet = aTag.DoOffice2k7Tag(pszFilename,listTags,enumReadSummaryTag);
	
		
	
	if(nRet == 0)
	{
		TRACE(CELOG_DEBUG, L"Get tags OK!!!!!!!!!!!!!!!!!!!!!");
	}
	else
	{
		TRACE(CELOG_DEBUG, L"Get tags failed!!!!!!!!!!!!!!!!!!!!");
	}
	return nRet;
}



int DeleteTag(LPCWSTR pszFileName, LPCWSTR pszTagName)
{
	std::vector<TAGPAIR> vecTag;
	TAGPAIR theTag;
	theTag.bSuccess=false;
	theTag.strTagName=pszTagName;
	theTag.strTagValue=L"";
	vecTag.push_back(theTag);
	CTagOffice2k7 aTag;
	int nRet = aTag.DoOffice2k7Tag(pszFileName,vecTag,enumDeleteTag);
	return nRet;
}
int DeleteTags(LPCWSTR pszFileName, std::vector<TAGPAIR>& listTags)
{
	CTagOffice2k7 aTag;
	int nRet = aTag.DoOffice2k7Tag(pszFileName,listTags,enumDeleteTag);
	
		
	
	return nRet;
}
int DeleteAllTags(LPCWSTR pszFileName)
{
	std::vector<TAGPAIR> vecTag;
	CTagOffice2k7 aTag;
	int nRet = aTag.DoOffice2k7Tag(pszFileName,vecTag,enumDeleteTag);
	
		
	
	return nRet;
}

bool IsOffice2k7File(LPCWSTR strFilePath)
{
	return CTagOffice2k7::CheckOffice2k7file(strFilePath);
}
#pragma warning(pop)
///////////////////////////////////////////////////////