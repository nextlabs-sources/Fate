/*************************************************************************
 *
 * Compliant Enterprise Resource Attribute Manager
 *
 * Implementation file for reading/writing resource attributes either
 * via a default scheme (provided) or a runtime selected dll
 *
 ************************************************************************/
#include "resattrmgr.h"

#ifdef TAGLIB_WINDOWS
#include <windows.h>
#include <KtmW32.h>
#include "defimplreader.h"
#include "wfse_pc_env_declare.h"
#include "eframework\platform\windows_file_server_enforcer.hpp"

//	these three are used in other files, so can't make it static 
_CommitTransaction MyCommitTransaction = NULL;
_CopyFileTransactedW MyCopyFileTransactedW = NULL;
_CreateTransaction MyCreateTransaction = NULL;

//	these two are locally used only, so make it static
static HMODULE hKtmW32 = NULL;
static HMODULE hKernel32 = NULL;


#else
#include "utils.h"
#include "nlclassification.hpp"
#include "indexer.h"
#endif

#include <iostream>
#include "NxlFormatFile.h"

// for CELog2
#include "NLCELog.h"
#define  CELOG_CUR_FILE static_cast<celog_filepathint_t>(EMNLFILEINTEGER_RESATTRMGRCPP)

using namespace std;

struct ResourceAttributeManager
{
public:
#ifdef TAGLIB_WINDOWS
    typedef int (*init_fn_ptr)(void **);
    typedef int (*deinit_fn_ptr)(void *);
    typedef int (*get_fn_ptr)(void *, const WCHAR *, ResourceAttributes *, TagType );
    typedef int (*set_fn_ptr)(void *, const WCHAR *, ResourceAttributes *, TagType );
	typedef int (*remove_fn_ptr)(void *, const WCHAR *, ResourceAttributes *);
    
    init_fn_ptr init_fn;
    deinit_fn_ptr deinit_fn;
    get_fn_ptr get_fn;
    set_fn_ptr set_fn;
	remove_fn_ptr remove_fn;
#endif
    void *pluginData;

private:
#ifdef TAGLIB_WINDOWS
    HMODULE loadUserImpl() {
        return ::LoadLibraryA("attraccess.dll");
    };
    void setDefaultHandlers()
    {
        init_fn = DefImpl_Initialize;
        deinit_fn = DefImpl_Deinitialize;
        get_fn = DefImpl_GetAttributes;
        set_fn = DefImpl_SetAttributes;
        remove_fn = DefImpl_RemoveAttributes;
    };
#endif
public:
    void setPluginData(void *data)
    {
        pluginData = data;
    }

    void *getPluginData() const
    {
        return pluginData;
    }
#ifdef TAGLIB_WINDOWS
    ResourceAttributeManager()
    {
        HMODULE hUserImpl = loadUserImpl();
        
        if (hUserImpl == NULL)
        {
            setDefaultHandlers();
        }
        else
        {
            init_fn = (init_fn_ptr)GetProcAddress(hUserImpl, "Initialize");
            deinit_fn = (deinit_fn_ptr)GetProcAddress(hUserImpl, "Deinitialize");
            get_fn = (get_fn_ptr)GetProcAddress(hUserImpl, "GetAttributes");
            set_fn = (set_fn_ptr)GetProcAddress(hUserImpl, "SetAttributes");
            remove_fn = (remove_fn_ptr)GetProcAddress(hUserImpl, "RemoveAttributes");

            if (init_fn == NULL || deinit_fn == NULL || get_fn == NULL || set_fn == NULL || remove_fn == NULL)
            {
               NLCELOG_LOG( NLCELOG_WARNING, L"Missing functions from attraccess.dll.  Using defaults\n");
                setDefaultHandlers();
            }
        }

		
		//	init transaction functions if we are in wfse pc env
		if (true == nextlabs::windows_fse::is_wfse_installed())
		{
			hKtmW32 = LoadLibraryW(L"KtmW32.dll");
			if (hKtmW32)
			{
				MyCreateTransaction = (_CreateTransaction)GetProcAddress(hKtmW32, "CreateTransaction");	
				MyCommitTransaction = (_CommitTransaction)GetProcAddress(hKtmW32, "CommitTransaction");	
			}
			hKernel32 = LoadLibraryW(L"Kernel32.dll");
			if (hKernel32)
			{
				MyCopyFileTransactedW = (_CopyFileTransactedW)GetProcAddress(hKernel32, "CopyFileTransactedW");	
			}
			if (!MyCreateTransaction || !MyCommitTransaction || !MyCopyFileTransactedW)
			{
				//	when we fail to load transaction function, we do nothing here, 
				//	but we'll make sure we check if those function pointer are not null before use it.
				NLCELOG_LOG( NLCELOG_WARNING, L"Missing Windows Transaction functions\n");
			}

		}
    };
#endif

};

