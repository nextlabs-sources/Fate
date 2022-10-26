#pragma once
#include <string>
#include <vector>
#include <map>
#include "resattrlib.h"
#include "resattrmgr.h"
#include "FileAttributeReaderWriter.h"
using namespace std;

#define MAX_VALUE_LENGTH 254	//summary=(254chars),summary1=(254chars)... they are not allowed to be longer than 254 chars after broken

#define MAX_TAG_NAME_LENGTH 200//tag name is not allowed to be longer than 200chars
#define MAX_TAG_NAME_LENGTH_WITH_POSTFIX (MAX_TAG_NAME_LENGTH+4)
#define MAX_TAG_VALUE_LENGTH 4096//summary=(4096chars),it is not allowed to be longer than 4096 chars before broken

class CLengthMax
{
public:
	static bool is_tagname_too_long(const ResourceAttributes *attrs)
	{
		for(int i=0;i<GetAttributeCount(attrs);i++)
		{
			const wchar_t* pname=GetAttributeName(attrs,i);
			if (wcslen(pname) > MAX_TAG_NAME_LENGTH)
			{
				return true;
			}
		}
		return false;
	}
	static void truncate_tagvalue(ResourceAttributes *attrs)//the tag value should not exceed 4096
	{
		for(int i=0;i<GetAttributeCount(attrs);i++)
		{
			const wchar_t* pValue=GetAttributeValue(attrs,i);
			if (!pValue)
			{
				continue;
			}
			wstring oldvalue(pValue);
			if (oldvalue.length() > MAX_TAG_VALUE_LENGTH)
			{
				wstring newvalue=oldvalue.substr(0,MAX_TAG_VALUE_LENGTH);
				SetAttributeValueW(attrs,i,newvalue.c_str());
			}
		}
	}
};


class CLongTagItem
{
public:
	DWORD m_dwMaxIndex;
	wstring m_tagname;
	wstring m_tagvalue;
public:
	CLongTagItem()
	{
		m_dwMaxIndex=0;
	}
	~CLongTagItem()
	{

	}
};

class CLongTag
{
public:
	//parse all tags and values from attrs,
	//try to analyze and select the long tags out from attrs.
	//put each long tags into a clongtagitem, and put all clongtagitem into m_longtags
	//clongtag represents all long tags from attrs.
	
	//test case:
	//attrs contains summary=abc,summary1=efg,createdate=abc
	//result:
	//m_longtags has two items,
	//one is for summary, its m_dwMaxIndex is 1; another item is for createdate, but its m_dwMaxIndex is 0, so this item is not a really long tag
	void Trans2LongTagItem(const ResourceAttributes *attrs)
	{
		for (int i=0;i<GetAttributeCount(attrs);i++)
		{
			const wchar_t* tagname=GetAttributeName(attrs,i);
			const wchar_t* tagvalue=GetAttributeValue(attrs,i);

			wstring strTagname(tagname);
			wstring strTagvalue(tagvalue);

			//strTagname could be summary,
			//could be summary 1, summary2, if this is summary1, we need to find summary
			wstring::size_type pos=strTagname.rfind(L'.');
			if (pos!=wstring::npos)
			{
				wstring temp=strTagname.substr(pos+1);
				int temptagindex= _wtoi(temp.c_str());
				if (temptagindex!=0)
				{
					strTagname=strTagname.substr(0,pos);;
				}
			}

			bool bFoundTag=false;
			for(DWORD j=0;j<m_longtags.size();j++)
			{
				if (m_longtags[j].m_tagname==strTagname)
				{
					bFoundTag=true;
					m_longtags[j].m_dwMaxIndex++;
					m_longtags[j].m_tagvalue+=strTagvalue;
					break;
				}
			}
			if (bFoundTag==false)
			{
				CLongTagItem item;
				item.m_tagname=strTagname;
				item.m_tagvalue=strTagvalue;
				m_longtags.push_back(item);
			}
		}
	}
public:
	vector<CLongTagItem>& get_tags()
	{
		return m_longtags;
	}
private:
	vector<CLongTagItem> m_longtags;
public:
	CLongTag()
	{

	}
	~CLongTag()
	{

	}
};

