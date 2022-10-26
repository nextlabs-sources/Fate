#include "stdafx.h"
#include "FtpSocket.h"
#include "ParserResult.h"
#include "FtpDataconn.h"
#include "FtpCtrlConn.h"
#include <cassert>
#include <string>
#include <algorithm>
using namespace std;
#include "Utilities.h"
#include "timeout_list.hpp"

extern CTimeoutList g_listForCut;

const FTP_MSG_MAP_ENTRY** CFtpCtrlConn::GetFtpMsgMapEntriesAry()
{
	static const FTP_MSG_MAP_ENTRY _ary_ftp_msg_map_entry0[] = { { &CFtpCtrlConn::ParseUSERCmd,"USER"}, {&CFtpCtrlConn::DefaultHandler,"331"}, {NULL,NULL} };
	static const FTP_MSG_MAP_ENTRY _ary_ftp_msg_map_entry1[] = { { &CFtpCtrlConn::ParsePASVCmd,"PASV"}, {&CFtpCtrlConn::ParsePASVRes,"227"}, {NULL,NULL} };
	static const FTP_MSG_MAP_ENTRY _ary_ftp_msg_map_entry2[] = { { &CFtpCtrlConn::ParsePORTCmd,"PORT"}, {&CFtpCtrlConn::ParsePORTRes,"200"}, {NULL,NULL} };
	static const FTP_MSG_MAP_ENTRY _ary_ftp_msg_map_entry3[] = { { &CFtpCtrlConn::DefaultHandler,"PWD"}, {&CFtpCtrlConn::ParsePWDRes,"257"}, {NULL,NULL} };
	static const FTP_MSG_MAP_ENTRY _ary_ftp_msg_map_entry4[] = { { &CFtpCtrlConn::DefaultHandler,"MODE Z"}, {&CFtpCtrlConn::ParseMODEZRes,"200"}, {NULL,NULL} };
	static const FTP_MSG_MAP_ENTRY _ary_ftp_msg_map_entry5[] = { { &CFtpCtrlConn::DefaultHandler,"MODE S"}, {&CFtpCtrlConn::ParseMODESRes,"200"}, {NULL,NULL} };
	static const FTP_MSG_MAP_ENTRY _ary_ftp_msg_map_entry6[] = { { &CFtpCtrlConn::ParseCWDCmd,"CWD"}, {&CFtpCtrlConn::ParseCWDRes,"250"}, {NULL,NULL} };
	static const FTP_MSG_MAP_ENTRY _ary_ftp_msg_map_entry7[] = { { &CFtpCtrlConn::ParseSTORCmd,"STOR"}, { &CFtpCtrlConn::ParseSTOR550Res,"550"}, {NULL,NULL} };
	static const FTP_MSG_MAP_ENTRY _ary_ftp_msg_map_entry8[] = { { &CFtpCtrlConn::ParseRETRCmd,"RETR"}, {NULL,NULL} };
	static const FTP_MSG_MAP_ENTRY _ary_ftp_msg_map_entry9[] = { { &CFtpCtrlConn::DefaultHandler,"FEAT"}, { &CFtpCtrlConn::ParseFEATRes,"211"}, {NULL,NULL} };
	static const FTP_MSG_MAP_ENTRY _ary_ftp_msg_map_entry10[] = { { &CFtpCtrlConn::DefaultHandler,"CDUP"}, {&CFtpCtrlConn::ParseCWDRes,"250"}, {NULL,NULL} };
	static const FTP_MSG_MAP_ENTRY _ary_ftp_msg_map_entry11[] = { { &CFtpCtrlConn::ParseAPPECmd,"APPE"}, {NULL,NULL} };
	static const FTP_MSG_MAP_ENTRY _ary_ftp_msg_map_entry12[] = { { &CFtpCtrlConn::ParseEPSVCmd,"EPSV"}, {&CFtpCtrlConn::ParseEPSVRes,"229"}, {NULL,NULL} };	
	static const FTP_MSG_MAP_ENTRY _ary_ftp_msg_map_entry13[] = { { &CFtpCtrlConn::ParseEPRTCmd,"EPRT"}, {&CFtpCtrlConn::ParseEPRTRes,"200"}, {NULL,NULL} };
	static const FTP_MSG_MAP_ENTRY _ary_ftp_msg_map_entry14[] = { { &CFtpCtrlConn::ParseDELECmd,"DELE"}, {NULL,NULL} };
	static const FTP_MSG_MAP_ENTRY* _ary_ftp_msg_map_entries[] = 
	{
		_ary_ftp_msg_map_entry0,
		_ary_ftp_msg_map_entry1,
		_ary_ftp_msg_map_entry2,
		_ary_ftp_msg_map_entry3,
		_ary_ftp_msg_map_entry4,
		_ary_ftp_msg_map_entry5,
		_ary_ftp_msg_map_entry6,
		_ary_ftp_msg_map_entry7,
		_ary_ftp_msg_map_entry8,
		_ary_ftp_msg_map_entry9,
		_ary_ftp_msg_map_entry10,
		_ary_ftp_msg_map_entry11,
		_ary_ftp_msg_map_entry12,
		_ary_ftp_msg_map_entry13,
		_ary_ftp_msg_map_entry14,
		NULL
	};
	return &_ary_ftp_msg_map_entries[0];
}