int CreateAttributeManager(ResourceAttributeManager **mgr)
{
    void *pluginData = 0;
    *mgr = new ResourceAttributeManager();
#ifdef TAGLIB_WINDOWS
    (*mgr)->init_fn(&pluginData);
#endif
    (*mgr)->setPluginData(pluginData);

    return 0;
}
#ifndef TAGLIB_WINDOWS
BOOL initCEI = FALSE;
#define	DEFAULT_DB_PATH	"/opt/PolicyController/pep/data/fp.db"

void getTagsFromDB(const char*pchFileName,ResourceAttributes*pAttr)
{NLCELOG_ENTER
  if (initCEI == FALSE) {
    char buf[256];
    initCEI = TRUE;
    const char *dbLocation = DEFAULT_DB_PATH;
    NLClassificationConfig config;
    if (config.get_db_path(buf, 255) == true)
      dbLocation = buf;
    if (cei_init((char*)dbLocation) != 0) {
      //fprintf (stderr, "Initializing CEI failed (DB in %s)\n", dbLocation);
      initCEI = FALSE;
      NLCELOG_RETURN;
    }
  }

  char* tagstr = NULL;
  if (cei_process_file((char*)pchFileName, &tagstr) != 0)
    {
      //fprintf(stderr, "cei_process_file return failed\n");
      NLCELOG_RETURN;
    }
  else
    {
      string StrPtags = tagstr;
      size_t Pos = 0;
      size_t LastPos = 0;
      for(;;)
	{
	  string Pair;
	  Pos = StrPtags.find(";", LastPos);
	  if(Pos == string::npos)
	    Pair = StrPtags.substr(LastPos);
	  else
	    Pair = StrPtags.substr(LastPos, (Pos - LastPos));
	  
	  size_t PairPos = Pair.find("=");
	  if(PairPos != string::npos)
	    {
	      string Name = Pair.substr(0, PairPos);
	      string Value = Pair.substr(PairPos+1);
	      AddAttributeA(pAttr, Name.c_str(), Value.c_str());
	      
	    }
	  
	  if(Pos == string::npos)
	    break;
	  
	  LastPos = Pos;
	  LastPos++;
	}
      cei_free(tagstr);
    }
  
}

enum OPCODE{GET=1,SET,REMOVE};
static int ActResourceAttributes(void*pluginData,const char*pchFileName,ResourceAttributes*pAttr,OPCODE opCode)
{NLCELOG_ENTER
	NSLinuxTagLib::FileTagging* pFTag=NSLinuxTagLib::Utils::CreateFileTagging(pchFileName);
	if(pFTag)
	{
		BOOL bRet=FALSE;
		switch(opCode)
		{
		case GET:
		  bRet=pFTag->GetTags(pluginData,pchFileName,pAttr);
		  getTagsFromDB(pchFileName, pAttr);
		  break;
		case SET:
			bRet=pFTag->SetTags(pluginData,pchFileName,pAttr);
			break;
		case REMOVE:
			bRet=pFTag->RemoveTags(pluginData,pchFileName,pAttr);
			break;
		default:
			break;
		}
		delete pFTag;
		NLCELOG_RETURN_VAL( bRet==TRUE?1:0 )
	}
	else
	  NLCELOG_RETURN_VAL( 1 )
}
#endif

/***********************************************************************
 * ReadResourceAttributes for Windows or Linux
 ***********************************************************************/
#ifdef TAGLIB_WINDOWS
int ReadResourceAttributesA(ResourceAttributeManager *mgr, const char *filename, ResourceAttributes *attrs)
{NLCELOG_ENTER
   size_t size = 0;
	mbstowcs_s(&size, NULL, 0, filename, 0);
    WCHAR *wcFilename = new WCHAR[size];
	mbstowcs_s(&size, wcFilename, size, filename, _TRUNCATE);
    int nRet = mgr->get_fn(mgr->getPluginData(), wcFilename, attrs,TagTypeDefault);

    delete[] wcFilename;
    NLCELOG_RETURN_VAL( nRet )
}

