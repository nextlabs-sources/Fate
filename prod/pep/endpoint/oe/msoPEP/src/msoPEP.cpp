
#pragma once
// msoPEP.cpp : Implementation of DLL Exports.


#include "stdafx.h"
#include <Psapi.h>
#include "resource.h"
#include "msoPEP.h"
#include "../outlook/outlookObj.h"
#include "../common/Hook.h"
#include "PALoad.h"
#include "nlconfig.hpp"
#include "../common/log.h"
#include "../outlook/outlookUtilities.h"
#include "ProductVersions.h"
COutlookObj*    g_pOutlookObj = NULL;
HINSTANCE       g_hInstance   = NULL;
HFONT           g_fntNormal   = NULL;
HFONT           g_fntBold     = NULL;
_ext_AppType    g_eAppType    = apptUnknown;

static _ext_AppType GetAppType();
static HRESULT RegisterDll(LPCWSTR AppName);
static HRESULT UnRegisterDll(LPCWSTR AppName);
CRITICAL_SECTION g_csPolicyInstance;


CreateAttributeManagerType glfCreateAttributeManager = NULL;
AllocAttributesType glfAllocAttributes = NULL;
ReadResourceAttributesWType glfReadResourceAttributesW = NULL;
GetAttributeCountType glfGetAttributeCount = NULL;
FreeAttributesType glfFreeAttributes = NULL;
CloseAttributeManagerType glfCloseAttributeManager = NULL;
AddAttributeWType glfAddAttributeW = NULL;
GetAttributeNameType glfGetAttributeName = NULL;
GetAttributeValueType glfGetAttributeValue = NULL;
WriteResourceAttributesWType glfWriteResourceAttributesW = NULL;
IsNxlFormatType glfIsNxlFormat = NULL;
ReadResrcSummaryAttrType glfReadResrcSummaryAttr = NULL;

class CmsoPEPModule : public CAtlDllModuleT< CmsoPEPModule >
{
public :
	DECLARE_LIBID(LIBID_msoPEPLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_MSOPEP, "{B73862C3-CABA-4046-90EA-E8B98FA641F0}")

    BOOL CreateSubObj();
};

CmsoPEPModule _AtlModule;


#ifdef _MANAGED
#pragma managed(push, off)
#endif
HINSTANCE	g_hODHD;
/*
added by chellee for Encryption & Tagging
*/
HINSTANCE	g_hTAG ;
HINSTANCE	g_hENC ;
HINSTANCE	g_hPE ;		//Portable encryption
void CreateCustomFont()
{
    //Gdiplus::GdiplusStartup(&g_pGdiToken, &g_gdiplusStartupInput, NULL);
    g_fntNormal = CreateFontW(14,
        0,
        0,
        0,
        550,
        0,
        0,
        0,
        DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, //CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE,
        L"Arial"
        );
    g_fntBold = CreateFontW(14,
        0,
        0,
        0,
        650,
        0,
        0,
        0,
        DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, //CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE,
        L"Arial"
        );
}
void ReleaseCustomFont()
{
    //Gdiplus::GdiplusShutdown(g_pGdiToken);
    if(g_fntNormal) DeleteObject(g_fntNormal); g_fntNormal=NULL;
    if(g_fntBold) DeleteObject(g_fntBold); g_fntBold=NULL;
}


static std::wstring GetCommonComponentsDir()
{
	wchar_t szDir[MAX_PATH] = {0};
	if(NLConfig::ReadKey(L"SOFTWARE\\NextLabs\\CommonLibraries\\InstallDir", szDir, MAX_PATH))
	{
		bool bEndWithSep = szDir[wcslen(szDir) - 1] == L'\\';
#ifdef _WIN64
		wcsncat_s(szDir, MAX_PATH, bEndWithSep ? L"bin64\\" : L"\\bin64\\", _TRUNCATE);
#else
		wcsncat_s(szDir, MAX_PATH, bEndWithSep ? L"bin32\\" : L"\\bin32\\", _TRUNCATE);
#endif
		return szDir;
	}

	return L"";
}