CFtpCtrlConn::CFtpCtrlConn(SOCKET fd) : CFtpSocket(fd) 
{
	m_pDataConn = NULL;
	m_nDataPort = -1;
	m_sDataIP = "";
	m_eConnMode = FCM_UNSPECIFIED;
	m_sCurrentCmd = "INVALID_COMMAND";
	m_eDataTransferMode = FDTM_MODE_S;
	m_eFtpProtocol = FPT_REGULAR;
}

CFtpDataConn* CFtpCtrlConn::GetDataConn() const
{
	return m_pDataConn;
}

CFtpDataConn* CFtpCtrlConn::SetDataConn(CFtpDataConn* pDataConn)
{
	CFtpDataConn* pOldDataConn = m_pDataConn;
	m_pDataConn = pDataConn;
	if(m_pDataConn == NULL)
	{
		m_nDataPort = -1;
		m_sDataIP = "";
	}
	else
	{
		m_pDataConn->SetCtrlConn(this);
	}
	return pOldDataConn;
}

ParserResult CFtpCtrlConn::ParseSend(const string& sBuf)
{
	m_sCurrentCmd = "INVALID_COMMAND";

	const FTP_MSG_MAP_ENTRY** ptrFtpMsgMapEntriesAry = GetFtpMsgMapEntriesAry();
	while(*ptrFtpMsgMapEntriesAry != NULL)
	{
		const FTP_MSG_MAP_ENTRY* ptrFtpMsgMapEntries = *ptrFtpMsgMapEntriesAry;
		if(StringFindNoCase(sBuf, ptrFtpMsgMapEntries[0].pcstrCmdCode) == 0)
		{
			m_sCurrentCmd = ptrFtpMsgMapEntries[0].pcstrCmdCode;
			return (this->*ptrFtpMsgMapEntries[0].lpfnHandler)(sBuf);
		}
		++ptrFtpMsgMapEntriesAry;
	}

	return PARSER_INPROCESS;
}

ParserResult CFtpCtrlConn::ParseRecv(const string& sBuf)
{
	if(m_sCurrentCmd == "INVALID_COMMAND")
		return PARSER_INPROCESS;

	const FTP_MSG_MAP_ENTRY** ptrFtpMsgMapEntriesAry = GetFtpMsgMapEntriesAry();
	while(*ptrFtpMsgMapEntriesAry != NULL)
	{
		const FTP_MSG_MAP_ENTRY* ptrFtpMsgMapEntries = *ptrFtpMsgMapEntriesAry;
		if(StringFindNoCase(m_sCurrentCmd, ptrFtpMsgMapEntries[0].pcstrCmdCode) == 0)
		{
			++ptrFtpMsgMapEntries;
			while(ptrFtpMsgMapEntries->pcstrCmdCode != NULL)
			{
				if(StringFindNoCase(sBuf, ptrFtpMsgMapEntries->pcstrCmdCode) == 0)
				{
					return (this->*ptrFtpMsgMapEntries->lpfnHandler)(sBuf);
				}
				++ptrFtpMsgMapEntries;
			}
		}
		++ptrFtpMsgMapEntriesAry;
	}
	return PARSER_INPROCESS;
}

