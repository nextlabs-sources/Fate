#include <string>
#include <ole2.h>
#include "eframework\platform\windows_file_server_enforcer.hpp"
#include "wfse_pc_env_declare.h"
// #include "celog.h"
#include "FileAttributeReaderWriter.h"
#include "oleAttrs.h"
#include <atlcomcli.h>
#include <vector>
#include <strsafe.h>

// for CELog2
#include "NLCELog.h"
#define  CELOG_CUR_FILE static_cast<celog_filepathint_t>(EMNLFILEINTEGER_OLEATTRSCPP)

using namespace std;

#define TAGNAME_MAX_LENGTH			255
#define TAGVALUE_MAX_LENGTH			255

// #define GET_BUILTIN_PROPERTIES



//
// Global variables
//

extern _CommitTransaction MyCommitTransaction;
extern _CopyFileTransactedW MyCopyFileTransactedW;
extern _CreateTransaction MyCreateTransaction;



//
// Internal variables
//

/*FMTID g_fmtid = { // d170df2e-1117-11d2-aa01-00805ffe11b8 
	0xd170df2e,
	0x1117,
	0x11d2,
	{0xaa, 0x01, 0x00, 0x80, 0x5f, 0xfe, 0x11, 0xb8}
};*/

static FMTID g_fmtid = FMTID_UserDefinedProperties; //use the user-defined property

enum PropType
{
	enSumInfo,
	enDocSumInfo
};

#define GET_SUMMERY_PROPERTIES
//
// Internal functions
//

// Dumps simple PROPVARIANT values.
static void GetPropVariant(PROPVARIANT *pPropVar, std::wstring &val) 
{
    WCHAR tmpStr[1024] = { 0 };
    size_t requiredSize = 0;
    // size_t pNumConverted;
    val=L"";

    // Don't iterate arrays, just inform as an array.
    if(pPropVar->vt & VT_ARRAY) {
        //printf("(Array)\n");
        return;
    }

    // Don't handle byref for simplicity, just inform byref.
    if(pPropVar->vt & VT_BYREF) {
        //printf("(ByRef)\n");
        return;
    }

    // Switch types.
    switch(pPropVar->vt) {
        case VT_BOOL:
            //printf("%s (VT_BOOL)\n",pPropVar->boolVal ? "TRUE/YES" : "FALSE/NO");
            val=pPropVar->boolVal ? L"TRUE" : L"FALSE";
            break;
        case VT_I2: // 2-byte signed int.
            _snwprintf_s(tmpStr, 1024, _TRUNCATE, L"%d", (int)pPropVar->iVal);
            val=tmpStr;
            break;
        case VT_I4: // 4-byte signed int.
            _snwprintf_s(tmpStr, 1024, _TRUNCATE, L"%d", (int)pPropVar->lVal);
            val=tmpStr;
            break;
        case VT_R4: // 4-byte real.
            _snwprintf_s(tmpStr, 1024, _TRUNCATE, L"%.2lf", (double)pPropVar->fltVal);
            val=tmpStr;
            break;
        case VT_R8: // 8-byte real.
            _snwprintf_s(tmpStr, 1024, _TRUNCATE, L"%.2lf", (double)pPropVar->dblVal);
            val=tmpStr;
            break;
        case VT_BSTR: // OLE Automation string.
            val=pPropVar->bstrVal;
            break;
        case VT_LPSTR: // Null-terminated string.
            mbstowcs_s(&requiredSize, NULL, 0, pPropVar->pszVal, 0);
            mbstowcs_s(&requiredSize, tmpStr, requiredSize, pPropVar->pszVal, _TRUNCATE);
            val=tmpStr;
            break;
        case VT_LPWSTR:
            val = pPropVar->pwszVal;
            break;
        case VT_FILETIME:
            {
                char *dayPre[] =
                    {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};

        char tmpMbStr[1024] = { 0 };
        FILETIME lft;
        FileTimeToLocalFileTime(&pPropVar->filetime, &lft);
        SYSTEMTIME lst;
        FileTimeToSystemTime(&lft, &lst);

        _snprintf_s(tmpMbStr, 1024, _TRUNCATE, "%02d:%02d.%02d %s, %s %02d/%02d/%d",
            1 + (lst.wHour - 1) % 12, lst.wMinute, lst.wSecond,
            (lst.wHour >= 12) ? "pm" : "am",
            dayPre[lst.wDayOfWeek % 7],
            lst.wMonth, lst.wDay, lst.wYear);
        
        if (mbstowcs_s(&requiredSize, NULL, 0, tmpMbStr, 0) == 0)
        {
            mbstowcs_s(&requiredSize, tmpStr, requiredSize, tmpMbStr, _TRUNCATE);
            val = tmpStr;
        }
    }
        break;
    default: // Unhandled type, consult wtypes.h's VARENUM structure.
        //printf("(Unhandled type: 0x%08lx)\n", pPropVar->vt);
        break;
    }
}

