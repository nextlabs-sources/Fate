#include "utilities.h"
#include "nlconfig.hpp"
#include "MenuItem.h"

#include <Windows.h>
#include <string>
#include <vector>

#include <atlbase.h>

using namespace std;


#pragma warning(push)
#pragma warning(disable:6334 6011 4996 4244 4512)
#include "boost\unordered_map.hpp"
#include "boost\shared_ptr.hpp"
#include "boost\weak_ptr.hpp"
#include "boost\algorithm\string.hpp"
#include <boost/thread.hpp>  
#pragma warning(pop)

#define CELOG_CUR_MODULE L"ADOBEPEP"
#undef CELOG_CUR_FILE
#define CELOG_CUR_FILE   CELOG_FILEPATH_PROD_PEP_ENDPOINT_ADOBEPEP_PLUGIN_SAMPLESADOBEPEP_SRC_UTILITIES_CPP

extern BOOL bInIExplore;

std::wstring GetCommonComponentsDir()
{
	wchar_t szDir[MAX_PATH + 1] = {0};
	if(NLConfig::ReadKey(L"SOFTWARE\\NextLabs\\CommonLibraries\\InstallDir", szDir, MAX_PATH))
	{
#ifdef _M_IX86
		wcsncat_s(szDir, MAX_PATH, L"\\bin32", _TRUNCATE);
#else
		wcsncat_s(szDir, MAX_PATH, L"\\bin64", _TRUNCATE);
#endif
		return szDir;
	}
	return L"";
}

std::wstring GetEnforceBinDir()
{
	wchar_t szDir[MAX_PATH + 1] = {0};
	if(NLConfig::ReadKey(L"SOFTWARE\\NextLabs\\Compliant Enterprise\\Desktop Enforcer\\InstallDir", szDir, MAX_PATH))
	{
		wcsncat_s(szDir, MAX_PATH, L"\\bin", _TRUNCATE);
		return szDir;
	}
	return L"";
}

string FormatPath(const string& path)
{
	string strRet = path;
	CELOG_LOGA(CELOG_DEBUG, "start to FormatPath, %s\n", strRet.c_str());

	if (true==boost::algorithm::istarts_with(path,"file:///"))
	{
		strRet = strRet.substr(8);//8 is the length of "file:///"
		boost::algorithm::replace_all(strRet,(string)"|",(string)":");
	}

	if (boost::algorithm::istarts_with(strRet, "http://") || boost::algorithm::istarts_with(strRet, "https://")
		|| (bInIExplore && (boost::algorithm::istarts_with(strRet, "//") || boost::algorithm::istarts_with(strRet, "\\\\")))
		)
	{
		string buf;
		if (url_decode(strRet, buf))
		{
			strRet = buf;
		}
	}

	CELOG_LOGA(CELOG_DEBUG, "end FormatPath, %s\n", strRet.c_str());

	if (!boost::algorithm::istarts_with(strRet, "http"))
	{
		boost::algorithm::replace_all(strRet,L"/",L"\\");
	}
	
	return strRet;
}

bool istempfile(const char* file)
{
	char buffer[1024];
	GetTempPathA(1024, buffer);
	if (0!=strstr(file, buffer))
	{
		return true;
	}
	if (boost::algorithm::istarts_with(file,"Acro"))
	{
		return true;
	}
	return false;
}
void GetPathFromASFile(ASFile handle, string& outpath)
{
	if (!handle)
	{
		return;
	}

	ASFileSys sys = ASFileGetFileSys(handle);
	ASPathName pathname = ASFileAcquirePathName(handle);

	if (!sys||!pathname)
	{
		return;
	}

	char* out_path = ASFileSysDisplayStringFromPath(sys, pathname);
	ASFileSysReleasePath(sys, pathname);
	if (out_path)
	{
		outpath=(string)out_path;

		ASfree(out_path);
	}
}
void GetPathfromPDDoc(PDDoc doc, string& outfile)
{
	if (!doc)
	{
		return;
	}

	ASFile file = PDDocGetFile(doc);
	
	if (!file)
	{
		return;
	}

	ASFileSys sys = ASFileGetFileSys(file);
	ASPathName pathname = ASFileAcquirePathName(file);

	if (!sys||!pathname)
	{
		return;
	}

	char* out_path = ASFileSysDisplayStringFromPath(sys, pathname);
	ASFileSysReleasePath(sys, pathname);
	if (out_path)
	{
		outfile=(string)out_path;
	}
	else
	{
        // In portfolia, ASFileGetURL maybe crash.
        try
        {
		    out_path = ASFileGetURL(file);
        }
        catch (...)
        {
        }
        
		if (out_path)
		{
			outfile=(string)out_path;
		}
		else
		{
			CELOG_LOG(CELOG_DEBUG, L"GetPathfromPDDoc get no file path\n");
		}
	}

	if (out_path != NULL)
	{
		ASfree(out_path);
		out_path = NULL;
	}

	outfile = FormatPath(outfile);
	
}