static bool InitResattr()
{
	static bool bInit = false;
	if(!bInit)
	{
		std::wstring strCommonPath = GetCommonComponentsDir();
		if(strCommonPath.empty())
		{
			DP((L"can't find common libraries path, check if you have installed common libraries please.\r\n"));
			return false;
		}
		else
		{
			DP(( L"Common library path: %s\r\n", strCommonPath.c_str()));
			
#ifdef _WIN64
			std::wstring strLib = strCommonPath + L"\\resattrlib.dll";
			std::wstring strMgr = strCommonPath + L"\\resattrmgr.dll";
			std::wstring strCeLog = strCommonPath + L"\\celog.dll";
			std::wstring strCeLog2 = strCommonPath + L"\\celog2.dll";
			std::wstring strSE = strCommonPath + L"\\NL_SYSENC_LIB.dll";
			std::wstring strLibTiff = strCommonPath + L"\\libtiff.dll";
#else
			std::wstring strLib = strCommonPath + L"\\resattrlib32.dll";
			std::wstring strMgr = strCommonPath + L"\\resattrmgr32.dll";
			std::wstring strCeLog = strCommonPath + L"\\celog32.dll";
			std::wstring strCeLog2 = strCommonPath + L"\\celog232.dll";
			std::wstring strSE = strCommonPath + L"\\NL_SYSENC_LIB32.dll";
			std::wstring strLibTiff = strCommonPath + L"\\libtiff32.dll";
#endif

			/*
			we can't call SetDllDirectoryW, otherwise it will cause bug14142.
			I don't know why, but it's a fact, once we call SetDllDirectoryW, 14142 happens.
			in order to fix this problem, we use a work around, we just load all dependencies of resattrlib/resattrmgr
			first, and then try to load resattrlib/resattrmgr
			*/
			LoadLibraryW(strCeLog.c_str());
			LoadLibraryW(strCeLog2.c_str());
			LoadLibraryW(strSE.c_str());
			LoadLibraryW(strLibTiff.c_str());
			HMODULE hModLib = (HMODULE)LoadLibraryW(strLib.c_str());
			HMODULE hModMgr = (HMODULE)LoadLibraryW(strMgr.c_str());

			if( !hModLib || !hModMgr)
			{
				DP(( L"Can't load %s, %s\r\n", strLib.c_str(), strMgr.c_str()));
				return false;
			}

			glfCreateAttributeManager = (CreateAttributeManagerType)GetProcAddress(hModMgr, "CreateAttributeManager");
			glfAllocAttributes = (AllocAttributesType)GetProcAddress(hModLib, "AllocAttributes");
			glfReadResourceAttributesW = (ReadResourceAttributesWType)GetProcAddress(hModMgr, "ReadResourceAttributesW");
			glfGetAttributeCount = (GetAttributeCountType)GetProcAddress(hModLib, "GetAttributeCount");
			glfFreeAttributes = (FreeAttributesType)GetProcAddress(hModLib, "FreeAttributes");
			glfCloseAttributeManager = (CloseAttributeManagerType)GetProcAddress(hModMgr, "CloseAttributeManager");
			glfAddAttributeW = (AddAttributeWType)GetProcAddress(hModLib, "AddAttributeW");
			glfGetAttributeName = (GetAttributeNameType)GetProcAddress(hModLib, "GetAttributeName");
			glfGetAttributeValue = (GetAttributeValueType)GetProcAddress(hModLib, "GetAttributeValue");
			glfWriteResourceAttributesW = (WriteResourceAttributesWType)GetProcAddress(hModMgr, "WriteResourceAttributesW");
			glfIsNxlFormat = (IsNxlFormatType)GetProcAddress(hModMgr, "IsNxlFormat");
			glfReadResrcSummaryAttr = (ReadResrcSummaryAttrType)GetProcAddress(hModMgr, "ReadResrcSummaryAttr");

			if( !(glfCreateAttributeManager && glfAllocAttributes &&
				glfReadResourceAttributesW && glfGetAttributeCount &&
				glfFreeAttributes && glfCloseAttributeManager && glfAddAttributeW &&
				glfGetAttributeName && glfGetAttributeValue &&
				glfWriteResourceAttributesW && glfIsNxlFormat&&glfReadResrcSummaryAttr) )
			{
				DP((L"failed to get resattrlib/resattrmgr functions\r\n"));
				return false;
			}

			bInit = true;
			return true;
		}	
	}

	return true;
}