#ifdef GET_BUILTIN_PROPERTIES
// Get built-in properties of a property storage.
static BOOL GetFileBuiltInProps(IPropertySetStorage *pPropSetStg, ResourceAttributes *attrs)
{
    CComPtr<IPropertyStorage> pPropStg = NULL;
    HRESULT hr;
    BOOL result=TRUE;

    // Open summary information, getting an IpropertyStorage.
    hr = pPropSetStg->Open(FMTID_SummaryInformation,
                           STGM_READ | STGM_SHARE_EXCLUSIVE, &pPropStg);
    if(FAILED(hr)) {
        //printf("No Summary-Information.\n");
        return FALSE;
    }

    // Array of PIDSI's you are interested in.
    struct pidsiStruct {
        char *name;
        long pidsi;
    } pidsiArr[] = {
        {"Title",            PIDSI_TITLE}, // VT_LPSTR
        {"Subject",          PIDSI_SUBJECT}, // ...
        {"Author",           PIDSI_AUTHOR},
        {"Keywords",         PIDSI_KEYWORDS},
        {"Comments",         PIDSI_COMMENTS},
        {"Template",         PIDSI_TEMPLATE},
        {"LastAuthor",       PIDSI_LASTAUTHOR},
        {"Revision Number",  PIDSI_REVNUMBER},
        {"Edit Time",        PIDSI_EDITTIME}, // VT_FILENAME (UTC)
        {"Last printed",     PIDSI_LASTPRINTED}, // ...
        {"Created",          PIDSI_CREATE_DTM},
        {"Last Saved",       PIDSI_LASTSAVE_DTM},
        {"Page Count",       PIDSI_PAGECOUNT}, // VT_I4
        {"Word Count",       PIDSI_WORDCOUNT}, // ...
        {"Char Count",       PIDSI_CHARCOUNT},

        {"Thumpnail",        PIDSI_THUMBNAIL}, // VT_CF
        {"AppName",          PIDSI_APPNAME}, // VT_LPSTR
        {"Doc Security",     PIDSI_DOC_SECURITY}, // VT_I4
        {0, 0}
    };
    // Count elements in pidsiArr.
    int nPidsi = 0;
    for(nPidsi=0; pidsiArr[nPidsi].name; nPidsi++);

    // Initialize PROPSPEC for the properties you want.
    PROPSPEC *pPropSpec = new PROPSPEC [nPidsi];
    PROPVARIANT *pPropVar = new PROPVARIANT [nPidsi];

    for(int i=0; i<nPidsi; i++) {
        ZeroMemory(&pPropSpec[i], sizeof(PROPSPEC));
        pPropSpec[i].ulKind = PRSPEC_PROPID;
        pPropSpec[i].propid = pidsiArr[i].pidsi;
    }

    // Read properties.
    hr = pPropStg->ReadMultiple(nPidsi, pPropSpec, pPropVar);

    if(FAILED(hr)) {
        //printf("IPropertyStg::ReadMultiple() failed w/error %08lx\n", hr);
        result=FALSE;
    }
    else {
        // Dump properties.
        std::wstring val;
        WCHAR tmpStr[1024]={0};
        for(int i=0; i<nPidsi; i++) {
            //wsprintf(tmpStr, L"%s", pidsiArr[i].name);
            size_t requiredSize = 0;
            mbstowcs_s(&requiredSize, tmpStr, 1024, pidsiArr[i].name, _TRUNCATE); // C4996
            GetPropVariant(pPropVar + i, val);

            if (val.size() > 0)
            {
                GenericNextLabsTagging::AddKeyValueHelperW(attrs, tmpStr, val.c_str());
            }
        }
    }

    // De-allocate memory.
    delete [] pPropVar;
    delete [] pPropSpec;

    return TRUE;
}
#endif

