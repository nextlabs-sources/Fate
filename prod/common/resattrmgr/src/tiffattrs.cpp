#include <stdio.h>
#include <windows.h>
#include <string>
#include <iostream>
#include <map>

#include "resattrlib.h"
#include "tiffattrs.h"
#include "tiffiop.h"

// for CELog2
#include "NLCELog.h"
#define  CELOG_CUR_FILE static_cast<celog_filepathint_t>(EMNLFILEINTEGER_TIFFATTRSCPP)

#define FIELD_CUSTOM_NXTLBS			EXIFTAG_USERCOMMENT + 1

const wchar_t g_szSeparate_Tags[] = {0x01, 0x00};
const wchar_t g_szSeparate_NameValue[] = {0x02, 0x00};

BOOL Tag_Exists(std::map<std::wstring, std::wstring>& mapTags, LPCWSTR pszTagName, OUT std::map<std::wstring, std::wstring>::iterator& o_itr)
{
	if(!pszTagName)
		return FALSE;

//	wchar_t szTagName[101] = {0};
	std::map<std::wstring, std::wstring>::iterator itr;
	for(itr = mapTags.begin(); itr != mapTags.end(); itr++)
	{
		std::pair<std::wstring, std::wstring> pair;
		pair = *itr;
		if(_wcsicmp(pair.first.c_str(), pszTagName ) == 0)
		{
			o_itr = itr;
			return TRUE;
		}
		/*
		memset(szTagName, 0, sizeof(szTagName));
				memcpy(szTagName, pair.first.c_str(), (pair.first.length() > 100? 100:pair.first.length()) * sizeof(wchar_t));
				wcslwr(szTagName)*/
		
	}
	return FALSE;
}

void GetMapOfTags(ResourceAttributes * attrs, std::map<std::wstring, std::wstring>& mapTags)
{
	if( !attrs )
		return;

	int nCount = GetAttributeCount(attrs);
	
	for(int i = 0; i < nCount; i++)
	{
		WCHAR *tagName = (WCHAR *)GetAttributeName(attrs, i);
		WCHAR *tagValue = (WCHAR *)GetAttributeValue(attrs, i);

		mapTags[tagName] = tagValue;
	}

}

BOOL WriteTiffTags(const WCHAR *fileName, std::map<std::wstring, std::wstring>& mapTags)
{NLCELOG_ENTER
	TIFF* tif = TIFFOpenW(fileName, "r+");
	if(!tif)
		NLCELOG_RETURN_VAL( FALSE )

	TIFFDirectory *td = &tif->tif_dir;	
	td;

	static const TIFFFieldInfo xtiffFieldInfo[] = {

		// XXX Insert Your tags here 
		{ FIELD_CUSTOM_NXTLBS,	-3,-3, TIFF_ASCII,	FIELD_CUSTOM,
		TRUE,	TRUE,	"NXTLBS_TAG" },
	};

//	const TIFFField *fip = TIFFFindField(tif, FIELD_CUSTOM_NXTLBS, TIFF_ANY);

	const TIFFField *fip = NULL;

	char* buffer = NULL;
	int count = 0;
	int nNXT_Tag = TIFFGetField(tif, FIELD_CUSTOM_NXTLBS, &count, &buffer);

	if(!nNXT_Tag)
		TIFFMergeFieldInfo(tif, xtiffFieldInfo, 1);

		fip = TIFFFieldWithTag(tif, FIELD_CUSTOM_NXTLBS);


	std::map<std::wstring, std::wstring>::iterator itr;
	std::wstring strTags(L"");
	for(itr = mapTags.begin(); itr != mapTags.end(); itr++)
	{
		std::pair<std::wstring, std::wstring> pair;
		pair = *itr;

		std::wstring tag;
		tag.append(pair.first);
		tag.append(g_szSeparate_NameValue);
		tag.append(pair.second);
		tag.append(g_szSeparate_Tags);

		strTags += tag;

	}

	size_t size = 0;
	errno_t err = wcstombs_s(&size, NULL, 0, strTags.c_str(), 0);
	char * mbBuffer = new char[size];
	if(!mbBuffer)
	{
		TIFFClose(tif);
		NLCELOG_RETURN_VAL( FALSE )
	}
	memset(mbBuffer, 0, size);
	err = wcstombs_s(&size, mbBuffer, size, strTags.c_str(), _TRUNCATE);
	if(err)
	{
		TIFFClose(tif);
		NLCELOG_RETURN_VAL( FALSE )
	}

	int nRet;
	int len = (int)strlen(mbBuffer);
	if(fip && fip->field_passcount)
		nRet = TIFFSetField(tif, FIELD_CUSTOM_NXTLBS, (uint32)len, mbBuffer);



	nRet = TIFFWriteCheck(tif, 0, "NXTLBS");
	nRet = TIFFFlush(tif);

	TIFFClose(tif);

	NLCELOG_RETURN_VAL( TRUE )
}

BOOL IsTIFFFile(LPCWSTR pwzFile)
{
	LPCWSTR pSuffix = wcsrchr(pwzFile, L'.');
	if(NULL == pSuffix) 
		return FALSE;

	if(0 == _wcsicmp(pSuffix, L".tiff") ||0 == _wcsicmp(pSuffix, L".TIFF")||0 == _wcsicmp(pSuffix, L".tif") ||0 == _wcsicmp(pSuffix, L".TIF"))
	{
		TIFFSetWarningHandler(NULL);
		TIFFSetWarningHandlerExt(NULL);
		TIFFSetErrorHandler(NULL);
		TIFFSetErrorHandlerExt(NULL);
		return TRUE;
	}

	return FALSE;
}

