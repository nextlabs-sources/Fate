
#pragma once
#ifndef _EMAIL_PROCESS_H_
#define _EMAIL_PROCESS_H_
#include <assert.h>
#include <string>
#include <vector>
#include <stdlib.h>
#define CE_REQUEST_SCRIPT_BEGIN_TAG		L"<script language=\"JavaScript\">\r\n/*\r\n"
#define CE_REQUEST_SCRIPT_END_TAG		L"*/</script>"
#define CE_REQUEST_MAGIC				L"<!--Compliant Enforcer Request MAGIC Number F4052F2D-CCC0-49A0-B768-B6D3F68662E6-->\r\n"
//#define CE_REQUEST_MAGIC				L"--Compliant Enforcer Request MAGIC Number F4052F2D-CCC0-49A0-B768-B6D3F68662E6--\r\n"
#define CE_PARAMID_REQUEST_TYPE             L"CE_PARAMID_REQUEST_TYPE"
#define CE_PARAMID_REQUESTER_NAME           L"CE_PARAMID_REQUESTER_NAME"
#define CE_PARAMID_REQUESTER_ADDRESS        L"CE_PARAMID_REQUESTER_ADDRESS"
#define CE_PARAMID_REQUESTER_SID            L"CE_PARAMID_REQUESTER_SID"
#define CE_PARAMID_ARCHIVER_ADDRESS         L"CE_PARAMID_ARCHIVER_ADDRESS"
#define CE_PARAMID_APPROVAL_ADDRESS         L"CE_PARAMID_APPROVAL_ADDRESS"
#define CE_PARAMID_APPROVAL_DIRECTORY       L"CE_PARAMID_APPROVAL_DIRECTORY"
#define CE_PARAMID_CUSTOMER_KEY             L"CE_PARAMID_ENCRYPT_KEY"
#define CE_PARAMID_ENCRYPT_PSWD             L"CE_PARAMID_ENCRYPT_PSWD"
#define CE_PARAMID_CUSTOMER                 L"CE_PARAMID_CUSTOMER"
#define CE_PARAMID_ORIGINAL_SUBJECT         L"CE_PARAMID_ORIGINAL_SUBJECT"
#define CE_PARAMID_PURPOSE                  L"CE_PARAMID_PURPOSE"
#define CE_PARAMID_RECIPIENT                L"CE_PARAMID_RECIPIENT"
#define CE_PARAMID_CHECK_FILE               L"CE_PARAMID_CHECK_FILE"
#define CE_PARAMID_PROCESS_RESULT           L"CE_PARAMID_PROCESS_RESULT"

//FTP Server staff, e.g. FTP user, FTP password
#define CE_PARAMID_FTPSERVER				L"CE_PARAMID_FTPSERVER"
#define CE_PARAMID_FTPUSER					L"CE_PARAMID_FTPUSER"
#define CE_PARAMID_FTPPASSWD				L"CE_PARAMID_FTPPASSWD"
#define CE_PARAMID_FTPPORT					L"CE_PARAMID_FTPPORT"

#define CE_REQUEST_FTP                      L"Ftp"
#define CE_REQUEST_REMOVABLEMEDIA           L"Removable Media"
#define CE_REQUEST_EMAILATTACHMENT          L"Email"

#define EMAIL_TABLE_WIDTH                   600
#define EMAIL_CELL_WIDTH                    90

template<class T>
class InheritLock
{
public:
	//friend class T;
    InheritLock(){};
    ~InheritLock(){};
};

typedef std::pair<std::wstring, std::wstring>   FilePair;   // First is source path, Last is quarantine path
typedef std::vector<FilePair>                   FileVector;
typedef std::vector<std::wstring>               RecipientVector;
typedef std::vector<std::wstring>               ApproverVector;

class ApprovalEmail : private virtual InheritLock<ApprovalEmail>
{
private:
	std::wstring m_wstrFtpServer;
	std::wstring m_wstrFtpUser;
	std::wstring m_wstrFtpPasswd;
	int			m_iPort;
public:
    ApprovalEmail(){m_nMagicLength = (int)wcslen(CE_REQUEST_MAGIC); m_iPort=0;m_vFiles.clear();}
    ~ApprovalEmail()
	{
		if(m_vFiles.size()>0)
			m_vFiles.clear();
		if(m_vecFiles.size()>0)
			m_vecFiles.clear();
	}

