#include "stdafx.h"
#include "ParameterForMulQuery.h"

extern nextlabs::cesdk_loader cesdk;

ReleaseResource::ReleaseResource()
{

}
ReleaseResource::~ReleaseResource()
{
	if(!m_vecRecipientsAddr.empty())
	{
		vector<CEString*>::iterator itor;
		for (itor = m_vecRecipientsAddr.begin(); itor != m_vecRecipientsAddr.end(); itor++)
		{
			if (*itor)
			{
				delete [](*itor);
			}
		}
		m_vecRecipientsAddr.clear();
	}
	
	if (!m_vecSourceAttributes.empty())
	{
		vector<CEAttributes*>::iterator itorSourceAttr;
		for(itorSourceAttr = m_vecSourceAttributes.begin(); itorSourceAttr != m_vecSourceAttributes.end(); itorSourceAttr++)
		{
			for (int i=0; i<(*itorSourceAttr)->count; i++)
			{
				cesdk.fns.CEM_FreeString((*itorSourceAttr)->attrs[i].key);
				cesdk.fns.CEM_FreeString((*itorSourceAttr)->attrs[i].value);
			}
			if((*itorSourceAttr)->attrs)
			{
				delete []((*itorSourceAttr)->attrs);
			}
			if ((*itorSourceAttr))
			{
				delete (*itorSourceAttr);
			}
		}
		m_vecSourceAttributes.clear();
	}

	if(m_vecUserAttributes.empty())
	{
		vector<CEAttributes*>::iterator itorUserAttr;
		for(itorUserAttr = m_vecUserAttributes.begin(); itorUserAttr != m_vecUserAttributes.end(); itorUserAttr++)
		{
			cesdk.fns.CEM_FreeString((*itorUserAttr)->attrs->key);
			cesdk.fns.CEM_FreeString((*itorUserAttr)->attrs->value);
			
			if((*itorUserAttr)->attrs)
			{
				delete ((*itorUserAttr)->attrs);
			}
			if ((*itorUserAttr))
			{
				delete (*itorUserAttr);
			}
		}
		m_vecUserAttributes.clear();
	}

	if (!m_vecUser.empty())
	{
		vector<CEUser*>::iterator itorUser;
		for (itorUser = m_vecUser.begin(); itorUser != m_vecUser.end(); itorUser++)
		{
			cesdk.fns.CEM_FreeString((*itorUser)->userID);
			cesdk.fns.CEM_FreeString((*itorUser)->userName);
			if (*itorUser)
			{
				delete (*itorUser);
			}
		}
		m_vecUser.clear();
	}

	if (!m_vecApp.empty())
	{
		vector<CEApplication*>::iterator iterApp;
		for (iterApp = m_vecApp.begin(); iterApp != m_vecApp.end(); iterApp++)
		{
			cesdk.fns.CEM_FreeString((*iterApp)->appURL);
			cesdk.fns.CEM_FreeString((*iterApp)->appName);
			cesdk.fns.CEM_FreeString((*iterApp)->appPath);
			if (*iterApp)
			{
				delete (*iterApp);
			}
		}
		m_vecApp.empty();
	}


	if (!m_vecFilePath.empty())
	{
		vector<std::wstring>::iterator iterVecstrFilePath;
		for (iterVecstrFilePath = m_vecFilePath.begin(); iterVecstrFilePath != m_vecFilePath.end(); iterVecstrFilePath++)
		{
			::DeleteFile((*iterVecstrFilePath).c_str());
		}
		m_vecFilePath.clear();
	}

	if (m_vecResource.empty())
	{
		vector<CEResource*>::iterator iterVecSource;
		for (iterVecSource = m_vecResource.begin(); iterVecSource != m_vecResource.end(); iterVecSource++)
		{
			cesdk.fns.CEM_FreeResource(*iterVecSource);
		}
		m_vecResource.clear();
	}
}



COPParaMeterCERequest::COPParaMeterCERequest()
{
	m_CERequest.additionalAttributes = NULL;
	m_CERequest.app = NULL;
	m_CERequest.appAttributes = NULL;
	m_CERequest.noiseLevel = CE_NOISE_LEVEL_USER_ACTION;
	m_CERequest.numAdditionalAttributes = 0;
	m_CERequest.numRecipients = 0;
	m_CERequest.operation = NULL;
	m_CERequest.performObligation = CETrue;
	m_CERequest.recipients = NULL;
	m_CERequest.source = NULL;
	m_CERequest.sourceAttributes = NULL;
	m_CERequest.target = NULL;
	m_CERequest.targetAttributes = NULL;
	m_CERequest.user = NULL;
	m_CERequest.userAttributes = NULL;
}
COPParaMeterCERequest::~COPParaMeterCERequest()
{

}

void COPParaMeterCERequest::SetRecipients(_In_ CEString **cestrRecipients)
{
	m_CERequest.recipients = *cestrRecipients;
}

void COPParaMeterCERequest::SetSourceAttribute(_In_ CEAttributes ** cesourceAttributes)
{
	m_CERequest.sourceAttributes = *cesourceAttributes;
}

void COPParaMeterCERequest::SetTarget(_In_ CEResource** ceTarget)
{
	
	m_CERequest.target = *ceTarget;
}

void COPParaMeterCERequest::SetTargetAttribute(_In_ CEAttributes** ceTargetAttr)
{
	m_CERequest.targetAttributes = *ceTargetAttr;
}

void COPParaMeterCERequest::SetUser(_In_ CEUser **ceuser)
{
	m_CERequest.user = *ceuser;
}

void COPParaMeterCERequest::SetUserAttribute(_In_ CEAttributes** ceUserAttr)
{
	m_CERequest.userAttributes = *ceUserAttr;
}

void COPParaMeterCERequest::SetApp(_In_ CEApplication **ceApp)
{
	m_CERequest.app = *ceApp;
}

void COPParaMeterCERequest::SetAppAttributes(_In_ CEAttributes** ceAttr)
{
	m_CERequest.appAttributes = *ceAttr;
}

void COPParaMeterCERequest::SetNumRecipients(_In_ const CEint32 num)
{
	m_CERequest.numRecipients = num;
}

void COPParaMeterCERequest::SetAdditionalAttributes(_In_ CENamedAttributes **ceAdditionalAttr)
{
	m_CERequest.additionalAttributes = *ceAdditionalAttr;
}

void COPParaMeterCERequest::SetNumAdditionalAttributes(_In_ const CEint32 num)
{
	m_CERequest.numAdditionalAttributes = num;
}

void COPParaMeterCERequest::SetPerformObligation(_In_ const CEBoolean cePerformOBl)
{
	m_CERequest.performObligation =cePerformOBl;
}

void COPParaMeterCERequest::SetNoiseLevel(_In_ const CENoiseLevel_t & NoiseLevel)
{
	m_CERequest.noiseLevel = NoiseLevel;
}

void COPParaMeterCERequest::SetOperation(_In_ CEString *cestrOperation)
{
	m_CERequest.operation = *cestrOperation;
}

void COPParaMeterCERequest::SetCEResource(_In_ CEResource **ceRes)
{
	m_CERequest.source = *ceRes;
}