ParserResult CFtpCtrlConn::DefaultHandler(const string& /*sBuf*/)
{
	return PARSER_INPROCESS;
}

ParserResult CFtpCtrlConn::ParseUSERCmd(const string& sBuf)
{
	const char* USER_STR = "USER ";
	m_sUser = sBuf.substr(strlen(USER_STR),sBuf.find(0x0D) - strlen(USER_STR));
	return PARSER_INPROCESS;
}

ParserResult CFtpCtrlConn::ParsePASVCmd(const string& /*sBuf*/)
{
	m_eConnMode = FCM_PASV;
	return PARSER_INPROCESS;
}

void CFtpCtrlConn::ParseIP_Port(const string& sBuf)
{
	/*
	Modified on 17-07-2009,	for the bug 9456
	*/
	m_sDataIP.clear() ;
	for(size_t idx = 0, comma = 0; idx < sBuf.length(); ++idx)
	{
		if(sBuf[idx] == ',')
			++comma;
		if(comma < 4)
		{
			if(sBuf[idx] == ',')
				m_sDataIP.push_back('.');
			else
				m_sDataIP.push_back(sBuf[idx]);
		}
		if(comma == 4)
		{
			string hport;
			string lport;
			for(++idx; idx < sBuf.length() && sBuf[idx] != ','; ++idx)
				hport.push_back(sBuf[idx]);
			for(++idx; idx < sBuf.length(); ++idx)
				lport.push_back(sBuf[idx]);
			m_nDataPort = atoi(hport.c_str()) * 256 + atoi(lport.c_str());
		}
	}
}

ParserResult CFtpCtrlConn::ParsePASVRes(const string& sBuf)
{
	assert(m_eConnMode == FCM_PASV);

	// Just for debug use
	assert(m_pDataConn == NULL && m_nDataPort == -1 && m_sDataIP == "");

	size_t nBeg = sBuf.find('(');
	size_t nEnd = sBuf.find(')');
	if(nBeg != string::npos && nEnd != string::npos)
	{
		ParseIP_Port(sBuf.substr(nBeg + 1, nEnd - (nBeg + 1)));
	}

	return PARSER_INPROCESS;
}

ParserResult CFtpCtrlConn::ParseEPSVCmd(const string& sBuf)
{
	sBuf ;
	m_eConnMode = FCM_PASV;
	return PARSER_INPROCESS;
}
ParserResult CFtpCtrlConn::ParseEPSVRes(const string& sBuf)
{	 
	size_t nBeg = sBuf.find('(');
	size_t nEnd = sBuf.find(')');
	if( nBeg != string::npos && nEnd != string::npos && (nEnd>nBeg))
	{
		string strport = sBuf.substr(nBeg + 1, nEnd - (nBeg + 1)) ;
		string::iterator itor;
		for(itor = strport.begin() ; itor!= strport.end(); )
		{	 
			char temp =	 (*itor);
			if( temp == '|' )
			{
				strport.erase( itor ) ;
				itor = strport.begin() ;
			}
			else
			{
				 itor++	 ;
			}
		}
		m_nDataPort =	atoi(strport.c_str())   ;
		m_sDataIP = GetPeerIP() ;

		DPA(("FTPE Parer EPSV cmd port number:[%i],string[%s],IP:[%s]",m_nDataPort,strport.c_str(),m_sDataIP.c_str())) ;
	}
	return  PARSER_INPROCESS ; 
}
ParserResult CFtpCtrlConn::ParsePORTCmd(const string& sBuf)
{
	const char* PORT_STR = "PORT ";
	// Just for debug use
	//assert(m_pDataConn == NULL && m_nDataPort == -1 && m_sDataIP == "");
	size_t nBeg = strlen(PORT_STR);
	size_t nEnd = sBuf.find(0x0D);
	if(nBeg != string::npos && nEnd != string::npos)
	{
		ParseIP_Port(sBuf.substr(nBeg, nEnd - nBeg));
	}

	return PARSER_INPROCESS;
}
ParserResult CFtpCtrlConn::ParsePORTRes(const string& /*sBuf*/)
{
	m_eConnMode = FCM_PORT;
	return PARSER_INPROCESS;
}