    std::wstring Compose()
    {
		std::wstring strMail;
		strMail = CE_REQUEST_SCRIPT_BEGIN_TAG;
        strMail += CE_REQUEST_MAGIC;
        ApproverVector::iterator  ita;
        RecipientVector::iterator itr;
		std::vector<std::wstring>::iterator      itf;

        // set requester address
        /*strMail += L"<!--";
        strMail += CE_PARAMID_REQUEST_TYPE;
        strMail += L"=";
        strMail += m_strType;
        strMail += L"-->\r\n";*/

        // set requester address
		strMail += L"<!--";
        strMail += CE_PARAMID_REQUESTER_NAME;
        strMail += L"=";
        strMail += m_strRequesterName;
		strMail += L"-->\r\n";

        // set requester address
		strMail += L"<!--";
        strMail += CE_PARAMID_REQUESTER_ADDRESS;
        strMail += L"=";
        strMail += m_strRequesterAddr;
        strMail += L"-->\r\n";

        // set requester SID
        strMail += L"<!--";
        strMail += CE_PARAMID_REQUESTER_SID;
        strMail += L"=";
        strMail += m_strRequesterSid;
        strMail += L"-->\r\n";

        // set archiver address
        /*strMail += L"<!--";
        strMail += CE_PARAMID_ARCHIVER_ADDRESS;
        strMail += L"=";
        strMail += m_strArchiverAddr;
        strMail += L"-->\r\n";*/

        // set approvers
        for(ita = m_vApprovers.begin(); ita!=m_vApprovers.end(); ++ita)
        {
            strMail += L"<!--";
            strMail += CE_PARAMID_APPROVAL_ADDRESS;
            strMail += L"=";
            strMail += (*ita).c_str();
            strMail += L"-->\r\n";
        }

        // set approver directory
        /*strMail += L"<!--";
        strMail += CE_PARAMID_APPROVAL_DIRECTORY;
        strMail += L"=";
        strMail += m_strApprovalDir;
        strMail += L"-->\r\n";*/

        // set encrypt key
        /*strMail += L"<!--";
        strMail += CE_PARAMID_CUSTOMER_KEY;
        strMail += L"=";
        strMail += m_strCustomerKey;
        strMail += L"-->\r\n";*/

        // set encrypt password
        strMail += L"<!--";
        strMail += CE_PARAMID_ENCRYPT_PSWD;
        strMail += L"=";
        strMail += m_strEncryptPasswd;
        strMail += L"-->\r\n";

        // set customer
        /*strMail += L"<!--";
        strMail += CE_PARAMID_CUSTOMER;
        strMail += L"=";
        strMail += m_strCustomer;
        strMail += L"-->\r\n";*/
		
		// ftp server
		strMail += L"<!--";
        strMail += CE_PARAMID_FTPSERVER;
        strMail += L"=";
        strMail += m_wstrFtpServer;
        strMail += L"-->\r\n";

		// ftp user
		strMail += L"<!--";
        strMail += CE_PARAMID_FTPUSER;
        strMail += L"=";
        strMail += m_wstrFtpUser;
        strMail += L"-->\r\n";

		// ftp passwd
		strMail += L"<!--";
        strMail += CE_PARAMID_FTPPASSWD;
        strMail += L"=";
        strMail += m_wstrFtpPasswd;
        strMail += L"-->\r\n";
		//set subject
        strMail += L"<!--";
        strMail += CE_PARAMID_ORIGINAL_SUBJECT;
        strMail += L"=";
		strMail += this->m_strOrigSubject;
        strMail += L"-->\r\n";
		
        // set purpose
        strMail += L"<!--";
        strMail += CE_PARAMID_PURPOSE;
        strMail += L"=";
        strMail += m_strPurpose;
        strMail += L"-->\r\n";

        // set recipients
        for(itr = m_vRecipients.begin(); itr!=m_vRecipients.end(); ++itr)
        {
            strMail += L"<!--";
            strMail += CE_PARAMID_RECIPIENT;
            strMail += L"=";
            strMail += (*itr).c_str();
            strMail += L"-->\r\n";
        }

        // set files to be checked
        for(itf = m_vecFiles.begin(); itf!=m_vecFiles.end(); ++itf)
        {
            strMail += L"<!--";
            strMail += CE_PARAMID_CHECK_FILE;
            strMail += L"=";
            strMail += (*itf);
            /*strMail += L";";
            strMail += (*itf).second;*/
            strMail += L"-->\r\n";
        }
		strMail +=CE_REQUEST_SCRIPT_END_TAG;
        const std::wstring& strInfo = ComposeVisibleInformtion();
        strMail += strInfo;

        return strMail;
    }
    std::wstring ComposeVisibleInformtion()
    {
        std::wstring strInfo = L"";
        RecipientVector::iterator itr;
		std::vector<std::wstring>::iterator      itf;

        // Information Header
        strInfo += L"<table width=\"610\" style=\"font-family:Arial\" style=\"font-size:19\"><tr>\r\n";
        strInfo += L"    <td valign=\"center\" align=\"left\" height=\"50\"><b>";
        strInfo += m_strSubject;
        strInfo += L"</b></td>\r\n</tr></table>\r\n";
        // Information Body
        strInfo += L"<TABLE width=\"610\" style=\"font-family:Arial\" style=\"font-size:15\">\r\n";
        // add "pre information"
        strInfo += L"  <tr>\r\n";
        strInfo += L"    <td colspan=\"2\" valign=\"top\" align=\"left\">Please approve sharing of following files requested by ";
        strInfo += L"<a href=mailto:"; strInfo += m_strRequesterAddr; strInfo+=L">"; strInfo += m_strRequesterName; strInfo += L"</a>";
        /*strInfo += L" using "; strInfo += m_strType;*/ strInfo += L"<br>&nbsp;<br></td>\r\n";
        strInfo += L"  </tr>\r\n";
        // add "Subject"
        strInfo += L"  <tr>\r\n";
        strInfo += L"    <td valign=\"top\" align=\"left\" width=\"10%\"><b>Subject:</b></td>\r\n";  // 
        strInfo += L"    <td valign=\"top\" align=\"left\">";  strInfo += m_strOrigSubject; strInfo += L"<br>&nbsp;<br></td>\r\n";
        strInfo += L"  </tr>\r\n";
        // add Content/"Purpose"
        strInfo += L"  <tr>\r\n";
        strInfo += L"    <td valign=\"top\" align=\"left\"><b>Content:</b></td>\r\n";
        strInfo += L"    <td valign=\"top\" align=\"left\"><pre>";  strInfo += m_strPurpose; strInfo += L"<br></pre>&nbsp;<br></td>\r\n";
        strInfo += L"  </tr>\r\n";
        // add "Recipients"
        strInfo += L"  <tr>\r\n";
        strInfo += L"    <td valign=\"top\" align=\"left\"><b>Recipients:&nbsp;&nbsp;&nbsp;&nbsp;</b></td>\r\n";
        strInfo += L"    <td valign=\"top\" align=\"left\"><br>";
        for(itr = m_vRecipients.begin(); itr!=m_vRecipients.end(); ++itr)
        {
            strInfo += (*itr).c_str(); strInfo += L"<br>";
        }
        strInfo += L"&nbsp;<br></td>\r\n";
        strInfo += L"  </tr>\r\n";
        // add "Files"
        strInfo += L"  <tr>\r\n";
        strInfo += L"    <td valign=\"top\" align=\"left\"><b>Files:</b></td>\r\n";
        strInfo += L"    <td valign=\"top\" align=\"left\">\r\n<br>";
        for(itf = m_vecFiles.begin(); itf!=m_vecFiles.end(); ++itf)
        {
            
			strInfo += L"      <a href=\"file:////";
            strInfo += (*itf).c_str();
            strInfo += L"\">";
            strInfo += /*pwzFileName?(pwzFileName+1):*/((*itf).c_str());
            strInfo += L"</a><br>\r\n";
        }
        strInfo += L"    &nbsp;<br></td>\r\n";
        strInfo += L"  </tr>\r\n";

        strInfo += L"</TABLE>\r\n";

        return strInfo;
    }
    BOOL IsApprovalMail(LPCWSTR pwzMailBody)
	{ 
		std::wstring strOrig=pwzMailBody;
		std::wstring strPatten=CE_REQUEST_SCRIPT_BEGIN_TAG;
		strPatten+=CE_REQUEST_MAGIC;
		std::wstring::size_type nPos=strOrig.find(strPatten);
		if(nPos==-1)
			return FALSE;
		std::wstring strRequest=strOrig.substr(nPos+strPatten.length()-wcslen(CE_REQUEST_MAGIC));
		int bResult=wcsncmp(/*pwzMailBody*/strRequest.c_str(), CE_REQUEST_MAGIC, m_nMagicLength);
		if(bResult==0)
			return TRUE;
		return FALSE;
	}
    std::wstring ReplaceSubStr(std::wstring& strIn, LPCWSTR pwzSub, LPCWSTR pwzNew)
    {
        std::wstring::size_type stPos;
        std::wstring strSub = pwzSub;

        stPos = strIn.find(strSub);
        while(stPos != std::wstring::npos)
        {
            strIn.replace(stPos, strSub.size(), pwzNew);
            stPos = strIn.find(strSub, stPos);
        }

        return strIn;
    }
	BOOL HTMLDecode(const std::wstring &strSource,std::wstring &strDest)
	{
		//strSource=L"\\\\hz-ts02\\Upload\\jjin\\Custom Projects\\Flextronics\\&#20013;&#25991;\\approver\\srcdata\\&#20013;&#25991;\\Log.txt";
		int iPosBegin=0,iPosEnd=0;
		std::vector<std::wstring> vecTokens;
		while(iPosEnd!=-1)
		{
			iPosEnd=(int)strSource.find(L"&#",iPosBegin,2);
			std::wstring wstrOneSource=strSource.substr(iPosBegin,iPosEnd-iPosBegin);
			iPosBegin=iPosEnd+2;
			if(wstrOneSource.length())
				vecTokens.push_back(wstrOneSource);
		}
		//std::wstring strConverted;
		std::vector<std::wstring>::iterator itToken;
		for(itToken=vecTokens.begin();itToken!=vecTokens.end();itToken++)
		{
			std::wstring& strToken=(*itToken);
			/*if(itToken!=vecTokens.begin())
			{*/
				iPosBegin=(int)strToken.find(L";",0,1);
				if(iPosBegin!=(int)std::wstring::npos)
				{
					std::wstring strCh=strToken.substr(0,iPosBegin);
					int i=::_wtoi(strCh.c_str());
					if(i==0)
						strDest+=strToken;
					else
					{
						WCHAR wch=(WCHAR)i;
						strDest+=wch;
#pragma warning(push)
#pragma warning(disable: 4245)
						strCh=strToken.substr(iPosBegin+1,-1);
#pragma warning(pop)
						strDest+=strCh;
					}
				}
				else
				{
					strDest+=strToken;
				}
			/*}
			else
				strDest=strToken;*/
		}
		return TRUE;
	}
    BOOL Parse(LPCWSTR pwzMailBody)
    {
        if (!IsApprovalMail(pwzMailBody))
            return FALSE;

		std::wstring strDest;
		std::wstring strOrig=pwzMailBody;
		std::wstring strPatten=CE_REQUEST_SCRIPT_BEGIN_TAG;
		strPatten+=CE_REQUEST_MAGIC;
		std::wstring::size_type nPos=strOrig.find(strPatten);
		if(nPos==-1)
			return FALSE;
		std::wstring strRequest=strOrig.substr(nPos+strPatten.length()-wcslen(CE_REQUEST_MAGIC));

		pwzMailBody =strRequest.c_str();
        pwzMailBody += m_nMagicLength;
		

        do 
        {
			strDest=L"";
            const WCHAR* pwzSplitter = 0;
            std::wstring    strLine;
            std::wstring    strID;
            pwzSplitter = wcsstr(pwzMailBody, L"-->\r\n");
            if(pwzSplitter)
            {
                strLine.append(pwzMailBody, (pwzSplitter+3-pwzMailBody));
                pwzMailBody = pwzSplitter + 5;  // point to next line
            }
            else
            {
                strLine.append(pwzMailBody);    // It is last line
            }
            if(strLine == L"")                  // empty line
                break;
            const std::wstring& strValue = ParseCellLine(strLine.c_str(), strID);
            if(strID == L"")                    // it is the end of parameter
                break;
			//DP((L"]]]]]]]]]]]%s=%s",strID.c_str(),strValue.c_str()));
            if(0 == strID.compare(CE_PARAMID_REQUEST_TYPE))
            {
                m_strType = strValue;
                m_strSubject = L"Approval request for sharing information using "; 
                m_strSubject += m_strType;
            }
            else if(0 == strID.compare(CE_PARAMID_REQUESTER_NAME))
            {
                m_strRequesterName = strValue;
            }
            else if(0 == strID.compare(CE_PARAMID_REQUESTER_ADDRESS))
            {
                m_strRequesterAddr = strValue;
            }
            else if(0 == strID.compare(CE_PARAMID_REQUESTER_SID))
            {
                m_strRequesterSid = strValue;
            }
            else if(0 == strID.compare(CE_PARAMID_ARCHIVER_ADDRESS))
            {
                m_strArchiverAddr = strValue;
            }
            else if(0 == strID.compare(CE_PARAMID_APPROVAL_ADDRESS))
            {
                m_vApprovers.push_back(strValue);
            }
			else if(0 == strID.compare(CE_PARAMID_ORIGINAL_SUBJECT))
            {
				HTMLDecode(strValue,strDest);
				this->m_strOrigSubject=strDest;
            }
            else if(0 == strID.compare(CE_PARAMID_APPROVAL_DIRECTORY))
            {
                m_strApprovalDir = strValue;
                ReplaceSubStr(m_strApprovalDir, L"/", L"\\");
            }
            else if(0 == strID.compare(CE_PARAMID_CUSTOMER_KEY))
            {
                m_strCustomerKey = strValue;
                ReplaceSubStr(m_strCustomerKey, L"/", L"\\");
            }
            else if(0 == strID.compare(CE_PARAMID_ENCRYPT_PSWD))
            {
                m_strEncryptPasswd = strValue;
            }
            else if(0 == strID.compare(CE_PARAMID_CUSTOMER))
            {
                m_strCustomer = strValue;
            }
            else if(0 == strID.compare(CE_PARAMID_PURPOSE))
            {
				HTMLDecode(strValue,strDest);
                m_strPurpose = strDest;
            }
            else if(0 == strID.compare(CE_PARAMID_RECIPIENT))
            {
                m_vRecipients.push_back(strValue);
            }
            else if(0 == strID.compare(CE_PARAMID_PROCESS_RESULT))
            {
                m_strProcessResult = strValue;
            }
            else if(0 == strID.compare(CE_PARAMID_CHECK_FILE))
            {
                //const FilePair& filepair = ParseFilePair(strValue.c_str());
                //if(0!=filepair.first.length() && 0!=filepair.second.length())
                //    m_vFiles.push_back(filepair);
				HTMLDecode(strValue,strDest);
				m_vecFiles.push_back(strDest.c_str());

            }
			else if(0 == strID.compare(CE_PARAMID_FTPSERVER))
			{
				m_wstrFtpServer=strValue;
			}
			else if(0 == strID.compare(CE_PARAMID_FTPUSER))
			{
				m_wstrFtpUser=strValue;
			}
			else if(0 == strID.compare(CE_PARAMID_FTPPASSWD))
			{
				m_wstrFtpPasswd=strValue;
			}
			else if(0 == strID.compare(CE_PARAMID_FTPPORT))
			{
				this->m_iPort=_wtol(strValue.c_str());
			}
            else
            {
            }

            // Is it last line?
            if(!pwzSplitter)    // it is the last line
                break;
        } while(1);
		if(m_iPort==0)
			m_iPort=21;

		
        return TRUE;
    }

	
    std::wstring get_Subject(){ return m_strSubject; }

