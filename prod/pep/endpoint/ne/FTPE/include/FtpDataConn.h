#pragma once

class CFtpSocket;
class CFtpCtrlConn;
struct FTP_EVAL_INFO;

enum EvaluationStatus
{
	NOT_EVALUATED,
	EVALUATEED_ALLOW,
	EVALUATED_DENY
};

class CFtpDataConn : public CFtpSocket
{
public:
	explicit CFtpDataConn(SOCKET fd);
	~CFtpDataConn(void) {}

	void SetCtrlConn(CFtpCtrlConn* pCtrlConn);
	CFtpCtrlConn* GetCtrlConn() const
	{ return m_pCtrlConn; }

	ParserResult ParseSend(const string& sBuf);
	ParserResult ParseRecv(const string& sBuf);

// helpers
private:
	void CollectFtpEvalInfo(FTP_EVAL_INFO& evalInfo) const;

private:
	CFtpCtrlConn* m_pCtrlConn;
	EvaluationStatus m_eEvalStatus;
	string m_sRecvBuf;
};