ParserResult CFtpCtrlConn::ParseEPRTCmd(const string& sBuf)
{
	const char* EPRT_STR = "EPRT ";
	size_t nBeg = sBuf.find(EPRT_STR);
	size_t nEnd = sBuf.find("|");
	if(nBeg != string::npos && nEnd != string::npos)
	{
		/*EPRT |1|132.235.1.2|6275|*/
		m_sDataIP.clear() ;
		string strsub = sBuf.substr(nEnd+1, sBuf.length() -(nEnd +1)) ;
		
		nEnd = strsub.find("|");	
		if(  nEnd != string::npos )
		{
			strsub = strsub.substr(nEnd+1, strsub.length() -(nEnd +1)) ;
			nEnd = strsub.find("|");
			if(  nEnd != string::npos )
			{
				m_sDataIP = strsub.substr( 0, nEnd-1 ) ;
				strsub = strsub.substr(nEnd+1, strsub.length() -(nEnd +1)) ;
				nEnd = strsub.find("|"); 
				if(  nEnd != string::npos )
				{
					m_nDataPort =	atoi(strsub.substr( 0, nEnd-1 ).c_str())   ;
				}
			}
		}
	}

	return PARSER_INPROCESS;
}
ParserResult CFtpCtrlConn::ParseEPRTRes(const string& /*sBuf*/)
{
	m_eConnMode = FCM_PORT;
	return PARSER_INPROCESS;
}


ParserResult CFtpCtrlConn::ParsePWDRes(const string& sBuf)
{
	size_t nBeg = sBuf.find('\"');
	if(nBeg != string::npos && nBeg + 1 < sBuf.length())
	{
		size_t nEnd = sBuf.find('\"',nBeg + 1);
		if(nEnd != string::npos)
		{
			m_sSvrWorkPath = sBuf.substr(nBeg + 1, nEnd - (nBeg + 1));
		}
	}
	return PARSER_INPROCESS;
}

ParserResult CFtpCtrlConn::ParseMODEZRes(const string& /*sBuf*/)
{
	m_eDataTransferMode = FDTM_MODE_Z;
	return PARSER_INPROCESS;
}

ParserResult CFtpCtrlConn::ParseMODESRes(const string& /*sBuf*/)
{
	m_eDataTransferMode = FDTM_MODE_S;
	return PARSER_INPROCESS;
}


ParserResult CFtpCtrlConn::ParseCWDCmd(const string& sBuf)
{
	const char* CWD_STR = "CWD ";
	m_sTempSvrWorkPath = sBuf.substr(strlen(CWD_STR), sBuf.find(0x0D) - strlen(CWD_STR));
	return PARSER_INPROCESS;
}