std::string hex_to_string(const std::string& input)
{
	static const char* const lut = "0123456789ABCDEF";
	size_t len = input.length();
	if (len & 1) throw std::invalid_argument("odd length");
	std::string output;
	output.reserve(len / 2);
	for (size_t i = 0; i < len; i += 2)
	{
		char a = input[i];
		const char* p = std::lower_bound(lut, lut + 16, a);
		if (*p != a) throw std::invalid_argument("not a hex digit");
		char b = input[i + 1];
		const char* q = std::lower_bound(lut, lut + 16, b);
		if (*q != b) throw std::invalid_argument("not a hex digit");
		output.push_back((char)(((p - lut) << 4) | (q - lut)));
	}
	return output;
}

bool CheckCharIsHex(char ch)
{
	if (ch >= '0' && ch <= '9')
	{
		return true;
	}
	if (ch >= 'A' && ch <= 'F')
	{
		return true;
	}
	return false;
}

bool CheckHexEncode(_In_ const string& strBuf,_Out_ string& strValue)
{
	strValue = "";
	if (strBuf.length() < 6)
	{
		return false;
	}

	string strHead = strBuf.substr(0,4);
	if (strcmp(strHead.c_str(),"FEFF")  == 0)
	{
		string strTemp = strBuf.substr(4);
		int buflen = (int)strTemp.length();
		int number = buflen/4;
		if ((buflen+ 3)/4 == number)
		{
			int j = 0;
			for (int i = 0; i < number; i++)
			{
				if (strTemp[j] == '0'&&strTemp[j + 1] == '0'&& CheckCharIsHex(strTemp[j+2])&&strTemp[j + 3])
				{
					string strTempValue = strTemp.substr(j + 2,2);
					strValue += strTempValue;
				}
				else
				{
					return false;
				}
				j = (i+1)*4;
			}
			return true;
		}

	}

	return false;
}

bool GetTagValueFromTagName(wstring &strTagName, wstring &strTagValue)
{
	wstring strTemp = L"";
	size_t nPosStart = strTagName.find(L"<");
	if (nPosStart != wstring::npos)
	{
		size_t nPosEnd = strTagName.find(L">");
		if (nPosEnd != wstring::npos && nPosEnd>nPosStart)
		{
			strTemp = strTagName.substr(0,nPosStart);
			if (!strTemp.empty())
			{
				strTagValue = strTagName.substr(nPosStart+1, nPosEnd-nPosStart-1);
				boost::algorithm::replace_all(strTemp,L" ",L"");
				strTagName = strTemp;
				return true;
			}
		}
	}
	return false;
}

wstring GetRealTagValue(wstring& wstrTemp,bool &bIsHexDecode, wstring& wstrHexToUn)
{
	wstring strRealTagValue = L"";
	bIsHexDecode = false;
	wstrHexToUn = L"";
	if (wstrTemp.empty())
	{
		return strRealTagValue;
	}
	
	wstring wstr = L"";
	wstring wstrHeader = wstrTemp.substr(0,1);
	wstring strTail = wstrTemp.substr(wstrTemp.length()-1);
	if (_wcsicmp(wstrHeader.c_str(),L"<")==0 && _wcsicmp(strTail.c_str(),L">")==0)
	{
		wstr = wstrTemp.substr(1,wstrTemp.length()-2); 
	}
	else
	{
		wstr = wstrTemp;
	}

	string strCovertTemp = OLUtilities::MyWideCharToMultipleByte(wstr);

	string strValue = "";
	string strHexToUn ;
	bIsHexDecode = CheckHexEncode(strCovertTemp,strValue);
	if (bIsHexDecode)
	{
		strHexToUn = hex_to_string(strValue);
		wstrHexToUn = OLUtilities::MyMultipleByteToWideChar(strHexToUn);

		strRealTagValue = L"\"";
		strRealTagValue += wstrHexToUn;
		strRealTagValue += L"\"";						
	}
	else
	{
		strRealTagValue = wstrTemp;
	}
	return strRealTagValue;
}

