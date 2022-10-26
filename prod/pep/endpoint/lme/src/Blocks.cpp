#include "stdafx.h"
#include "Blocks.h"
#include "Helper.h"

std::list<std::wstring> gMsgBlocks;
std::list<std::wstring> gVoiceBlocks;
std::list<std::wstring> gConfBlocks;
std::list<std::wstring> gAppBlocks;

#define BLOCKS_CFG_NAME	L"block.cfg"

void LoadBlocks(HMODULE Module)
{
	TCHAR FileName[MAX_PATH]={0};
	TCHAR Path[MAX_PATH]={0};
	GetModuleFileName(Module,FileName,sizeof(FileName)/sizeof(TCHAR));
	PathName2Path(FileName,Path);
	wcsncat_s(Path,sizeof(Path)/sizeof(TCHAR),L"\\", _TRUNCATE);
	wcsncat_s(Path,sizeof(Path)/sizeof(TCHAR),BLOCKS_CFG_NAME, _TRUNCATE);

	for(int i=1;true;++i)
	{
		TCHAR KeyName[32]={0};
		swprintf_s(KeyName,sizeof(KeyName)/sizeof(TCHAR),L"id%d",i);
		TCHAR RetBuf[256]={0};
		GetPrivateProfileString(L"msg",KeyName,L"",RetBuf,sizeof(RetBuf)/sizeof(TCHAR),Path);
		if(wcslen(RetBuf)==0) break;

		_wcslwr_s(RetBuf,sizeof(RetBuf)/sizeof(TCHAR));
		gMsgBlocks.push_back(RetBuf);
		std::wstring Error = L"msg block ";
		Error += RetBuf;
		Error += L"\n";
		OutputDebugString(Error.c_str());
	}

	for(int i=1;true;++i)
	{
		TCHAR KeyName[32]={0};
		swprintf_s(KeyName,sizeof(KeyName)/sizeof(TCHAR),L"id%d",i);
		TCHAR RetBuf[256]={0};
		GetPrivateProfileString(L"voice",KeyName,L"",RetBuf,sizeof(RetBuf)/sizeof(TCHAR),Path);
		if(wcslen(RetBuf)==0) break;

		_wcslwr_s(RetBuf,sizeof(RetBuf)/sizeof(TCHAR));
		gVoiceBlocks.push_back(RetBuf);
		std::wstring Error = L"voice block ";
		Error += RetBuf;
		Error += L"\n";
		OutputDebugString(Error.c_str());
	}
}

bool IsBlock(TCHAR const* Name,enum UCC_SESSION_TYPE SessionType)
{
	if(!Name) return false;

	std::list<std::wstring>* pBlocks = NULL;
	switch(SessionType)
	{
	case UCCST_INSTANT_MESSAGING:
		pBlocks = &gMsgBlocks;
		break;
	case UCCST_AUDIO_VIDEO:
		pBlocks = &gVoiceBlocks;
		break;
	case UCCST_CONFERENCE:
		pBlocks = &gConfBlocks;
		break;
	case UCCST_APPLICATION:
		pBlocks = &gAppBlocks;
		break;
	default:
		return false;
	}

	TCHAR LowcaseName[256]={0};
	wcscpy_s(LowcaseName,sizeof(LowcaseName)/sizeof(TCHAR),Name);
	_wcslwr_s(LowcaseName,sizeof(LowcaseName)/sizeof(TCHAR));
	for(std::list<std::wstring>::const_iterator it = pBlocks->begin();
		it != pBlocks->end(); ++it)
	{
		if(*it == LowcaseName)
		{
			return true;
		}
	}

	return false;
}