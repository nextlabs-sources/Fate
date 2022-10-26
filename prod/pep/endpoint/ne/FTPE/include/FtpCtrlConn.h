#pragma once

class CFtpSocket;
class CFtpDataConn;

class CFtpCtrlConn;


struct FTP_MSG_MAP_ENTRY
{

	ParserResult (CFtpCtrlConn::*lpfnHandler) (const string&);	   
	const char* pcstrCmdCode;
};

enum FtpConnMode
{
	FCM_UNSPECIFIED,
	FCM_PASV,
	FCM_PORT
};

enum FtpDataTransferMode
{
	FDTM_MODE_S,
	FDTM_MODE_Z,
};

enum FtpProtocolType
{
	FPT_UNKNOWN,
	FPT_REGULAR,
	FTP_FTPS_IMPLICIT
};

class CFtpCtrlConn : public CFtpSocket
{
public:
	explicit CFtpCtrlConn(SOCKET fd);
	~CFtpCtrlConn(void) {}

	CFtpDataConn* SetDataConn(CFtpDataConn* pDataConn);
	CFtpDataConn* GetDataConn() const;

	int GetDataPort() const
	{ return m_nDataPort; }
	string GetDataIP() const
	{ return m_sDataIP; }

	string GetSvrWorkPath() const
	{ return m_sSvrWorkPath; }
	string GetSvrFileName() const
	{ return m_sSvrFileName; }

	string GetCurrentCmd() const
	{ return m_sCurrentCmd; }

	string GetUser() const
	{ return m_sUser; }

	FtpConnMode GetConnMode() const
	{ return m_eConnMode; }

	FtpDataTransferMode GetDataTransferMode() const
	{ return m_eDataTransferMode; }

	void SetFtpProtocolType(FtpProtocolType pt)
	{ m_eFtpProtocol = pt; }
	FtpProtocolType GetFtpProtocolType() const
	{ return m_eFtpProtocol; }

	ParserResult ParseSend(const string& sBuf);
	ParserResult ParseRecv(const string& sBuf);

// helpers
private:
	void ParseIP_Port(const string& sBuf);

private:

	ParserResult DefaultHandler(const string& sBuf);

	ParserResult ParseUSERCmd(const string& sBuf);

	ParserResult ParsePASVCmd(const string& sBuf);
	ParserResult ParsePASVRes(const string& sBuf);

	ParserResult ParsePORTCmd(const string& sBuf);
	ParserResult ParsePORTRes(const string& sBuf);

	ParserResult ParseDELECmd( const string& sBuf); 

	/*
	Port Extensive
	*/
	ParserResult ParseEPSVCmd(const string& sBuf);
	ParserResult ParseEPSVRes(const string& sBuf);

	ParserResult ParseEPRTCmd(const string& sBuf);
	ParserResult ParseEPRTRes(const string& sBuf);


	ParserResult ParsePWDRes(const string& sBuf);
	ParserResult ParseMODEZRes(const string& sBuf);
	ParserResult ParseMODESRes(const string& sBuf);

	ParserResult ParseCWDCmd(const string& sBuf);
	ParserResult ParseCWDRes(const string& sBuf);

	ParserResult ParseSTORCmd(const string& sBuf);
	ParserResult ParseAPPECmd(const string& sBuf) ;
	ParserResult ParseRETRCmd(const string& sBuf);

	ParserResult ParseSTOR550Res(const string& sBuf);

	ParserResult ParseFEATRes(const string& sBuf);

private:
	CFtpDataConn* m_pDataConn;
	int m_nDataPort;
	string m_sDataIP;

	string m_sUser;
	FtpConnMode m_eConnMode;
	string m_sCurrentCmd;
	string m_sSvrWorkPath;
	string m_sSvrFileName;
	FtpDataTransferMode m_eDataTransferMode;
	FtpProtocolType m_eFtpProtocol;

	string m_sTempSvrWorkPath;

	static const FTP_MSG_MAP_ENTRY** GetFtpMsgMapEntriesAry();
};
