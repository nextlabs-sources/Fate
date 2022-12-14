// TagFiles.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "resattrmgr.h"
#include "celog.h"
#include "celog_policy_windbg.hpp"
#include "celog_policy_file.hpp"
#include "celog_policy_stderr.hpp"

static bool bDeleteTags = false;
static CELog fail_log;
static CELog succeed_log;


#define LOG_MAX_SIZE (1024 * 1024 * 1024)  /* 1G */

static void usage(const char *app)
{
	printf("Usage\n");
	printf("%s tagkey1=tagval1,tagkey2=tagval2,...,tagkeyn=tagvaln [-delete] [-log log_filename] [-verbose] [-tagType NTFS] {-data filename | filename }\n\n", app);
	printf("  Tags are specified as a comma separated list of key=value.\n");
	printf("  Tags will be applied in order.  If a later tag key has the same\n");
	printf("  name as an earlier one the later one will take precedence\n\n");
	printf("[-tagType NTFS]\n");
	printf("  Forces the tagging to be done using NTFS streams rather than any\n");
	printf("  document specific method\n\n");
	printf("[-delete]\n");
	printf("  delete specified tags for specified files\n\n");
	printf("[-log log_filename]\n");
	printf("  logs will be output to log_filename\n\n");
	printf("[-verbose]\n");
	printf("  print log messages to console \n\n");
	printf("-data filename\n");
	printf("  filename will be treated as a carriage return separated list of file\n");
	printf("  names.  The specifed tags will be applied to each file\n\n");
	printf("filename\n");
	printf("  The tags will be applied to this file\n\n");

	exit(-1);
}

typedef std::pair<std::string, std::string> attrpair;
typedef std::vector<std::pair<std::string, std::string>> attrvec;

static bool get_all_tags_string(const ResourceAttributes* from, std::string& strtags)
{
	std::wstring wstrtags;
	int size = GetAttributeCount(from);
	for (int i = 0; i < size; ++i)
	{
		wstrtags += GetAttributeName(from, i);
		wstrtags += L" = ";
		wstrtags += GetAttributeValue(from, i);
		if (i != size - 1)
		{
			wstrtags += L", ";
		}
	}

	std::string tmp(wstrtags.begin(), wstrtags.end());
	strtags = tmp;

	return true;
}

static void logError(const std::string &fileName)
{
	std::ofstream output("tagging.log", std::ios::app);

	output << "Unable to write tags to file " << fileName << std::endl;
	output.close();
}

static std::pair<std::string, std::string> split(const std::string& str, char splitchar)
{
	std::string::size_type pos = str.find_first_of(splitchar, 0);

	if (std::string::npos == pos || pos == 0) {
		return attrpair(str, "");
	} else {
		return attrpair(str.substr(0, pos), str.substr(pos+1, std::string::npos));
	}
}

static void tokenize(const std::wstring& str, std::vector<std::wstring>& tokens, wchar_t splitAt)
{
	std::wstring::size_type lastPos = 0;
	std::wstring::size_type pos = str.find_first_of(splitAt, lastPos);

	while (std::wstring::npos != pos || std::wstring::npos != lastPos)
	{
		// Found a token, add it to the vector.
		tokens.push_back(str.substr(lastPos, pos - lastPos));
		if (pos != std::wstring::npos)
		{
			pos = str.find_first_not_of(L' ', pos+1);
		}
		lastPos = pos;
		pos = str.find_first_of(splitAt, lastPos);
	}
}

static void tokenize(const std::string& str, std::vector<std::string>& tokens, char splitAt)
{
	std::string::size_type lastPos = 0;
	std::string::size_type pos = str.find_first_of(splitAt, lastPos);

	while (std::string::npos != pos || std::string::npos != lastPos)
	{
		// Found a token, add it to the vector.
		tokens.push_back(str.substr(lastPos, pos - lastPos));
		if (pos != std::string::npos)
		{
			pos = str.find_first_not_of(' ', pos+1);
		}
		lastPos = pos;
		pos = str.find_first_of(splitAt, lastPos);
	}
}

static void getkeyvals(const std::string& str, attrvec& keyvalue)
{
	std::vector<std::string> keyvals;
	tokenize(str, keyvals, ',');

	for (std::vector<std::string>::const_iterator iter = keyvals.begin();
		iter != keyvals.end();
		++iter)
	{
		keyvalue.push_back(split(*iter, '='));
	}

	return;
}

static ResourceAttributes *convertToAttributes(attrvec& keyvals)
{
	ResourceAttributes *attrs;
	AllocAttributes(&attrs);

	for (attrvec::const_iterator iter = keyvals.begin();
		iter != keyvals.end();
		++iter)
	{
		AddAttributeA(attrs, (*iter).first.c_str(), (*iter).second.c_str());
	}

	return attrs;
}

