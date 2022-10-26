#include "SaveAsObligation.h"
#include "celog.h"

extern AVTVersionNumPart MajorVersion;
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//����Ҫ��tag�ı�־��ͬʱ����tag
void CDoTag_SaveAs::setFlagAndObligationTags(const vector<pair<wstring,wstring>>& tags)
{
	m_bNeedDoTag=true;
	m_tags = tags;

	CELOG_LOGA(CELOG_DEBUG, "CDoTagBase::setFlagAndObligationTags\n");
}
//�ж�Ҫ��Ҫ��tag
bool CDoTag_SaveAs::getFlag()
{
	return m_bNeedDoTag;
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//��tag lib��tag
void CDoTag_SaveAs::DoTag_TagLib()
{
	CELOG_LOGA(CELOG_DEBUG, "CDoTagBase::DoTag, path: %s\n", m_file.c_str());

	CTag* ins_tag=CTag::GetInstance();
	wstring path(m_file.begin(),m_file.end());
	ins_tag->add_tag_using_resattrmgr(path,m_tags);
	//�����ڴ���tag��reset
	reset();
}



//��tag lib����tag�������溯����֮ͬ�������ṩ��һ�����������Ҳ�������
void CDoTag_SaveAs::DoTag_TagLib_2(const string& strPath)
{
	CTag* ins_tag=CTag::GetInstance();
	wstring path(strPath.begin(),strPath.end());
	ins_tag->add_tag_using_resattrmgr(path,m_tags);
}


void CDoTag_SaveAs::tag_opened_pdf_with_resattrmgr(PDDoc doc)
{
	//�������ڰ�pdf save as��pdf������������adobe reader��û��PDDocSetInfo�����ã�����
	//����Ҫ�Ȱѵ�ǰ��PDF�ļ���ģ���û�����˵����رյ�ǰ��PDF��Ȼ���ٴ�tag��Ȼ����ģ���û����ļ�
	//Ҫ�ҵ�file��handle���ܹص�handle���������ܴ�tag���������û���ҵ����Ͳ��ܴ�tag
	CELOG_LOG(CELOG_DEBUG, L"enters tag_opened_pdf_with_resattrmgr\n");
	ASFile file = PDDocGetFile(doc);
	if (file)
	{
		m_b_closed=false;

		string str_file_path;
		GetPathFromASFile(file,str_file_path);

		if (str_file_path.length())
		{

			CMenuItem* ins_menuitem=CMenuItem::GetInstance();
			ins_menuitem->execute_close();
			CELOG_LOGA(CELOG_DEBUG, "Close the file if the dest file was opened: %s, m_b_closed: %d\n", str_file_path.c_str(), m_b_closed);

#if ACRO_SDK_LEVEL==0x000A0000
#else
            if (10 == MajorVersion || 11 == MajorVersion)//For reader x, we will do tagging at fileclose
			{
				CELOG_LOG(CELOG_DEBUG, L"For Reader X, do tag in fileclose\n");
				return;
			}
#endif 
			if (false==m_b_closed)
			{
				real_asfileclose(file);
				CELOG_LOG(CELOG_DEBUG, L"close file handle after save as done\n");
			}


			DoTag_TagLib_2(str_file_path);

		}
	}
	else
	{
		//ʲôҲ��Ҫ��
	}

	reset();
}


//���Ѿ����˵�pdf��tag��acrobat��reader��ʵ�ֲ�ͬ
void CDoTag_SaveAs::DoTag_On_Opened_PDF(PDDoc doc)
{
#if ACRO_SDK_LEVEL==0x000A0000	//for acrobat
	CELOG_LOG(CELOG_DEBUG, L"CDoTagBase::DoTag_On_Opened_PDF, for acrobat\n");
	bool bTagged=false;
	if (m_tags.size()>0)
	{
		bTagged=true;
	}
	CTag* ins_tag=CTag::GetInstance();

	if (true==m_SE_Flag)
	{
		CELOG_LOG(CELOG_DEBUG, L"CDoTagBase::DoTag_On_Opened_PDF, for acrobat but SE file\n");

		tag_opened_pdf_with_resattrmgr(doc);
	}
	else
	{
		ins_tag->add_tag_using_native_sdk(m_tags,doc);

		//bDoTagAfterSave��ζ�ű�־�������ˣ�����Ҫ��tag��
		//�ڴ�tagǰ��Ҫ�ѱ�־���ã�
		//��Ϊ��������PDDocSetInfo��tag
		//�������tag����Ҫ�����ĵ���PDDocDoSave����סtag����������ñ�־�����Ǿͻ������ѭ��
		reset();

		if (bTagged==true)
		{
			AVDoc avDoc = AVDocFromPDDoc(doc);
			AVDocDoSave(avDoc);
		}
	}


#else
	CELOG_LOG(CELOG_DEBUG, L"CDoTagBase::DoTag_On_Opened_PDF, for reader\n");

	tag_opened_pdf_with_resattrmgr(doc);
#endif
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//������ʱ���ܴ�tag�������е�COPY��ʱ����Ϊ��ʱ�ļ���handle��adobe own�ˣ�����ֻ�ܵȵ�adobe release�����handle��ʱ����ܴ�tag
//���������ṩ����ӿڣ����û��������ļ���handle���Ժ��û�����ȡ���handle������ƥ�䷢�ָ�handle�Ƿ�adobe release��
void CDoTag_SaveAs::setFileHandle(ASFile handle)
{
	CELOG_LOG(CELOG_DEBUG, L"CDoTagBase::setFileHandle %p\n", handle);

	m_handle=handle;
}
//ȡfile handle
ASFile CDoTag_SaveAs::getFileHandle()
{
	char temp[1024]={0};
	_snprintf_s(temp, 1024, _TRUNCATE,"CDoTagBase::getFileHandle\n");
	return m_handle;
}
//�����ļ�·��
void CDoTag_SaveAs::SetFilePath(const string& file_path)
{
	CELOG_LOG(CELOG_DEBUG, L"CDoTagBase::SetFilePath\n");

	m_file=file_path;
}
//ȡ�ļ�·��
const string& CDoTag_SaveAs::GetFilePath()
{
	char temp[1024]={0};
	_snprintf_s(temp, 1024, _TRUNCATE,"CDoTagBase::GetFilePath\n");
	return m_file;
}


//�������еĳ�Ա
void CDoTag_SaveAs::reset()
{
	CELOG_LOG(CELOG_DEBUG, L"CDoTagBase::Reset\n");

	m_bNeedDoTag=false;
	m_tags.clear();
	m_file.clear();
	m_files_with_postfix.clear();
	m_handle=NULL;	
	m_strType.clear();

	m_SE_Flag=false;
	m_b_closed=false;
}

void CDoTag_SaveAs::execute_pddocdidsave(PDDoc doc)
{
	CELOG_LOG(CELOG_DEBUG, L"CDoTag_SaveAs::execute_pddocdidsave\n");


	//�����save as��jpg���͵Ļ������Ƶ�
	if (IsMulti_BasedOnPages_Format(m_strType))
	{
		CELOG_LOG(CELOG_DEBUG, L"jpg like file\n");
		DoTag_On_Files_WithPostFix();
	}
	else if (".pdf"==m_strType)
	{
		string path;
		GetPathfromPDDoc(doc, path);
		if (_stricmp(path.c_str(), m_file.c_str()) != 0)
		{
			return;
		}

		CELOG_LOG(CELOG_DEBUG, L"pdf file\n");
		DoTag_On_Opened_PDF(doc);
	}
	else
	{
		CELOG_LOG(CELOG_DEBUG, L"normal file format, like office, rtf, xml, ps, etc....\n");
		DoTag_TagLib();
	}
}

void CDoTag_SaveAs::execute_avappsavedialog(DWORD dwPageNum)
{
	//�������avappsavedialogȥsave pdf��jpg��Ҫ�����pdf�ж���ҳ��Ȼ�������dest files��·����Ȼ�󱣴�����
	if (IsMulti_BasedOnPages_Format(m_strType))
	{
		string::size_type pos=m_file.rfind('.');
		string strName=m_file.substr(0,pos);
		for (DWORD i=1;i<=dwPageNum;i++)
		{
			char temp[32]={0};

			if (".eps"==m_strType)
			{
				_snprintf_s(temp,32, _TRUNCATE, "_%d",i);
			}
			else
			{
				_snprintf_s(temp,32, _TRUNCATE, "_Page_%d",i);
			}
			string strFullPath=strName+temp;
			strFullPath+=m_strType;
			AddFiles_WithPostFix(strFullPath);
		}
	}
}
void CDoTag_SaveAs::execute_avappendsave()
{
	//����save as sharepoint�ϵ��ļ�������pdf��������reader 9��save
	if ("endsave"==m_strType)
	{
		DoTag_TagLib();
	}
}