class CLongTagOperation
{
public:
	//test case
	//file contains summary,summary.1,summary.2, their values are exceed MAX_VALUE_LENGTH
	//i need to combine these 3 values together
	static void combine(const ResourceAttributes* input_attrs, ResourceAttributes* combined_attrs)
	{
		CLongTag longtag;
		longtag.Trans2LongTagItem(input_attrs);
		
		vector<CLongTagItem> long_tag_items=longtag.get_tags();
		for (DWORD i=0;i<long_tag_items.size();i++)
		{
			AddAttributeW(combined_attrs,long_tag_items[i].m_tagname.c_str(),long_tag_items[i].m_tagvalue.c_str());
		}
	}
	static void pre_add(const ResourceAttributes *user_input_attrs,ResourceAttributes *attrs_to_set)
	{
		broken(user_input_attrs,attrs_to_set);
	}
	//test case
	//file contains tags: summary,summary1,summary2,
	//user want to add summary, this is in user_input_attrs, 
	//because we overwrite value, so we first remove summary,summary1,summary2,
	//then add new summary, we need to broken new summary value, the output is attrs_to_set
	static void pre_add(const ResourceAttributes *user_input_attrs,const WCHAR *filename,ResourceAttributes *attrs_to_set)
	{
		//read existing tag
		ResourceAttributes *exist_attrs;
		AllocAttributes(&exist_attrs);
		GenericNextLabsTagging::GetFileCustomAttributes(filename, exist_attrs, TagTypeDefault);


		//parse it to clongtag
		CLongTag longtag;
		longtag.Trans2LongTagItem(exist_attrs);
		FreeAttributes(exist_attrs);

		//get clongtag
		vector<CLongTagItem> exist_longtag=longtag.get_tags();
		
		//remove existing tag if the tag is contained in user_input_attrs
		remove(user_input_attrs,exist_longtag,filename);

		//broken if the value is long value
		broken(user_input_attrs,attrs_to_set);
	}
	//user_input_attrs is attrs that user want to remove from file. it can be any tag, even it is not exist in file.
	static int remove(const ResourceAttributes *user_input_attrs,const WCHAR *filename)
	{
		//read existing tag
		ResourceAttributes *exist_attrs;
		AllocAttributes(&exist_attrs);
		GenericNextLabsTagging::GetFileCustomAttributes(filename, exist_attrs, TagTypeDefault);

		//parse it to clongtag
		CLongTag longtag;
		longtag.Trans2LongTagItem(exist_attrs);
		FreeAttributes(exist_attrs);

		//remove existing tag if the tag is contained in user_input_attrs
		vector<CLongTagItem> longtagitem=longtag.get_tags();
		return remove(user_input_attrs,longtagitem,filename);
	}

private:
	//test case:
	//user_input_attrs is summary=700bytes
	static void broken(const ResourceAttributes *user_input_attrs, ResourceAttributes * splitted_attrs)
	{
		for (int i=0;i<GetAttributeCount(user_input_attrs);i++)
		{
			const wchar_t* tagname=GetAttributeName(user_input_attrs,i);
			const wchar_t* tagvalue=GetAttributeValue(user_input_attrs,i);

			wstring strTagvalue(tagvalue);

			if(strTagvalue.length()>MAX_VALUE_LENGTH)
			{
				wstring tempvalue=strTagvalue.substr(0,MAX_VALUE_LENGTH);
				AddAttributeW(splitted_attrs,tagname,tempvalue.c_str());
				strTagvalue=strTagvalue.substr(MAX_VALUE_LENGTH);
			}
			else
			{
				AddAttributeW(splitted_attrs,tagname,tagvalue);
				continue;
			}

			DWORD j=1;
			while(strTagvalue.length()>MAX_VALUE_LENGTH)
			{
				wchar_t temptagname[MAX_TAG_NAME_LENGTH_WITH_POSTFIX]={0};
				wsprintfW(temptagname,L"%s.%d",tagname,j);
				wstring tempvalue=strTagvalue.substr(0,MAX_VALUE_LENGTH);
				AddAttributeW(splitted_attrs,temptagname,tempvalue.c_str());
				strTagvalue=strTagvalue.substr(MAX_VALUE_LENGTH);
				j++;
			}
			wchar_t temptagname[MAX_TAG_NAME_LENGTH_WITH_POSTFIX]={0};
			wsprintfW(temptagname,L"%s.%d",tagname,j);
			AddAttributeW(splitted_attrs,temptagname,strTagvalue.c_str());
		}
	}




