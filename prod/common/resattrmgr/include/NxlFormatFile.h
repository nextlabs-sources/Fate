#ifndef _CNXLFORMATFILE_H_
#define _CNXLFORMATFILE_H_

#pragma warning(disable: 4995)
#include "stdio.h"
#include <windows.h>
#include "strsafe.h"
#include <string>
#include <vector>
#include "resattrlib.h"
using namespace std;


enum NXLERRORVALUE
{
	emSUCCESS,
	emFILE_NOT_EXIST,
	emPOINT_NULL,
	emTAG_NUM_INVALID,
	emNO_TAG_INFO,
	emFILE_OPEN_FAIL,
	emUNKNOWERROR
};


#define  MAX_PATH_LENGTH			  1024
#define  FILE_ATTR_DATA_SIZE_OFFSET   904
#define  FILE_RIGHT_DATA_SIZE_OFFSET  920
#define  FILE_TAG_DATA_SIZE_OFFSET	  936
#define  FILE_TYPE_OFFSET			  2048

class CNxlFormatFile
{
public:
	CNxlFormatFile(void);
	~CNxlFormatFile(void);

	NXLERRORVALUE OpenNXLFile(_In_ const wchar_t *FileName,_Out_ FILE **fp);
	NXLERRORVALUE CloseNXLFile(_In_ wstring strFileName,_In_ FILE *fp);
	bool IsNXLFormatFile(_In_ const wchar_t *FileName);
	NXLERRORVALUE ReadNxlFileTag(_In_ const wchar_t *FileName,_Out_ vector<pair<wstring,wstring>> & vecTagInfo);
	int GetNXLFileProps(const wchar_t* pszFileName, ResourceAttributes *attrs);
private:
	int GetOffSet(_In_ FILE *fp, _In_ int offset);
	NXLERRORVALUE GetTagInfo(_In_ FILE *pf,_In_ int TagOffSet,_Out_ vector<pair<wstring,wstring>> & vecTagInfo);
};

#endif