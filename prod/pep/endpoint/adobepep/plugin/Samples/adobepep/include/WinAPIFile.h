#pragma once
#include <windows.h>
#include <string>
#include <vector>
using namespace std;

#include "celog.h"

#define CELOG_CUR_MODULE L"ADOBEPEP"
#undef CELOG_CUR_FILE
#define CELOG_CUR_FILE   CELOG_FILEPATH_PROD_PEP_ENDPOINT_ADOBEPEP_PLUGIN_SAMPLESADOBEPEP_INCLUDE_WINAPIFILE_H

class CWinAPIFile
{
public:
	static CWinAPIFile* GetInstance()
	{
		static CWinAPIFile ins;
		return &ins;
	}

	bool AddFileCreated(const string& file)
	{
		EnterCriticalSection(&m_cs_created);
		vector<STRUCT_CREATE_FILE>::iterator it = m_file_created.begin();
		for (; it != m_file_created.end(); it++)
		{
			if (it->strFilepath == file)
			{
				//文件已经在了，删掉原来的，确保vector里面的item是根据时间来排序的，先被创建的文件在前面，后被创建的文件在后面，这样做是有原因的，
				//如果想要改变这一点，一定要先了解清楚原因，该原因是为了能够根据临时文件找到原来的非临时文件，这个临时文件是add attachment的时候adobe创建的，
				//adobe会将原来的文件的数据读出来然后写到该临时文件里，然后adobe将临时文件add到host文件作为attachment
				m_file_created.erase(it);
				break;
			}
		}

		//增加新的item
		STRUCT_CREATE_FILE file_created;
		file_created.strFilepath = file;
		file_created.dwTime = ::GetTickCount();
		m_file_created.push_back(file_created);

		LeaveCriticalSection(&m_cs_created);
		return true;
	}

	bool GetOriginFileByTempFile(const string& temp, string& originFile)
	{
		bool res = false;
		EnterCriticalSection(&m_cs_created);
		vector<STRUCT_CREATE_FILE>::iterator it = m_file_created.end();
		//倒退找临时文件
		it--;
		for (; it != m_file_created.begin(); it--)
		{
			if (it->strFilepath == temp)
			{
				//找到了临时文件，根据临时文件的文件名，找同名的，之前被CreateFileW的文件
				//先计算文件名
				string::size_type pos = temp.rfind(L'\\');
				if (pos != string::npos)
				{
					string filename = temp.substr(pos + 1, temp.length() - pos -1);
					//倒退找同名的原文件的路径
					while( (--it) != m_file_created.begin())
					{
						pos = it->strFilepath.find(filename);
						if (pos != string::npos)
						{
							//找到了，函数返回
							originFile = it->strFilepath;
							res = true;
							
							CELOG_LOGA(CELOG_DEBUG, "find origin file by temp file name in GetOriginFileByTempFile, %s\n", originFile.c_str());
							goto exit;
						}
						//继续倒退找原文件
					}
					//发生了未知错误，没有找到同名的原文件
					
					CELOG_LOG(CELOG_DEBUG, L"can't find origin file by temp file name in GetOriginFileByTempFile\n");
					goto exit;
				}
				else
				{
					//发生了未知错误
					CELOG_LOG(CELOG_DEBUG, L"can't find filename by temp file name in GetOriginFileByTempFile\n");
					goto exit;
				}
			}
			//继续找临时文件
		}

		if (res==false)
		{
			CELOG_LOG(CELOG_DEBUG, L"return false in GetOriginFileByTempFile\n");
		}

exit:
		LeaveCriticalSection(&m_cs_created);
		return res;
	}
private:

	CRITICAL_SECTION m_cs_created;

	typedef struct 
	{
		string strFilepath;
		DWORD dwTime;
	}STRUCT_CREATE_FILE;
	vector<STRUCT_CREATE_FILE> m_file_created;//被CreateFileW打开的文件,同一个文件（路径完全相同）只有一个item在vector里面
private:
	CWinAPIFile(void);
	~CWinAPIFile(void);
};