    std::wstring get_RequestType() { return m_strType; }
    void put_RequestType(LPCWSTR pwzType)
    {
        m_strType=pwzType;
        m_strSubject = L"Approval request for sharing information using "; 
        m_strSubject += m_strType;
    }

	void put_Subject(LPCWSTR pwzSubject)
	{
		m_strSubject = pwzSubject;
	}

	std::wstring get_OrigSubject(){return m_strOrigSubject;};
	void put_OrigSubject(LPCWSTR pwzOrigSubject){m_strOrigSubject=pwzOrigSubject;};

    std::wstring get_RequesterName() { return m_strRequesterName; }
    void put_RequesterName(LPCWSTR pwzName) { m_strRequesterName=pwzName; }

    std::wstring get_RequesterAddress() { return m_strRequesterAddr; }
    void put_RequesterAddress(LPCWSTR pwzAddr) { m_strRequesterAddr=pwzAddr; }

    std::wstring get_RequesterSid() { return m_strRequesterSid; }
    void put_RequesterSid(LPCWSTR pwzSid) { m_strRequesterSid=pwzSid; }

    std::wstring get_ArchiverAddress() { return m_strArchiverAddr; }
    void put_ArchiverAddress(LPCWSTR pwzAddr) { m_strArchiverAddr=pwzAddr; }

