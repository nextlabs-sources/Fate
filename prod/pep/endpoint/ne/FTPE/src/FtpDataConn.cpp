#include "stdafx.h"
#include "FtpSocket.h"
#include "ParserResult.h"
#include "FtpCtrlConn.h"
#include "FTPEEval.h"
#include "FtpDataConn.h"
#include <cassert>
#include <sstream>
using namespace std;
#include <list>
#include <vector>
#include <string>
#include "MapperMgr.h"
#include "Utilities.h"

extern nextlabs::cesdk_loader cesdkLoader;

CFtpDataConn::CFtpDataConn(SOCKET fd) : CFtpSocket(fd)
{
	m_pCtrlConn = NULL;
	m_eEvalStatus = NOT_EVALUATED;
}

void CFtpDataConn::SetCtrlConn(CFtpCtrlConn* pCtrlConn)
{
	assert(m_pCtrlConn == NULL && pCtrlConn !=NULL);
	m_pCtrlConn = pCtrlConn;
}

void CFtpDataConn::CollectFtpEvalInfo(FTP_EVAL_INFO& evalInfo) const
{
	string sDstFileName;
	if(m_pCtrlConn->GetSvrFileName().length() > 0 && 
		((m_pCtrlConn->GetSvrFileName().at(0) == '\\')||
		(m_pCtrlConn->GetSvrFileName().at(0) == '/'))
		)
	{
		sDstFileName = m_pCtrlConn->GetSvrFileName();
	}
	else
	{
		sDstFileName = m_pCtrlConn->GetSvrWorkPath();
	if( sDstFileName.length() == 0 || 
	   (sDstFileName[sDstFileName.length() - 1] != '/' && sDstFileName[sDstFileName.length() - 1] != '\\') )
	{
		sDstFileName.append(1,'/');
	}
	sDstFileName.append(m_pCtrlConn->GetSvrFileName());
	}
	std::string::size_type index = sDstFileName.find( "\\" ) ; 
	while( index != std::wstring::npos) 
	{
		sDstFileName.replace(index, strlen("\\"), "/" ) ; 
		index = sDstFileName.find( "\\" ) ; 
	}
	if(	  sDstFileName[0] != '/' )
	{
		evalInfo.pszDestFileName = L"ftp://"+StringT1toT2<char, wchar_t>(GetPeerIP())+L"/"+StringT1toT2<char, wchar_t>(sDstFileName);
	}
	else
	{
	evalInfo.pszDestFileName = L"ftp://"+StringT1toT2<char, wchar_t>(GetPeerIP())+StringT1toT2<char, wchar_t>(sDstFileName);
	}
	evalInfo.pszFTPUserName = StringT1toT2<char, wchar_t>(m_pCtrlConn->GetUser());
	evalInfo.pszServerIP = StringT1toT2<char, wchar_t>(GetPeerIP());
	evalInfo.iProtocolType = (UINT)m_pCtrlConn->GetFtpProtocolType() ;
	wchar_t wszPortBuf[10];
	_snwprintf_s(wszPortBuf, 10, _TRUNCATE, L"%d", GetRemotePort());
	evalInfo.pszServerPort = wszPortBuf;
}

ParserResult CFtpDataConn::ParseSend(const string& sBuf)
{
	if( (m_pCtrlConn->GetCurrentCmd() == "STOR") ||
		(m_pCtrlConn->GetCurrentCmd() == "APPE")  )
	{
		if(m_eEvalStatus == NOT_EVALUATED)
		{
			CMapperMgr& mapperIns = CMapperMgr::Instance();
			string sPath = mapperIns.PopFileInfoFromList(sBuf);
			if(sPath.length() > 0)
			{
				/*
				Upload file	to do evaluation
				*/
				CPolicy *pPolicy = CPolicy::CreateInstance() ;
				FTP_EVAL_INFO evalInfo;
				CollectFtpEvalInfo(evalInfo);
				evalInfo.pszSrcFileName = MyMultipleByteToWideChar(sPath);

				if(evalInfo.IsValid())
				{
					CEEnforcement_t enforcement ;
					memset(&enforcement, 0, sizeof(CEEnforcement_t));

					FTPE_STATUS status = FTPE_SUCCESS ;
					FtpProtocolType type = m_pCtrlConn->GetFtpProtocolType() ;
					if( type == FPT_REGULAR )
					{
						status = pPolicy->QuerySingleFilePolicy( CPolicy::m_ftp, evalInfo , enforcement ) ;
					}else if(  type == FTP_FTPS_IMPLICIT )
					{
						status = pPolicy->QuerySingleFilePolicy( CPolicy::m_ftps, evalInfo , enforcement ) ;
					}
					else
					{
						status = FTPE_ERROR ;
					}

					CEResponse_t response = enforcement.result;
					cesdkLoader.fns.CEEVALUATE_FreeEnforcement(enforcement);

					pPolicy->Release();
					if(status == FTPE_SUCCESS)
					{
						switch(response)
						{
						case CEAllow:
							m_eEvalStatus = EVALUATEED_ALLOW;
							DPA(("%s is allowed\n", sPath.c_str()));
							return PARSER_ALLOW;
							break ;
						case CEDeny:
							m_eEvalStatus = EVALUATED_DENY;
							DPA(("%s is denied\n", sPath.c_str()));
							return PARSER_DENY;
							break ;
						default:
							DPA(("%s is unknown\n", sPath.c_str()));
							break;
						}
					}
				}
			}
			
			m_eEvalStatus = EVALUATEED_ALLOW;
			DPA(("No evaluation\n"));
			return PARSER_ALLOW;
		}
		else if (m_eEvalStatus == EVALUATED_DENY)
		{
			DPA(("PARSER_DENY\n"));
			return PARSER_DENY;
		}
		else
		{
			DPA(("PARSER_ALLOW\n"));
			return PARSER_ALLOW;
		}
	}
	return PARSER_INPROCESS;
}

ParserResult CFtpDataConn::ParseRecv(const string& sBuf)
{
	if(m_pCtrlConn->GetCurrentCmd() == "RETR")
	{
		size_t nBufLen = m_sRecvBuf.length();
		if(nBufLen < CMapperMgr::MAX_CONTENT_SIZE)
		{
			m_sRecvBuf.append(sBuf.data(), nBufLen + sBuf.length() <= CMapperMgr::MAX_CONTENT_SIZE ? sBuf.length() : CMapperMgr::MAX_CONTENT_SIZE - nBufLen);
			FTP_EVAL_INFO evalInfo;
			CollectFtpEvalInfo(evalInfo);

			CMapperMgr& ins = CMapperMgr::Instance();
			ins.PushSocketBufEval(GetSocket(), m_sRecvBuf, evalInfo);
		}
	}
	return PARSER_INPROCESS;
}