static void GetContent(IPropertySetStorage *pPropSetStg, ResourceAttributes *attrs, PropType type, vector<pair<wstring, wstring>>* pVecSummery = NULL)
{
	CComPtr<IPropertyStorage> pPropStg = NULL;
	CComPtr<IEnumSTATPROPSTG> pEnumProp = NULL;
	STATPROPSTG stat;
	PROPVARIANT pvar;
	PROPSPEC spec;
	HRESULT hr=S_OK;

	if(type == enSumInfo)
		hr = pPropSetStg->Open(FMTID_SummaryInformation,STGM_READ | STGM_SHARE_EXCLUSIVE, &pPropStg);
	else	
		hr = pPropSetStg->Open(FMTID_DocSummaryInformation,STGM_READ | STGM_SHARE_EXCLUSIVE, &pPropStg);
	
	if(FAILED(hr)) 
	{
		return;
	}

    hr = pPropStg->Enum(&pEnumProp);
    if (S_OK == hr)
    {
        std::wstring val;
        wchar_t* szName = NULL;
        bool bKeyWords = false, bTitle = false, bSubject = false, bManager = false;
        bool bAuthor = false, bCategory = false, bComments = false, bCompany = false;

        for (vector<pair<wstring, wstring>>::const_iterator it = pVecSummery->begin();
            it != pVecSummery->end(); it++)
        {
            if (_wcsicmp(L"KeyWords", it->first.c_str()) == 0)
            {
                bKeyWords = true;
            }
            else if (_wcsicmp(L"Title", it->first.c_str()) == 0)
            {
                bTitle = true;
            }
            else if (_wcsicmp(L"Subject", it->first.c_str()) == 0)
            {
                bSubject = true;
            }
            else if (_wcsicmp(L"Manager", it->first.c_str()) == 0)
            {
                bManager = true;
            }
            else if (_wcsicmp(L"Author", it->first.c_str()) == 0)
            {
                bAuthor = true;
            }
            else if (_wcsicmp(L"Category", it->first.c_str()) == 0)
            {
                bCategory = true;

            }
            else if (_wcsicmp(L"Comments", it->first.c_str()) == 0)
            {
                bComments = true;
            }
            else if (_wcsicmp(L"Company", it->first.c_str()) == 0)
            {
                bCompany = true;
            }
        }

        while (S_OK == pEnumProp->Next(1, &stat, NULL))
        {
            PropVariantInit(&pvar);
            spec.ulKind = PRSPEC_PROPID;
            spec.propid = stat.propid;
            hr = pPropStg->ReadMultiple(1, &spec, &pvar);
            if (FAILED(hr))
            {
                PropVariantClear(&pvar);
                if (stat.lpwstrName != NULL)
                    CoTaskMemFree(stat.lpwstrName);
                break;
            }
 
            if (type == enSumInfo && stat.propid == PIDSI_KEYWORDS)
            {
                GetPropVariant(&pvar, val);
                if (val.size() > 0)
                {
                    szName = L"KeyWords";
                    if (attrs != NULL)		GenericNextLabsTagging::AddKeyValueHelperW(attrs, szName, val.c_str());
                    else if (pVecSummery != NULL && !bKeyWords) pVecSummery->push_back(pair<wstring, wstring>(szName, val));
                }
            }

            if (type == enSumInfo && stat.propid == PIDSI_TITLE)
            {
                GetPropVariant(&pvar, val);
                if (val.size() > 0)
                {
                    szName = L"Title";
                    if (attrs != NULL)		GenericNextLabsTagging::AddKeyValueHelperW(attrs, szName, val.c_str());
                    else if (pVecSummery != NULL && !bTitle)    pVecSummery->push_back(pair<wstring, wstring>(szName, val));

                }
            }

            if (type == enSumInfo && stat.propid == PIDSI_SUBJECT)
            {
                GetPropVariant(&pvar, val);
                if (val.size() > 0)
                {
                    szName = L"Subject";
                    if (attrs != NULL)		GenericNextLabsTagging::AddKeyValueHelperW(attrs, szName, val.c_str());
                    else if (pVecSummery != NULL && !bSubject)  pVecSummery->push_back(pair<wstring, wstring>(szName, val));
                }
            }

            if (type == enSumInfo && stat.propid == PIDSI_AUTHOR)
            {
                GetPropVariant(&pvar, val);
                if (val.size() > 0)
                {
                    szName = L"Author";
                    if (attrs != NULL)		GenericNextLabsTagging::AddKeyValueHelperW(attrs, szName, val.c_str());
                    else if (pVecSummery != NULL && !bAuthor)   pVecSummery->push_back(pair<wstring, wstring>(szName, val));
                }
            }


            if (type == enSumInfo && stat.propid == PIDSI_COMMENTS)
            {
                GetPropVariant(&pvar, val);
                if (val.size() > 0)
                {
                    szName = L"Comments";
                    if (attrs != NULL)		GenericNextLabsTagging::AddKeyValueHelperW(attrs, szName, val.c_str());
                    else if (pVecSummery != NULL && !bComments) pVecSummery->push_back(pair<wstring, wstring>(szName, val));
                }
            }

            if (type == enDocSumInfo && stat.propid == PIDDSI_CATEGORY)
            {
                GetPropVariant(&pvar, val);
                if (val.size() > 0)
                {
                    szName = L"Category";
                    if (attrs != NULL)		GenericNextLabsTagging::AddKeyValueHelperW(attrs, szName, val.c_str());
                    else if (pVecSummery != NULL && !bCategory) pVecSummery->push_back(pair<wstring, wstring>(szName, val));
                }
            }

            if (type == enDocSumInfo && stat.propid == PIDDSI_MANAGER)
            {
                GetPropVariant(&pvar, val);
                if (val.size() > 0)
                {
                    szName = L"Manager";
                    if (attrs != NULL)		GenericNextLabsTagging::AddKeyValueHelperW(attrs, szName, val.c_str());
                    else if (pVecSummery != NULL && !bManager)  pVecSummery->push_back(pair<wstring, wstring>(szName, val));
                }
            }

            if (type == enDocSumInfo && stat.propid == PIDDSI_COMPANY)
            {
                GetPropVariant(&pvar, val);
                if (val.size() > 0)
                {
                    szName = L"Company";
                    if (attrs != NULL)		GenericNextLabsTagging::AddKeyValueHelperW(attrs, szName, val.c_str());
                    else if (pVecSummery != NULL && !bCompany)  pVecSummery->push_back(pair<wstring, wstring>(szName, val));
                }
            }

            PropVariantClear(&pvar);
            if (stat.lpwstrName != NULL)
                CoTaskMemFree(stat.lpwstrName);
        }
    }
}
// Dump's custom properties of a property storage.
static void GetFileCustomProps(IPropertySetStorage *pPropSetStg, ResourceAttributes *attrs)
{
    CComPtr<IPropertyStorage> pPropStg = NULL;
    HRESULT hr;
    CComPtr<IEnumSTATPROPSTG> pEnumProp;

    // Open User-Defined-Properties, getting an IpropertyStorage.
    hr = pPropSetStg->Open(g_fmtid,
                           STGM_READ | STGM_SHARE_EXCLUSIVE, &pPropStg);
    if(FAILED(hr)) {
        //printf("No User Defined Properties.\n");
        return;
    }

    // Get property enumerator.
    hr = pPropStg->Enum(&pEnumProp);
    if(FAILED(hr)) {
        //printf("Couldn't enumerate custom properties.\n");
        return;
    }

    // Enumerate properties.
    STATPROPSTG sps;
    ULONG fetched;
    PROPSPEC propSpec[1];
    PROPVARIANT propVar[1];
    std::wstring val;
    while(pEnumProp->Next(1, &sps, &fetched) == S_OK) {
        // Build a PROPSPEC for this property.
        ZeroMemory(&propSpec[0], sizeof(PROPSPEC));
        propSpec[0].ulKind = PRSPEC_PROPID;
        propSpec[0].propid = sps.propid;

        // Read this property.
        hr = pPropStg->ReadMultiple(1, &propSpec[0], &propVar[0]);
        if(!FAILED(hr)) {
            GetPropVariant(&propVar[0], val);
            if(val.size() > 0) 
            {
                GenericNextLabsTagging::AddKeyValueHelperW(attrs, sps.lpwstrName, val.c_str());
            }
        }
    }


}