	void put_FtpServer(LPCWSTR wstrFtpServer){ m_wstrFtpServer=wstrFtpServer;}
	std::wstring get_FtpServer(){return m_wstrFtpServer;}

	void put_FtpUser(LPCWSTR wstrFtpUser){ m_wstrFtpUser=wstrFtpUser;}
	std::wstring get_FtpUser(){return m_wstrFtpUser;}

	void put_FtpPasswd(LPCWSTR wstrFtpPasswd){m_wstrFtpPasswd=wstrFtpPasswd;};
	std::wstring get_FtpPasswd(){return m_wstrFtpPasswd;}

	void put_FtpPort(int iPort){ m_iPort=iPort;}
	int get_FtpPort(){return m_iPort;}

    BOOL IsValidApprover(LPCWSTR pwzApprover)
    {
        ApproverVector::iterator  ita;
        assert(pwzApprover);
        for(ita=m_vApprovers.begin(); ita!=m_vApprovers.end(); ++ita)
        {
            if (0 == _wcsicmp(pwzApprover, (*ita).c_str()))
                return TRUE;
        }
        return FALSE;
    }
    const RecipientVector& get_Approvers(){return m_vApprovers;}
	void get_Approvers(std::wstring & strApprovers)
	{
		RecipientVector::iterator it=m_vApprovers.begin();
		for(it;it!=m_vApprovers.end();it++)
		{
			strApprovers+=(*it);
			strApprovers+=L";";
		}
	};
	std::vector<std::wstring>& get_vecFiles(){return m_vecFiles;}
    void add_Approver(LPCWSTR pwzApprover){assert(pwzApprover); m_vApprovers.push_back(pwzApprover);}

