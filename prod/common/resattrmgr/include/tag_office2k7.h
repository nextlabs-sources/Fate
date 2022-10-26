// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the tag_office2k7
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// PA_RMSCORE_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.

#ifdef TAG_OFFICE2K7_EXPORTS
#define TAG_OFFICE2K7_API __declspec(dllexport)
#else
#define TAG_OFFICE2K7_API __declspec(dllimport)
#endif

#define ERROR_NO_ERROR	0
#define GENEROR_ERROR	-1
#define	UNZIP_FILE_FAILED	-2
#define ZIP_FILE_FAILED	-3
#define	FILE_NOT_SUPPORT	-4
#define FILE_NOT_EXIST	-5
#define FILE_IS_USING	-6
#define TAG_FAILED	-8
#define READ_ONLY_FILE	-10

typedef enum
{
	enumReadTag,
	enumAddTag,
	enumDeleteTag,
	enumReadSummaryTag
}tagOperate;

typedef struct struTag_Pair 
{
	std::wstring strTagName;
	std::wstring strTagValue;
	bool bSuccess;//fill TRUE if add/get/remove tags successfully
}TAGPAIR, *LPTAGPAIR;


extern "C" {

	TAG_OFFICE2K7_API int AddTag(LPCWSTR pszFileName, LPCWSTR pszTagName, LPCWSTR pszTagValue, bool bRewriteIfExists);

	TAG_OFFICE2K7_API int AddTags(LPCWSTR pszFileName, std::vector<TAGPAIR>& listTags, bool bRewriteIfExists);

	TAG_OFFICE2K7_API int GetTag(LPCWSTR pszFileName, LPCWSTR pszTagName, OUT std::wstring& strTagValue);
	TAG_OFFICE2K7_API int GetTags(LPCWSTR pszFilename, OUT std::vector<TAGPAIR>& listTags);// Fill in the tag values for the indicated tag names.

	TAG_OFFICE2K7_API int DeleteTag(LPCWSTR pszFileName, LPCWSTR pszTagName);
	TAG_OFFICE2K7_API int DeleteTags(LPCWSTR pszFileName, std::vector<TAGPAIR>& listTags);
	TAG_OFFICE2K7_API int DeleteAllTags(LPCWSTR pszFileName);
	// true is ,false no
	TAG_OFFICE2K7_API bool IsOffice2k7File(LPCWSTR pszFileName);

	TAG_OFFICE2K7_API int GetSummaryTags(LPCWSTR pszFilename, OUT std::vector<TAGPAIR>& listTags);
};