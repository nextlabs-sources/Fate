#ifndef __ENCRYPTION_WRAPPER_H
#define __ENCRYPTION_WRAPPER_H

typedef struct _tagEncryptAdapter
{
	std::wstring	wstrAdapterName;	// The key name of the Encryption Adapter
	HINSTANCE		hAdapterDLL;		// The handle of Encryption Adapter DLL
	std::wstring	wstrDLLPath;		// The file path of Encryption Adapter DLL
	std::wstring	wstrExtension;		// The encrypted file extensions
	BOOL			bSymmSupported;		// Whether the Encryption Adapter supports symmetric encryption
	BOOL			bAsymmSupported;	// Whether the Encryption Adapter supports asymmetric encryption
	_tagEncryptAdapter()
	{
		wstrAdapterName = L"";
		hAdapterDLL = NULL;
		wstrDLLPath = L"";
		wstrExtension = L"";
		bSymmSupported = FALSE;
		bAsymmSupported = FALSE;
	}
} EncryptAdapter, *LPEncryptAdapter;

typedef std::list<EncryptAdapter> AdapterList;

typedef struct _tagEncryptAdapters
{
	AdapterList		listAdapters;		// All registered Encryption Adapters
	std::wstring	wstrDefaultSymmAdapter;	// The default Encryption Adapter that supports symmetric encryption
	std::wstring	wstrDefaultAsymmAdapter;	// The default Encryption Adapter that supports asymmetric encryption
	_tagEncryptAdapters()
	{
		wstrDefaultSymmAdapter = L"";
		wstrDefaultAsymmAdapter = L"";
	}
} EncryptAdapters, *LPEncryptAdapters;

// Load all registered Encryption Adapters
PA_STATUS LoadRegisteredAdapters( void );
PA_STATUS UnloadRegisteredAdapters( void );

// Call the right Encryption Adapter to do file encryption
PA_STATUS EncryptionWrapper(LPEA_AssistantData lpAssistant);

#endif // __ENCRYPTION_WRAPPER_H