BOOL SetTIFFFileProps(const WCHAR *fileName, ResourceAttributes *attrs)
{NLCELOG_ENTER
	if(!fileName || !attrs)
		NLCELOG_RETURN_VAL( FALSE )

	ResourceAttributes * old_attrs;
	AllocAttributes(&old_attrs);

	GetTIFFFileProps(fileName, old_attrs);//Get the existing tags
	std::map<std::wstring, std::wstring> mapTags;
	mapTags.clear();
	GetMapOfTags(old_attrs, mapTags);

	FreeAttributes(old_attrs);

	std::map<std::wstring, std::wstring>::iterator itr;
	for(int i = 0; i < GetAttributeCount(attrs); i++)//add the new tags into mapTags.
	{
		WCHAR *tagName = (WCHAR *)GetAttributeName(attrs, i);
		WCHAR *tagValue = (WCHAR *)GetAttributeValue(attrs, i);

		if(Tag_Exists(mapTags, tagName, itr))
			mapTags.erase(itr);

		mapTags[tagName] = tagValue;
		
	}

	NLCELOG_RETURN_VAL( WriteTiffTags(fileName, mapTags) )
	
}

BOOL GetTIFFFileProps(const WCHAR *fileName, ResourceAttributes *attrs)
{NLCELOG_ENTER
	if(!fileName || !attrs)
		NLCELOG_RETURN_VAL( FALSE )

	TIFF* tif = TIFFOpenW(fileName, "r");
	if(!tif)
		NLCELOG_RETURN_VAL( FALSE )

	int count = 0;
	char* buffer = NULL;

	TIFFGetField(tif, FIELD_CUSTOM_NXTLBS, &count, &buffer);

	if(!buffer)
	{
		TIFFClose(tif);
		NLCELOG_RETURN_VAL( TRUE )//kevin fix bug411
	}

	char *pTmp = new char[count + 1];
	if(!pTmp)
	{
		TIFFClose(tif);
		NLCELOG_RETURN_VAL( FALSE )
	}
	memset(pTmp, 0, count + 1);
	memcpy(pTmp, buffer, count);
	size_t size = 0;
	mbstowcs_s(&size, NULL, 0, pTmp, 0);
	wchar_t * wbuffer = new wchar_t[size];
	if(!wbuffer)
	{
		TIFFClose(tif);
		NLCELOG_RETURN_VAL( FALSE )
	}

	mbstowcs_s(&size, wbuffer, size, pTmp, _TRUNCATE);
	if(pTmp)
	{
		delete []pTmp;
		pTmp = NULL;
	}

	std::wstring temp(wbuffer);
	if(wbuffer)
	{
		delete []wbuffer;
		wbuffer = NULL;
	}
	if(temp.length() <= 0)
	{
		TIFFClose(tif);
		NLCELOG_RETURN_VAL( FALSE )
	}

	int nIndex = 0;
	int nCount = 0;
	while((nIndex = (int)temp.find(g_szSeparate_Tags)) > 0)
	{
		std::wstring tag = temp.substr(0, nIndex); 
		int i = (int)tag.find(g_szSeparate_NameValue);
		if(i <= 0)
		{
			temp = temp.substr(nIndex + 1, temp.length() - nIndex - 1);
			continue;
		}

		std::wstring tagName = tag.substr(0, i);
		std::wstring tagValue = tag.substr(i + 1, tag.length() - i - 1);

		AddAttributeW(attrs, tagName.c_str(), tagValue.c_str());
		nCount++;

		temp = temp.substr(nIndex + 1, temp.length() - nIndex - 1);
	}

	
	std::wstring tag = temp;
	int i = (int)tag.find(g_szSeparate_NameValue);
	if(i <= 0)
	{
		TIFFClose(tif);
		NLCELOG_RETURN_VAL( TRUE )
	}

	std::wstring tagName = tag.substr(0, i);
	std::wstring tagValue = tag.substr(i + 1, tag.length() - i - 1);

	AddAttributeW(attrs, tagName.c_str(), tagValue.c_str());
	nCount++;

	TIFFClose(tif);
	NLCELOG_RETURN_VAL( TRUE )
}

BOOL RemoveTIFFFileProps(const WCHAR *fileName, ResourceAttributes *attrs)
{NLCELOG_ENTER
	if(!fileName || !attrs)
		NLCELOG_RETURN_VAL( FALSE )

	ResourceAttributes * old_attrs;
	AllocAttributes(&old_attrs);

	BOOL bRet = GetTIFFFileProps(fileName, old_attrs);

	if(!bRet)
	{
		FreeAttributes(old_attrs);
		NLCELOG_RETURN_VAL( FALSE )
	}

	std::map<std::wstring, std::wstring> mapTags;
	mapTags.clear();
	GetMapOfTags(old_attrs, mapTags);

	FreeAttributes(old_attrs);

	BOOL bRemoved = FALSE;
	for(int i = 0; i < GetAttributeCount(attrs); i++)//remove the indicated tags
	{
		WCHAR *tagName = (WCHAR *)GetAttributeName(attrs, i);

		std::map<std::wstring, std::wstring>::iterator itr;
		if(Tag_Exists(mapTags, tagName, itr))
		{
			mapTags.erase(itr);
			bRemoved = TRUE;
		}

	}

	if(bRemoved)
		bRemoved = WriteTiffTags(fileName, mapTags);

	NLCELOG_RETURN_VAL( bRemoved )

}