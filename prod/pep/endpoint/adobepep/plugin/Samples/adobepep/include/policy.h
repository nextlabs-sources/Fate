#pragma once

#include <string>
#include <vector>
using namespace std;


#pragma warning(push)
#pragma warning(disable:6334 6011 4996 4189 4100 4819)
#include "boost\unordered_map.hpp"
#include "boost\shared_ptr.hpp"
#include "boost\weak_ptr.hpp"
#include "boost\format.hpp"
#pragma warning(pop)

#include "comm_helper.hpp"
#include "evalCache.h"
#include "obMgr.h"
#include "nltag.h"
#include "Inheritance.h"
#include "Encrypt.h"
#include "OverLay.h"
#include "celog.h"
#include "strsafe.h"

#define CELOG_CUR_MODULE L"ADOBEPEP"
#undef CELOG_CUR_FILE
#define CELOG_CUR_FILE   CELOG_FILEPATH_PROD_PEP_ENDPOINT_ADOBEPEP_PLUGIN_SAMPLESADOBEPEP_INCLUDE_POLICY_H

extern boost::shared_mutex g_mFilesSEStatus;
extern boost::unordered_map<string, BOOL> gFilesSEStatus;

class CPolicy
{
private:
	CPolicy(void)
	{
		m_bInit=false;
	}
	virtual ~CPolicy(void)
	{
		nextlabs::comm_helper::Release_Cesdk(&m_context);
	}
	void init()
	{
		if (!m_bInit)
		{
			m_bInit=true;
			nextlabs::comm_helper::Init_Cesdk(&m_context);//you only need to initialize one time.
			m_ptr = nextlabs::comm_helper::CreateComm(nextlabs::type_cesdk, &m_context);
		}
	}

public:
	static CPolicy* GetInstance()
	{
		static CPolicy ins;
		ins.init();
		return &ins;
	}
	
	//对比如share point的文件查询策略
	void queryHttpSource(const char* path,PDDoc pdDoc,bool& bdeny,const string& action,bool force_resattrmgr=false,bool force_query_pc=false,_CENoiseLevel_t EmNoise = CE_NOISE_LEVEL_USER_ACTION )
	{
		if (force_resattrmgr==false && pdDoc==NULL)
		{
			return;
		}

		if (false==IsURLPath(path))
		{
			return;
		}
		

		CELOG_LOG(CELOG_DEBUG, L"this is URL file\n");

		//先读source tag
		CTag* tag_ins=CTag::GetInstance();
		string strSrc(path);
		wstring wstrSrc = MyMultipleByteToWideChar(strSrc);
		vector<pair<wstring,wstring>> wtags_src;
		tag_ins->read_tag(wstrSrc,wtags_src,pdDoc);

		string unused;
		CPolicy* ins_policy=CPolicy::GetInstance();

		nextlabs::Obligations obs;
		ins_policy->query(action, path, unused, bdeny, obs,&wtags_src,
			NULL,
			NULL,
			NULL,
			NULL,
			force_query_pc,false,false,EmNoise);
	}

