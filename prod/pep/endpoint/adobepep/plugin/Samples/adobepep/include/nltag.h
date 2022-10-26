#pragma once

#include <Shlwapi.h>
#include <string>
#include <vector>
using namespace std;

#include "resattrlib.h"
#include "resattrmgr.h"
#include "utilities.h"

#pragma warning(push)
#pragma warning(disable:6334 6011 4996 4819 4189 4100 4244 4512)
#include "boost\unordered_map.hpp"
#include "boost\shared_ptr.hpp"
#include "boost\weak_ptr.hpp"
#include <boost/algorithm/string.hpp>
#include <boost/thread.hpp>  
#include <boost/function.hpp> 
#pragma warning(pop)

#include "encrypt.h"
#include "celog.h"

#define CELOG_CUR_MODULE L"ADOBEPEP"
#undef CELOG_CUR_FILE
#define CELOG_CUR_FILE CELOG_FILEPATH_PROD_PEP_ENDPOINT_ADOBEPEP_PLUGIN_SAMPLESADOBEPEP_INCLUDE_NLTAG_H

typedef int (*CreateAttributeManagerType)(ResourceAttributeManager **mgr);
typedef int (*AllocAttributesType)(ResourceAttributes **attrs);
typedef int (*ReadResourceAttributesWType)(ResourceAttributeManager *mgr, const WCHAR *filename, ResourceAttributes *attrs);
typedef int (*ReadResourceAttributesForNTFSWType)(ResourceAttributeManager *mgr, const WCHAR *filename, ResourceAttributes *attrs);
typedef int (*GetAttributeCountType)(const ResourceAttributes *attrs);
typedef void (*FreeAttributesType)(ResourceAttributes *attrs);
typedef void (*CloseAttributeManagerType)(ResourceAttributeManager *mgr);
typedef void (*AddAttributeWType)(ResourceAttributes *attrs, const WCHAR *name, const WCHAR *value);
typedef const WCHAR *(*GetAttributeNameType)(const ResourceAttributes *attrs, int index);
typedef const WCHAR * (*GetAttributeValueType)(const ResourceAttributes *attrs, int index);
typedef int (*WriteResourceAttributesWType)(ResourceAttributeManager *mgr, const WCHAR *filename, ResourceAttributes *attrs);
typedef int (*RemoveResourceAttributesWType)(ResourceAttributeManager *mgr, const WCHAR *filename, ResourceAttributes *attrs);
typedef int (*Convert_Raw_2_PC_For_Non_Office_Type)(ResourceAttributes *raw_attrs,ResourceAttributes* PC_attrs);
typedef int (*Convert_PC_2_RAW_For_Non_Office_Type)(ResourceAttributes *PC_attrs,ResourceAttributes* raw_attrs);
typedef int (*WriteResourceAttributesForNTFSWType)(ResourceAttributeManager *mgr, const WCHAR *filename, ResourceAttributes *attrs);

typedef boost::shared_lock<boost::shared_mutex> boost_share_lock;  
typedef boost::unique_lock<boost::shared_mutex> boost_unique_lock; 

typedef vector<pair<wstring,wstring>> TAGS;
typedef boost::unordered_map<wstring, TAGS> TAGSCACHE;

static boost::shared_mutex g_mutex; 

struct TAG 
{
	char szTagName[1025];
	char szTagValue[4097];
};

typedef int (*PDF_ParseTagsType)(LPCSTR pszRawData, void** pOut, int* nCount);
typedef void (*PDF_FreeMemType)(TAG* pTags);


const wstring g_strCustomAttr = L"ce::file_custom_attributes_included";
const wstring g_strNlTag = L"NLRESATTR";

class CTag
{
public:
	static CTag* GetInstance()
	{
		static CTag ins;
		return &ins;
	}