	//test case
	//file_exist_attrs could be summary,summary1,createdate,author
	//user_input_attrs could be summary(it will never be summary,summary1, it must be single), it could be author, it could be itar, which is not in file_exist_attrs.
	//this function will find out itar is not in file_exist_attrs, so itar is not effect input
	//so only summary and author is effect input,
	static int remove(const ResourceAttributes *user_input_attrs,const vector<CLongTagItem>& file_exist_attrs,const WCHAR *filename)
	{
		vector<wstring> tags_to_remove;
		for (int i=0;i<GetAttributeCount(user_input_attrs);i++)
		{
			const wchar_t* tagname=GetAttributeName(user_input_attrs,i);
			wstring strTagname(tagname);
			for(DWORD j=0;j<file_exist_attrs.size();j++)
			{
				if (file_exist_attrs[j].m_tagname==strTagname)
				{
					int res=remove(file_exist_attrs[j],filename);
					if (res==0)
					{
						return 0;
					}
				}
			}
		}
		return 1;
	}

	//test case:
	//the file contains be summary,summary1,createdate,author
	//tag could be summary, 
	//result is summary,summary1 are removed,
	static int remove(const CLongTagItem& tag,const WCHAR *filename)
	{
		ResourceAttributes *attrs;
		AllocAttributes(&attrs);
		AddAttributeW(attrs,tag.m_tagname.c_str(),L"fake");
		for(DWORD i=1;i<=tag.m_dwMaxIndex;i++)
		{
			WCHAR temptagname[256]={0};
			wsprintfW(temptagname,L"%s.%d",tag.m_tagname.c_str(),i);
			AddAttributeW(attrs,temptagname,L"fake");
		}
		int res=GenericNextLabsTagging::RemoveFileCustomAttributes(filename, attrs);
		FreeAttributes(attrs);
		return res;
	}
};

class CMultiValueOperation
{
public:
	static void split(const ResourceAttributes *input_attrs, ResourceAttributes* splitted_attrs)
	{
		for (int i=0;i<GetAttributeCount(input_attrs);i++)
		{
			const wchar_t* tagname=GetAttributeName(input_attrs,i);
			const wchar_t* tagvalue=GetAttributeValue(input_attrs,i);

			wstring strTagname(tagname);
			wstring strTagvalue(tagvalue);

			wstring::size_type pos=strTagvalue.find(L'|');

			if (pos==wstring::npos)
			{
				AddAttributeW(splitted_attrs,strTagname.c_str(),strTagvalue.c_str());
				continue;
			}
			while (pos!=wstring::npos)
			{
				wstring temp=strTagvalue.substr(0,pos);
				AddAttributeW(splitted_attrs,strTagname.c_str(),temp.c_str());

				strTagvalue=strTagvalue.substr(pos+1);
				pos=strTagvalue.find(L'|');
			}
			AddAttributeW(splitted_attrs,strTagname.c_str(),strTagvalue.c_str());
		}
	}
	static void merge(const ResourceAttributes *user_input_attrs, ResourceAttributes* merged_attrs)
	{
		map<wstring,vector<wstring>>::iterator itr;
		map<wstring,vector<wstring>> merged_tags;
		for (int i=0;i<GetAttributeCount(user_input_attrs);i++)
		{
			const wchar_t* tagname=GetAttributeName(user_input_attrs,i);
			const wchar_t* tagvalue=GetAttributeValue(user_input_attrs,i);

			wstring strTagname(tagname);
			wstring strTagvalue(tagvalue);

			itr=merged_tags.find(strTagname);
			if(itr!=merged_tags.end())
			{
				bool bDup=false;
				vector<wstring> temp_values=merged_tags[strTagname];
				for (DWORD j=0;j<temp_values.size();j++)
				{
					if (strTagvalue==temp_values[j])
					{
						bDup=true;
						break;
					}
				}
				if (false==bDup)
				{
					merged_tags[strTagname].push_back(strTagvalue);
				}
			}
			else
			{
				vector<wstring> temp_values;
				temp_values.push_back(strTagvalue);
				merged_tags[strTagname]=temp_values;
			}
		}

		for(itr = merged_tags.begin(); itr != merged_tags.end(); itr++)
		{
			vector<wstring> temp_values=itr->second;
			wstring temp_combined_values=temp_values[0];
			for (DWORD i=1;i<temp_values.size();i++)
			{
				temp_combined_values+=L'|';
				temp_combined_values+=temp_values[i];
			}
			AddAttributeW(merged_attrs,itr->first.c_str(),temp_combined_values.c_str());
		}
	}
};