// Dump's custom and built-in properties of a compound document.
HRESULT GetOLEFileSummaryProps(const WCHAR *filename, ResourceAttributes *attrs)
{
    NLCELOG_ENTER
        CComPtr<IStorage> pStorage = NULL;
    CComPtr<IPropertySetStorage> pPropSetStg = NULL;
    HRESULT hr;


    wstring file_to_read = filename;


    //	check if it is wfse pc env
    //	if it is, we have to copy a temp file in order to workaround bug #15731
    //	otherwise there will be a deadlock
    //	we have the deadlock between cepdpman.exe and system process(in which WFSE
    //	resides). the deadlock is caused by:
    //	1, system process(WFSE) query cepdpman for an IO request to a file.
    //	2, cepdpman call resattrmgr to read tag from the file, as the file is already
    //	locked by system process, the read operation(e.g. ReadFile, CopyFile, fread,
    //	StgOpenStorageEx) will wait system process to release the lock. then deadlock
    //	takes place.
    if (true == nextlabs::windows_fse::is_wfse_installed())
    {

        if (!MyCreateTransaction || !MyCommitTransaction || !MyCopyFileTransactedW)
        {
            NLCELOG_LOG(NLCELOG_WARNING, L"transaction function pointer is NULL\n");

            //	return error to tell caller we can't get tag for him,
            //	we have to return error because we can't call the legacy method to read tag for caller
            //	because we don't want the hang happen
            NLCELOG_RETURN_VAL(ERROR_INVALID_PARAMETER)
        }


        HANDLE hTransaction = MyCreateTransaction(
            NULL,
            0,
            0,
            0,
            0,
            INFINITE,
            NULL
            );
        if (INVALID_HANDLE_VALUE == hTransaction)
        {
            NLCELOG_LOG(NLCELOG_WARNING, L"CreateTransaction fail\n");

            //	return error to tell caller we can't get tag for him,
            //	we have to return error because we can't call the legacy method to read tag for caller
            //	because we don't want the hang happen
            NLCELOG_RETURN_VAL(GetLastError())
        }
        wstring temp = filename;
        wstring::size_type pos = temp.rfind(L'\\');
        if (pos == wstring::npos)
        {
            NLCELOG_LOG(NLCELOG_WARNING, L"can't find filename before copyfiletransacted\n");
            //	return error to tell caller we can't get tag for him,
            NLCELOG_RETURN_VAL(ERROR_INVALID_PARAMETER)
        }
        temp = temp.substr(pos + 1, temp.length() - pos - 1);
        temp += L".wfse_pc_tmp";

        WCHAR temppath[MAX_PATH] = { 0 };
        if (!GetTempPath(MAX_PATH, temppath))
        {
            NLCELOG_LOG(NLCELOG_WARNING, L"GetTempPath fail\n");
            //	return error to tell caller we can't get tag for him,
            NLCELOG_RETURN_VAL(GetLastError())
        }
        temp = (wstring)temppath + temp;

        if (!MyCopyFileTransactedW(filename, temp.c_str(), NULL, NULL, NULL, COPY_FILE_FAIL_IF_EXISTS, hTransaction))
        {
            //	return error to tell caller we can't get tag for him,
            NLCELOG_RETURN_VAL(GetLastError())
        }
        MyCommitTransaction(hTransaction);
        CloseHandle(hTransaction);

        //	set temp as the file_to_read
        file_to_read = temp;
    }

    // Open the document as an OLE compound document.
    hr = ::StgOpenStorage(file_to_read.c_str(), NULL,
        STGM_DIRECT | STGM_READ | STGM_SHARE_DENY_NONE | STGM_PRIORITY, NULL, 0, &pStorage);

    if (FAILED(hr)) {
        if (true == nextlabs::windows_fse::is_wfse_installed())
        {
            DeleteFile(file_to_read.c_str());
        }

        NLCELOG_RETURN_VAL(hr)
    }

    // Obtain the IPropertySetStorage interface.
    hr = pStorage->QueryInterface(IID_IPropertySetStorage, (void **)&pPropSetStg);

    if (FAILED(hr)) {
        if (true == nextlabs::windows_fse::is_wfse_installed())
        {
            DeleteFile(file_to_read.c_str());
        }
        NLCELOG_RETURN_VAL(hr)
    }

    GetContent(pPropSetStg, attrs, enSumInfo);
    GetContent(pPropSetStg, attrs, enDocSumInfo);
    if (true == nextlabs::windows_fse::is_wfse_installed())
    {
        DeleteFile(file_to_read.c_str());
    }
    NLCELOG_RETURN_VAL(0)
}
//
// Exported functions
//

