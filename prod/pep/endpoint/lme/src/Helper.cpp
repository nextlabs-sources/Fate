#include "stdafx.h"
#include "Helper.h"

// return value doesn't include '\'
// input c:\abc\1.exe
// output c:\abc
bool PathName2Path(char* szPathName,char Path[MAX_PATH])
{
	memset(Path,0,MAX_PATH);

	size_t Length = strlen(szPathName);
	if(!Length) return false;		

	for(size_t i=0;i<Length;++i)
	{
		if(szPathName[Length-1-i] == '\\')
		{
			memcpy(Path,szPathName,Length-i-1);
			return true;
		}
	}
	return false;
}

bool PathName2Path(TCHAR* wszPathName,TCHAR Path[MAX_PATH])
{
	memset(Path,0,MAX_PATH*sizeof(TCHAR));

	size_t Length = wcslen(wszPathName);
	if(!Length) return false;	

	for(size_t i=0;i<Length;++i)
	{
		if(wszPathName[Length-1-i] == '\\')
		{
			memcpy(Path,wszPathName,(Length-i-1)*sizeof(TCHAR));
			return true;
		}
	}
	return false;
}

void GetSessionTypeString(char SessionType[256],enum UCC_SESSION_TYPE enSessionType)
{
	switch(enSessionType)
	{
	case UCCST_INSTANT_MESSAGING:
		strcpy_s(SessionType,256,"UCCST_INSTANT_MESSAGING");
		break;
	case UCCST_AUDIO_VIDEO:
		strcpy_s(SessionType,256,"UCCST_AUDIO_VIDEO");
		break;
	case UCCST_CONFERENCE:
		strcpy_s(SessionType,256,"UCCST_CONFERENCE");
		break;
	case UCCST_APPLICATION:
		strcpy_s(SessionType,256,"UCCST_APPLICATION");
		break;
	default:
		strcpy_s(SessionType,256,"UNKNOWN");
    	//__T()
		break;
	}
	return;
}