/*
* This is a near duplicate of UpdateIndexTag in filetaggingPA under
* enforcers.  Unfortunatly that code was not written with a mind to
* being usable outside of that library
*/
static void addIndexTag(ResourceAttributeManager *mgr, const std::string& infile, ResourceAttributes *attrs, ResourceAttributes *NextlabsIndexattrs)
{
	ResourceAttributes *currentAttributes;
	AllocAttributes(&currentAttributes);
	ReadResourceAttributesA(mgr, infile.c_str(), currentAttributes);

	int currentSize = GetAttributeCount(currentAttributes);
	const WCHAR *value = L"";

	for (int i = 0; i < currentSize; ++i)
	{
		if (wcscmp(GetAttributeName(currentAttributes, i), L"NXTLBS_TAGS") == 0)
		{
			value = GetAttributeValue(currentAttributes, i);
			break;
		}
	}

	std::wstring nextlabsIndex = value;
	FreeAttributes(currentAttributes);
	std::vector<std::wstring> currentKeys;

	// The value contains a ; separated list of our tags.  We break it apart, check to see
	// that each key in 'attrs' is mentioned, add any ones that aren't, and then add the
	// index to the set of attributes to be written
	if (value != NULL)
	{
		tokenize(nextlabsIndex, currentKeys, L';');
	}

	if (!bDeleteTags)
	{
		int size = GetAttributeCount(attrs);
		for (int j = 0; j < size; ++j)
		{
			bool found = false;
			const WCHAR *key = GetAttributeName(attrs, j);

			for (std::vector<std::wstring>::const_iterator iter = currentKeys.begin();
				iter != currentKeys.end();
				++iter)
			{

				if (wcscmp((*iter).c_str(), key) == 0)
				{
					found = true;
					break;
				}
			}


			if (!found)
			{
				nextlabsIndex.append(std::wstring(key));
				nextlabsIndex.append(L";");
			}
		}
		AddAttributeW(NextlabsIndexattrs, L"NXTLBS_TAGS", nextlabsIndex.c_str());
	}
	else
	{
		int size = GetAttributeCount(attrs);
		for (int j = 0; j < size; ++j)
		{
			bool found = false;
			const WCHAR *key = GetAttributeName(attrs, j);

			for (std::vector<std::wstring>::const_iterator iter = currentKeys.begin();
				iter != currentKeys.end();
				++iter)
			{
				if (wcscmp((*iter).c_str(), key) == 0)
				{
					found = true;
					currentKeys.erase(iter);
					break;
				}
			}
		}
		std::wstring new_nextlabsIndex;
		for (std::vector<std::wstring>::const_iterator iter = currentKeys.begin();
			iter != currentKeys.end();
			++iter)
		{
			new_nextlabsIndex.append((*iter));
			new_nextlabsIndex.append(L";");
		}
		AddAttributeW(attrs, L"NXTLBS_TAGS", L"key");
		if (new_nextlabsIndex.length())
		{
			AddAttributeW(NextlabsIndexattrs, L"NXTLBS_TAGS", new_nextlabsIndex.c_str());
		}
	}
}

static ResourceAttributes *copyAttributes(const ResourceAttributes *from)
{
	// Make a copy of the attributes because we are going to change them
	ResourceAttributes *attrCopy;
	AllocAttributes(&attrCopy);
	int size = GetAttributeCount(from);
	for (int i = 0; i < size; ++i)
	{
		AddAttributeW(attrCopy, GetAttributeName(from, i), GetAttributeValue(from, i));
	}

	return attrCopy;
}

/*

Tagfiles.exe should add a new tag/tags with  a corresponding value/values if the tag/tags with that name does not already exist.  

Tagfiles.exe should update only the value of a tag with the corresponding value if the tag with that name already exists.

*/
static int ReplaceResourceAttributesA(ResourceAttributeManager *mgr, const char *filename, ResourceAttributes *attrs)
{
	RemoveResourceAttributesA(mgr, filename, attrs);

	return WriteResourceAttributesA(mgr, filename, attrs);
}

static bool fileExists(const std::string& filename)
{
	std::ifstream input(filename.c_str(), std::ifstream::in);
	input.close();

	return !input.fail();
}

