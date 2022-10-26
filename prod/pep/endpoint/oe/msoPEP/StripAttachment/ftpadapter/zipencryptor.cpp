#include "stdafx.h"
#include "zipencryptor.h"

HMODULE CZipEncryptor::m_hZipAdapterModule=NULL;
CZipEncryptor::FTEncrypt CZipEncryptor::m_fpEncrypt=NULL;
const WCHAR CZipEncryptor::MODULENAME_ZIPADAPTER[]=L"zip_adapter.dll";
