#pragma  warning(push)
#pragma  warning(disable:4819 4996)
#include <boost\algorithm\string.hpp>
#pragma  warning(pop)
#include <string>
#include <fstream>
#include <iostream>
#include <vector>
using namespace std;
#include <shlwapi.h>
#include "resattrlib.h"
#include "resattrmgr.h"
#include "Exttag.h"

#define ADD_ACTION	L"add"
#define DEL_ACTION	L"del"
#define CLEAR_ACTION	L"clear"
#define VIEW_ACTION	L"view"


#define SPECIAL_DELALL	L"del_all"
extern resAttrPtrs ptrs;
extern ResourceAttributeManager *mgr;
void readtag(__in const wchar_t* szfilename,__out vector<pair<wstring,wstring>>& vectagpair,ResourceAttributes *attrs=NULL)
{
	bool bFree = false;
	if(attrs == NULL)
	{
		if( ERROR_SUCCESS != ptrs.alloc_attrs_fn(&attrs) )
		{
			fprintf(stderr, "AllocAttributes() failed\n" ) ;
			return ;
		}
		bFree = true;
	}
	/* Read the custom attributes on the file */
	if( false == ( ptrs.read_attrs_fn( mgr, (wchar_t*)szfilename, attrs ) ) )
	{
		fprintf(stderr, "ReadAttributes() failed\n" ) ;
		ptrs.free_attrs_fn(attrs);
		return;
	}
	int count = ptrs.get_count_fn(attrs);
	if(count > 0)
	{
		for (int i = 0; i < count; i++) 
		{
			const WCHAR *name = ptrs.get_name_fn(attrs, i);
			const WCHAR *value = ptrs.get_value_fn(attrs, i);
			if(name != NULL && value != NULL)
			{
				vectagpair.push_back(pair<wstring,wstring>(name,value));
			}
		}
	}
	if(bFree)
	{
		ptrs.free_attrs_fn(attrs);
		return;
	}
}
/*
*\brief: read tag after operation done,seperated by |
*/
void readtag(__in const wchar_t* szfilename,__out wstring& strtapitmes,ResourceAttributes* attrs = NULL)
{
	vector<pair<wstring,wstring>> vecpair;
	readtag(szfilename,vecpair,attrs);
	for(size_t n=0;n<vecpair.size();n++)
	{
		strtapitmes += vecpair[n].first;
		strtapitmes += L"=";
		strtapitmes += vecpair[n].second;
		if(n < vecpair.size()-1)		strtapitmes += L"|";
	}
}
/*
*\ strtagpair: itar = a|b
*	bAdd = false, it means has itar items.
*/
void GetTagPair(__in const wstring& strtagpair,__in bool bAdd,__out vector<pair<wstring,wstring>>& vecTagPair)
{
	size_t nPos = strtagpair.find(L'=');
	if(nPos == wstring::npos && bAdd)	
	{
		OutputDebugString(L" wrong Item .\n");
		return ;
	}
	wstring strtagname = strtagpair;
	if(nPos != wstring::npos)
	{
		strtagname = strtagpair.substr(0,nPos);
		boost::algorithm::trim(strtagname);
		wstring strtagvalues = strtagpair.substr(nPos+1);
		vector<wstring> vectemp;
		boost::algorithm::split(vectemp,strtagvalues,boost::algorithm::is_any_of(L"|"));
		for(size_t n=0;n<vectemp.size();n++)
		{
			wstring strtagvalue = vectemp[n];
			boost::algorithm::trim(strtagvalue);
			vecTagPair.push_back(pair<wstring,wstring>(strtagname,strtagvalue));
		}
	}
	else
	{
		vecTagPair.push_back(pair<wstring,wstring>(strtagname,SPECIAL_DELALL));
	}
};
/*
*\ brief: put the result into vecOut
*/
bool parseline(__in const wstring& strline,__out vector<wstring>& vecOutline)
{
	wstring strerror = strline;
	if(vecOutline.empty())	
	{
		// first format line
		vecOutline.push_back(strerror);	
		return true;
	}
	/* Allocate the ResourceAttributes object */ 
	ResourceAttributes *attrs = NULL;
	if( ERROR_SUCCESS != ptrs.alloc_attrs_fn(&attrs) || attrs == NULL)
	{
		fprintf(stderr, "AllocAttributes() failed\n" ) ;
		strerror += L", alloc attres failed.";
		vecOutline.push_back(strerror);
		return true;
	}

	vector<wstring> vecInfo;
	// get all items
	boost::algorithm::split(vecInfo,strline,boost::algorithm::is_any_of(","));
	// trim items and remove empty items
	vector<wstring>::iterator it = vecInfo.begin();
	for(;it != vecInfo.end();)
	{
		boost::algorithm::trim((*it));
		if((*it).empty())	it = vecInfo.erase(it);
		else it++;
	}
	size_t nsize = vecInfo.size();
	if(nsize < 2)
	{
		strerror = strline + L",[error: hi guys wrong format.]";
		vecOutline.push_back(strerror);
		return true;
	}
	// get file
	wstring& strfile = vecInfo[0];
	/* Check if file exists */
	if(!PathFileExists(strfile.c_str())) 
	{
		strerror += L",[error: file is not existed.]";
		vecOutline.push_back(strerror);	
		return true;
	}

	// get action
	wstring &straction = vecInfo[1];

	if(_wcsicmp(straction.c_str(),ADD_ACTION) == 0)
	{
		// tag infor start from the second item
		vector<pair<wstring,wstring>> tagpair;
		for(size_t i=2;i<nsize;i++)
		{
			// ITAR=YES|NO
			wstring& strtagpair = vecInfo[i];
			// read tag pair from each item
			GetTagPair(strtagpair,true,tagpair);
		}

		// read tag first
		if( false == ( ptrs.read_attrs_fn( mgr, (wchar_t*)strfile.c_str(), attrs ) ) )
		{
			strerror += L", read attres failed.";
			ptrs.free_attrs_fn(attrs);
			attrs = NULL;
			vecOutline.push_back(strerror);
			return	false;
		}
		for(size_t n=0;n<tagpair.size();n++)
		{
			// add tag to attrs
			ptrs.add_attrs_fn ( attrs, tagpair[n].first.c_str(), tagpair[n].second.c_str());
		}
		/* write tag to the file */
		if( ptrs.write_attrs_fn( mgr, (wchar_t*)strfile.c_str(), attrs ) )	strerror += L",[true],";
		else strerror += L",[false],";

		// re-read the tag and put into strerror
		readtag(strfile.c_str(),strerror);

		vecOutline.push_back(strerror);
	}
	else if(_wcsicmp(straction.c_str(),DEL_ACTION) ==0 )
	{
		// read all tag first
		vector<pair<wstring,wstring>> vectagpair;
		readtag(strfile.c_str(),vectagpair,attrs);
		// get the removed tagpair
		vector<pair<wstring,wstring>> tagpair;
		for(size_t i=2;i<nsize;i++)
		{
			// ITAR=YES|NO
			wstring& strtagpair = vecInfo[i];
			// read tag pair from each item
			GetTagPair(strtagpair,false,tagpair);
		}

		/* Remove all and rewrite the ones we want */
		if ( false == ( ptrs.remove_attrs_fn( mgr, (wchar_t*)strfile.c_str(), attrs ) ) )
		{
			fprintf(stderr, "RemoveAttributes() failed\n" ) ;
			return false;
		}
		ptrs.free_attrs_fn ( attrs );
		/* Realloc a copy of the attrbiutes to be reqritten */
		if ( ERROR_SUCCESS != ( ptrs.alloc_attrs_fn( &attrs ) ) )
		{
			fprintf(stderr, "AllocAttributes() failed\n" ) ;
			return false;
		}
		if( false == ( ptrs.read_attrs_fn( mgr, (wchar_t*)strfile.c_str(), attrs ) ) )
		{
			fprintf(stderr, "AddTag: 1) ReadAttributes() failed\n" ) ;
			ptrs.free_attrs_fn(attrs);
			return false;
		}
		for(size_t n=0;n<vectagpair.size();n++)
		{
			wstring& strname = vectagpair[n].first;
			wstring& strvalue = vectagpair[n].second;
			bool badd = true;
			for(size_t j=0;j<tagpair.size();j++)
			{
				if(0 == _wcsicmp(strname.c_str(),tagpair[j].first.c_str()))
				{
					if(0 == _wcsicmp(SPECIAL_DELALL,tagpair[j].second.c_str()) ||
						0 == _wcsicmp(strvalue.c_str(),tagpair[j].second.c_str()))
					{
						badd =false;
						break;
					}
				}
			}
			// add tag
			if(badd)	ptrs.add_attrs_fn ( attrs, strname.c_str(), strvalue.c_str());
		}
		/* Write the newly added custom attributes to the file */
		if( false == ( ptrs.write_attrs_fn( mgr, (wchar_t*)strfile.c_str(), attrs ) ) )
		{
			strerror += L",[false],";
		}
		else		strerror += L",true,";
		readtag(strfile.c_str(),strerror);
		vecOutline.push_back(strerror);
	}
	else if(_wcsicmp(straction.c_str(),CLEAR_ACTION) == 0)
	{
		if( false == ( ptrs.read_attrs_fn( mgr, (wchar_t*)strfile.c_str(), attrs ) ) )
		{
			fprintf(stderr, "AddTag: 1) ReadAttributes() failed\n" ) ;
			ptrs.free_attrs_fn(attrs);
			return false;
		}
		/* Remove all and rewrite the ones we want */
		if ( false == ( ptrs.remove_attrs_fn( mgr, (wchar_t*)strfile.c_str(), attrs ) ) )
		{
			strerror += L",[false]";
		}
		else strerror += L",[true]";
		readtag(strfile.c_str(),strerror);
		vecOutline.push_back(strerror);
	}
	else if(_wcsicmp(straction.c_str(),VIEW_ACTION) == 0)
	{
		strerror += L",[true],";
		readtag(strfile.c_str(),strerror);
		vecOutline.push_back(strerror);
	}
	else
	{
		strerror = strline + L",[error: hi guys wrong format.]";
		vecOutline.push_back(strerror);
	}
	if(attrs != NULL)
	{
		ptrs.free_attrs_fn(attrs);
		attrs = NULL;
	}
	return true;
}
bool parseTags(__in const wchar_t* szCVSFile,__in const wchar_t* szOutFile)
{
	bool bRet = false;
	wifstream file (szCVSFile); 
	if(!file.good())	return false;
	wstring strline;
	vector<wstring> vecOutline;
	// parse input file 
	while (getline ( file, strline))
	{
		wcout << strline << endl;
		parseline(strline,vecOutline);
	} 
	file.close();
	// write to output file
	wofstream ofile(szOutFile);
	if(ofile.good())
	{
		vector<wstring>::iterator it = vecOutline.begin();
		for(;it!= vecOutline.end();it++)
		{
			ofile.write((*it).c_str(),(*it).length());
			ofile.write(L"\n",1);
		}
		ofile.close();
	}
	return bRet;
}