BOOL IsOLEFile(LPCWSTR pszFileName)
{
        if(!pszFileName)
        {
                return FALSE;
        }
        if (true == nextlabs::windows_fse::is_wfse_installed())
        {
                //	if this is WFSE PC env, we identify OLE files by extension(office 2007 files are identified by extension, so I think 
                //	this is ok for OLE files too),
                //	this is because, if we are in wfse pc env, if we call StdOpenStorageEx to determine if the file is OLE file,
                //	it will cause hang
                const wchar_t* pSuffix = wcsrchr(pszFileName,L'.');
                if(pSuffix == NULL)	
                {	
                        return FALSE;
                }

                bool bFind = false;

                const wchar_t* table[] = 
                { 
                        L".doc",
                        L".dot",
                        L".xls",
                        L".xlt",
                        L".pot",
                        L".ppt",
                        L".pps"
                };

                unsigned int nsize = _countof(table);

                for(unsigned int i=0;i<nsize;i++)
                {
                        if(_wcsicmp(pSuffix,table[i]) == 0)
                        {
                                bFind = TRUE;
                                break;
                        }
                }

                return bFind;
        }
        else
        {
                HRESULT hr = S_OK;
                CComPtr<IPropertySetStorage> pPropSetStg = NULL;

                hr = ::StgOpenStorageEx(pszFileName,
                        STGM_DIRECT | STGM_READ | STGM_SHARE_DENY_NONE | STGM_PRIORITY, 
                        STGFMT_STORAGE,
                        NULL,
                        NULL,
                        NULL,
                        IID_IPropertySetStorage,
                        reinterpret_cast<void**>(&pPropSetStg) );

                if( FAILED(hr) ) { 
                        return FALSE;
                } 

                return TRUE;
        }
}