void ReadTag(const wstring&file, vector<pair<wstring,wstring>>& tags )
{

	if(!PathFileExistsW(file.c_str()))
	{
		return;
	}
	ResourceAttributeManager *mgr = NULL;
	glfCreateAttributeManager(&mgr);
	if(!mgr)
	{
		return;
	}
	ResourceAttributes *attrs;
	glfAllocAttributes(&attrs);
	if(!attrs)
	{
		glfCloseAttributeManager(mgr);
		return;
	}

	int nRet = 0;
	
	nRet = glfReadResourceAttributesW(mgr, file.c_str(), attrs);
	

	if(!nRet)
	{
		glfFreeAttributes(attrs);
		glfCloseAttributeManager(mgr);
		return;
	}
	else
	{
		int size = glfGetAttributeCount(attrs);
		if (size == 0)
		{
			glfFreeAttributes(attrs);
			glfCloseAttributeManager(mgr);
			return;
		}
		else
		{
			BOOL bIsPDFFile = FALSE;
			bIsPDFFile = OLUtilities::IsPDFFile(file.c_str());
			
			for (int i = 0; i < size; ++i)
			{	
				wstring tagname = (WCHAR *)glfGetAttributeName(attrs, i);
				wstring tagvalue = L"";
				wstring wstrHexToUn = L"";
				bool bIsHexDecode = false;
				bool bIsContainBracket = false;
			
				
				if (bIsPDFFile)
				{

					wstring wstrTemp = (WCHAR *)glfGetAttributeValue(attrs, i);
					if (wstrTemp.empty())
					{
						bool bGetValue =  GetTagValueFromTagName(tagname,wstrTemp);
						if (!bGetValue||wstrTemp.empty())
						{
							continue;
						}
					}

					tagvalue = GetRealTagValue(wstrTemp,bIsHexDecode,wstrHexToUn);
					
				}
				else
				{
					tagvalue = (WCHAR *)glfGetAttributeValue(attrs, i);
				}
				
				if (!tagvalue.empty())
				{
					tags.push_back(pair<wstring,wstring>(tagname,tagvalue));
					if (bIsHexDecode)
					{
						tags.push_back(pair<wstring,wstring>(tagname,wstrHexToUn));
					}
				}

				if (bIsPDFFile)
				{
					tagvalue = L"";
					bIsContainBracket = GetTagValueFromTagName(tagname,tagvalue);
					if (bIsContainBracket && !tagvalue.empty())
					{
						tagvalue = GetRealTagValue(tagvalue,bIsHexDecode,wstrHexToUn);
						if (!tagvalue.empty())
						{
							tags.push_back(pair<wstring,wstring>(tagname,tagvalue));
							if (bIsHexDecode)
							{
								tags.push_back(pair<wstring,wstring>(tagname,wstrHexToUn));
							}
						}
					}
				} 
			}
		}
	}

	glfFreeAttributes(attrs);
	glfCloseAttributeManager(mgr);

	return;
}


