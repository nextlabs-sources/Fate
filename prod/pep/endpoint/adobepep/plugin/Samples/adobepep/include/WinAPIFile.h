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
				//�ļ��Ѿ����ˣ�ɾ��ԭ���ģ�ȷ��vector�����item�Ǹ���ʱ��������ģ��ȱ��������ļ���ǰ�棬�󱻴������ļ��ں��棬����������ԭ��ģ�
				//�����Ҫ�ı���һ�㣬һ��Ҫ���˽����ԭ�򣬸�ԭ����Ϊ���ܹ�������ʱ�ļ��ҵ�ԭ���ķ���ʱ�ļ��������ʱ�ļ���add attachment��ʱ��adobe�����ģ�
				//adobe�Ὣԭ�����ļ������ݶ�����Ȼ��д������ʱ�ļ��Ȼ��adobe����ʱ�ļ�add��host�ļ���Ϊattachment
				m_file_created.erase(it);
				break;
			}
		}

		//�����µ�item
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
		//��������ʱ�ļ�
		it--;
		for (; it != m_file_created.begin(); it--)
		{
			if (it->strFilepath == temp)
			{
				//�ҵ�����ʱ�ļ���������ʱ�ļ����ļ�������ͬ���ģ�֮ǰ��CreateFileW���ļ�
				//�ȼ����ļ���
				string::size_type pos = temp.rfind(L'\\');
				if (pos != string::npos)
				{
					string filename = temp.substr(pos + 1, temp.length() - pos -1);
					//������ͬ����ԭ�ļ���·��
					while( (--it) != m_file_created.begin())
					{
						pos = it->strFilepath.find(filename);
						if (pos != string::npos)
						{
							//�ҵ��ˣ���������
							originFile = it->strFilepath;
							res = true;
							
							CELOG_LOGA(CELOG_DEBUG, "find origin file by temp file name in GetOriginFileByTempFile, %s\n", originFile.c_str());
							goto exit;
						}
						//����������ԭ�ļ�
					}
					//������δ֪����û���ҵ�ͬ����ԭ�ļ�
					
					CELOG_LOG(CELOG_DEBUG, L"can't find origin file by temp file name in GetOriginFileByTempFile\n");
					goto exit;
				}
				else
				{
					//������δ֪����
					CELOG_LOG(CELOG_DEBUG, L"can't find filename by temp file name in GetOriginFileByTempFile\n");
					goto exit;
				}
			}
			//��������ʱ�ļ�
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
	vector<STRUCT_CREATE_FILE> m_file_created;//��CreateFileW�򿪵��ļ�,ͬһ���ļ���·����ȫ��ͬ��ֻ��һ��item��vector����
private:
	CWinAPIFile(void);
	~CWinAPIFile(void);
};