// Dump's custom and built-in properties of a compound document.
HRESULT GetOLEFileProps(const WCHAR *filename, ResourceAttributes *attrs)
{
    NLCELOG_ENTER
        CComPtr<IStorage> pStorage = NULL;
    CComPtr<IPropertySetStorage> pPropSetStg = NULL;
    HRESULT hr;


            wstring file_to_read = filename;


            //	check if it is wfse pc env
            //	if it is, we have to copy a temp file in order to workaround bug #15731
            //	otherwise there will be a deadlock
            //	we have the deadlock between cepdpman.exe and system process(in which WFSE
            //	resides). the deadlock is caused by:
            //	1, system process(WFSE) query cepdpman for an IO request to a file.
            //	2, cepdpman call resattrmgr to read tag from the file, as the file is already
            //	locked by system process, the read operation(e.g. ReadFile, CopyFile, fread,
            //	StgOpenStorageEx) will wait system process to release the lock. then deadlock
            //	takes place.
            if(true == nextlabs::windows_fse::is_wfse_installed())
            {

                    if (!MyCreateTransaction || !MyCommitTransaction || !MyCopyFileTransactedW)
                    {
											NLCELOG_LOG( NLCELOG_WARNING, L"transaction function pointer is NULL\n");
                           
											//	return error to tell caller we can't get tag for him,
                      //	we have to return error because we can't call the legacy method to read tag for caller
                      //	because we don't want the hang happen
                      NLCELOG_RETURN_VAL( ERROR_INVALID_PARAMETER )
                    }


                    HANDLE hTransaction = MyCreateTransaction(
                            NULL,
                            0,
                            0,
                            0,
                            0,
                            INFINITE,
                            NULL
                            );
                    if(INVALID_HANDLE_VALUE == hTransaction)
                    {
                           NLCELOG_LOG( NLCELOG_WARNING, L"CreateTransaction fail\n");
                         
														//	return error to tell caller we can't get tag for him,
                            //	we have to return error because we can't call the legacy method to read tag for caller
                            //	because we don't want the hang happen
                            NLCELOG_RETURN_VAL( GetLastError() )	
                    }
                    wstring temp = filename;
                    wstring::size_type pos = temp.rfind(L'\\');
                    if (pos == wstring::npos)
                    {
                            NLCELOG_LOG( NLCELOG_WARNING, L"can't find filename before copyfiletransacted\n");
                            //	return error to tell caller we can't get tag for him,
                            NLCELOG_RETURN_VAL( ERROR_INVALID_PARAMETER )	
                    }
                    temp = temp.substr(pos + 1, temp.length() - pos - 1);
                    temp += L".wfse_pc_tmp";

                    WCHAR temppath[MAX_PATH] = {0};
                    if (!GetTempPath(MAX_PATH, temppath))
                    {
                            NLCELOG_LOG( NLCELOG_WARNING, L"GetTempPath fail\n");
                            //	return error to tell caller we can't get tag for him,
                            NLCELOG_RETURN_VAL( GetLastError() )	
                    }
                    temp = (wstring)temppath + temp;

                    if (!MyCopyFileTransactedW(filename, temp.c_str(), NULL, NULL, NULL, COPY_FILE_FAIL_IF_EXISTS, hTransaction))
                    {
                            //	return error to tell caller we can't get tag for him,
                            NLCELOG_RETURN_VAL( GetLastError() )
                    }
                    MyCommitTransaction(hTransaction);	
                    CloseHandle(hTransaction);

                    //	set temp as the file_to_read
                    file_to_read = temp;
            }

    // Open the document as an OLE compound document.
    hr = ::StgOpenStorage(file_to_read.c_str(), NULL,
                          STGM_DIRECT | STGM_READ|STGM_SHARE_DENY_NONE|STGM_PRIORITY, NULL, 0, &pStorage);

    if(FAILED(hr)) {
                    if (true == nextlabs::windows_fse::is_wfse_installed())
                    {
                            DeleteFile(file_to_read.c_str());
                    }

                    NLCELOG_RETURN_VAL( hr )
    }

    // Obtain the IPropertySetStorage interface.
    hr = pStorage->QueryInterface(IID_IPropertySetStorage, (void **)&pPropSetStg);

    if(FAILED(hr)) {
                    if (true == nextlabs::windows_fse::is_wfse_installed())
                    {
                            DeleteFile(file_to_read.c_str());
                    }
        NLCELOG_RETURN_VAL( hr )
    }

    // Get properties.
#ifdef GET_BUILTIN_PROPERTIES
    GetFileBuiltInProps(pPropSetStg, attrs,NULL);
#endif
    wchar_t szLog[2046] = { 0 };
#ifdef GET_SUMMERY_PROPERTIES
    vector<pair<wstring, wstring>> vecSummery;
    GetContent(pPropSetStg, NULL, enSumInfo, &vecSummery);
    GetContent(pPropSetStg, NULL, enDocSumInfo, &vecSummery);
    for (vector<pair<wstring, wstring>>::const_iterator it = vecSummery.begin();
        it != vecSummery.end(); it++)
    {
#if 1
        StringCchPrintfW(szLog, 2045, L"--------> read summery tag of %s=%s in function of GetOLEFileProps.\n", it->first.c_str(), it->second.c_str());
        OutputDebugStringW(szLog);
#endif // 1
        GenericNextLabsTagging::AddKeyValueHelperW(attrs, it->first.c_str(), it->second.c_str());
    }
#endif
    GetFileCustomProps(pPropSetStg, attrs);

            if (true == nextlabs::windows_fse::is_wfse_installed())
            {
                    DeleteFile(file_to_read.c_str());
            }
    NLCELOG_RETURN_VAL( 0 )
}