void ReadSummaryTag(const wstring&file, vector<pair<wstring,wstring>>& tags )
{

	if(!PathFileExistsW(file.c_str()))
	{
		return;
	}
	ResourceAttributeManager *mgr = NULL;
	glfCreateAttributeManager(&mgr);
	if(!mgr)
	{
		return;
	}
	ResourceAttributes *attrs;
	glfAllocAttributes(&attrs);
	if(!attrs)
	{
		glfCloseAttributeManager(mgr);
		return;
	}

	int nRet = 0;

	nRet = glfReadResrcSummaryAttr(mgr, file.c_str(), attrs);


	if(!nRet)
	{
		glfFreeAttributes(attrs);
		glfCloseAttributeManager(mgr);
		return;
	}
	else
	{
		int size = glfGetAttributeCount(attrs);
		if (size == 0)
		{
			glfFreeAttributes(attrs);
			glfCloseAttributeManager(mgr);
			return;
		}
		else
		{
			for (int i = 0; i < size; ++i)
			{	
				wstring tagname = (WCHAR *)glfGetAttributeName(attrs, i);
				wstring tagvalue = (WCHAR *)glfGetAttributeValue(attrs, i);
				if (!tagvalue.empty())
				{
					tags.push_back(pair<wstring,wstring>(tagname,tagvalue));
				}
			}
		}
	}

	glfFreeAttributes(attrs);
	glfCloseAttributeManager(mgr);

	return;
}

bool IsNxlFormat(const WCHAR * filename)
{
	if (filename == NULL)
	{
		return false;
	}
	return glfIsNxlFormat(filename);
}

// DLL Entry Point
extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	DWORD lenBaseName=0;
	hInstance;
    g_hInstance = hInstance;

    switch(dwReason)
    {
    case DLL_PROCESS_ATTACH:
        {
            DP((L"DllMain::DLL_PROCESS_ATTACH -> Start!\n"));
            ::DisableThreadLibraryCalls(hInstance);
            CCommonLog::Initialize();
			InitializeCriticalSection(&g_csPolicyInstance);

            CreateCustomFont();

            WCHAR     DllPath[MAX_PATH+1]; memset(DllPath, 0, sizeof(DllPath));  
            lenBaseName=GetModuleFileName((HMODULE)hInstance, DllPath, MAX_PATH);   
            (_tcsrchr(DllPath,'\\'))[1] = 0; 
            std::wstring baseName(DllPath);

	#ifdef WSO2K7
		#ifdef _WIN64
					std::wstring fileNameODHD=baseName+std::wstring(L"odhd2k7.dll");
		#else
					std::wstring fileNameODHD=baseName+std::wstring(L"odhd2k732.dll");
		#endif
	#endif
	#ifdef WSO2K10
			#ifdef _WIN64
						std::wstring fileNameODHD=baseName+std::wstring(L"odhd2010.dll");
			#else
						std::wstring fileNameODHD=baseName+std::wstring(L"odhd201032.dll");
			#endif
    #endif
	#ifdef WSO2K13
		   #ifdef _WIN64
						std::wstring fileNameODHD=baseName+std::wstring(L"odhd2013.dll");
		   #else
						std::wstring fileNameODHD=baseName+std::wstring(L"odhd201332.dll");
		   #endif
	#endif
	#ifdef WSO2K16
		#ifdef _WIN64
						std::wstring fileNameODHD=baseName+std::wstring(L"odhd2016.dll");
		#else
						std::wstring fileNameODHD=baseName+std::wstring(L"odhd201632.dll");
		#endif
	#endif


			g_hODHD=LoadLibrary(fileNameODHD.c_str());
			if(g_hODHD==NULL)
			{
				DP((L"DllMain::Can't find %s!\n", fileNameODHD.c_str()));
			}
			/*
			added by chellee for encryption and tagging
			*/
			//-------------------------------------------------------------------------
			baseName = OLUtilities::GetCommonComponentsDir() ;
			if( PA_LOAD::LoadModuleByName(	PA_LOAD::PA_MODULE_NAME_ENC, PA_LOAD::PA_ENCRYPTION,	g_hENC, baseName.c_str())  == FALSE )
			{

				DP((L"DllMain::Can't find %s!\n", PA_LOAD::PA_MODULE_NAME_ENC));
			}
			if( PA_LOAD::LoadModuleByName(	PA_LOAD::PA_MODULE_NAME_TAG, PA_LOAD::PA_FILETAGGING,	g_hTAG, baseName.c_str() )  == FALSE )
			{

				DP((L"DllMain::Can't find %s!\n", PA_LOAD::PA_MODULE_NAME_TAG));
			}
			if( PA_LOAD::LoadModuleByName(	PA_LOAD::PA_MODULE_NAME_PE, PA_LOAD::PA_FILETAGGING,	g_hPE, baseName.c_str() )  == FALSE )
			{

				DP((L"DllMain::Can't find %s!\n", PA_LOAD::PA_MODULE_NAME_TAG));
			}

			//-------------------------------------------------------------------------
			if(InitResattr()==false)
				return FALSE;

			g_eAppType = GetAppType();

			
            //if(apptUnknown == g_eAppType) return FALSE;
            switch(g_eAppType)
			{
			case apptOutlook:
				{
					DPW((L"It is the outlook object")) ;
					g_pOutlookObj = new COutlookObj();
					if(NULL == g_pOutlookObj) 
					{
						DPW((L"Load error")) ;
						return FALSE;
					}

					StartStopHook(true);

				}
				break;
			case apptWord:
            case apptExcel:
            case apptPPT:
            default:
                break;
            }
            break;
        }
    case DLL_PROCESS_DETACH:
		DP((L"DllMain::DLL_PROCESS_DETACH -> Start!\n"));
		DeleteCriticalSection(&g_csPolicyInstance);
        ReleaseCustomFont();
		FreeLibrary(g_hODHD);
		//added by chellee for tagging and encryption
		if( g_hENC )
		{
			FreeLibrary(g_hENC);
			g_hENC = NULL ;
		}
		if(g_hPE)
		{
			FreeLibrary(g_hPE);
			g_hPE = NULL ;
		}
		/*if( g_hTAG)
		{
			FreeLibrary(g_hTAG);
			g_hTAG = NULL ;
		}*/
		
        StartStopHook(false);
        switch(g_eAppType)
        {
        case apptOutlook:
            if(NULL != g_pOutlookObj) delete g_pOutlookObj;
            g_pOutlookObj = NULL;
            break;
        case apptWord:
        case apptExcel:
        case apptPPT:
        default:
            break;
        }
		DP((L"DllMain::DLL_PROCESS_DETACH -> End!\n"));
        break;
    }

    return _AtlModule.DllMain(dwReason, lpReserved); 
}

