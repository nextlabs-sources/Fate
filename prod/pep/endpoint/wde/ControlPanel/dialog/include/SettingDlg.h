#pragma once

#include "resource.h"       // main symbols
#include "EDPMgrUtilities.h"
#include "ProgressDlg.h"

typedef struct struSettingData
{
	CEDPMUtilities::NOTIFY_DIS_DURATION m_duration;
	CEDPMUtilities::NOTIFY_DIS_LEVEL m_level;
	wchar_t m_szPCPwd[256];
	BOOL m_bPassword;
	BOOL m_bEnableLogging;
}SETTINGDATA;

typedef enum ERROR_SETTING{SETTING_SUCCESS = 0, SETTING_STARTPC_FAILED, SETTING_VERBOSELOG_FAILED, SETTING_DISPLAYLEVEL_FAILED, SETTING_DURATION_FAILED, 
							SETTING_STOPPC_NOPERMISSION, SETTING_STOPPC_WRONGPWD, SETTING_STOPPC_UNKNOWNERROR};

class CSettingDlg: public CAxDialogImpl<CSettingDlg>
{
public:
	CSettingDlg(void);
	~CSettingDlg(void);

	enum { IDD = IDD_SETTING };



	BEGIN_MSG_MAP(CSettingDlg)
		COMMAND_HANDLER(IDC_CHECK_ENABLE_LOG, BN_CLICKED, OnBnClickedCheckEnableLog)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
		MESSAGE_HANDLER(WM_PAINT, OnPaint) 
		MESSAGE_HANDLER(WM_NXTLBS_MYSUBMIT, OnSubmit)
		MESSAGE_HANDLER(WM_CTLCOLORDLG, OnCtlColorDlg)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnBnClickedOk)
		COMMAND_HANDLER(IDC_RADIO_ALL, BN_CLICKED, OnBnClickedRadioAll)
		COMMAND_HANDLER(IDC_RADIO_ONLY_BLOCK, BN_CLICKED, OnBnClickedRadioOnlyBlock)
		COMMAND_HANDLER(IDC_RADIO_SUPRESS, BN_CLICKED, OnBnClickedRadioSupress)
		COMMAND_HANDLER(IDC_RADIO_REQUIRE_USER_CLOSE, BN_CLICKED, OnBnClickedRadioRequireUserClose)
		COMMAND_HANDLER(IDC_RADIO_DISPLAY_DURATION, BN_CLICKED, OnBnClickedRadioDisplayDuration)
		COMMAND_HANDLER(IDC_RADIO_DEFAULT_DURATION, BN_CLICKED, OnBnClickedRadioDefaultDuration)
	END_MSG_MAP()
	LRESULT OnBnClickedCheckEnableLog(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnBnClickedOk(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCtlColorStatic(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSubmit(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnCtlColorDlg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
public:
	/*
	
	submit verbose logging setting


	return value:

	true --			means submit succeed
	false	--	means unexpected error happened
	
	*/
	BOOL ProcessVerboseLogSetting();

	/*

	submit display level setting and display duration setting.


	return value:

	true --			submit succeed
	false	--	means unexpected error happened

	*/
	BOOL ProcessDisplayLevelSetting();
	BOOL ProcessDisplayDurationSetting();



	/*
	
	enable or disable controls in display duration group
	
	*/
	void EnableDisplayDuration(BOOL bEnable);

	void GetCurrentSetting();

	void UpdateProgressDlgStatus(DWORD dwStringID);
	void CloseProgressDlg();

	BOOL IsProgressDlgRunning();
	void HandleError();
public:
	LRESULT OnBnClickedRadioAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedRadioOnlyBlock(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedRadioSupress(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedRadioRequireUserClose(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedRadioDisplayDuration(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedRadioDefaultDuration(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
protected:
	HFONT m_hFont;
	SETTINGDATA m_newSettings;
	SETTINGDATA m_curSettings;
	
public:
	CProgressDlg* m_pProgressDlg;
	ERROR_SETTING m_SettingError;
	BOOL m_bProgressDlg;
};