	//可以读本地的，也可以读sharepoint的文件的tag,如果要读sharepoint，要传入PDDoc
	//PDDoc是用来读sharepoint文件的tag的
	void read_tag(const wstring&file, vector<pair<wstring,wstring>>& tags, PDDoc pdDoc=NULL, BOOL bPCReadTag = FALSE )
	{
		wstring strDocPath;
		EncryptRetValueType res = emEncryptSuccess;
		if (NULL != pdDoc)
		{
			//如果pdDoc不为空，我们首先要做的是检验这个pdDoc的值是不是真正的参数file对应的pdDoc，
			//参数file肯定是正确的，是我们要读tag的文件的path
			//但是pdDoc就不一定正确了，它可能是，也可能不是我们要读tag的文件所对应的pdDoc
			//这是因为，pdDoc的获取有两种方式，一种是我们hook的adobe的函数的参数传进来的，
			//另一种情况是，adobe的函数没有传进来，是我们自己调用Get active PDDoc获取的，这就有很大的问题了，很可能当前active的pddoc并不是我们要读tag的文件的pddoc
			//比如，拖拽一个pdf到一个portfolio里面去，这时候当前的pddoc是portfolio的PDDoc，但如果我们要读被拖拽的pdf文件的tag，就出大错误了！！！
			//所以，在这里，我们要检查pddoc是否有效
			string strPath;

			//所以，先根据pddoc取得文件的路径，如果这个路径和参数file相同，毫无疑问，pddoc的值是正确的。我们就可以用pddoc来读文件的tag了！！
			GetPathfromPDDoc(pdDoc, strPath);
			strDocPath = MyMultipleByteToWideChar(strPath);

			//当然，能不能用pddoc来读文件的tag，还取决于这个文件是不是SE文件，根据SPEC，SE文件的tag只能用resattrmgr来读！！
			res=CEncrypt::Encrypt(file,false,true);
		}

		CELOG_LOG(CELOG_DEBUG, L"read_tag, path: %s, pdDoc path: %s\n", file.c_str(), strDocPath.c_str());

		if (!strDocPath.empty())
		{//cache the native tags.
			TAGS nativetags;
			bool bNeedRead = true;
#if ACRO_SDK_LEVEL==0x000A0000	
#else
			//for reader, only need to read 1 time.
			bNeedRead = !get_cachednativetag(strDocPath, nativetags);
#endif
			if(bNeedRead)
			{
				read_tag_using_native_sdk(pdDoc, nativetags);
				if (nativetags.size() > 0)
				{
					cache_nativetag(strDocPath, nativetags);
					CELOG_LOG(CELOG_DEBUG, L"Cached native tags, path: %s, tag count: %d\n", strDocPath.c_str(), nativetags.size());
				}
			}
		}

		//如果pddoc有效，并且文件不是SE文件，我们就用pddoc去读tag
		if ( (pdDoc != NULL && (boost::algorithm::istarts_with(strDocPath, L"http://") || boost::algorithm::istarts_with(strDocPath, L"https://"))) 
			|| (_wcsicmp(strDocPath.c_str(), file.c_str()) == 0 && (emNotEncrytFile==res||emEncryptError==res) ))
		{
			read_tag_using_native_sdk(pdDoc,tags);

			if (tags.empty())
			{//failed to read tags with native sdk (or no tags), then try to read tags from cache.
				get_cachedtag(file, tags);
			}
			else
			{
				if ( !bPCReadTag )
				{
					pair<wstring, wstring> tag;
					tag.first = g_strCustomAttr;
					tag.second = L"yes";

					bool bFound = false;
					for(vector<pair<wstring, wstring>>::iterator itr = tags.begin(); itr != tags.end(); itr++)
					{
						if (_wcsicmp(itr->first.c_str(), g_strCustomAttr.c_str()) == 0)
						{
							itr->second = L"yes";
							bFound = true;
							break;
						}
					}

					if (!bFound)
					{
						tags.push_back(tag);
					}				
				}

				//cache the tag
				cache_tag(file, tags);
			}
		}
		else
		{
			//try to read tags from cache first.
			if (!get_cachedtag(file, tags))
			{
				read_tag_using_resattrmgr(file,tags);

				if (tags.size() > 0)
				{
					if ( !bPCReadTag )
					{
						pair<wstring, wstring> tag;
						tag.first = g_strCustomAttr;
						tag.second = L"yes";
						
						bool bFound = false;
						for(vector<pair<wstring, wstring>>::iterator itr = tags.begin(); itr != tags.end(); itr++)
						{
							if (_wcsicmp(itr->first.c_str(), g_strCustomAttr.c_str()) == 0)
							{
								itr->second = L"yes";
								bFound = true;
								break;
							}
						}

						if (!bFound)
						{
							tags.push_back(tag);
						}
					}

					//cache the tag
					cache_tag(file, tags);
				}
			}
		}

		
		if (bPCReadTag)
		{
			for (vector<pair<wstring, wstring>>::iterator itr = tags.begin(); itr != tags.end(); itr++)
			{
				if (_wcsicmp(itr->first.c_str(), g_strCustomAttr.c_str()) == 0)
				{
					itr->second = L"no";
					break;
				}
				
				
			}
		}
		CELOG_LOG(CELOG_DEBUG, L"file path: %s, pd path: %s, tags: %d\n", file.c_str(), strDocPath.c_str(), tags.size());
	}


