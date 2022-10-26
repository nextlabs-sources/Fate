#include "StdAfx.h"
#include "WinAD.h"

CWinAD::CWinAD(void)
{
	CoInitialize(NULL);
	m_bGetSuccess =	ReadScheme();
}

CWinAD::~CWinAD(void)
{
	::CoUninitialize();
}

// // search current user's manager's email
bool CWinAD::SearchUserInfo(std::wstring& strEMail,std::wstring& strSID,const wchar_t* wstrkeyWord)
{
	if(!m_bGetSuccess)	return false;
	m_theUserInfo.strEmail=L"";
	m_theUserInfo.strSID=L"";
	// Initialize COM.
	HRESULT hr = S_OK;
	// Get rootDSE and the current user domain container distinguished name.
	CComPtr<IADs> pObject = NULL;
	CComPtr<IDirectorySearch> pContainerToSearch = NULL;
	LPOLESTR szPath = new OLECHAR[MAX_PATH];
	VARIANT var;
	hr = ADsOpenObject(L"LDAP://rootDSE",
		NULL,
		NULL,
		ADS_SECURE_AUTHENTICATION, // Use Secure Authentication.
		IID_IADs,
		(void**)&pObject);
	if (FAILED(hr))
	{
		//OutputDebugStringW(L"Cannot execute query. Cannot bind to LDAP://rootDSE.\n");
		delete[] szPath;
		return false;
	}
	if (SUCCEEDED(hr))
	{
		hr = pObject->Get(_bstr_t("defaultNamingContext"),&var);
		if (SUCCEEDED(hr))
		{
			// Build path to the domain container.
			wcsncpy_s(szPath,MAX_PATH/sizeof(OLECHAR),L"LDAP://", _TRUNCATE);
			wcsncat_s(szPath,MAX_PATH/sizeof(OLECHAR),var.bstrVal, _TRUNCATE);
			hr = ADsOpenObject(szPath,
				NULL,
				NULL,
				ADS_SECURE_AUTHENTICATION, // Use Secure Authentication.
				IID_IDirectorySearch,
				(void**)&pContainerToSearch);

			if (SUCCEEDED(hr))
			{
				// Convert to a char*
				if(wstrkeyWord == NULL)
					wstrkeyWord = m_strFileterKeyWords.c_str();
				hr = FindUsers(pContainerToSearch, // IDirectorySearch pointer to domainDNS container.
					(LPOLESTR)wstrkeyWord,  
					NULL, // Return all properties.
					TRUE 
					);
			}
		}
		VariantClear(&var);
	}

	delete [] szPath;	

	if (S_FALSE==hr || !SUCCEEDED(hr)) return false;

	strEMail = m_theUserInfo.strEmail;
	strSID = m_theUserInfo.strSID;
	if(strEMail.empty() ||
		_wcsicmp(strEMail.c_str(),L"")==0 ||
		strSID.empty() ||
		_wcsicmp(strSID.c_str(),L"")==0 )
		return false;
	return true;
}



