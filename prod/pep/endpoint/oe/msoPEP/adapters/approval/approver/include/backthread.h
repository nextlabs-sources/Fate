#ifndef __BACKTHREAD_H__
#define __BACKTHREAD_H__
#include "emailprocess.h"
#include "olHandler.h"
#include "zipencryptor.h"
#ifndef SECURITY_WIN32
#define SECURITY_WIN32
#endif
#include <security.h>
#include <secext.h>
struct BackThreadParam
{
	ApprovalEmail* pae;

	//CComPtr<Outlook::_olApplication> pAppOutlook;
	LPSTREAM	pAppStream;
	
	//CComPtr<Outlook::_MailItem>		pMailItem;
	LPSTREAM	pItemStream;
	
	CFtpSite				ftpSite;
	CMultiFTPManager::MATCH_TYPE matchType;
};

unsigned int __stdcall ZipFtpBackThread(void* lpParam);
#endif //__BACKTHREAD_H__