    std::wstring get_ApprovalDirectory() { return m_strApprovalDir; }
    void put_ApprovalDirectory(LPCWSTR pwzDirectory) { m_strApprovalDir=pwzDirectory; }

    std::wstring get_CustomerKey() { return m_strCustomerKey; }
    void put_CustomerKey(LPCWSTR pwzEncryptKey) { m_strCustomerKey=pwzEncryptKey; }

    std::wstring get_EncryptPasswd() { return m_strEncryptPasswd; }
    void put_EncryptPasswd(LPCWSTR pwzEncryptPasswd) { m_strEncryptPasswd=pwzEncryptPasswd; }

    std::wstring get_Customer() { return m_strCustomer; }
    void put_Customer(LPCWSTR pwzCustomer) { m_strCustomer=pwzCustomer; }

    std::wstring get_Purpose() { return m_strPurpose; }
    void put_Purpose(LPCWSTR pwzPurpose) { m_strPurpose=pwzPurpose; }

    std::wstring get_ProcessResult() { return m_strProcessResult; }
    void put_ProcessResult(LPCWSTR pwzProcessResult) { m_strProcessResult=pwzProcessResult; }

    const RecipientVector& get_Recipients(){return m_vRecipients;}
    void add_Recipient(LPCWSTR pwzRecipient)
    {
        assert(pwzRecipient);
        m_vRecipients.push_back(pwzRecipient);
    }