HRESULT SetOLEFileProps(const WCHAR *filename, ResourceAttributes *attrs)//fix bug315
{
    NLCELOG_ENTER
    if (!filename || !attrs)
        NLCELOG_RETURN_VAL(HRESULT_FROM_WIN32(RPC_X_NULL_REF_POINTER))

        HRESULT hr = S_OK;
        CComPtr<IPropertySetStorage> pPropSetStg = NULL;
        hr = ::StgOpenStorageEx(filename,
                STGM_DIRECT|STGM_SHARE_EXCLUSIVE|STGM_READWRITE, 
                STGFMT_STORAGE,
                NULL,
                NULL,
                NULL,
                IID_IPropertySetStorage,
                reinterpret_cast<void**>(&pPropSetStg) );

    if (FAILED(hr)) {
        NLCELOG_RETURN_VAL(hr)
    }

    wchar_t szLog[2046] = { 0 };
#ifdef GET_SUMMERY_PROPERTIES
    // we need to filter the summery attributes before written to custom property page
    vector<pair<wstring, wstring>> VecSummery;
    GetContent(pPropSetStg.p, NULL, enSumInfo, &VecSummery);
    GetContent(pPropSetStg.p, NULL, enDocSumInfo, &VecSummery);
#if 1
    for (vector<pair<wstring, wstring>>::const_iterator it = VecSummery.begin();
        it != VecSummery.end(); it++)
    {
        StringCchPrintfW(szLog, 2045, L"--------> read summery tag of %s=%s in function of SetOLEFileProps.\n", it->first.c_str(), it->second.c_str());
        OutputDebugStringW(szLog);
    }
#endif // 1
#endif

        CComPtr<IPropertyStorage> pPropStg = NULL;
        hr = pPropSetStg->Open(g_fmtid,  STGM_READWRITE | STGM_SHARE_EXCLUSIVE, &pPropStg);	
        if(FAILED(hr))
        {
                hr = pPropSetStg->Create( g_fmtid, NULL, PROPSETFLAG_DEFAULT, 
                        STGM_CREATE|STGM_READWRITE|STGM_SHARE_EXCLUSIVE,
                        &pPropStg );
                if( FAILED(hr) ) 
                {
                        NLCELOG_RETURN_VAL( hr )
                }
        }

        int numAttrs = GetAttributeCount(attrs);

        PROPSPEC *propspec = new (std::nothrow) PROPSPEC[numAttrs]; 
        PROPVARIANT *propvarWrite = new (std::nothrow) PROPVARIANT[numAttrs]; 

        if (propspec == NULL || propvarWrite == NULL)
        {
                NLCELOG_RETURN_VAL( HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY) )
        }

    STATPROPSETSTG stg;
    stg.grfFlags = PROPSETFLAG_DEFAULT;
    if (pPropStg)
        pPropStg->Stat(&stg);
    BOOL bAnsi = stg.grfFlags & PROPSETFLAG_ANSI;
    DWORD j = 0;
    for (int i = 0; i < numAttrs; ++i)
    {
        WCHAR *propertyName = (WCHAR *)GetAttributeName(attrs, i);
        WCHAR *propertyValue = (WCHAR *)GetAttributeValue(attrs, i);

                if(!propertyName || !propertyValue)
                        continue;

                if(wcslen(propertyName) > TAGNAME_MAX_LENGTH)
                        propertyName[TAGNAME_MAX_LENGTH] = '\0';

                if(wcslen(propertyValue) > TAGNAME_MAX_LENGTH)
                        propertyValue[TAGNAME_MAX_LENGTH] = '\0';

        bool bFind = false;
        for (vector<pair<wstring, wstring>>::const_iterator it = VecSummery.begin();
            it != VecSummery.end(); it++)
        {
            if (_wcsicmp(it->first.c_str(), propertyName) == 0)
            {
                bFind = true;
                break;
            }
        }
        if (bFind)  continue;
        propspec[j].ulKind = PRSPEC_LPWSTR;
        propspec[j].lpwstr = propertyName;
        PropVariantInit(&propvarWrite[j]);
        if (bAnsi)
        {
            int nLen = WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, propertyValue, (int)wcslen(propertyValue), NULL, 0, NULL, NULL);

                        char* pPropValue = new char[nLen + 1];
                        if(!pPropValue)
                                continue;
                        memset(pPropValue, 0, nLen +1);
                        nLen = WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, propertyValue, (int)wcslen(propertyValue), pPropValue, nLen, NULL, NULL); 

            propvarWrite[j].vt = VT_LPSTR;
            propvarWrite[j].pszVal = pPropValue;
        }
        else
        {
            // Write a Unicode string property to the property set
            propvarWrite[j].vt = VT_LPWSTR;
            propvarWrite[j].pwszVal = propertyValue;
        }
        j++;
    }

    hr = pPropStg->WriteMultiple(j, propspec, propvarWrite,
        PID_FIRST_USABLE);

    if (bAnsi)
    {
        for (DWORD i = 0; i < j; ++i)
        {
            if (propvarWrite[i].pszVal)
            {
                delete[] propvarWrite[i].pszVal;
                propvarWrite[i].pszVal = NULL;
            }
        }
    }

        delete[] propspec;
        delete[] propvarWrite;

        if( FAILED(hr) ) {
                NLCELOG_RETURN_VAL( hr )
        }

        // Not required, but give the property set a friendly 
        // name.

        PROPID propidDictionary = PID_DICTIONARY;
        WCHAR *pwszFriendlyName = 
                L"NextLabs Tags";
        hr = pPropStg->WritePropertyNames( 1, &propidDictionary, 
                &pwszFriendlyName );
        if( FAILED(hr) ) 
        {
                NLCELOG_RETURN_VAL( hr )
        }

        // Commit changes to the property set.
        hr = pPropStg->Commit(STGC_DEFAULT);
        if( FAILED(hr) ) {
                NLCELOG_RETURN_VAL( hr )
        }

        NLCELOG_RETURN_VAL( hr )


}

