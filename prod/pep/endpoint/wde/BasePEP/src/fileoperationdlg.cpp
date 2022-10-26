#include "fileoperationdlg.h"

extern HMODULE g_hModule;

namespace nextlabs
{
    // fileOperation dialog
    LRESULT CFileOperationDlg::OnInitDialog ( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
    {

        HICON hIcon = (HICON)LoadImageW(g_hModule, MAKEINTRESOURCE(IDI_LOGOICON), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);

        if(hIcon)
        {

            SetIcon(hIcon, FALSE);
        }

        SendDlgItemMessage ( IDC_FILEOPERATION_PROGRESS, PBM_SETRANGE, 0 , (LPARAM) ( MAKELPARAM ( 0, 100 ) ) );

        return 0;
    }

    LRESULT CFileOperationDlg::OnClose ( UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/ )
    {
        bCancel = TRUE;
        return 0;   
    } 


    LRESULT CFileOperationDlg::OnMyClose ( UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/ )
    {
        DestroyWindow ();
        PostThreadMessageW ( dwCurrentThreadId, WM_MYEXITMESSAGE, 0, 0 ); 

        return 0;   
    } 

    LRESULT CFileOperationDlg::OnMySetProgressPos( UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/ )
    {
        int Pos = (int)lParam;
        SendDlgItemMessage ( IDC_FILEOPERATION_PROGRESS, PBM_SETPOS, (UINT) ( Pos * 100 / TotalFileCount ), 0 );

        if ( Pos == static_cast<int>(TotalFileCount) )
        {
            SendMessage ( WM_MYCLOSE );
        }

        return 0;
    }

    LRESULT CFileOperationDlg::OnClickedCancel (WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        bCancel = TRUE;

        return 0;
    }

    DWORD WINAPI FileOperationThread(LPVOID lpParameter)
    {

        FILEOPERATION* pFileOperation = (FILEOPERATION*)lpParameter;  
        CFileOperationDlg dlg;
        dlg.TotalFileCount = pFileOperation->fileCOUNT;
        pFileOperation->pFileOperationDlg = &dlg;

        dlg.dwCurrentThreadId = GetCurrentThreadId();

        dlg.Create(NULL);  
        dlg.ShowWindow(SW_SHOW); 

        SetEvent(pFileOperation->hEvent);
        MSG msg = {0};
        while (GetMessage(&msg, NULL, 0, 0))
        {
            if (msg.message == WM_MYEXITMESSAGE)
            {
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        return 0;
    }
}