#ifdef _MANAGED
#pragma managed(pop)
#endif

_ext_AppType GetAppType()
{
    WCHAR wzCurrentApp[MAX_PATH+1];   memset(wzCurrentApp, 0, sizeof(wzCurrentApp));
    ::GetModuleFileNameW(NULL, wzCurrentApp, MAX_PATH);
    WCHAR* pFileName = wcsrchr(wzCurrentApp, L'\\');
    if(NULL == pFileName) return apptUnknown;
	DP((pFileName));

    if(0 == _wcsnicmp(pFileName, L"\\WINWORD.EXE",wcslen(pFileName)))
    {
        return apptWord;
    }
    else if(0 == _wcsnicmp(pFileName, L"\\EXCEL.EXE",wcslen(pFileName)))
    {
        return apptExcel;
    }
    else if(0 == _wcsnicmp(pFileName, L"\\POWERPNT.EXE",wcslen(pFileName)))
    {
        return apptPPT;
    }
    else if(0 == _wcsnicmp(pFileName, L"\\OUTLOOK.EXE",wcslen(pFileName)))
    {
        return apptOutlook;
    }
    else
    {
    	DP((L"Unknow"));
        return apptUnknown;
    }

    return apptUnknown;
}

HRESULT RegisterDll(LPCWSTR AppName)
{
    HRESULT hr = S_OK;
    DWORD   dwDisposition = 0;
    HKEY    hKeyOutlook   = NULL;
    HKEY    hKeyAddin     = NULL;
    LONG    lResult       = 0;
    DWORD   dwVal         = 0;
    char    szVal[MAX_PATH];    memset(szVal, 0, sizeof(szVal));
    WCHAR   wzKeyName[MAX_PATH];memset(wzKeyName, 0, sizeof(wzKeyName));

    try
    {
        _snwprintf_s(wzKeyName, MAX_PATH, _TRUNCATE, L"SOFTWARE\\Microsoft\\Office\\%s\\Addins", AppName);
        //lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, wzKeyName, 0, KEY_ALL_ACCESS, &hKeyOutlook);
        lResult = RegCreateKeyEx(HKEY_LOCAL_MACHINE, wzKeyName, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKeyOutlook, NULL);
		if(ERROR_SUCCESS != lResult)     // get office/outlook key
		{
			DP((L"RegisterDll::Fail to open or create key:%s\\addins \n", AppName));
			hr = E_UNEXPECTED;
			throw;
		}
        lResult = RegCreateKeyEx( hKeyOutlook, L"msoPEP.msoObj.1",0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hKeyAddin,&dwDisposition);
        if(ERROR_SUCCESS != lResult)
		{
			DP((L"RegisterDll::Fail to open key: msoPEP.msoObj.1\n"));
            hr = E_UNEXPECTED;
            throw;
        }
        dwVal = 0;
        RegSetValueEx(hKeyAddin, L"CommandLineSafe", 0, REG_DWORD, (const BYTE*)&dwVal, sizeof(DWORD));
        dwVal = 3;
        RegSetValueEx(hKeyAddin, L"LoadBehavior", 0, REG_DWORD, (const BYTE*)&dwVal, sizeof(DWORD));
        _snprintf_s(szVal, MAX_PATH, _TRUNCATE, VERSION_PRODUCT_OE/*"Compliant Enterprise Office PEP"*/);
        RegSetValueExA(hKeyAddin, "FriendlyName", 0, REG_SZ, (const BYTE*)szVal, 1+(DWORD)strlen(szVal));
        RegSetValueExA(hKeyAddin, "Description", 0, REG_SZ, (const BYTE*)szVal, 1+(DWORD)strlen(szVal));
    }
    catch(...)
    {
        if(NULL != hKeyAddin) RegCloseKey(hKeyAddin);
        if(NULL != hKeyOutlook) RegCloseKey(hKeyOutlook);

        hKeyAddin   = NULL;
        hKeyOutlook = NULL;
        return hr;
    }

    return hr;
}