static void setAttributesForFiles(ResourceAttributeManager *mgr, std::string& infile, ResourceAttributes *attrs)
{
	if (fileExists(infile))
	{
		bool foundData = false;

		std::ifstream input(infile.c_str(), std::ifstream::in);
		std::string l;
		while(std::getline(input, l))
		{
			foundData = true;

			if (!fileExists(l))
			{
				fail_log.Log(CELOG_DEBUG, "failed to tag file in function setAttributesForFiles: file [%s] don't exist\n", l.c_str());
				continue;
			}


			ResourceAttributes *copyOfAttrs = copyAttributes(attrs);

			ResourceAttributes* NextlabsIndexattrs;
			AllocAttributes(&NextlabsIndexattrs);
			addIndexTag(mgr, l.c_str(), copyOfAttrs, NextlabsIndexattrs);

			std::string strtags;
			get_all_tags_string(copyOfAttrs, strtags);


			if (bDeleteTags)
			{
				if (!RemoveResourceAttributesA(mgr, l.c_str(), copyOfAttrs))
				{
					fail_log.Log(CELOG_DEBUG, "failed to delete tags in file [%s] from file [%s] with %s\n", l.c_str(), infile.c_str(), strtags.c_str());
				}
				else
				{
					succeed_log.Log(CELOG_DEBUG, "successfully delete tags in file [%s] from file [%s] with %s\n", l.c_str(), infile.c_str(), strtags.c_str());
					if (GetAttributeCount(NextlabsIndexattrs))
					{
						ReplaceResourceAttributesA(mgr, l.c_str(), NextlabsIndexattrs);
					}
				}
			}
			else
			{
				DWORD dwstart = GetTickCount();
				if (!ReplaceResourceAttributesA(mgr, l.c_str(), copyOfAttrs))
				{
					DWORD diff = GetTickCount() - dwstart;			
					fail_log.Log(CELOG_DEBUG, "failed to tag file [%s] from file [%s] with %s (%dms)\n", l.c_str(), infile.c_str(), strtags.c_str(), diff);
					logError(l);
				}
				else
				{
					DWORD diff = GetTickCount() - dwstart;
					if (GetAttributeCount(NextlabsIndexattrs))
					{
						ReplaceResourceAttributesA(mgr, l.c_str(), NextlabsIndexattrs);
					}
					succeed_log.Log(CELOG_DEBUG, "successfully tagged file [%s] from file [%s] with %s (%dms)\n", l.c_str(), infile.c_str(), strtags.c_str(), diff);
				}
			}
			FreeAttributes(copyOfAttrs);
			FreeAttributes(NextlabsIndexattrs);
		}
		input.close();

		if (foundData == false)
		{
			std::string strtags;
			get_all_tags_string(attrs, strtags);
			fail_log.Log(CELOG_DEBUG, "failed to tag file as [%s] is empty, with %s\n", infile.c_str(), strtags.c_str());
			std::cout << "Datafile " << infile << " was empty\n";
		}
	}
	else
	{
		std::string strtags;
		get_all_tags_string(attrs, strtags);

		fail_log.Log(CELOG_DEBUG, "failed to tag file as [%s] does not exist or cannot be read, with %s\n", infile.c_str(), strtags.c_str());
		std::cout << "Datafile " << infile << " does not exist or cannot be read\n";
	}
}

static attrvec keyvals;
static bool ntfsOnlyTags = false;
static bool useInputFile = false;
static std::string inputFileName;
static std::string dataFileName;
static bool blogtofile = false;

static void initializeResourceAttributeManager(ResourceAttributeManager **mgr)
{
	if (ntfsOnlyTags)
	{
		CreateAttributeManager(mgr);

		ResourceAttributes *attrs;
		AllocAttributes(&attrs);
		AddAttributeA(attrs, "Tagging", "NTFS");
		FreeAttributes(attrs);
	}
	else
	{
		CreateAttributeManager(mgr);
	}
}