bool getCurrentPDFPath(string& output_file)
{
	//取得当前的pdf路径
	AVDoc activeAVDoc = AVAppGetActiveDoc();

	if (!activeAVDoc)
	{
		CELOG_LOG(CELOG_DEBUG, L"current pdf pointer is null\n");
		return false;
	}


	PDDoc activePDDoc = AVDocGetPDDoc(activeAVDoc);
	ASFile file = PDDocGetFile(activePDDoc);
	ASFileSys sys = ASFileGetFileSys(file);
	ASPathName pathname = ASFileAcquirePathName(file);
	char* current_pdf = ASFileSysDisplayStringFromPath(sys, pathname);
	ASFileSysReleasePath(sys, pathname);
	if (current_pdf)
	{
		CELOG_LOGA(CELOG_DEBUG, "current pdf is: %s\n", current_pdf);
		output_file = (string)current_pdf;
	}
	else
	{
        // In portfolia, ASFileGetURL maybe crash.
        try
        {
		    current_pdf = ASFileGetURL(file);
        }
        catch (...)
        {
        }
        
		if (current_pdf)
		{
			CELOG_LOGA(CELOG_DEBUG, "current pdf url is: %s\n", current_pdf);

			output_file = (string)current_pdf;

			//file:///C|/Users/bsong/AppData/Local/Temp/A9R18C1.tmp/2.pdf
			output_file = FormatPath(output_file);
		}
		else
		{
			CELOG_LOG(CELOG_DEBUG, L"current pdf pointer is null\n");
			return false;
		}
	}

	if(current_pdf != NULL)
	{
		ASfree(current_pdf);
		current_pdf = NULL;
	}

	return true;
}
const char* get_attachment_name(PDFileAttachment attachment)
{
	//输出attachment的名字	
	ASText text = PDFileAttachmentGetFileName(attachment);
	const char* char_text = ASTextGetEncoded(text, NULL);
	if (char_text)
	{
		//可以得到attachment的名字，就是pdf里显示的名字，没有路径
		CELOG_LOGA(CELOG_DEBUG, "attachment name: %s\n", char_text);
	}
	else
	{
		CELOG_LOG(CELOG_DEBUG, L"can't get attachment name\n");
	}
	return char_text;
}

void getCurrentPDDoc(PDDoc& pddoc)
{
	pddoc=NULL;
	AVDoc activeAVDoc = AVAppGetActiveDoc();
	if (!activeAVDoc)
	{
		CELOG_LOG(CELOG_DEBUG, L"current pdf pointer is null\n");
	}
	pddoc = AVDocGetPDDoc(activeAVDoc);
}

bool IsMulti_BasedOnPages_Format(const string& strFormat)
{
	if (".jpg"==strFormat || ".jpf"==strFormat || ".tif"==strFormat || ".tiff"==strFormat || ".png"==strFormat || ".eps"==strFormat)
	{
		CELOG_LOG(CELOG_DEBUG, L"this is Multi_BasedOnPages format\n");
		return true;
	}

	return false;
}

bool IsOfficeFormat(const string& strFormat)
{
	if (".doc"==strFormat || ".docx"==strFormat || ".xlsx"==strFormat)
	{
		CELOG_LOG(CELOG_DEBUG, L"this is office format\n");
		return true;
	}

	return false;
}

bool IsAutoSaveFile(const string& strFile)
{
	//the auto save file is file like "C:\Users\bsong\AppData\Roaming\Adobe\Acrobat\10.0\AutoSave\1.tmp"
	if (false==boost::algorithm::iends_with(strFile,".tmp"))
	{
		return false;
	}
	if (string::npos==strFile.find("Adobe"))
	{
		return false;
	}
	if (string::npos==strFile.find("AutoSave"))
	{
		return false;
	}

	CELOG_LOGA(CELOG_DEBUG, "below is adobe auto save file: %s\n", strFile.c_str());
	return true;
}

//is this file path a local file path or UNC file path
//false means not local path, like UNC path
#define MAX_PATHLEN 256
#pragma comment(lib, "mpr.lib")
bool IsLocalPath(LPCWSTR pszFileName)
{
	if (pszFileName == NULL)
	{
		return false;
	}

	bool bRet = false;
	if (wcslen(pszFileName) >= 3 && pszFileName[1] == ':')
	{
		wchar_t drive[3] = {0};
		wcsncpy_s(drive, 3, pszFileName, _TRUNCATE);

		if (GetDriveTypeW(drive) == DRIVE_FIXED)
		{
			bRet = true;
		}
	}

	CELOG_LOG(CELOG_DEBUG, L"IsLocalFile? ret: %d, path: %s\n", bRet, pszFileName);

	return bRet;
};