	int add_tag_using_resattrmgr(_In_ const wstring& strPath,
		_In_ const vector<pair<wstring,wstring>>& vecTagPair, BOOL NTFSStream = FALSE )
	{
		if (vecTagPair.size()==0)
		{
			return 0;
		}
		if(!PathFileExists(strPath.c_str()))
		{
			return 1;
		}
		if (!m_bIsGetFunSuc)
		{
			return 1;
		}
		ResourceAttributeManager *mgr = NULL;
		m_lfCreateAttributeManager(&mgr);
		if(!mgr)
		{
			CELOG_LOG(CELOG_DEBUG, L"CreateAttributeManager fail!\n");
			return 1;
		}
		ResourceAttributes *attrs;
		m_lfAllocAttributes(&attrs);
		if(!attrs)
		{
			CELOG_LOG(CELOG_DEBUG, L"AllocAttributes fail!\n");
			m_lfCloseAttributeManager(mgr);
			return 1;
		}

		vector<pair<wstring, wstring>>::const_iterator itr;
		for(itr = vecTagPair.begin(); itr != vecTagPair.end(); itr++)
		{
			m_lfAddAttributeW(attrs, (*itr).first.c_str(), (*itr).second.c_str());
		}
		
		int nRet = 0;

		if ( NTFSStream )
		{
			nRet = m_lfWriteResourceAttributesForNTFSW(mgr, strPath.c_str(), attrs);
		}
		else
		{
			nRet = m_lfWriteResourceAttributesW(mgr, strPath.c_str(), attrs);
		}

		m_lfFreeAttributes(attrs);
		m_lfCloseAttributeManager(mgr);

		CELOG_LOG(CELOG_DEBUG, L"add_tag_using_resattrmgr %s ok\n",strPath.c_str());
		return nRet;
	}

	void cache_tag(const wstring& strFilePath, TAGS& tags)
	{
		boost_unique_lock lockWriter(g_mutex);  
		m_mapTagCache[strFilePath] = tags;
	}

	void cache_nativetag(const wstring& strFilePath, TAGS& tags)
	{
		boost_unique_lock lockWriter(g_mutex);  
		m_mapNativeTagCache[strFilePath] = tags;
	}


	void remove_cachedtag(const wstring& strFilePath)
	{
		boost_unique_lock lockWriter(g_mutex);  
		TAGSCACHE::const_iterator itr = m_mapTagCache.find(strFilePath);
		if (itr != m_mapTagCache.end())
		{
			m_mapTagCache.erase(itr);
		}
	}

	void remove_cachednativetag(const wstring& strFilePath)
	{
		boost_unique_lock lockWriter(g_mutex);  
		TAGSCACHE::const_iterator itr = m_mapNativeTagCache.find(strFilePath);
		if (itr != m_mapNativeTagCache.end())
		{
			m_mapNativeTagCache.erase(itr);
		}
	}