HRESULT RemoveOLEFileProps(const WCHAR *filename, ResourceAttributes *attrs)
{
    NLCELOG_ENTER
    if (!filename || !attrs)
        NLCELOG_RETURN_VAL(HRESULT_FROM_WIN32(RPC_X_NULL_REF_POINTER))

        HRESULT hr = S_OK;
        CComPtr<IPropertySetStorage> pPropSetStg = NULL;
        hr = ::StgOpenStorageEx(filename,
                STGM_DIRECT|STGM_SHARE_EXCLUSIVE|STGM_READWRITE, 
                STGFMT_STORAGE,
                NULL,
                NULL,
                NULL,
                IID_IPropertySetStorage,
                reinterpret_cast<void**>(&pPropSetStg) );

        if( FAILED(hr) ) {
                NLCELOG_RETURN_VAL( hr )
        }

        CComPtr<IPropertyStorage> pPropStg = NULL;
        // Open User-Defined-Properties, getting an IpropertyStorage.
        hr = pPropSetStg->Open(g_fmtid,  STGM_READWRITE | STGM_SHARE_EXCLUSIVE, &pPropStg);	//fix bug330
        if(FAILED(hr))
        {
                hr = pPropSetStg->Create(g_fmtid,
                NULL,
                PROPSETFLAG_DEFAULT,
                STGM_CREATE | STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 
                &pPropStg);
        if(FAILED(hr)) {
                // Lost the original data.  Oh well.
                NLCELOG_RETURN_VAL( hr )
        }
        }



        int nCountAttrs = GetAttributeCount(attrs);
        PROPSPEC *propspec = new (std::nothrow) PROPSPEC[nCountAttrs]; 
//		PROPVARIANT *propvarWrite = new (std::nothrow) PROPVARIANT[nCountAttrs]; 

        if (propspec == NULL /*|| propvarWrite == NULL*/)
        {
                NLCELOG_RETURN_VAL( HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY) )
        }

        for (int i = 0; i < nCountAttrs; ++i)
        {
                WCHAR *propertyName = (WCHAR *)GetAttributeName(attrs, i);

                propspec[i].ulKind = PRSPEC_LPWSTR;
                propspec[i].lpwstr = propertyName;

        }
        if(pPropStg)
                hr = pPropStg->DeleteMultiple(nCountAttrs, propspec);

        if(propspec)
                delete []propspec;

        // Commit changes to the property set.
        hr = pPropStg->Commit(STGC_DEFAULT);
        if( FAILED(hr) ) {
                NLCELOG_RETURN_VAL( hr )
        }

        NLCELOG_RETURN_VAL( hr )

}