ParserResult CFtpCtrlConn::ParseCWDRes(const string& sBuf)
{
	bool bFound = false;
	const char* CWD_RES = "250 ";
	string::size_type nRealResBeg = sBuf.find(CWD_RES);
	if(nRealResBeg != string::npos)
	{
		string sRealRes = sBuf.substr(nRealResBeg);
		string::size_type nBeg = sRealRes.find('/');
		if(nBeg != string::npos)
		{
			string::size_type nEnd = sRealRes.find(0x0D);
		if(nEnd != string::npos)
		{
				bFound = true;
				string strTemp = sRealRes.substr(nBeg, nEnd - nBeg);
				nBeg = strTemp.find('/');
				if(	(nBeg != string::npos)&&( nBeg == 0 ))
				{
					m_sSvrWorkPath =  strTemp ;
				}
				else
				{
					string::size_type begin = m_sSvrWorkPath.find('/');
					if(	(begin != string::npos)&&( begin == m_sSvrWorkPath.length() -1 ))
					{
						m_sSvrWorkPath = m_sSvrWorkPath + strTemp ;
					}
					else
					{
						m_sSvrWorkPath = m_sSvrWorkPath + "/"+ strTemp ;
					}

				}
			nEnd = m_sSvrWorkPath.find('\"');
			if(nEnd != string::npos)
			{
				m_sSvrWorkPath = m_sSvrWorkPath.substr(0, nEnd);
			}
		}
	}

	if(bFound == false)
	{
			string::size_type begin = m_sTempSvrWorkPath.find('/');
			if(	(begin != string::npos)&&( begin == 0 ))
			{
		m_sSvrWorkPath = m_sTempSvrWorkPath;
	}
			else
			{
				begin = m_sSvrWorkPath.find('/');
				if(	(begin != string::npos)&&( begin == m_sSvrWorkPath.length() -1 ))
				{
					m_sSvrWorkPath = m_sSvrWorkPath + m_sTempSvrWorkPath ;
				}
				else
				{
					m_sSvrWorkPath = m_sSvrWorkPath + "/"+m_sTempSvrWorkPath ;
				}
			}
			//m_sSvrWorkPath = m_sTempSvrWorkPath;
		}
	}
	return PARSER_INPROCESS;
}


ParserResult CFtpCtrlConn::ParseSTORCmd(const string& sBuf)
{
	const char* STOR_STR = "STOR ";
	m_sSvrFileName = sBuf.substr(strlen(STOR_STR),sBuf.find(0x0D) - strlen(STOR_STR));
	return PARSER_INPROCESS;
}
ParserResult CFtpCtrlConn::ParseAPPECmd(const string& sBuf)
{
	const char* APPE_STR = "APPE ";
	m_sSvrFileName = sBuf.substr(strlen(APPE_STR),sBuf.find(0x0D) - strlen(APPE_STR));
	return PARSER_INPROCESS;
}

ParserResult CFtpCtrlConn::ParseRETRCmd(const string& sBuf)
{
	const char* RETR_STR = "RETR ";
	m_sSvrFileName = sBuf.substr(strlen(RETR_STR),sBuf.find(0x0D) - strlen(RETR_STR));
	return PARSER_INPROCESS;
}

ParserResult CFtpCtrlConn::ParseFEATRes(const string& sBuf)
{
	const char* MODE_Z_STR = "MODE Z";
	string::size_type pos = StringFindNoCase(sBuf, MODE_Z_STR);
	if(pos != string::npos)
	{
		string& sModifiedBuf = const_cast<string&>(sBuf);
		string::size_type endpos = pos + strlen(MODE_Z_STR);
		for(;pos < endpos; ++pos)
		{
			sModifiedBuf[pos] = ' ';
		}
		return PARSER_BUF_MODIFIED;
	}
	return PARSER_INPROCESS;
}

ParserResult CFtpCtrlConn::ParseSTOR550Res(const string& /*sBuf*/)
{
	CFtpDataConn* pDataConn = SetDataConn(NULL);
	// No need to check against NULL
	delete pDataConn;
	return PARSER_INPROCESS;
}

ParserResult CFtpCtrlConn::ParseDELECmd(const string& sBuf)
{
	//DELETE COMMAND, like: DELE foo.txt
	//command type: DELE filename
	DPA(("Delete command:%s",sBuf.c_str())) ;

	if(IsProcess(L"ftpte.exe") || IsProcess(L"explorer.exe"))
	{
		const char* DELE_STR = "DELE ";

		if(sBuf.find(DELE_STR) != string::npos && sBuf.find(0x0D, strlen(DELE_STR)) != string::npos )
		{
			string strfilename = sBuf.substr(strlen(DELE_STR), sBuf.find(0x0D) - strlen(DELE_STR));

			std::wstring strFile =  StringT1toT2<char, wchar_t>(strfilename);
			if(g_listForCut.FindItem(strFile))
			{
				DPA(("FTPE::Deny CUT, %s", strfilename.c_str()) );
				g_listForCut.DeleteItem(strFile);
				return PARSER_DENY;
			}
		}
	}
	
	return PARSER_INPROCESS;
}