	void parse_oldformattag(const wstring& tagValue, vector<pair<wstring, wstring>>& tags)//compatible with 5.5
	{
		CELOG_LOG(CELOG_DEBUG, L"try to load pdflib.dll to parse 5.5 tags\n");
		string temp = MyWideCharToMultipleByte(tagValue);
#ifdef _WIN64
		HMODULE hMod = ::GetModuleHandleW(L"pdflib.dll");
#else
		HMODULE hMod = ::GetModuleHandleW(L"pdflib32.dll");
#endif
		if (hMod == NULL)
		{
			wstring dir = GetCommonComponentsDir();
#ifdef _WIN64
			dir.append(L"\\pdflib.dll");
#else
			dir.append(L"\\pdflib32.dll");
#endif

			hMod = ::LoadLibraryW(dir.c_str());
		}

		if (hMod != NULL)
		{
			PDF_ParseTagsType fnPDF_ParseTags = (PDF_ParseTagsType)GetProcAddress(hMod, "PDF_ParseTags");
			PDF_FreeMemType fnPDF_FreeMem = (PDF_FreeMemType)GetProcAddress(hMod, "PDF_FreeMem");
			if (fnPDF_ParseTags && fnPDF_FreeMem)
			{
				CELOG_LOG(CELOG_DEBUG, L"Start to parse with pdflib.dll\n");

				TAG* pTags = NULL;
				int nCount = 0;
				if(fnPDF_ParseTags(temp.c_str(), (void**)&pTags, &nCount) == 0 && nCount > 0 && pTags != NULL)
				{
					for (int i = 0; i < nCount; i++)
					{
						pair<wstring, wstring> tag;
						tag.first = MyMultipleByteToWideChar(string(pTags[i].szTagName));
						tag.second = MyMultipleByteToWideChar(string(pTags[i].szTagValue));

						tags.push_back(tag);
					}

					CELOG_LOG(CELOG_DEBUG, L"parsed %d tags\n", nCount);

					fnPDF_FreeMem(pTags);

				}
			}
		}

		
	}

	bool get_cachedtag(const wstring& strFilePath, TAGS& tags)
	{
		boost_share_lock lockReader(g_mutex);
		TAGSCACHE::const_iterator itr = m_mapTagCache.find(strFilePath);

		bool ret = false;
		if (itr != m_mapTagCache.end())
		{
			tags = itr->second;
			ret = true;
		}

		return ret;
	}

	bool get_cachednativetag(const wstring& strFilePath, TAGS& tags)
	{
		boost_share_lock lockReader(g_mutex);
		TAGSCACHE::const_iterator itr = m_mapNativeTagCache.find(strFilePath);

		bool ret = false;
		if (itr != m_mapNativeTagCache.end())
		{
			tags = itr->second;
			ret = true;
		}

		return ret;
	}

	void add_tag_using_native_sdk(_In_ const vector<pair<wstring,wstring>>& tags,const PDDoc doc)
	{
#if ACRO_SDK_LEVEL==0x000A0000	//for acrobat
		if (!doc)
		{
			return;
		}
		for (DWORD i=0;i<tags.size();i++)
		{
			CELOG_LOG(CELOG_DEBUG, L"Acrobat PEP tag NON-SE file using native SDK\n");
			string tagname(tags[i].first.begin(),tags[i].first.end());
			string tagvalue(tags[i].second.begin(),tags[i].second.end());
			PDDocSetInfo(doc, tagname.c_str(), tagvalue.c_str(), tagvalue.length());
		}
#endif
	}
private:
	void read_tag_using_native_sdk(PDDoc pdDoc,vector<pair<wstring,wstring>>& tags)
	{
		CELOG_LOG(CELOG_DEBUG, L"read_tag_using_native_sdk\n");

		if (!m_lfAllocAttributes||
			!m_lfAddAttributeW||
			!m_lfGetAttributeCount||
			!m_lfGetAttributeName||
			!m_lfGetAttributeValue||
			!m_lfFreeAttributes||
			!m_lfConvert_PC_2_RAW_For_Non_Office)
		{
			CELOG_LOG(CELOG_DEBUG,L"read_tag_using_native_sdk fail\n");
			return;
		}

		if (pdDoc)
		{
			vector<pair<wstring,wstring>> raw_tags;
			getTagsFromPDDoc(pdDoc,raw_tags);
			
			for (DWORD i=0;i<raw_tags.size();i++)
			{
				raw_tags[i].second = trimBeginEndSpace(raw_tags[i].second);
			}

			//translate raw tag to PC tag
			ResourceAttributes* raw_attrs=NULL;
			ResourceAttributes* pc_attrs=NULL;
			m_lfAllocAttributes(&raw_attrs);
			m_lfAllocAttributes(&pc_attrs);
			if (!raw_attrs || !pc_attrs)
			{
				//allocate attributes fail, unexpected error
				tags=raw_tags;
				
				CELOG_LOG(CELOG_DEBUG, L"allocate attributes fail, unexpected error\n");
				return;
			}
			for (DWORD i=0;i<raw_tags.size();i++)
			{
				m_lfAddAttributeW(raw_attrs,raw_tags[i].first.c_str(),raw_tags[i].second.c_str());
			}
			m_lfConvert_PC_2_RAW_For_Non_Office(raw_attrs,pc_attrs);
			for (LONG j=0;j<m_lfGetAttributeCount(pc_attrs);j++)
			{
				wstring tagname = (WCHAR *)m_lfGetAttributeName(pc_attrs, j);
				wstring tagvalue = (WCHAR *)m_lfGetAttributeValue(pc_attrs, j);
				
				vector<pair<wstring, wstring>> parsedTags;
				if (_wcsicmp(tagname.c_str(), g_strNlTag.c_str()) == 0)//this is possible tagged in 5.5, then we need to call pdflib to parse it
				{
					parse_oldformattag(tagvalue, parsedTags);
				}

				if (!parsedTags.empty())
				{// this is 5.5 tag
					for (vector<pair<wstring, wstring>>::iterator itr = parsedTags.begin(); itr != parsedTags.end(); itr++)
					{
						tags.push_back(*itr);
					}
				}
				else// this is not 5.5 tag, then show it directly
					tags.push_back(pair<wstring,wstring>(tagname,tagvalue));
			}
			m_lfFreeAttributes(raw_attrs);
			m_lfFreeAttributes(pc_attrs);
			return;
		}
	}