void processCommandLineArguments(int argc, char *argv[])
{
	if (argc < 3) {
		usage(argv[0]);
	}

	getkeyvals(std::string(argv[1]), keyvals);

	for (int i = 2; i < argc; i++)
	{
		if (strcmp(argv[i], "-tagType") == 0)
		{
			if (i+1 < argc && strcmp(argv[i+1], "NTFS")==0)
			{
				++i;
				ntfsOnlyTags = true;
			}
			else
			{
				usage(argv[0]);
			}
		}
		else if (strcmp(argv[i], "-data") == 0)
		{
			if (i+1 < argc)
			{
				useInputFile = true;
				inputFileName = argv[i+1];
				++i;
			}
			else
			{
				usage(argv[0]);
			}
		}
		else if(strcmp(argv[i], "-log") == 0)
		{
			std::string logFileName;
			if (i+1 < argc)
			{
				logFileName = argv[i+1];
				++i;

				char path[1025] = {0};
				GetModuleFileNameA(NULL, path, 1024);

				char* tmp = strrchr(path, '\\');
				*(tmp + 1)= 0;


				std::string logpath = path;
				logpath += logFileName;


				std::string fail_logpath = logpath + "_error.log";
				std::string succeed_logpath = logpath + "_success.log";

				CELogPolicy_File* pFail = new CELogPolicy_File(fail_logpath.c_str());
				CELogPolicy_File* pOk = new CELogPolicy_File(succeed_logpath.c_str());
				pFail->SetMaxLogSize(LOG_MAX_SIZE);
				pOk->SetMaxLogSize(LOG_MAX_SIZE);
				fail_log.SetPolicy( pFail );
				succeed_log.SetPolicy( pOk );



				fail_log.Enable();                              // enable log
				fail_log.SetLevel(CELOG_DEBUG);                 // log threshold to debug level


				succeed_log.Enable();                              // enable log
				succeed_log.SetLevel(CELOG_DEBUG);                 // log threshold to debug level


				OutputDebugStringA(fail_logpath.c_str());
				OutputDebugStringA(succeed_logpath.c_str());

				blogtofile = true;
			}
			else
			{
				usage(argv[0]);
			}
		}
		else if (strcmp(argv[i], "-verbose") == 0)
		{
			fail_log.SetPolicy( new CELogPolicy_Stderr() );
			succeed_log.SetPolicy( new CELogPolicy_Stderr() );

			fail_log.Enable();                              // enable log
			fail_log.SetLevel(CELOG_DEBUG);                 // log threshold to debug level

			succeed_log.Enable();                              // enable log
			succeed_log.SetLevel(CELOG_DEBUG);                 // log threshold to debug level
		}
		else if(strcmp(argv[i], "-delete") == 0)
		{
			bDeleteTags = true;
		}
		else
		{
			dataFileName = argv[i];

			// This will be the last thing if it appears
			break;
		}
	}

	if (dataFileName == "" && inputFileName == "")
	{
		usage(argv[0]);
	}
}


int main(int argc, char* argv[])
{
	/*
	this tool does not read existing tags out before add new tag, so it does not support multi-value tags,
	Tyco does not need multi-values tag, That is for KLA only.
	But this needs to be done when we merge multi-values tag back to product.
	*/
	processCommandLineArguments(argc, argv);

	ResourceAttributeManager *mgr = NULL;

	initializeResourceAttributeManager(&mgr);
	ResourceAttributes *attrs = convertToAttributes(keyvals);

	if (useInputFile)
	{
		setAttributesForFiles(mgr, inputFileName, attrs);
	}
	else
	{
		if (!fileExists(dataFileName))
		{
			fail_log.Log(CELOG_DEBUG, "failed to tag file in dataFileName case: file [%s] don't exist\n", dataFileName.c_str());
		}
		else
		{
			ResourceAttributes* NextlabsIndexattrs;
			AllocAttributes(&NextlabsIndexattrs);
			addIndexTag(mgr, dataFileName, attrs, NextlabsIndexattrs);


			std::string strtags;
			get_all_tags_string(attrs, strtags);

			if (bDeleteTags)
			{
				if (!RemoveResourceAttributesA(mgr, dataFileName.c_str(), attrs))
				{
					fail_log.Log(CELOG_DEBUG, "failed to delete tags in file [%s], with %s\n", dataFileName.c_str(), strtags.c_str());
				}
				else
				{
					if (GetAttributeCount(NextlabsIndexattrs))
					{
						ReplaceResourceAttributesA(mgr, dataFileName.c_str(), NextlabsIndexattrs);
					}
					succeed_log.Log(CELOG_DEBUG, "successfully delete tags in file [%s], with %s\n", dataFileName.c_str(), strtags.c_str());
				}
			}
			else
			{
				DWORD dwstart = GetTickCount();
				if (!ReplaceResourceAttributesA(mgr, dataFileName.c_str(), attrs))
				{
					DWORD diff = GetTickCount() - dwstart;					
					fail_log.Log(CELOG_DEBUG, "failed to tag file [%s] on API [ReplaceResourceAttributesA], with %s, (%dms)\n", dataFileName.c_str(), strtags.c_str(), diff);
					logError(dataFileName);
				}
				else
				{
					if (GetAttributeCount(NextlabsIndexattrs))
					{
						ReplaceResourceAttributesA(mgr, dataFileName.c_str(), NextlabsIndexattrs);
					}
					DWORD diff = GetTickCount() - dwstart;
					succeed_log.Log(CELOG_DEBUG, "successfully tagged file [%s] on API [ReplaceResourceAttributesA], with %s, (%dms)\n", dataFileName.c_str(), strtags.c_str(), diff);
				}
			}
			FreeAttributes(NextlabsIndexattrs);
		}
	}

	FreeAttributes(attrs);
	return 0;
}

