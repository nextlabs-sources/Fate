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
//设置要打tag的标志，同时保存tag
void CDoTag_SaveAs::setFlagAndObligationTags(const vector<pair<wstring,wstring>>& tags)
{
	m_bNeedDoTag=true;
	m_tags = tags;

	CELOG_LOGA(CELOG_DEBUG, "CDoTagBase::setFlagAndObligationTags\n");
}
//判断要不要打tag
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
//用tag lib打tag
void CDoTag_SaveAs::DoTag_TagLib()
{
	CELOG_LOGA(CELOG_DEBUG, "CDoTagBase::DoTag, path: %s\n", m_file.c_str());

	CTag* ins_tag=CTag::GetInstance();
	wstring path(m_file.begin(),m_file.end());
	ins_tag->add_tag_using_resattrmgr(path,m_tags);
	//必须在打完tag后，reset
	reset();
}



//用tag lib来打tag，和上面函数不同之处在于提供了一个参数，而且不会重置
void CDoTag_SaveAs::DoTag_TagLib_2(const string& strPath)
{
	CTag* ins_tag=CTag::GetInstance();
	wstring path(strPath.begin(),strPath.end());
	ins_tag->add_tag_using_resattrmgr(path,m_tags);
}


void CDoTag_SaveAs::tag_opened_pdf_with_resattrmgr(PDDoc doc)
{
	//我们是在把pdf save as成pdf，而且是在用adobe reader，没有PDDocSetInfo可以用，所以
	//我们要先把当前的PDF文件，模拟用户点击菜单来关闭当前的PDF，然后再打tag，然后再模拟用户打开文件
	//要找到file的handle才能关掉handle，这样才能打tag，所以如果没有找到，就不能打tag
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
		//什么也不要做
	}

	reset();
}


//对已经打开了的pdf打tag，acrobat和reader的实现不同
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

		//bDoTagAfterSave意味着标志被设置了，所以要打tag，
		//在打tag前，要把标志重置，
		//因为我们是用PDDocSetInfo打tag
		//用这个打tag必须要主动的调用PDDocDoSave保存住tag，如果不重置标志，我们就会进入死循环
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
//我们有时候不能打tag，比如有的COPY的时候，因为当时文件的handle被adobe own了，我们只能等到adobe release掉这个handle的时候才能打tag
//所以我们提供这个接口，让用户来设置文件的handle，以后用户可以取这个handle，用于匹配发现该handle是否被adobe release了
void CDoTag_SaveAs::setFileHandle(ASFile handle)
{
	CELOG_LOG(CELOG_DEBUG, L"CDoTagBase::setFileHandle %p\n", handle);

	m_handle=handle;
}
//取file handle
ASFile CDoTag_SaveAs::getFileHandle()
{
	char temp[1024]={0};
	_snprintf_s(temp, 1024, _TRUNCATE,"CDoTagBase::getFileHandle\n");
	return m_handle;
}
//设置文件路径
void CDoTag_SaveAs::SetFilePath(const string& file_path)
{
	CELOG_LOG(CELOG_DEBUG, L"CDoTagBase::SetFilePath\n");

	m_file=file_path;
}
//取文件路径
const string& CDoTag_SaveAs::GetFilePath()
{
	char temp[1024]={0};
	_snprintf_s(temp, 1024, _TRUNCATE,"CDoTagBase::GetFilePath\n");
	return m_file;
}


//重置所有的成员
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


	//如果是save as成jpg类型的或者类似的
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
	//如果是在avappsavedialog去save pdf成jpg，要算清楚pdf有多少页，然后算出来dest files的路径，然后保存起来
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
	//是在save as sharepoint上的文件到本地pdf或者是用reader 9做save
	if ("endsave"==m_strType)
	{
		DoTag_TagLib();
	}
}