	//read tag using nextlabs tag library
	void read_tag_using_resattrmgr(const wstring&file, vector<pair<wstring,wstring>>& tags, BOOL NTFSStream = FALSE )
	{
		CELOG_LOG(CELOG_DEBUG,L"read_tag_using_resattrmgr\n");

		if (!m_bIsGetFunSuc)
		{
			return;
		}
		if(!PathFileExistsW(file.c_str()))
		{
			return;
		}
		ResourceAttributeManager *mgr = NULL;
		m_lfCreateAttributeManager(&mgr);
		if(!mgr)
		{
			CELOG_LOG(CELOG_DEBUG, L"CreateAttributeManager fail!\n");
			return;
		}
		ResourceAttributes *attrs;
		m_lfAllocAttributes(&attrs);
		if(!attrs)
		{
			CELOG_LOG(CELOG_DEBUG, L"AllocAttributes fail!\n");
			m_lfCloseAttributeManager(mgr);
			return;
		}

		int nRet = 0;

		if ( NTFSStream )
		{
			nRet = m_lfReadResourceAttributesForNTFSW(mgr, file.c_str(), attrs);
		}
		else
		{
			nRet = m_lfReadResourceAttributesW(mgr, file.c_str(), attrs);
		}

		if(!nRet)
		{
			CELOG_LOG(CELOG_DEBUG, L"ReadResourceAttributesW fail!\n");
			m_lfFreeAttributes(attrs);
			m_lfCloseAttributeManager(mgr);
			return;
		}
		else
		{
			int size = m_lfGetAttributeCount(attrs);
			if (size == 0)
			{
				CELOG_LOG(CELOG_DEBUG, L"GetAttributeCount count is 0\n");
				m_lfFreeAttributes(attrs);
				m_lfCloseAttributeManager(mgr);
				return;
			}
			else
			{
				for (int i = 0; i < size; ++i)
				{	
					wstring tagname = (WCHAR *)m_lfGetAttributeName(attrs, i);
					wstring tagvalue = (WCHAR *)m_lfGetAttributeValue(attrs, i);
					tags.push_back(pair<wstring,wstring>(tagname,tagvalue));
				}
			}
		}

		m_lfFreeAttributes(attrs);
		m_lfCloseAttributeManager(mgr);

		return;
	}