std::string MyWideCharToMultipleByte(const std::wstring & strValue)
{
	int nLen = WideCharToMultiByte(CP_ACP, 0, strValue.c_str(), (int)strValue.length(), NULL, 0, NULL, NULL); 

	char* pBuf = new char[nLen + 1];
	if(!pBuf)
		return "";
	memset(pBuf, 0, nLen +1);
	nLen = WideCharToMultiByte(CP_ACP, 0, strValue.c_str(), (int)strValue.length(), pBuf, nLen, NULL, NULL); 

	std::string strResult(pBuf);

	if(pBuf)
	{
		delete []pBuf;
		pBuf = NULL;
	}
	return strResult;
}

std::wstring MyMultipleByteToWideChar(const std::string & strValue)
{
	int nLen = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, strValue.c_str(), (int)strValue.length(), NULL, 0 );
	DWORD dError = 0 ;
	if( nLen == 0 )
	{
		dError = ::GetLastError() ;
		if( dError == ERROR_INVALID_FLAGS )
		{
			nLen = MultiByteToWideChar(CP_ACP, 0, strValue.c_str(), (int)strValue.length(), NULL, 0 );
		}
	}
	wchar_t* pBuf = new wchar_t[nLen + 1];
	if(!pBuf)
		return L"";

	memset(pBuf, 0, (nLen + 1) * sizeof(wchar_t));
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, strValue.c_str(), (int)strValue.length(), pBuf, nLen);
	dError = ::GetLastError() ;
	if( dError == ERROR_INVALID_FLAGS )
	{
		MultiByteToWideChar(CP_ACP, 0, strValue.c_str(), (int)strValue.length(), pBuf, nLen);
	}
	std::wstring strResult(pBuf);

	if(pBuf)
	{
		delete []pBuf;
		pBuf = NULL;
	}
	return strResult;
}

bool IsURLPath(const string& path)
{
	if (false==boost::algorithm::istarts_with(path,"http"))
	{
		return false;
	}

	return true;
}
bool IsAcrobatcom(const string& path)
{
	return boost::algorithm::istarts_with(path,"Acrobat.com:");
}
bool IsIETempFolder(const string& path)
{
	return boost::algorithm::icontains(path,"\\AppData\\Local\\Microsoft\\Windows\\Temporary Internet Files\\Content.IE");
}

string GetPath(ASFileSys fileSys, ASPathName pathName)
{
	char* path = ASFileSysDisplayStringFromPath(fileSys, pathName);

	if (!path)
	{
        // In portfolia, ASFileSysURLFromPath maybe crash.
        try
        {
		    path = ASFileSysURLFromPath(fileSys, pathName);
        }
        catch (...)
        {
        }
	}

	if (path)
	{
		CELOG_LOGA(CELOG_DEBUG, "GetPath() returns: %s\n", path);

		string ret(path);

		ASfree(path);

		ret = FormatPath(ret);

		return ret;
	}

	return "";
}


bool url_decode(const std::string& in, std::string& out)
{
	out.clear();
	out.reserve(in.size());
	for (std::size_t i = 0; i < in.size(); ++i)
	{
		if (in[i] == '%')
		{
			if (i + 3 <= in.size())
			{
				int value = 0;
				std::istringstream is(in.substr(i + 1, 2));
				if (is >> std::hex >> value)
				{
					out += static_cast<char>(value);
					i += 2;
				}
				else
				{
					return false;
				}
			}
			else
			{
				return false;
			}
		}
		else if (in[i] == '+')
		{
			out += ' ';
		}
		else
		{
			out += in[i];
		}
	}
	return true;
}

BOOL isContentData(IDataObject *pDataObject)
{
	BOOL bContent = FALSE;

	//Enumerate clipboard data
	CComPtr<IEnumFORMATETC> pIEnumFORMATETC = NULL;
	HRESULT hr = pDataObject->EnumFormatEtc(DATADIR_GET, &pIEnumFORMATETC);
	if(FAILED(hr) || NULL == pIEnumFORMATETC) 
	{
		return bContent;
	}

	FORMATETC etc;
	while (1) 
	{
		memset(&etc, 0, sizeof(etc));
		if(S_FALSE == pIEnumFORMATETC->Next(1, &etc, NULL))
		{
			break;
		}

		// Standard
		if(CF_TEXT == etc.cfFormat) 
		{
			bContent = TRUE;
		} 
		else if(CF_BITMAP == etc.cfFormat) 
		{
			bContent = TRUE;
		} 
		else if(CF_OEMTEXT == etc.cfFormat) 
		{
			bContent = TRUE;
		} 
		else if(CF_UNICODETEXT == etc.cfFormat) 
		{
			bContent = TRUE;
		} 
		else 
		{
			WCHAR szFormat[256] = {0};
			GetClipboardFormatNameW(etc.cfFormat, szFormat, 256);
			if(wcscmp(szFormat, L"FileGroupDescriptor" ) == 0) 
			{
				//this email attachment
				bContent = FALSE;
				break;
			}
		} 
	}

	pIEnumFORMATETC.Release();

	return bContent;
}
