#include <stdio.h>
#include <windows.h>
#include <string>
#include <iostream>
#include <vector>

#include "Office2k7_attrs.h"
#include "tag_office2k7.h"

#ifdef _WIN64
	#define ATTRMGR_OFFICE2K7_DLL	L"tag_office2k7.dll"
#else
	#define ATTRMGR_OFFICE2K7_DLL	L"tag_office2k732.dll"
#endif

HMODULE g_hModule = NULL;


//Exported APIs
typedef int (*AddTag_Ptr)(LPCWSTR pszFileName, LPCWSTR pszTagName, LPCWSTR pszTagValue, bool bRewriteIfExists);
typedef int (*AddTags_Ptr)(LPCWSTR pszFileName, std::vector<TAGPAIR>& listTags, bool bRewriteIfExists);

typedef int (*GetTag_Ptr)(LPCWSTR pszFileName, LPCWSTR pszTagName, OUT std::wstring& strTagValue);
typedef int (*GetTags_Ptr)(LPCWSTR pszFilename, OUT std::vector<TAGPAIR>& listTags);// Fill in the tag values for the indicated tag names.
typedef int (*GetAllTags_Ptr)(LPCWSTR pszFileName, OUT std::vector<TAGPAIR>& listTags);// return all the tags.

typedef int (*DeleteTag_Ptr)(LPCWSTR pszFileName, LPCWSTR pszTagName);
typedef int (*DeleteTags_Ptr)(LPCWSTR pszFileName, std::vector<TAGPAIR>& listTags);
typedef int (*DeleteAllTags_Ptr)(LPCWSTR pszFileName);

typedef bool (*IsOffice2k7File_Ptr)(LPCWSTR pszFileName);


typedef int (*GetSummaryTags_Ptr)(LPCWSTR pszFilename, OUT std::vector<TAGPAIR>& listTags);

AddTag_Ptr fn_AddTag = NULL;
AddTags_Ptr fn_AddTags = NULL;
GetTag_Ptr fn_GetTag = NULL;
GetTags_Ptr fn_GetTags = NULL;
DeleteTag_Ptr fn_DeleteTag = NULL;
DeleteTags_Ptr fn_DeleteTags = NULL;
DeleteAllTags_Ptr fn_DeleteAllTags = NULL;
IsOffice2k7File_Ptr fn_IsOffice2k7File = NULL;
GetSummaryTags_Ptr fn_GetSummaryTags = NULL;

extern wchar_t g_szCurrentDir[MAX_PATH + 1];
void LoadOffice2k7Dll()
{
	std::wstring str2K7DLL_Path(g_szCurrentDir);
	str2K7DLL_Path.append(L"\\");
	str2K7DLL_Path.append(ATTRMGR_OFFICE2K7_DLL);
	g_hModule = LoadLibraryW(str2K7DLL_Path.c_str());
	if(g_hModule)
	{
		fn_AddTag = (AddTag_Ptr)GetProcAddress(g_hModule, "AddTag");
		fn_AddTags = (AddTags_Ptr)GetProcAddress(g_hModule, "AddTags");

		fn_GetTag = (GetTag_Ptr)GetProcAddress(g_hModule, "GetTag");
		fn_GetTags = (GetTags_Ptr)GetProcAddress(g_hModule, "GetTags");

		fn_DeleteTag = (DeleteTag_Ptr)GetProcAddress(g_hModule, "DeleteTag");
		fn_DeleteTags = (DeleteTags_Ptr)GetProcAddress(g_hModule, "DeleteTags");
		fn_DeleteAllTags = (DeleteAllTags_Ptr)GetProcAddress(g_hModule, "DeleteAllTags");

		fn_IsOffice2k7File = (IsOffice2k7File_Ptr)GetProcAddress(g_hModule, "IsOffice2k7File");

		fn_GetSummaryTags = (GetTags_Ptr)GetProcAddress(g_hModule, "GetSummaryTags");
	}
}

void FreeOffice2k7Dll()
{
	if(g_hModule)
	{
		FreeLibrary(g_hModule);
		g_hModule = NULL;
	}
}

int GetO2K7FileProps(const wchar_t* pszFileName, ResourceAttributes *attrs)
{
	if(!pszFileName || !attrs || !fn_GetTags)
		return -1;

	std::vector<TAGPAIR> vTags;
	vTags.clear();
	int nRet = 0;
	if(fn_GetTags)
		nRet = fn_GetTags(pszFileName,vTags);

	if(0 != nRet)
		return nRet;

	std::vector<TAGPAIR>::iterator itr;
	for(itr = vTags.begin(); itr != vTags.end(); itr++)
	{
		TAGPAIR& pair = *itr;
		if(pair.bSuccess)
			AddAttributeW(attrs, pair.strTagName.c_str(), pair.strTagValue.c_str());
	}
	return nRet;
}


int GetO2K7FileSummaryProps(const wchar_t* pszFileName, ResourceAttributes *attrs)
{
	if(!pszFileName || !attrs || !fn_GetSummaryTags)
		return -1;

	std::vector<TAGPAIR> vTags;
	vTags.clear();
	int nRet = 0;
	if(fn_GetSummaryTags)
		nRet = fn_GetSummaryTags(pszFileName,vTags);

	if(0 != nRet)
		return nRet;

	std::vector<TAGPAIR>::iterator itr;
	for(itr = vTags.begin(); itr != vTags.end(); itr++)
	{
		TAGPAIR& pair = *itr;
		if(pair.bSuccess)
			AddAttributeW(attrs, pair.strTagName.c_str(), pair.strTagValue.c_str());
	}
	return nRet;
}

int SetO2K7FileProps(const wchar_t* pszFileName, ResourceAttributes *attrs)
{
	if(!pszFileName || !attrs || !fn_AddTags)
		return -1;

	std::vector<TAGPAIR> vTags;
	vTags.clear();
	for(int i = 0; i < GetAttributeCount(attrs); i++)
	{
		WCHAR *tagName = (WCHAR *)GetAttributeName(attrs, i);
		WCHAR *tagValue = (WCHAR *)GetAttributeValue(attrs, i);

		TAGPAIR pair;
		if(tagName)
			pair.strTagName = std::wstring(tagName);
		if(tagValue)
			pair.strTagValue = std::wstring(tagValue);

		vTags.push_back(pair);
	}

	int nRet = 0;
	if(fn_AddTags)
		nRet = fn_AddTags(pszFileName, vTags, TRUE);

	return nRet;
}

int RemoveO2K7FileProps(const wchar_t* pszFileName, ResourceAttributes *attrs)
{
	if(!pszFileName || !attrs || !fn_DeleteTags)
		return -1;

	std::vector<TAGPAIR> vTags;
	vTags.clear();
	for(int i = 0; i < GetAttributeCount(attrs); i++)
	{
		WCHAR *tagName = (WCHAR *)GetAttributeName(attrs, i);
		WCHAR *tagValue = (WCHAR *)GetAttributeValue(attrs, i);

		TAGPAIR pair;
		if(tagName)
			pair.strTagName = std::wstring(tagName);
		if(tagValue)
			pair.strTagValue = std::wstring(tagValue);

		vTags.push_back(pair);
	}
	
	int nRet = 0;
	if(fn_DeleteTags)
		nRet = fn_DeleteTags(pszFileName, vTags);

	return nRet;
}

BOOL IsOffice2k7FileType(const wchar_t* pszFileName)
{
	if(!pszFileName)
		return FALSE;
	return fn_IsOffice2k7File && fn_IsOffice2k7File(pszFileName);
}