// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the PA_ENCRYPTION_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// PA_ENCRYPTION_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef PA_ENCRYPTION_EXPORTS
#define PA_ENCRYPTION_API __declspec(dllexport)
#else
#define PA_ENCRYPTION_API __declspec(dllimport)
#endif

#include <string>
#include <vector>
#include <list>
#include <wchar.h>

#include "PasswordDlg.h"
#include "AutoEncryptDlg.h"
#include "EncryptProgressDlg.h"
#include "EncryptProgressDlg2.h"

typedef enum
{
	EA_OperationType_Unknown = 0,
	EA_OperationType_Copy,
	EA_OperationType_Move,
	EA_OperationType_Email,
	EA_OperationType_Open,
	EA_OperationType_SaveAs,
	EA_OperationType_Close,
	EA_OperationType_Download,
	EA_OperationType_Upload
} EA_OperationType;

typedef enum
{
	EA_ObligationType_None = 0,
	EA_ObligationType_SymmetricEncryptionAssistant,
	EA_ObligationType_CertificateEncryptionAssistant,
} EA_ObligationType;

typedef std::vector<std::wstring> StringVector;

typedef struct _EA_Obligation
{
	EA_ObligationType		obType;
	BOOL				bOptional;
	std::wstring		wstrDescription;
	std::wstring		wstrEncryptAdapterName;
	std::wstring		wstrLogId;
	_EA_Obligation()
	{
		obType = EA_ObligationType_None;
		bOptional = FALSE;
		wstrDescription = L"";
		wstrEncryptAdapterName = L"";
		wstrLogId = L"";
	}
} EA_Obligation, *LPEA_Obligation;

typedef struct _EA_FileData
{
	//EA_Obligation		obligation;
	std::wstring		wstrFileDisplayName;
	std::wstring		wstrSrcFile;
	std::wstring		wstrTmpDstFolder;
	std::wstring		wstrBaseFileName;

	BOOL				bResult;
	BOOL				bFileNameChanged;
	std::wstring		wstrActualDstFile;

	std::wstring		wstrLogId;

	POBJECTINFO			pOriginalObjectInfo;
	_EA_FileData()
	{
		wstrSrcFile = L"";
		wstrTmpDstFolder = L"";
		bResult = FALSE;
		bFileNameChanged = FALSE;
		wstrActualDstFile = L"";
		wstrLogId = L"";
		pOriginalObjectInfo = NULL;
	}
} EA_FileData, *LPEA_FileData;

typedef struct _EA_EncryptAssistantData
{
	std::list<EA_FileData>		listFiles;
	DWORD						dwItemIndex;

	std::wstring				wstrSenderEmail;
	StringVector				vecRecipients;
	std::wstring				wstrPassword;

	EA_Obligation				obligation;

	BOOL						bSkiped;

	_EA_EncryptAssistantData()
	{
		dwItemIndex = 0;
		wstrSenderEmail = L"";
		wstrPassword = L"";

		bSkiped = FALSE;
	}
} EA_EncryptAssistantData, *LPEA_EncryptAssistantData;

typedef struct _EA_AssistantData
{
	EA_EncryptAssistantData		symmAssistantData;
	EA_EncryptAssistantData		certAssistantData;
	std::list<EA_FileData>		listNot2DoFiles;

	HWND						hPEPParantWnd;
	HWND						hParantWnd;
	HWND						hCurrentItemWnd;
	EA_ObligationType			currObType;
	BOOL						bCanceled;

	CPasswordDlg				*pSymmDlg;
	CAutoEncryptDlg				*pCertDlg;
	CEncryptProgressDlg2		*pProgressDlg;

	DWORD						dwNextPanelItem;
	DWORD						dwTotalPanelItems;

	BOOL						bLastPA;
	std::wstring				wstrLastButtonName;

 	BOOL						bMaintainFileNameAfterEncrypted;
	std::wstring				wstrErrorMessage;	// For return error message

	LogFunc						fLog;				// The callback of log to sever.
	PVOID						lpLogCtx ;			// The pep manages this pointer's sturcture.
	_EA_AssistantData()
	{
		hParantWnd = NULL;
		hCurrentItemWnd = NULL;
		currObType = EA_ObligationType_None;
		bCanceled = FALSE;

		pSymmDlg = NULL;
		pCertDlg = NULL;
		pProgressDlg = NULL;

		dwNextPanelItem = 0;
		dwTotalPanelItems = 0;

		bLastPA = FALSE;
		wstrLastButtonName = L"";
		bMaintainFileNameAfterEncrypted = FALSE;

		wstrErrorMessage = L"";

		fLog = NULL;
		lpLogCtx = NULL;
	}
} EA_AssistantData, *LPEA_AssistantData;

PA_STATUS WINAPI InitPAProps ( PA_PARAM &param,
							  EA_AssistantData &assistantData );

PA_STATUS WINAPI Begin_ShowPA ( EA_AssistantData &assistantData );

PA_STATUS WINAPI NextPAItem ( EA_AssistantData &assistantData, BOOL bOKClicked );

PA_STATUS WINAPI ReleasPA ( EA_AssistantData &assistantData );