	static std::wstring trimBeginEndSpace(const std::wstring& s)
	{
		const std::wstring drop = L" ";
		std::wstring temp = s;
		temp.erase(temp.find_last_not_of(drop) + 1);
		return temp.erase(0, temp.find_first_not_of(drop));
	}

	static ACCB1 ASBool ACCB2 EnumDictionaries (CosObj keyObj, CosObj value, void* clientData)
	{
		ASTCount count = 0;
		char* sz_name = NULL;
		char* sz_value = NULL;

		try
		{
			sz_name=CosCopyNameStringValue(keyObj, &count);
			sz_value=CosCopyStringValue(value, &count);

			CELOG_LOGA(CELOG_DEBUG, "tag name:%s,value:%s\n", sz_name, sz_value);

			vector<pair<wstring,wstring>>* wtags=(vector<pair<wstring,wstring>>*)clientData;
			string strName(sz_name);
			string strValue(sz_value);
			wstring wstrName(strName.begin(),strName.end());
			wstring wstrValue(strValue.begin(),strValue.end());
			wtags->push_back(pair<wstring,wstring>(wstrName,wstrValue));
		}
		catch (...)
		{

		}

		if(sz_name != NULL)
		{
			ASfree(sz_name);
		}

		if(sz_value != NULL)
		{
			ASfree(sz_value);
		}

		return true;
	}
	//用于从PDDoc读取文件的tags
	bool getTagsFromPDDoc(PDDoc pdDoc,vector<pair<wstring,wstring>>& wtags)
	{
		if (m_myCosObjEnumProcCB==NULL || pdDoc == NULL)
		{
			return false;
		}

		CosDoc cosdoc=PDDocGetCosDoc(pdDoc);
		if (cosdoc)
		{
			try
			{
				CosObj cosobj = CosDocGetInfoDict(cosdoc);
				CosObjEnum (cosobj, m_myCosObjEnumProcCB, &wtags);
			}
			catch(...)
			{
				CELOG_LOGA(CELOG_DEBUG, "read tag with native sdk, exception\n");
			}
		}
		return true;
	}