/* ************************************************** */
HRESULT CWinAD::FindUsers(CComPtr<IDirectorySearch> pContainerToSearch,  // IDirectorySearch pointer to the container to search.
				  LPOLESTR szFilter, // Filter for finding specific users.
				  // NULL returns all user objects.
				  LPOLESTR *pszPropertiesToReturn, // Properties to return for user objects found.
				  // NULL returns all set properties.
				  BOOL bIsVerbose    // TRUE indicates that all properties for the found objects are displayed.
				  // FALSE indicates only the RDN.
				  )
{
	if (!pContainerToSearch)
		return E_POINTER;
	DWORD dwLength = MAX_PATH*2;
	// Create search filter.
	LPOLESTR pszSearchFilter = new OLECHAR[dwLength];

	// Add the filter.
	//swprintf_s(pszSearchFilter, dwLength, L"(&(objectClass=user)(objectCategory=person)%s)",szFilter);
	wnsprintf(pszSearchFilter, dwLength, L"(&(objectClass=user)(objectCategory=person)%ws)",szFilter); //m_strFileterKeyWords.c_str());

	std::wstring strKey = szFilter;
	bool bGetManager=false;
	if(strKey.find(L"sAMAccountName=") != std::wstring::npos)
	{
		bGetManager = true;
	}
	// Specify subtree search.
	ADS_SEARCHPREF_INFO SearchPrefs;
	SearchPrefs.dwSearchPref = ADS_SEARCHPREF_SEARCH_SCOPE;
	SearchPrefs.vValue.dwType = ADSTYPE_INTEGER;
	SearchPrefs.vValue.Integer = ADS_SCOPE_SUBTREE;
	DWORD dwNumPrefs = 1;

	// COL for iterations.
	LPOLESTR pszColumn = NULL;  
	ADS_SEARCH_COLUMN col;
	HRESULT hr = S_OK;

	// Interface Pointers
//	CComPtr<IADs> pObj = NULL;
//	CComPtr<IADs> pIADs = NULL;

	// Search handle.
	ADS_SEARCH_HANDLE hSearch = NULL;

	// Set search preference.
	hr = pContainerToSearch->SetSearchPreference( &SearchPrefs, dwNumPrefs);
	if (FAILED(hr))
	{
		delete[] pszSearchFilter;
		return hr;
	}

//	LPOLESTR pszBool = NULL;
//	DWORD dwBool = 0;
	PSID pObjectSID = NULL;
	LPOLESTR szSID = NULL;
//	LPOLESTR szDSGUID = new WCHAR [39];
//	LPGUID pObjectGUID = NULL;
	VARIANT varDate;
	LPOLESTR *pszPropertyList = NULL;
	LPOLESTR pszNonVerboseList[] = {L"name",L"distinguishedName"};

//	LPOLESTR szName = new OLECHAR[MAX_PATH];
	LPOLESTR szDN = new OLECHAR[MAX_PATH];

	VariantInit(&varDate);

	int iCount = 0;
//	DWORD x = 0L;



	if (!bIsVerbose)
	{
		// Return non-verbose list properties only.
		hr = pContainerToSearch->ExecuteSearch(pszSearchFilter,
			pszNonVerboseList,
			sizeof(pszNonVerboseList)/sizeof(LPOLESTR),
			&hSearch
			);
	}
	else
	{
		if (!pszPropertiesToReturn)
		{
			// Return all properties.
			hr = pContainerToSearch->ExecuteSearch(pszSearchFilter,
				NULL,
				(DWORD)-1,
				&hSearch
				);
		}
		else
		{
			// Specified subset.
			pszPropertyList = pszPropertiesToReturn;
			// Return specified properties.
			hr = pContainerToSearch->ExecuteSearch(pszSearchFilter,
				pszPropertyList,
				1,
				&hSearch
				);
		}
	}
	if ( SUCCEEDED(hr) )
	{  
		// Call IDirectorySearch::GetNextRow() to retrieve the next data row.
		hr = pContainerToSearch->GetFirstRow( hSearch);
		if (SUCCEEDED(hr))
		{
			while( hr != S_ADS_NOMORE_ROWS )
			{
				// Keep track of count.
				iCount++;
				// Loop through the array of passed column names,
				// print the data for each column.

				while( pContainerToSearch->GetNextColumnName( hSearch, &pszColumn ) != S_ADS_NOMORE_COLUMNS )
				{
					hr = pContainerToSearch->GetColumn( hSearch, pszColumn, &col );
					if(_wcsicmp(m_strMailName.c_str(),col.pszAttrName)==0)
					{
						switch(col.dwADsType)
						{
						case ADSTYPE_DN_STRING:
						case ADSTYPE_CASE_EXACT_STRING:    
						case ADSTYPE_CASE_IGNORE_STRING:    
						case ADSTYPE_PRINTABLE_STRING:    
						case ADSTYPE_NUMERIC_STRING:      
						case ADSTYPE_TYPEDNAME:        
						case ADSTYPE_FAXNUMBER:        
						case ADSTYPE_PATH:          
							if(col.dwNumValues>0)
							{
								m_theUserInfo.strEmail = col.pADsValues[0].CaseIgnoreString;
								if(!m_theUserInfo.strSID.empty() && !m_strManagerKey.empty())
								{
									pContainerToSearch->FreeColumn( &col );
									FreeADsMem( pszColumn );
									goto Return;
								}
							}
							break;
						case ADSTYPE_BOOLEAN:
							break;
						}
					}
					else if(_wcsicmp(m_strSidName.c_str(),col.pszAttrName) ==0)
					{
						switch(col.dwADsType)
						{
						case ADSTYPE_OCTET_STRING:
							{
								if(col.dwNumValues > 0)
								{
									pObjectSID = (PSID)(col.pADsValues[0].OctetString.lpValue);
									// Convert SID to string.
									ConvertSidToStringSid(pObjectSID, &szSID);
									m_theUserInfo.strSID = (wchar_t*)szSID;
									LocalFree(szSID);	
									if(!m_theUserInfo.strEmail.empty() && !m_strManagerKey.empty())
									{
										pContainerToSearch->FreeColumn( &col );
										FreeADsMem( pszColumn );
										goto Return;
									}
								}
							}
							break;
						default:
							break;
						}
					}
					else if(_wcsicmp(col.pszAttrName,L"manager")==0 && bGetManager)
					{
						switch(col.dwADsType)
						{
						case ADSTYPE_DN_STRING:
						case ADSTYPE_CASE_EXACT_STRING:    
						case ADSTYPE_CASE_IGNORE_STRING:    
						case ADSTYPE_PRINTABLE_STRING:    
						case ADSTYPE_NUMERIC_STRING:      
						case ADSTYPE_TYPEDNAME:        
						case ADSTYPE_FAXNUMBER:        
						case ADSTYPE_PATH:          
							if(col.dwNumValues>0)
							{
								m_strManagerKey = col.pADsValues[0].CaseIgnoreString;
								if(!m_theUserInfo.strEmail.empty() && !m_theUserInfo.strSID.empty())
								{
									pContainerToSearch->FreeColumn( &col );
									FreeADsMem( pszColumn );
									goto Return;
								}
							}
							break;
						case ADSTYPE_BOOLEAN:
							break;
						}
					}
					pContainerToSearch->FreeColumn( &col );
					FreeADsMem( pszColumn );
				}
				hr = pContainerToSearch->GetNextRow( hSearch);
			}

		}
Return:
		// Close the search handle to cleanup.
		pContainerToSearch->CloseSearchHandle(hSearch);
	} 
	if (SUCCEEDED(hr) && 0==iCount)
		hr = S_FALSE;

	//delete [] szName;
	delete [] szDN;
	//delete [] szDSGUID;
	delete [] pszSearchFilter;
	return hr;
}