int ReadResourceAttributesW(ResourceAttributeManager *mgr, const WCHAR *filename, ResourceAttributes *attrs)
{NLCELOG_ENTER
		NLCELOG_DEBUGLOG( L"parameters: mgr=%p, filename=%s, attrs=%p \n", mgr, NULL==filename?L"NULL":filename, attrs );
    NLCELOG_RETURN_VAL( mgr->get_fn(mgr->getPluginData(), filename, attrs, TagTypeDefault) );
}

int ReadResrcSummaryAttr(ResourceAttributeManager *mgr, const WCHAR *filename, ResourceAttributes *attrs)
{
	NLCELOG_ENTER
		NLCELOG_DEBUGLOG( L"parameters: mgr=%p, filename=%s, attrs=%p \n", mgr, NULL==filename?L"NULL":filename, attrs );
	NLCELOG_RETURN_VAL( mgr->get_fn(mgr->getPluginData(), filename, attrs, TagSummary) );
}



int ReadResourceAttributesForNTFSA(ResourceAttributeManager *mgr, const char *filename, ResourceAttributes *attrs)
{NLCELOG_ENTER
	size_t size = 0;
	mbstowcs_s(&size, NULL, 0, filename, 0);
	WCHAR *wcFilename = new WCHAR[size];
	mbstowcs_s(&size, wcFilename, size, filename, _TRUNCATE);
	int nRet = mgr->get_fn(mgr->getPluginData(), wcFilename, attrs,TagTypeNTFS);

	delete[] wcFilename;
	NLCELOG_RETURN_VAL( nRet )
}

int ReadResourceAttributesForNTFSW(ResourceAttributeManager *mgr, const WCHAR *filename, ResourceAttributes *attrs)
{NLCELOG_ENTER
	NLCELOG_DEBUGLOG( L"parameters: mgr=%p, filename=%s, attrs=%p \n", mgr, NULL==filename?L"NULL":filename, attrs );
	NLCELOG_RETURN_VAL( mgr->get_fn(mgr->getPluginData(), filename, attrs,TagTypeNTFS) );
}

#else
int ReadResourceAttributesA(ResourceAttributeManager *mgr, const char *filename, ResourceAttributes *attrs)
{NLCELOG_ENTER
	if(attrs==NULL||filename==NULL)
		NLCELOG_RETURN_VAL( 0 )
	void* pluginData=NULL;
	if(mgr)
		pluginData=mgr->getPluginData();
	
	NLCELOG_RETURN_VAL( ActResourceAttributes(pluginData,filename,attrs,GET) )	
}
#endif

/***********************************************************************
 * WriteResourceAttributes for Windows or Linux
 ***********************************************************************/
#ifdef TAGLIB_WINDOWS
int WriteResourceAttributesA(ResourceAttributeManager *mgr, const char *filename, ResourceAttributes *attrs)
{NLCELOG_ENTER
	size_t size = 0;
	mbstowcs_s(&size, NULL, 0, filename, 0);
    WCHAR *wcFilename = new WCHAR[size];
	//mbstowcs(wcFilename, filename, size+1);
	mbstowcs_s(&size, wcFilename, size, filename, _TRUNCATE);
    int nRet = mgr->set_fn(mgr->getPluginData(), wcFilename, attrs, TagTypeDefault);
    delete[] wcFilename;

    NLCELOG_RETURN_VAL( nRet )
}

int WriteResourceAttributesW(ResourceAttributeManager *mgr, const WCHAR *filename, ResourceAttributes *attrs)
{NLCELOG_ENTER
		NLCELOG_DEBUGLOG( L"parameters: mgr=%p, filename=%s, attrs=%p \n", mgr, NULL==filename?L"NULL":filename, attrs );
    NLCELOG_RETURN_VAL( mgr->set_fn(mgr->getPluginData(), filename, attrs, TagTypeDefault) )
}

int WriteResourceAttributesForNTFSA(ResourceAttributeManager *mgr, const char *filename, ResourceAttributes *attrs)
{NLCELOG_ENTER
	size_t size = 0;
	mbstowcs_s(&size, NULL, 0, filename, 0);
	WCHAR *wcFilename = new WCHAR[size];

	mbstowcs_s(&size, wcFilename, size, filename, _TRUNCATE);
	int nRet = mgr->set_fn(mgr->getPluginData(), wcFilename, attrs, TagTypeNTFS);
	delete[] wcFilename;

	NLCELOG_RETURN_VAL( nRet );
}

