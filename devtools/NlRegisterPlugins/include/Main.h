#pragma once

#include <windows.h>
#include <Msi.h>
#include <shellapi.h>
#include <string>

//////////////////////////////////////////////////////////////////////////

//The string for register
#define RegisterCommandString							L"--register"

//The string for unregister
#define UnregisterCommandString							L"--unregister"

//////////////////////////////////////////////////////////////////////////

//The related OE Office 32 bit plugin name
#define RelatedOEOffice32bitName						L"bin\\CEOffice32.dll"

//The related OE Office 64 bit plugin name
#define RelatedOEOffice64bitName						L"bin\\CEOffice.dll"

//The related OE Msopep 2003 plugin name
#define RelatedOEMsopep2K3Name						L"bin\\mso2k3pep32.dll"

//The related OE Msopep 2007 plugin name
#define RelatedOEMsopep2K7Name						L"bin\\mso2k7pep32.dll"

//The related OE Msopep 2010 32 bit plugin name
#define RelatedOEMsopep201032bitName			L"bin\\mso2010PEP32.dll"

//The related OE Msopep 2010 64 bit plugin name
#define RelatedOEMsopep201064bitName			L"bin\\mso2010PEP.dll"

//The related OE Explorer 32 bit plugin name
#define RelatedOEExplorer32bitName					L"bin\\CE_Explorer32.dll"

//The related OE Explorer 64 bit plugin name
#define RelatedOEExplorer64bitName					L"bin\\CE_Explorer.dll"

//////////////////////////////////////////////////////////////////////////

//The related WDE IEpep 32 bit plugin name
#define RelatedWDEIEpep32bitName					L"bin\\iePEP32.dll"

//The related WDE IEpep 64 bit plugin name
#define RelatedWDEIEpep64bitName					L"bin\\iePEP.dll"


// the office pep plugin name
#define RelatedOfficePepName	L"bin\\NLOfficePEP.dll";
// the office pep 32bit plugin name
#define RelatedOfficePEP32bitName L"bin\\NLOfficePEP32.dll"


//The related WDE Enhancement 32 bit plugin name
#define RelatedWDEEnhancement32bitName		L"bin\\enhancement32.dll"

//The related WDE Enhancement 64 bit plugin name
#define RelatedWDEEnhancement64bitName		L"bin\\enhancement.dll"

//The related WDE Outlook 32 bit add-in name
#define RelatedWDEOutlook32bitName				L"bin\\OutlookAddin32.dll"

//The related WDE Outlook 64 bit add-in name
#define RelatedWDEOutlook64bitName				L"bin\\OutlookAddin.dll"

//////////////////////////////////////////////////////////////////////////

//The related SE Explorer 32 bit plugin name
#define RelatedSEExplorer32bitName					L"bin\\NLPortableEncryptionCtx32.dll"

//The related SE Explorer 64 bit plugin name
#define RelatedSEExplorer64bitName					L"bin\\NLPortableEncryptionCtx.dll"

//////////////////////////////////////////////////////////////////////////

//The related CBE plugin name
#define RelatedWDEcbPep32bitName					L"bin\\cbPep32.dll"

#define RelatedWDEcbPep64bitName					L"bin\\cbPep.dll"

//ClSID of IEpep
const WCHAR IEpepCLSID[] = L"{FB159F40-0C40-4480-9A72-71C1D07606B7}";

//////////////////////////////////////////////////////////////////////////
//The related adobe pep
#define AdobeAcrobat11RegistryKey					L"\\Adobe Acrobat\\11.0\\Installer"
#define AdobeAcrobat10RegistryKey					L"\\Adobe Acrobat\\10.0\\Installer"
#define AdobeAcrobat9RegistryKey					L"\\Adobe Acrobat\\9.0\\Installer"
#define AdobeAcrobat2K15RegistryKey					L"\\Adobe Acrobat\\2015\\Installer"
#define AdobeAcrobat2K17RegistryKey					L"\\Adobe Acrobat\\2017\\Installer"
#define AdobeAcrobatDCRegistryKey					L"\\Adobe Acrobat\\DC\\Installer"
#define AcrobatReader10RegistryKey					L"\\Acrobat Reader\\10.0\\Installer"
#define AcrobatReader11RegistryKey					L"\\Acrobat Reader\\11.0\\Installer"
#define AcrobatReader9RegistryKey					L"\\Acrobat Reader\\9.0\\Installer"
#define AcrobatReaderDCRegistryKey					L"\\Acrobat Reader\\DC\\Installer"
#define AcrobatReader2K15RegistryKey				L"\\Acrobat Reader\\2015\\Installer"
#define AcrobatReader2K17RegistryKey				L"\\Acrobat Reader\\2017\\Installer"
//////////////////////////////////////////////////////////////////////////


//Print help message
void PrintUsage ( );

//////////////////////////////////////////////////////////////////////////