	//这个函数自己会去读source的tag，这个source可以是sharepoint文件，也可以是本地的文件
	//这个函数会返回给用户是否被deny，
	//如果被allow，这个函数会自己去检查source文件的tag和encryption的状态，
	//这个函数还会去检查enforcement是否有policy obligation，
	//最后，会结合inheritance和policy obligation的情况，告诉调用者，是否要对dest做encryption的obligation，以及对dest是否有tag的obligation。
	//这个函数还可能会需要用户传入pdDoc，这个参数在当source是sharepoint文件的时候，要被使用到，用来读source的tag
	void QueryCopy_Get_Obligation_Inheritance(_In_ const char* src,
											_In_ const char* dest,
											_Out_ bool& bdeny,
											_Out_ vector<pair<wstring,wstring>>& w_obligation_tags_dest, 
											_Out_ bool& bDest_Encrypt,
											PDDoc pdDoc = NULL, bool bDisplayDest = false,_CENoiseLevel_t EmNoise = CE_NOISE_LEVEL_USER_ACTION )
	{
		string strSrc(src);

		//先读source tag
		CTag* tag_ins=CTag::GetInstance();
		wstring wstrSrc = MyMultipleByteToWideChar(strSrc);
		vector<pair<wstring,wstring>> wtags_src;
		tag_ins->read_tag(wstrSrc,wtags_src,pdDoc);

		//query policy with file path and tags and get tag obligation for dest
		string strDest(dest);
		CPolicy* ins_policy=CPolicy::GetInstance();
		nextlabs::Obligations obs;
		ins_policy->query("COPY",strSrc,strDest,bdeny,obs,&wtags_src,NULL,NULL,&w_obligation_tags_dest,&bDest_Encrypt,false,false,bDisplayDest,EmNoise);

		//我们要做inheritance，所以要先把source的tag和dest tag obligation合并
		if(false==boost::algorithm::iends_with(dest,".pdf"))
		{
			//如果是pdf save as成pdf，我们不需要merge tag，因为把一个pdf save as成另一个pdf的时候，tag已经自动的inheritance了
			CInheritance::mergeSourceTagToDestObligationTags(wtags_src,w_obligation_tags_dest);
		}

		if (bDest_Encrypt != true)
		{
			wstring wstrEncryptDirectory = MyMultipleByteToWideChar(strDest);

			size_t nFind = wstrEncryptDirectory.rfind('\\');

			if (nFind !=  std::wstring::npos)
			{
				wstrEncryptDirectory.replace(nFind, std::wstring::npos, L"\\");
			}
			else if(wstrEncryptDirectory.rfind('/') != std::wstring::npos)
			{
				wstrEncryptDirectory.replace(wstrEncryptDirectory.rfind('/'), std::wstring::npos, L"/");
			}
			
			EncryptRetValueType ret = CEncrypt::Encrypt(wstrEncryptDirectory, false, true);
			if (ret == emIsEncrytFile)
			{
				bDest_Encrypt = true;
			}
		}

		//我们要做inheritance,所以要合并是否加密的状态
		string sourceFile(src);
		wstring wcurrent_pdf = MyMultipleByteToWideChar(sourceFile);
		bool bSrcEncrypted= (emIsEncrytFile==CEncrypt::Encrypt(wcurrent_pdf,false,true)) ? true:false;
		CInheritance::mergeSourceEncryptionToDest(bSrcEncrypted,bDest_Encrypt);

		if (bDest_Encrypt)
		{
			CELOG_LOG(CELOG_DEBUG, L"consider both inheritance and policy obligation, we need to encrypt destination file\n");
		}
		else
		{
			CELOG_LOG(CELOG_DEBUG, L"consider both inheritance and policy obligation, we don't need to encrypt destination file\n");
		}
	}

	//只针对本地的文件，如果是share point文件，直接返回allow
	//用户只要传入文件的路径，action的名字--这个函数不支持带destination的action，如COPY
	//然后函数会去读tag，然后返回enforcement的结果
	//对某些支持Tag obligation的action--比如OPEN，这个函数还会主动去打tag obligation，不需要用户去操心
	void queryLocalSourceAndDoTagObligation(const char* path,bool& bdeny,const string& action,PDDoc doc=NULL,_CENoiseLevel_t EmNoise = CE_NOISE_LEVEL_USER_ACTION, bool bIgnoreParseTag = false )
	{
		//只针对本地的文件，如果是share point文件，直接返回allow
		if (true==IsURLPath(path))
		{
			CELOG_LOG(CELOG_DEBUG, L"ignore URL in queryLocalSourceAndDoTagObligation\n");
			bdeny=false;
			return;
		}

		//过滤掉一些像acro00000001这样的名字
		string strPath(path);
		if (true==boost::algorithm::istarts_with(strPath,"Acro"))
		{
			bdeny=false;
			return;
		}
		//如果文件不存在，那就不用做query了，因为我们要做的query是针对OPEN，PRINT这样的action
		//在adobepep里，OPEN不负责监控新建文件，所以如果文件不存在，我们直接返回
		if ("OPEN"==action)
		{
			FILE *fp = fopen(path,"r");
			if( fp ) 
			{
				// exists
				fclose(fp);
			} 
			else 
			{
				// does not exist
				bdeny=false;
				return;
			}
		}

		//read tag
		CTag* tag_ins=CTag::GetInstance();
		string filepath(path);
		wstring wfilepath= MyMultipleByteToWideChar(filepath);
		vector<pair<wstring,wstring>> wtags_src;
		
		if (!doc)
		{
			getCurrentPDDoc(doc);
		}

		if ( 0 == _stricmp(action.c_str(), "DECRYPT") )
		{
			tag_ins->read_tag(wfilepath,wtags_src,doc,TRUE);
		}
		else
		{
			tag_ins->read_tag(wfilepath,wtags_src,doc);
		}
		
		//query policy with file path and tags
		string unused;
		vector<pair<wstring,wstring>> w_obligation_tags_src;
		CPolicy* ins_policy=CPolicy::GetInstance();
		nextlabs::Obligations obs;
		ins_policy->query(action.c_str(),path,unused,bdeny,obs,&wtags_src,&w_obligation_tags_src,NULL,NULL,NULL,false,bIgnoreParseTag,false,EmNoise);
		
		//do tag obligation 
		if (bdeny==false && w_obligation_tags_src.size()!=0)
		{
			tag_ins->add_tag_using_resattrmgr(wfilepath,w_obligation_tags_src);
		}
	}