    const FileVector& get_Files(){return m_vFiles;}
    void add_File(LPCWSTR pwzSource, LPCWSTR pwzQuarantine)
    {
        assert(pwzSource); assert(pwzQuarantine);
        m_vFiles.push_back(FilePair(pwzSource, pwzQuarantine));
    }
	void add_File(LPCWSTR pwzSource)
    {
        assert(pwzSource);
        m_vecFiles.push_back(pwzSource);
    }

protected:
    std::wstring CreateCellLine(LPCWSTR pwzID, LPCWSTR pwzValue)
    {
        std::wstring strLine = L"<!--";
        strLine += pwzID;
        strLine += L"=";
        strLine += pwzValue;
        strLine += L"-->\r\n";
        return strLine;
    }
    
    std::wstring ParseCellLine(LPCWSTR pwzLine, std::wstring& strID)
    {
        int nLineLength = (int)wcslen(pwzLine);
        const WCHAR*    pwzSpliter = 0;
        std::wstring    strValue;

        // Check comment flag
        if(0 != wcsncmp(pwzLine, L"<!--", 4))
            return L"";
        if(0 != wcscmp((pwzLine+nLineLength-3), L"-->"))
            return L"";
        if(0 == (pwzSpliter=wcsstr(pwzLine, L"=")))
            return L"";

        strID.append(pwzLine+4, (pwzSpliter-pwzLine-4));
        strValue.append(pwzSpliter+1, ((int)wcslen(pwzSpliter)-4));
        return strValue;
    }