//Register or unregister all OE plugins
BOOL HandleAllOEPlugin ( const WCHAR* arg ); 

//Register or unregister OE Explorer plugins
BOOL HandleOEExplorerPlugin ( const WCHAR* arg ); 

//Register or unregister OE Msopep plugins
BOOL HandleOEMsopepPlugin ( const WCHAR* arg ); 

//Register or unregister OE Office plugins
BOOL HandleOEOfficePlugin ( const WCHAR* arg ); 

//////////////////////////////////////////////////////////////////////////

//Register or unregister all WDE plugins
BOOL HandleAllWDEPlugin ( const WCHAR* arg ); 

//Register or unregister WDE Enhancement plugins
BOOL HandleWDEEnhancementPlugin ( const WCHAR* arg ); 

//Register or unregister WDE IEpep plugins
BOOL HandleWDEIEpepPlugin ( const WCHAR* arg ); 

//Register or unregister WDE Overlay plugins
BOOL HandleWDEOverlayPlugin ( const WCHAR* arg ); 

//Register or unregister WDE Outlook add-in
BOOL HandleWDEOutlookAddin ( const WCHAR* arg ); 

//Register or unregister Office PEP add-in
BOOL HandleOfficePEPPlugin (__in const WCHAR* arg );
//Register or unregister Adobe PEP add-in
BOOL HandleAdobePEPPlugin (__in const WCHAR* arg );

//////////////////////////////////////////////////////////////////////////

//Register or unregister all SE plugins
BOOL HandleAllSEPlugin ( const WCHAR* arg ); 

//Register or unregister SE NlPortableEncryption plugin
BOOL HandleSENLPE ( const WCHAR* arg ); 

//Registrer or unregister Common Browser Enforcer
BOOL HandleCBE ( const WCHAR* arg);

//////////////////////////////////////////////////////////////////////////

//Get DE installation directory
BOOL GetDEInstallDir ( std::wstring& InstallDir );

//Get Common installation directory
BOOL GetCommonInstallDir ( std::wstring& InstallDir );

//Get Control Panel installation directory
BOOL GetControlPanelInstallDir ( std::wstring& InstallDir );

//Get OE installation directory
BOOL GetOEInstallDir ( std::wstring& InstallDir );

//Get SE installation directory
BOOL GetSEInstallDir ( std::wstring& InstallDir );

//Get SE installation directory
BOOL GetPCTamperDir ( std::wstring& PCTamperDir );

//Get regsvr32.exe name
BOOL GetRegsvr32Name ( std::wstring& strRegsvr32Name );

//Get Word version
BOOL GetWordVersion ( int& Version );

//Get Word 32/64bit information
BOOL GetWordbitInfo ( BOOL& bIs64bit );

//Get Outlook version and 32/64 bit
BOOL GetOutlookVersion ( int& Version, BOOL& bIs64Bit );

//Get Operating System 32/64 bit information
BOOL GetOSbitInfo ( BOOL& bIs64bit );

//////////////////////////////////////////////////////////////////////////

//Register module
BOOL RegisterModule ( const std::wstring& ModuleName );

//Unregister module
BOOL UnregisterModule ( const std::wstring& ModuleName );

//////////////////////////////////////////////////////////////////////////

BOOL GetAdobePath(std::wstring AdobeReg, std::wstring& AdobePathDir);
BOOL AcrobatCopyPlugin(std::wstring sourcefile, std::wstring dest, std::wstring dstfile);
BOOL ReaderCopyPlugin(std::wstring sourcefile, std::wstring dest, std::wstring dstfile);
BOOL AcrobatDeletePlugin(std::wstring dest, std::wstring filename);
BOOL ReaderDeletePlugin(std::wstring dest, std::wstring filename);
BOOL ProtectAdobeAPI(const std::wstring& destfile);

//////////////////////////////////////////////////////////////////////////

//Add register value to protect IEpep from disabled 
BOOL AddIEpepRegisterValue ( );

//Delete register value
BOOL DeleteIEpepRegisterValue ( );

//Add Adobe Reader protected mode whitelist register value
BOOL AddWhiteListRegisterValue ( );

//Add Adobe Reader 2017 protected mode whitelist register value
BOOL AddWhiteListRegisterValueFor2017 ( );

//Add Adobe Reader protected mode whitelist file
BOOL AddWhiteListFile ( );

//Add Adobe Reader 2017 protected mode whitelist file
BOOL AddWhiteListFileFor2017 ( );

//add or delete the keys for common browser enforcer
BOOL AddCbPepKeys();
BOOL DeleteCbPepKeys();

BOOL AddAdobeOLReflashKeys();
BOOL DelAdobeOLReflashKeys();

// disable preview panel on win7 if install WDE
BOOL SetPreviewPaneKey(bool bEnable);

// disable new for office file
BOOL DisableNewMenuKey(bool bDisable);

void AddEnvironmentVariable();