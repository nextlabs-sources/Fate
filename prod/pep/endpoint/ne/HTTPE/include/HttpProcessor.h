#pragma once

#include "httpmsg.h"
#include "Eval.h"
#include "timeout_list.hpp"

class CHttpProcessor
{
public:
	CHttpProcessor(void);
	~CHttpProcessor(void);

public:
	static int ProcessMsg(smartHttpMsg& httpMsg);
	static int DoEvaluation(const wstring & strAction, const wstring & strSrc, const wstring & strDest, std::map<std::wstring, std::wstring> & mapAttributes, CObligation& obligation);
	static int DoEvaluation(const wstring & strAction, const wstring & strSrc, const wstring & strDest, std::map<std::wstring, std::wstring> & mapAttributes);
	static int DoEvaluation(const wstring & strAction, const wstring & strSrc, const wstring & strDest);
	static int DoEvaluation(const wstring & strAction, const wstring & strSrc, const wstring & strDest, CObligation& obligation);
	static int ParseObligation( CEAttributes *obligation, CObligation& Ob) ;

protected:
	static CTimeoutList m_listEvalCache;
};
