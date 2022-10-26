#ifndef MSOPEP_PARAMETERFORMULQUERY_H_
#define MSOPEP_PARAMETERFORMULQUERY_H_


#include <vector>
#include <map>
using namespace std;

#include "eframework/auto_disable/auto_disable.hpp"
#include "eframework/verdict_cache/verdict_cache.hpp"
#include "eframework/timer/timer_high_resolution.hpp"
#include "eframework/platform/cesdk_loader.hpp"
#include "eframework/platform/cesdk_attributes.hpp"

struct tagAttachInMsgFormat;
typedef struct tagAttachInMsgFormat *LpAttachInMsgFormat;
typedef std::vector<LpAttachInMsgFormat>   MSGATTACHLIST;

typedef struct tagAttachInMsgFormat
{
	wchar_t			m_path[MAX_SRC_PATH_LENGTH];	//contain the .msg file info,mainly about path
	MSGATTACHLIST	m_vecChilds;//all .msg attachments in an .msg file
	ATTACHMENTLIST	m_vecAttachs;//all other type files except .msg attachments.

}AttachInMsgFormat,*LpAttachInMsgFormat;

enum EmQueryResCommand
{
	BODY_COMMAND,
	SUBJECT_COMMAND,
	MSG_ATTACHMENT_COMMAND,
	ATTACHMENT_COMMAND,
	ATTACHMENT_RECIPIENT_AS_USER_COMMAND,
	GET_ATTRIBUTE_COMMAND,
	ATTACHEMENT_NXL_FILE_COMMAND,
	MESSAGE_HEADER_COMMAND,
	RECIPIENT_COMMAND
};

typedef struct 
{
	EmQueryResCommand command;
	std::wstring strFilePath;
	void *pAttachData;
	void *pMailAttach;
	int nAttachNum;
	int nRecipientNum;

}MulQueryPara ;

class ReleaseResource
{
public:
	ReleaseResource();
	~ReleaseResource();
public:
	vector<CEString*> m_vecRecipientsAddr;
	vector<CEString> m_vecCEstring;
	vector<std::wstring> m_vecFilePath;
	vector<CEResource*> m_vecResource;
	vector<CEAttributes*> m_vecSourceAttributes;
	vector<CEUser*> m_vecUser;
	vector<CEApplication*> m_vecApp;
	vector<CEAttributes*> m_vecUserAttributes;
};



class COPParaMeterCERequest
{
public:
	COPParaMeterCERequest();
	~COPParaMeterCERequest();
public:
	
	void SetRecipients(_In_ CEString **cestrRecipients);
	void SetSourceAttribute(_In_ CEAttributes** cesourceAttributes);
	void SetTarget(_In_ CEResource** ceTarget);
	void SetTargetAttribute(_In_ CEAttributes** ceTargetAttr);
	void SetUser(_In_ CEUser** ceuser);
	void SetUserAttribute(_In_ CEAttributes** ceUserAttr);
	void SetApp(_In_ CEApplication** ceApp);
	void SetAppAttributes(_In_ CEAttributes** ceAttr);
	void SetNumRecipients(_In_ const CEint32 num);
	void SetAdditionalAttributes(_In_ CENamedAttributes **ceAdditionalAttr);
	void SetNumAdditionalAttributes(_In_ const CEint32 num);
	void SetPerformObligation(_In_ const CEBoolean cePerformOBl);
	void SetNoiseLevel(_In_ const CENoiseLevel_t & NoiseLevel);
	void SetOperation(_In_ CEString* cestrOperation);
	void SetCEResource(_In_ CEResource **ceRes);

private:

public:
	CERequest m_CERequest;


};
#endif