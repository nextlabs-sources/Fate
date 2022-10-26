#include "utils.h"

#pragma warning( push )
#pragma warning( disable : 6011 )
#  include <boost/algorithm/string.hpp>
#pragma warning(pop)

#include <Winsock2.h>

#include "eframework/platform/cesdk_loader.hpp"
#include "eframework/platform/cesdk_attributes.hpp"
#include "nlconfig.hpp"

#include "celog.h"

#define CELOG_CUR_MODULE L"iePEP"
#define CELOG_CUR_FILE CELOG_FILEPATH_PROD_PEP_ENDPOINT_WDE_IEPEP_SRC_EVALUATE_CPP

void ConvertURLCharacterW(std::wstring& strUrl)
{
    CELOG_LOG(CELOG_DUMP, L"Parameter variables are: strUrl=%ls \n", strUrl.c_str());

    std::transform(strUrl.begin(), strUrl.end(), strUrl.begin(), towlower);
    boost::replace_all(strUrl,L"%25",L"%");	//'%28'-> '%'
    boost::replace_all(strUrl,L"%20",L" ");	//'%20' -> ' '
    boost::replace_all(strUrl,L"%21",L"!");	//'%21' -> '!'
    boost::replace_all(strUrl,L"%23",L"#");	//'%23' -> '#'
    boost::replace_all(strUrl,L"%24",L"$");	//'%24' -> '$'
    boost::replace_all(strUrl,L"%2a",L"*");	// '%2a  -> '*'
    boost::replace_all(strUrl,L"%2e",L".");	// '%2e  -> '.'
    boost::replace_all(strUrl,L"%2f",L"/");	// '%2f' -> '\'
    boost::replace_all(strUrl,L"%5f",L"_");	// '%5f' -> '_'
    boost::replace_all(strUrl,L"+",L" ");		// '+'   -> ' '
    boost::replace_all(strUrl,L"%2d",L"-");	//'%2d' -> '-'
    boost::replace_all(strUrl,L"%26",L"-");	//'%26' -> '&'
    boost::replace_all(strUrl,L"%3a",L":");	//'%3a' -> ':'
    boost::replace_all(strUrl,L"%3d",L"=");	//'%3d' -> '='
    boost::replace_all(strUrl,L"%3f",L"?");	//'%3f' -> '?'
    boost::replace_all(strUrl,L"%40",L"@");	//'%40' -> '@'
    boost::replace_all(strUrl,L"%28",L"(");	//'%28'-> '('
    boost::replace_all(strUrl,L"%29",L")");	//'%29'-> ')'
    boost::replace_all(strUrl,L"%5b",L"[");	//'%5b'-> '['
    boost::replace_all(strUrl,L"%5d",L"]");	//'%5d'-> ']'
    boost::replace_all(strUrl,L"%5e",L"^");	//'%5e'-> '^'
    boost::replace_all(strUrl,L"%7b",L"{");	//'%7b'-> '{'
    boost::replace_all(strUrl,L"%7d",L"}");	//'%7d'-> '}'
    boost::replace_all(strUrl,L"%7e",L"~");	//'%7d'-> '~'
}

unsigned int GetIp()
{
	char   szHostname[100];   
	HOSTENT   *pHostEnt;   
	in_addr   inAddr;   memset(&inAddr, 0, sizeof(in_addr));

	gethostname(szHostname, sizeof(szHostname));   
	pHostEnt = gethostbyname(szHostname);
	if(NULL != pHostEnt)
		memcpy(&inAddr.S_un,   pHostEnt->h_addr,   pHostEnt->h_length);

	return ntohl(inAddr.S_un.S_addr);
}