	void queryEdit_GetObligation(const char* path,bool& bdeny,
		PDDoc pdDoc,
		_Out_ vector<pair<wstring,wstring>>& w_obligation_tags, 
		_Out_ bool& bDest_Encrypt,
		_Out_ nextlabs::Obligations& obs,
		bool bIgnoreParseTag)
	{
		string unused;

		//read tag
		CTag* tag_ins=CTag::GetInstance();
		vector<pair<wstring,wstring>> wtags_src;
		string strpath(path);
		wstring wstrpath = MyMultipleByteToWideChar(strpath);
		tag_ins->read_tag(wstrpath,wtags_src,pdDoc);

		//query policy with file path and tags
		vector<pair<wstring,wstring>> w_obligation_tags_src;
		CPolicy* ins_policy=CPolicy::GetInstance();
		ins_policy->query("EDIT",path,unused,bdeny,obs,&wtags_src,&w_obligation_tags,NULL,NULL,&bDest_Encrypt,true,bIgnoreParseTag);
	}

	void QueryObl(const string& action,const wstring &wstrFilePath,bool& bdeny,_Out_ nextlabs::Obligations& obs,PDDoc doc = NULL)
	{
		CPolicy* ins_policy=CPolicy::GetInstance();
		string unused;
		string path = MyWideCharToMultipleByte(wstrFilePath);

		//read tags and pass to PC via source/dest attributes.
		CTag* tag_ins=CTag::GetInstance();
		vector<pair<wstring,wstring>> wtags_src;
		if(doc == NULL)
			getCurrentPDDoc(doc);
		tag_ins->read_tag(wstrFilePath,wtags_src,doc);
		
		ins_policy->query(action,path,unused,bdeny,obs,&wtags_src,NULL,NULL,NULL,NULL,true);
	}


	void QueryAcrobatCom(_In_ const string& action,_In_ const string &strFilePath,_Out_ bool& bdeny,_In_ PDDoc doc = NULL,_In_ _CENoiseLevel_t EmNoise = CE_NOISE_LEVEL_USER_ACTION)
	{
		CPolicy* ins_policy=CPolicy::GetInstance();
		string unused;
		//read tags and pass to PC via source/dest attributes.
		CTag* tag_ins=CTag::GetInstance();
		vector<pair<wstring,wstring>> wtags_src;
		if(doc == NULL)
			getCurrentPDDoc(doc);
		wstring wfilepath = MyMultipleByteToWideChar(strFilePath);
		tag_ins->read_tag(wfilepath,wtags_src,doc);

		nextlabs::Obligations obs;
		ins_policy->query(action,strFilePath,unused,bdeny,obs,&wtags_src,NULL,NULL,NULL,NULL,true,false,false,EmNoise);
	}

