#pragma once

#include "NL_PdfParser.h"
#include <map>

using namespace nextlabs;
namespace nextlabs
{
	class CPDFMgr
	{
	public:
		CPDFMgr();
		~CPDFMgr(void);
	
		bool GetSummaryTags(const char* pszFileName, map<string, string>& mapAllTags);
		bool GetTags(const char* pszFileName, map<string, string>& mapAllTags);
		bool AddTags(const char* pszFileName, std::map<string,string> &mapTags);
		bool DeleteTags(const char* pszFileName, std::map<string,string> &mapTags);
	protected:
	};

}
