#pragma once
// Acrobat Headers.
#pragma warning(push)
#pragma warning(disable: 4819)  // We won't fix code page issue in 3rd party's header file, just ignore it here

#ifndef MAC_PLATFORM
#include "PIHeaders.h"
#endif

#pragma warning(pop)

#include <string>
using namespace std;

bool istempfile(const char* file);

void GetPathfromPDDoc(PDDoc doc, string& outfile);

bool getCurrentPDFPath(string& output_file);

const char* get_attachment_name(PDFileAttachment attachment);

void getCurrentPDDoc(PDDoc& pddoc);

std::wstring GetCommonComponentsDir();

void GetPathFromASFile(ASFile handle, string& outpath);

ASErrorCode real_asfileclose(ASFile handle);

bool IsMulti_BasedOnPages_Format(const string& strFormat);

bool IsOfficeFormat(const string& strFormat);

std::wstring GetEnforceBinDir();

bool IsAutoSaveFile(const string& strFile);

//is this file path a local file path or UNC file path
//false means not local path, like UNC path
bool IsLocalPath(LPCWSTR pszFileName);

std::string MyWideCharToMultipleByte(const std::wstring & strValue);
std::wstring MyMultipleByteToWideChar(const std::string & strValue);

bool IsURLPath(const string& path);
bool IsAcrobatcom(const string& path);
bool IsIETempFolder(const string& path);

string FormatPath(const string& path);

string GetPath(ASFileSys fileSys, ASPathName pathName);

//copied from boost
bool url_decode(const std::string& in, std::string& out);

BOOL isContentData(IDataObject *pDataObject);