	void query(const string& action, 
		const string& source, 
		const string& dest, 
		bool& bdeny, 
		nextlabs::Obligations& obs,//output, for overlay for now
		vector<pair<wstring,wstring>>* src_tags=NULL,
		vector<pair<wstring,wstring>>* src_oligation_tags=NULL,//output, tags required to tag to src file by obligation
		vector<pair<wstring,wstring>>* dest_tags=NULL,
		vector<pair<wstring,wstring>>* dest_oligation_tags=NULL,//output, tags required to tag to dest file by obligation
		bool* bDest_Encrypt=NULL,//output, require to do encryption obligation for destination file
		bool bForceQueryPC=false,//input, require to ignore cache
		bool bIgnoreParseTag=false,//input, require to ignore parsing tag
		bool bDisplayDest=false,//input, require to display destination name on PA
		_CENoiseLevel_t EmNoise = CE_NOISE_LEVEL_USER_ACTION) 
	{
		bdeny = false;

		//query cache
		CEvalCache* cache_ins=CEvalCache::getInstance();
		
		bool bCacheFound=false;
		bool bCacheAllow=false;
		cache_ins->query(source,dest,action,bCacheFound,bCacheAllow);
		if (bCacheFound)
		{
			bdeny=(bCacheAllow==true)? false:true;

			if (true==bdeny)
			{
				//this is deny
				//if the cache is deny, then always use the cache
				return;
			}

			//this is allow
			if (false==bForceQueryPC)
			{
				//if the cache is allow, then we need to see bForceQueryPC to determine whether we uses cache
				//we don't need to force query if bForceQueryPC is false
				//if bForceQueryPC is true, then we can't use cache
				return;
			}
		}

		//query PC
		nextlabs::eval_parms parm;

		wstring waction = MyMultipleByteToWideChar(action);
		wstring wsource = MyMultipleByteToWideChar(source);
		wstring wdest = MyMultipleByteToWideChar(dest);

		parm.SetAction(waction.c_str());

		nextlabs::ATTRS src_attr;
		nextlabs::ATTRS dest_attr;

		if (src_tags && src_tags->size()>0)
		{
			//set ce::nocache=yes, to force PC to not use evaluation cache.
			//and send PC tag from src_tags

			std::pair<std::wstring, std::wstring> TagPair(L"ce::nocache", L"yes");
			src_attr.insert(TagPair);

			for (DWORD i=0;i<src_tags->size();i++)
			{
				wstring _name=(*src_tags)[i].first;
				wstring _value=(*src_tags)[i].second;
				if (_name.length() && _value.length())
				{
					TagPair.first = _name;
					TagPair.second = _value;

					if (_wcsicmp(_name.c_str(), g_strCustomAttr.c_str()) == 0)
					{
						string temp = source;
						std::transform(temp.begin(), temp.end(), temp.begin(), ::tolower);

						boost_share_lock readLock(g_mFilesSEStatus);
						BOOL b = gFilesSEStatus[temp];
						if (b)
						{
							TagPair.second = L"no";
						}
						CELOG_LOGA(CELOG_DEBUG, "check if need PC read tags, %s, %d\n", temp.c_str(), b);
					}

					src_attr.insert(TagPair);

					CELOG_LOG(CELOG_DEBUG, L"src tag:%s=%s\n", _name.c_str(), TagPair.second.c_str());
					
				}
			}
		}
		parm.SetSrc(wsource.c_str(),L"fso",&src_attr);

		if (dest_tags)
		{
			for (DWORD i=0;i<dest_tags->size();i++)
			{
				wstring _name=(*dest_tags)[i].first;
				wstring _value=(*dest_tags)[i].second;
				if (_name.length() && _value.length())
				{
					std::pair<std::wstring, std::wstring> TagPair(_name, _value);

					dest_attr.insert(TagPair);
					
					CELOG_LOG(CELOG_DEBUG, L"tags added for dest file\n");
				}
			}
		}
		parm.SetTarget(wdest.c_str(),L"fso",&dest_attr);

		
		nextlabs::ATTRS app_attr;
		wstring app_attr_name(L"NextLabs Enforcer");
		wstring app_attr_value(L"Adobe PEP");

		std::pair<std::wstring, std::wstring> TagPair(app_attr_name, app_attr_value);

		app_attr.insert(TagPair);

		parm.SetApplicationAttrs(&app_attr);
		
		parm.SetNoiseLevel(EmNoise);

		if(m_ptr->Query(&parm))
		{
			bool b = m_ptr->IsDenied();
			bdeny=b;
			
			CELOG_LOGA(CELOG_DEBUG, "eval result\nsource:%s\ndest:%s\naction:%s\ndenied: %d\n", source.c_str(), dest.c_str(), action.c_str(), b);

			obs = m_ptr->GetObligations();

			//打印obligation的信息
			std::list<nextlabs::Obligation> listOb = obs.GetObligations();
			for (std::list<nextlabs::Obligation>::const_iterator itr = listOb.begin(); itr != listOb.end(); itr++)
			{
				CELOG_LOG(CELOG_DEBUG, L"obligation name: %s, policy name: %s\n", (*itr).name.c_str(), (*itr).policy.c_str());

				for(nextlabs::ObligationOptions::const_iterator itOp = (*itr).options.begin(); itOp != (*itr).options.end(); itOp++)
				{
					CELOG_LOG(CELOG_DEBUG, L"obligation content name: %s, value: %s\n", (*itOp).first.c_str(), (*itOp).second.c_str());
				}
                
                if (EmNoise == CE_NOISE_LEVEL_USER_ACTION)
                {
                    if( itr->name == g_szOB_RichUserMessage )
                    {
                        nextlabs::ObligationOptions::const_iterator options_it = itr->options.begin();

                        std::wstring FirstValue = options_it->second.c_str();
                        ++options_it;
                        std::wstring SecondValue = options_it->second.c_str();
                        CObMgr::ShowBubble(FirstValue.c_str(), 1000 * _wtoi(SecondValue.c_str()), waction.c_str());

                        break;
                    }
                }
			}

			// don't support file tag when save as to sharepoing or acrobatcom
			if( 0 == _stricmp(action.c_str(),"COPY") && 
				(IsURLPath(dest) || IsAcrobatcom(dest)) && 
				(CObMgr::CheckIfExistInteractiveFileTaggingOb(obs) || CObMgr::CheckIfExistAutomaticFileTaggingOb(obs)))
			{
				MessageBoxA(NULL,"You are required to classify this document. To classify a document stored on a remote server, you must save and classify the document to your local machine first, and then copy it to the remote server.","Nextlabs DLP Warning",MB_OK|MB_ICONWARNING);
				bdeny = true;
				return;
			}

			//解析tag obligation的信息
			vector<pair<wstring,wstring>> srcTag,dstTag;
			if (false==bIgnoreParseTag)
			{
				if (0 != _stricmp(action.c_str(),"SEND") && 0 != _stricmp(action.c_str(),"CONVERT"))
				{
					if (-1 == CObMgr::GetObTags(obs, srcTag, dstTag, wsource.c_str(), wdest.length()!=0?wdest.c_str():NULL, bDisplayDest))
					{
						bdeny = true;
						return;
					}
				}
			}

			//cache eval result
			cache_ins->cache(source,dest,action,(bdeny==true)? false:true);

			if (src_oligation_tags!=NULL)
			{
				CELOG_LOG(CELOG_DEBUG, L"try to get source tag obligation, [%d] tags obligations\n", srcTag.size());
				*src_oligation_tags=srcTag;
			}
			if (dest_oligation_tags!=NULL)
			{
				CELOG_LOG(CELOG_DEBUG, L"try to get dest tag obligation, [%d] tags obligations\n", dstTag.size());

				*dest_oligation_tags=dstTag;
			}

			//解析encryption obligation的信息
			if (bDest_Encrypt)
			{
				*bDest_Encrypt=CObMgr::CheckIfExistEncryptOb(obs);
			}
		}
		else
		{
			CELOG_LOGA(CELOG_DEBUG, "query failed: %d\n", m_ptr->GetLastError());
		}
	}
private:
	bool m_bInit;
	nextlabs::cesdk_context m_context;
	boost::shared_ptr<nextlabs::comm_base> m_ptr;
};