int WriteResourceAttributesForNTFSW(ResourceAttributeManager *mgr, const WCHAR *filename, ResourceAttributes *attrs)
{NLCELOG_ENTER
	NLCELOG_DEBUGLOG( L"parameters: mgr=%p, filename=%s, attrs=%p \n", mgr, NULL==filename?L"NULL":filename, attrs );
	NLCELOG_RETURN_VAL( mgr->set_fn(mgr->getPluginData(), filename, attrs, TagTypeNTFS) )
}

#else
int WriteResourceAttributesA(ResourceAttributeManager *mgr, const char *filename, ResourceAttributes *attrs)
{NLCELOG_ENTER
	if(attrs==NULL||filename==NULL)
		NLCELOG_RETURN_VAL( 0 )
	if(GetAttributeCount(attrs)==0)
		NLCELOG_RETURN_VAL( 0 )
	void* pluginData=NULL;
	if(mgr)
		pluginData=mgr->getPluginData();
	NLCELOG_RETURN_VAL( ActResourceAttributes(pluginData,filename,attrs,SET) )
}
#endif

/***********************************************************************
 * RemoveResourceAttributes for Windows or Linux
 ***********************************************************************/
#ifdef TAGLIB_WINDOWS
int RemoveResourceAttributesA(ResourceAttributeManager *mgr, const char *filename, ResourceAttributes *attrs)
{NLCELOG_ENTER
    size_t size = 0;
    mbstowcs_s(&size, NULL, 0, filename, 0);
    WCHAR *wcFilename = new WCHAR[size];
    mbstowcs_s(&size, wcFilename, size, filename, _TRUNCATE);
    int nRet = mgr->remove_fn(mgr->getPluginData(), wcFilename, attrs);
    delete[] wcFilename;
    
    NLCELOG_RETURN_VAL( nRet )
}

int RemoveResourceAttributesW(ResourceAttributeManager *mgr, const WCHAR *filename, ResourceAttributes *attrs)
{NLCELOG_ENTER
		NLCELOG_DEBUGLOG( L"parameters: mgr=%p, filename=%s, attrs=%p \n", mgr, NULL==filename?L"NULL":filename, attrs );
    NLCELOG_RETURN_VAL( mgr->remove_fn(mgr->getPluginData(), filename, attrs) );
}
#else
int RemoveResourceAttributesA(ResourceAttributeManager *mgr, const char *filename, ResourceAttributes *attrs)
{NLCELOG_ENTER
	if(attrs==NULL||filename==NULL)
		NLCELOG_RETURN_VAL( 0 )
	if(GetAttributeCount(attrs)==0)
		NLCELOG_RETURN_VAL( 0 )
	void* pluginData=NULL;
	if(mgr)
		pluginData=mgr->getPluginData();
    NLCELOG_RETURN_VAL( ActResourceAttributes(pluginData,filename,attrs,REMOVE) )
}
#endif


void CloseAttributeManager(ResourceAttributeManager *mgr)
{
#ifdef TAGLIB_WINDOWS
	mgr->deinit_fn(mgr->getPluginData());
#endif
    delete mgr;
}

int Convert_Raw_2_PC_For_Non_Office(ResourceAttributes *raw_attrs,ResourceAttributes* PC_attrs)
{
	return Convert_Raw_2_PC_For_Non_Office_Imp(raw_attrs,PC_attrs);
}

int Convert_PC_2_RAW_For_Non_Office(ResourceAttributes *PC_attrs,ResourceAttributes* raw_attrs)
{
	return Convert_PC_2_RAW_For_Non_Office_Imp(PC_attrs,raw_attrs);
}

int Convert_GetAttributes(ResourceAttributes *attrs, ResourceAttributes* existing_attrs)
{
	return Convert4GetAttributes(attrs, existing_attrs);
}
int Convert_SetAttributes(ResourceAttributes* attrs_to_set, ResourceAttributes* merged_attrs)
{
	return Convert4SetAttributes(attrs_to_set, merged_attrs);
}


bool IsNxlFormat(const WCHAR *filename)
{
	CNxlFormatFile nxlFile;
	return nxlFile.IsNXLFormatFile(filename);
}