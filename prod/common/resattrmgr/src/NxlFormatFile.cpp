#include "StdAfx.h"
#include "NxlFormatFile.h"

#pragma warning(disable: 4996)

CNxlFormatFile::CNxlFormatFile(void)
{
}

CNxlFormatFile::~CNxlFormatFile(void)
{
}


NXLERRORVALUE CNxlFormatFile::OpenNXLFile(_In_ const wchar_t *FileName,_Out_ FILE **fp)
{
	if (FileName == NULL)
	{
		return emPOINT_NULL;
	}
	NXLERRORVALUE Ret = emSUCCESS;
	errno_t err;
	err  = _wfopen_s( fp, FileName, L"r" );
	if( err == 0 )
	{
		return Ret;
	}
	else if (err == 2)
	{
		Ret = emFILE_NOT_EXIST;
	}
	else
	{
		Ret = emUNKNOWERROR;
	}
	wchar_t strlog[MAX_PATH_LENGTH] = {0};
	StringCchPrintf(strlog,MAX_PATH_LENGTH,L"Open [%s] File error, the error value is [%d]\n",FileName,err);
	OutputDebugString(strlog);
	return Ret;
}

NXLERRORVALUE CNxlFormatFile::CloseNXLFile(_In_ wstring strFileName,_In_ FILE *fp)
{
	if(fp != NULL)
	{
		int err = fclose(fp);
		if (err != 0)
		{
			wchar_t strlog[MAX_PATH_LENGTH] = {0};
			StringCchPrintf(strlog,MAX_PATH_LENGTH,L"close file fail!err the error value is [%d]\n",err);
			OutputDebugString(strlog);
			return emUNKNOWERROR;
		}
	}
	return emSUCCESS;
}


bool CNxlFormatFile::IsNXLFormatFile(_In_ const wchar_t *FileName)
{
	bool bRet = false;
	if (FileName != NULL)
	{
		FILE *pf = NULL;
		pf = _wfopen(FileName,L"r");
		if (pf != NULL)
		{
			char FileFormat[9] = {0};
			fseek(pf,0,SEEK_SET);
			fread(FileFormat,sizeof(char),8,pf);
			if ( memcmp(FileFormat,"NXLFMT!",8) == 0)
			{
				bRet = true;
			}
			CloseNXLFile(FileName,pf);
		}
	}
	return bRet;
}

NXLERRORVALUE CNxlFormatFile::ReadNxlFileTag(_In_ const wchar_t *FileName,_Out_ vector<pair<wstring,wstring>> & vecTagInfo)
{
	
	if (FileName == NULL )
	{
		return emPOINT_NULL;
	}
	NXLERRORVALUE Ret = emSUCCESS;
	FILE *pf = NULL;
	pf = _wfopen(FileName,L"r");
	if (pf != NULL)
	{
		int nAttrOffSet = GetOffSet(pf,FILE_ATTR_DATA_SIZE_OFFSET);
		int nRightOffSet = GetOffSet(pf,FILE_RIGHT_DATA_SIZE_OFFSET);
		int nTagOffSet = GetOffSet(pf,FILE_TAG_DATA_SIZE_OFFSET);
		int TagOffSet = nAttrOffSet + nRightOffSet + FILE_TYPE_OFFSET;
		wstring strFileName(FileName);
		int num = nTagOffSet/4097 + 1;
		for (int i = 0; i < num; i++)
		{
			Ret = GetTagInfo(pf,TagOffSet,vecTagInfo);
			if (Ret != emSUCCESS)
			{
				break;
			}
		}
		CloseNXLFile(strFileName,pf);
	}
	else
	{
		return emFILE_OPEN_FAIL;
	}

	return emSUCCESS;
}

int CNxlFormatFile::GetOffSet(_In_ FILE *fp, _In_ int offset)
{
	if (fp == NULL)
	{
		return 0;
	}
	char TagCount[5] = {0};
	fseek(fp,offset,SEEK_SET);
	fread(TagCount,1,4,fp);
	unsigned int num;
	memcpy(&num,TagCount,4);
	return num;
}

NXLERRORVALUE CNxlFormatFile::GetTagInfo(_In_ FILE *pf,_In_ int TagOffSet,_Out_ vector<pair<wstring,wstring>> & vecTagInfo)
{
	if (pf == NULL)
	{
		return emPOINT_NULL;
	}
	wchar_t Tag[4097] = {0};
	fseek(pf,TagOffSet,SEEK_SET);
	fread(Tag,sizeof(wchar_t),4096,pf);
	size_t nOffSetPos = 0;
	int nTagCount = 0;
	wstring strTagName = L"";
	wstring strTagValue = L"";
	do
	{
		wstring strBuf(&Tag[nOffSetPos]);
		if (strBuf.empty())
		{
			break;
		}
		size_t nPos = strBuf.find(L"=");
		if (nPos != wstring::npos)
		{
			strTagName = strBuf.substr(0,nPos);
			if (!strTagName.empty())
			{
				strTagValue = strBuf.substr(nPos + 1);
				nTagCount++;
				vecTagInfo.push_back(make_pair(strTagName,strTagValue));

				wchar_t strlog[MAX_PATH_LENGTH] = {0};
				StringCchPrintf(strlog,MAX_PATH_LENGTH,L"Get NXL File TagName is [%s],TagValue is [%s], The Num is [%d]\n",
					strTagName.c_str(),strTagValue.c_str(),nTagCount);
				::OutputDebugString(strlog);
			}
		}
		nOffSetPos += strBuf.length() + 1;

	}while(1);
	return emSUCCESS;
}

int CNxlFormatFile::GetNXLFileProps(const wchar_t* pszFileName, ResourceAttributes *attrs)
{
	if(!pszFileName || !attrs )
		return -1;

	vector<pair<wstring,wstring>> vecTagInfo;
	NXLERRORVALUE nRet = ReadNxlFileTag(pszFileName,vecTagInfo);
	if (nRet == emSUCCESS)
	{
		vector<pair<wstring,wstring>>::iterator itr;
		for(itr = vecTagInfo.begin(); itr != vecTagInfo.end(); itr++)
		{
			AddAttributeW(attrs, itr->first.c_str(), itr->second.c_str());
		}
		return 0;
	}
	else
	{
		wchar_t strlog[MAX_PATH] = {0};
		StringCchPrintf(strlog,MAX_PATH,L"ReadNxlFileTag return value is [%d]\n",nRet);
		::OutputDebugString(strlog);
	}
	return -1;
}