HRESULT UnRegisterDll(LPCWSTR AppName)
{
	WCHAR   wzKeyName[MAX_PATH];memset(wzKeyName, 0, sizeof(wzKeyName));
	_snwprintf_s(wzKeyName, MAX_PATH, _TRUNCATE, L"SOFTWARE\\Microsoft\\Office\\%s\\Addins\\msoPEP.msoObj.1", AppName);
	HRESULT lResult =   RegDeleteKey(HKEY_LOCAL_MACHINE, wzKeyName);
	if(ERROR_SUCCESS != lResult)
	{
		_snwprintf_s(wzKeyName, MAX_PATH, _TRUNCATE, L"SOFTWARE\\Wow6432Node\\Microsoft\\Office\\%s\\Addins\\msoPEP.msoObj.1", AppName);
		lResult =   RegDeleteKey(HKEY_LOCAL_MACHINE, wzKeyName);
	}
  
    return S_OK;
}

// Used to determine whether the DLL can be unloaded by OLE
STDAPI DllCanUnloadNow(void)
{
    return _AtlModule.DllCanUnloadNow();
}


// Returns a class factory to create an object of the requested type
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    return _AtlModule.DllGetClassObject(rclsid, riid, ppv);
}


// DllRegisterServer - Adds entries to the system registry
STDAPI DllRegisterServer(void)
{
    // registers object, typelib and all interfaces in typelib
    HRESULT hr = _AtlModule.DllRegisterServer();
    RegisterDll(L"Outlook");
	return hr;
}


// DllUnregisterServer - Removes entries from the system registry
STDAPI DllUnregisterServer(void)
{
	HRESULT hr = _AtlModule.DllUnregisterServer();
    UnRegisterDll(L"Outlook");
	return hr;
}

