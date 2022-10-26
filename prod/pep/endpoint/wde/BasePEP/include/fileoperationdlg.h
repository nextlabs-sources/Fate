#ifndef __WDE_FILEOPERATIONDLG_H__
#define __WDE_FILEOPERATIONDLG_H__

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit

#include "resource.h"

#pragma warning(push)
#pragma warning(disable: 6387 6011) 

#include <atlbase.h>
#include <atlcom.h>
#include <atlctl.h>

#pragma warning(pop)



using namespace ATL;

#define WM_MYEXITMESSAGE 0x4015  
#define WM_MYCLOSE		0x4016
#define WM_MYSETPROGRESSPOS 0x4017

namespace nextlabs
{
    // Class FileOperationDlg -- used for file\folder copy and move      

    class CFileOperationDlg;  
    typedef struct _FILEOPERATION
    {
        UINT fileCOUNT;
        CFileOperationDlg* pFileOperationDlg;
        HANDLE hEvent;
    }FILEOPERATION;

    class CFileOperationDlg: public CDialogImpl<CFileOperationDlg>     
    {
    public:
        CFileOperationDlg():TotalFileCount(0), bCancel(FALSE), dwCurrentThreadId(0)
        {  
        }

        ~CFileOperationDlg()
        {
        }
        enum {IDD = IDD_FileOperationDlg}; 

        BEGIN_MSG_MAP(CFileOperationDlg)
            MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
            MESSAGE_HANDLER(WM_CLOSE, OnClose);
            MESSAGE_HANDLER(WM_MYCLOSE, OnMyClose);
            MESSAGE_HANDLER(WM_MYSETPROGRESSPOS, OnMySetProgressPos);
            COMMAND_HANDLER(IDC_CANCEL, BN_CLICKED, OnClickedCancel)  
        END_MSG_MAP()

        LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        LRESULT OnClose ( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
        LRESULT OnMyClose ( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
        LRESULT OnMySetProgressPos ( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
        LRESULT OnClickedCancel (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled ); 

    public:
        UINT TotalFileCount;
        BOOL bCancel;
        DWORD dwCurrentThreadId;
    };  // FileOperationDlg

    DWORD WINAPI FileOperationThread(LPVOID lpParameter);
} // ns nextlabs

#endif