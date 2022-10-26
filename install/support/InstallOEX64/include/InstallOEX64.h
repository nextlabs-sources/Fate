//OEInstallerPlugin.h
#pragma once

#include <stdio.h>
#include <tchar.h>
#include <windows.h>
#include <shellapi.h>
#include <string>
//////////////////////////////////////////////////////////////////////////

//Print help
void PrintUsage ( );

//Check OS's Version
void CheckOSVersion();

//Get NextLabs installation directory
BOOL GetNextLabsInstallDir ( std::wstring& InstallDir );

//Get OE installation directory
BOOL GetOEInstallDir ( std::wstring& InstallDir );

//Get PC installation directory
BOOL GetPCInstallDir ( std::wstring& InstallDir );

//Get PCPluginConfigDir installation directory
BOOL GetPCPluginConfigDir ( std::wstring& InstallDir );

//Get PCTamperResistanceConfigDir installation directory
BOOL GetPCTamperResistanceConfigDir ( std::wstring& InstallDir );

//Modify PCPluginConfigDir installation directory
BOOL ModifyPCPluginConfigDir ( );

//Modify PCTamperResistanceConfigDir installation directory
BOOL ModifyPCTamperResistanceConfigDir ( );

//Get Operating System 32/64 bit information
BOOL GetOSbitInfo ( BOOL& bIs64bit );

//Uninstall PlgIN
void UnInstallPlgIn();

//Modify files
BOOL ModifyFiles ( const std::wstring& SourcePath, const std::wstring& TextContent);

//Files copy 
void FilesCopy(const std::wstring& SourcePath, const std::wstring& DestinationPath);

//Set nl_OE_plugin.cfg file content
void  SetnlOEPluginFileContent(const std::wstring installPath, std::wstring& nlOEPluginFileContent);

//Set OutlookEnforcer_TamperResistance.cfg file content
void  SetOutlookEnforcerTamperResistanceFileContent(const std::wstring installPath, std::wstring& OutlookEnforcerTamperResistanceFileContent);