	CTag()
	{
		memset(m_strlog,0,sizeof(m_strlog));
		m_hLib = NULL;
		m_hMgr = NULL;
		GetTagDllHandle();
		GetTagFunAddr();

		//初始化myCosObjEnumProcCB，用于根据CosDoc枚举所有的tag
		m_myCosObjEnumProcCB = ASCallbackCreateProto(CosObjEnumProc, &EnumDictionaries);	
	}
	~CTag()
	{
		if (m_hLib != NULL)
		{
			FreeLibrary(m_hLib);
			m_hLib = NULL;
		}
		if (m_hMgr != NULL)
		{
			FreeLibrary(m_hMgr);
			m_hMgr = NULL;
		}
	}
	bool GetTagDllHandle()
	{
		std::wstring strCommonPath = GetCommonComponentsDir() ;
#ifdef _WIN64
		wstring strLib = strCommonPath + L"\\resattrlib.dll";
		wstring strMgr = strCommonPath + L"\\resattrmgr.dll";
#else
		wstring strLib = strCommonPath + L"\\resattrlib32.dll";
		wstring strMgr = strCommonPath + L"\\resattrmgr32.dll";
#endif
		SetDllDirectoryW(strCommonPath.c_str());
		m_hLib = LoadLibrary(strLib.c_str());
		if (m_hLib == NULL)
		{
			return false;
		}
		m_hMgr = LoadLibrary(strMgr.c_str());
		if (m_hMgr == NULL)
		{
			return false;
		}
		return true;
	}
	void GetTagFunAddr()
	{
		m_lfCreateAttributeManager = (CreateAttributeManagerType)GetProcAddress(m_hMgr, "CreateAttributeManager");
		m_lfAllocAttributes = (AllocAttributesType)GetProcAddress(m_hLib, "AllocAttributes");
		m_lfReadResourceAttributesW = (ReadResourceAttributesWType)GetProcAddress(m_hMgr, "ReadResourceAttributesW");
		m_lfReadResourceAttributesForNTFSW = (ReadResourceAttributesForNTFSWType)GetProcAddress(m_hMgr, "ReadResourceAttributesForNTFSW");
		m_lfGetAttributeCount = (GetAttributeCountType)GetProcAddress(m_hLib, "GetAttributeCount");
		m_lfFreeAttributes = (FreeAttributesType)GetProcAddress(m_hLib, "FreeAttributes");
		m_lfCloseAttributeManager = (CloseAttributeManagerType)GetProcAddress(m_hMgr, "CloseAttributeManager");
		m_lfAddAttributeW = (AddAttributeWType)GetProcAddress(m_hLib, "AddAttributeW");
		m_lfGetAttributeName = (GetAttributeNameType)GetProcAddress(m_hLib, "GetAttributeName");
		m_lfGetAttributeValue = (GetAttributeValueType)GetProcAddress(m_hLib, "GetAttributeValue");
		m_lfWriteResourceAttributesW = (WriteResourceAttributesWType)GetProcAddress(m_hMgr, "WriteResourceAttributesW");
		m_lfWriteResourceAttributesForNTFSW = (WriteResourceAttributesForNTFSWType)GetProcAddress(m_hMgr, "WriteResourceAttributesForNTFSW");
		m_lfRemoveResourceAttributesW = (RemoveResourceAttributesWType)GetProcAddress(m_hMgr, "RemoveResourceAttributesW");
		m_lfConvert_PC_2_RAW_For_Non_Office=(Convert_PC_2_RAW_For_Non_Office_Type)GetProcAddress(m_hMgr, "Convert_PC_2_RAW_For_Non_Office");
		m_lfConvert_Raw_2_PC_For_Non_Office=(Convert_Raw_2_PC_For_Non_Office_Type)GetProcAddress(m_hMgr, "Convert_Raw_2_PC_For_Non_Office");


		if( !(m_lfCreateAttributeManager && m_lfAllocAttributes &&
			m_lfReadResourceAttributesW && m_lfReadResourceAttributesForNTFSW && m_lfGetAttributeCount &&
			m_lfFreeAttributes && m_lfCloseAttributeManager && m_lfAddAttributeW &&
			m_lfGetAttributeName && m_lfGetAttributeValue &&
			m_lfWriteResourceAttributesW&& m_lfWriteResourceAttributesForNTFSW && 
			m_lfRemoveResourceAttributesW && m_lfConvert_PC_2_RAW_For_Non_Office && m_lfConvert_Raw_2_PC_For_Non_Office) )
		{
			CELOG_LOG(CELOG_DEBUG, L"failed to get resattrlib/resattrmgr functions\r\n");
			m_bIsGetFunSuc = false;
			return ;
		}
		m_bIsGetFunSuc = true;
	}
	HMODULE							m_hLib;
	HMODULE							m_hMgr;
	CreateAttributeManagerType			m_lfCreateAttributeManager;
	AllocAttributesType				m_lfAllocAttributes;
	ReadResourceAttributesWType		m_lfReadResourceAttributesW;
	ReadResourceAttributesForNTFSWType m_lfReadResourceAttributesForNTFSW;
	GetAttributeCountType				m_lfGetAttributeCount;
	FreeAttributesType					m_lfFreeAttributes;
	CloseAttributeManagerType			m_lfCloseAttributeManager;
	AddAttributeWType					m_lfAddAttributeW;
	GetAttributeNameType				m_lfGetAttributeName;
	GetAttributeValueType				m_lfGetAttributeValue;		
	WriteResourceAttributesWType		m_lfWriteResourceAttributesW;
	RemoveResourceAttributesWType		m_lfRemoveResourceAttributesW;
	Convert_PC_2_RAW_For_Non_Office_Type m_lfConvert_PC_2_RAW_For_Non_Office;
	Convert_Raw_2_PC_For_Non_Office_Type m_lfConvert_Raw_2_PC_For_Non_Office;
	WriteResourceAttributesForNTFSWType m_lfWriteResourceAttributesForNTFSW;


	bool								m_bIsGetFunSuc;
	wchar_t							m_strlog[1024];
	
	//用于从CosDict对象中枚举所有tag的回调函数
	CosObjEnumProc m_myCosObjEnumProcCB;

	TAGSCACHE m_mapTagCache;
	TAGSCACHE m_mapNativeTagCache;
};