    std::wstring ComposeFilePair(LPCWSTR pwzSource, LPCWSTR pwzQuarantine)
    {
        std::wstring    strValue = pwzSource;
        strValue += L";";
        strValue += pwzQuarantine;
        return strValue;
    }
    std::wstring ComposeFilePair(const FilePair& filepair)
    {
        std::wstring    strValue = filepair.first;
        strValue += L";";
        strValue += filepair.second;
        return strValue;
    }

    FilePair    ParseFilePair(LPCWSTR pwzValue)
    {
        const WCHAR*    pwzSplitter = 0;
        FilePair filepair(L"", L"");
        std::wstring strSource;
        std::wstring strQuarantine;

        assert(pwzValue);
        if (0 == (pwzSplitter=wcsstr(pwzValue, L";")))
            return filepair;
        strSource.append(pwzValue, (pwzSplitter-pwzValue));
        ReplaceSubStr(strSource, L"/", L"\\");
        strQuarantine.append(pwzSplitter+1);
        ReplaceSubStr(strQuarantine, L"/", L"\\");
        filepair.first = strSource;
        filepair.second= strQuarantine;
        return filepair;
    }

private:
    std::wstring    m_strSubject;           // Subject
	std::wstring	m_strOrigSubject;		// Original Subject
    std::wstring    m_strType;              // Request type
    std::wstring    m_strRequesterName;     // Requester Name
    std::wstring    m_strRequesterAddr;     // Requester EMail Address
    std::wstring    m_strRequesterSid;      // Requester SID
    std::wstring    m_strArchiverAddr;      // Archive EMail address
    ApproverVector  m_vApprovers;           // Approvers
    std::wstring    m_strApprovalDir;       // Approval directory
    std::wstring    m_strCustomerKey;       // encrypt key file
    std::wstring    m_strEncryptPasswd;     // encrypt password
    std::wstring    m_strCustomer;          // Customer Name
    std::wstring    m_strPurpose;           // Purpose of this action
    std::wstring    m_strProcessResult;     // Process result: "Approval" "Reject"
    RecipientVector m_vRecipients;          // Recipients to which these files is sent
    FileVector      m_vFiles;               // Files to be checked
	std::vector<std::wstring> m_vecFiles;
    int             m_nMagicLength;
};

#endif