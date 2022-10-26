#pragma once
#include <map>
#include <vector>
#include <list>

typedef LRESULT (CALLBACK* FuncWindowProc)( 
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    );
typedef struct tagThreadHookLink
{
	DWORD ThreadID;
	HHOOK HookMSG;
	HHOOK HookhdCallBack;
	HHOOK HookhdMouse;

}THREADHOOKLINK, *PTHREADHOOKLINK;
typedef struct tag_SubMsgHandler
{
    std::vector<UINT32>                                         m_vctControlId;
    std::list< std::wstring >                                   m_lstControlClassName;
    FuncWindowProc                                              m_pMsgHandler;

    tag_SubMsgHandler():m_pMsgHandler(0){}

}SubMsgHandler;

typedef struct tag_MsgHandler
{
    UINT32                                                      m_uMsgId;
    //std::map< UINT32, std::pair< UINT32, FuncWindowProc > >     m_mpSubMsgHandler;

    UINT32                                                      m_uSubMsgId;
    SubMsgHandler                                               m_SubMsgHandler;

    tag_MsgHandler():m_uMsgId(0), m_uSubMsgId(0){}

}MsgHandler;

class CMsgHook
{
private:
    CMsgHook(): m_hHook(0), m_dwThreadId(0), m_hInstDll(0), m_pOrgProc(0),m_hWndAsk(0),m_bShare(true),m_bChat(true)
    {
        ::InitializeCriticalSection(&s_csMsgHook);
    }
	~CMsgHook(){	::DeleteCriticalSection(&s_csMsgHook);};
    static CMsgHook* m_pInstance;

public:

    static CMsgHook* GetInstance()
    {
        if( !m_pInstance )
        {
            m_pInstance = new CMsgHook();
        }
        return m_pInstance;
    }

public:

    void SetDllInst( HINSTANCE hDllInst ){ m_hInstDll = hDllInst; }

    DWORD GetThreadId(){ return m_dwThreadId; }
    HHOOK GetHookHandle(){ return m_hHook; }
    WNDPROC GetOrgProcFunc(){ return m_pOrgProc; }
    void SetOrgProcFunc( WNDPROC pProc ){ m_pOrgProc = pProc; }

    BOOL SetMsgHook(DWORD dwThreadId);

    HWND GetWndAsk()
    {
        if( !m_hWndAsk )
        {
            m_hWndAsk = FindWindow( TEXT("WTL_CrossFadeButton"), TEXT("Ask") );
        }
        return m_hWndAsk;
    }

    static LRESULT WINAPI CallWndProc(int nCode, WPARAM wParam, LPARAM lParam);
    static LRESULT WINAPI GetMsgProc1(int nCode, WPARAM wParam, LPARAM lParam);
    static LRESULT WINAPI GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam);
    
    
    static INT_PTR WINAPI Dlg_Proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static bool HandleMsg( HWND hwnd, WPARAM wParam, UINT nMsg );

    static LRESULT CALLBACK WindowProc(          HWND hwnd,
        UINT uMsg,
        WPARAM wParam,
        LPARAM lParam
        );

    static LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam);


    WNDPROC GetOrgProcFunc( HWND hWnd )
    { 
        std::map< HWND, WNDPROC >::iterator it = m_mpProcSub.find( hWnd );
        if( it != m_mpProcSub.end() )
        {
            return (*it).second;
        }
        return 0;
    }

    void SubWndPorc( HWND hWnd );
    void SubWndPorc( HWND hWnd, WNDPROC pWndProc );

    void SetLastMsg( UINT nMsgId, DWORD dwTime )
    {
        m_nLastMsgId = nMsgId;
        m_nLastMsgTime = dwTime;
    }

    UINT GetLastMsgId(){ return m_nLastMsgId; }
    DWORD GetLastMsgTime(){ return m_nLastMsgTime; }

    void SetAllowShare( bool bShare ){ m_bShare = bShare; }
    void SetAllowChat( bool bChat ){ m_bChat = bChat; }

    bool GetAllowShare() const { return m_bShare; }
    bool GetAllowChat() const{ return m_bChat; }


    void AddMsgHandler( const MsgHandler& aMsgHandler );
    void CheckHandler(  );
public:
	static std::list<THREADHOOKLINK> s_pThreadHookLink;
	BOOL DoUnHookMSG(DWORD dThreadID);
	BOOL HasHooked(DWORD dThreadID);
	HHOOK static GetHookedCallBackHandle(DWORD dThreadID);
	HHOOK static GetHookedMsgHandle(DWORD dThreadID);
	HHOOK static GetHookedMouseHandle(DWORD dThreadID);
	static BOOL		m_IsQAChat ;
	static CRITICAL_SECTION s_csMsgHook;
private:

    HINSTANCE   m_hInstDll;
    DWORD       m_dwThreadId;
    HHOOK       m_hHook;

    WNDPROC     m_pOrgProc;

    std::map< HWND, WNDPROC > m_mpProcSub;

    UINT        m_nLastMsgId;
    DWORD       m_nLastMsgTime;

    HWND        m_hWndAsk;

    bool        m_bShare;
    bool        m_bChat;

    
    std::map< UINT32, std::map< UINT32, SubMsgHandler > > m_mpMsgHandler;
};


#include "HookBase.h"

class CHookedCreateWindow:public CHookBase
{
    INSTANCE_DECLARE( CHookedCreateWindow );
public:

    void Hook( void* pSignalingMsg );

    typedef HWND  (WINAPI *Func_CreateWindowExW)(
        _In_ DWORD dwExStyle,
        _In_opt_ LPCWSTR lpClassName,
        _In_opt_ LPCWSTR lpWindowName,
        _In_ DWORD dwStyle,
        _In_ int X,
        _In_ int Y,
        _In_ int nWidth,
        _In_ int nHeight,
        _In_opt_ HWND hWndParent,
        _In_opt_ HMENU hMenu,
        _In_opt_ HINSTANCE hInstance,
        _In_opt_ LPVOID lpParam);

    

    static HWND WINAPI Hooked_CreateWindowExW(
        _In_ DWORD dwExStyle,
        _In_opt_ LPCWSTR lpClassName,
        _In_opt_ LPCWSTR lpWindowName,
        _In_ DWORD dwStyle,
        _In_ int X,
        _In_ int Y,
        _In_ int nWidth,
        _In_ int nHeight,
        _In_opt_ HWND hWndParent,
        _In_opt_ HMENU hMenu,
        _In_opt_ HINSTANCE hInstance,
        _In_opt_ LPVOID lpParam);

    static BOOL WINAPI Hooked_ShowWindow( _In_ HWND hWnd, _In_ int nCmdShow) ;

    static ATOM WINAPI Hooked_RegisterClassW( _In_ CONST WNDCLASSW *lpWndClass);

    static ATOM WINAPI Hooked_RegisterClassExW( _In_ CONST WNDCLASSEXW  *lpWndClass);
};



