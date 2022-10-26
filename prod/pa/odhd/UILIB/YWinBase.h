

/************************************************************************/

/* HDR UI                                                               */

/*     -- Gavin Ye                                                      */

/*        02/25/2008                                                    */

/************************************************************************/

#pragma once

#ifndef _Y_BASE_WND_H_

#define _Y_BASE_WND_H_

#include <assert.h>



#pragma warning(disable : 4311 4312)





#define RECT_X(x)		x.left

#define RECT_Y(x)		x.top

#define RECT_WIDTH(x)	((x.right-x.left)>0?(x.right-x.left):0)

#define RECT_HEIGHT(x)	((x.bottom-x.top)>0?(x.bottom-x.top):0)



#pragma pack(push,1)        // align with BYTE

struct _YWndProcThunk

{

    DWORD   m_mov;          // mov dword ptr [esp+0x4], pThis (esp+0x4 is hWnd)

    DWORD   m_this;         //

    BYTE    m_jmp;          // jmp WndProc

    DWORD   m_relproc;      // relative jmp

};

#pragma pack(pop)



class YWndProcThunk

{

public:

    _YWndProcThunk thunk;



    void Init(WNDPROC proc, void* pThis)

    {

        thunk.m_mov = 0x042444C7;  //C7 44 24 0C

        thunk.m_this = (DWORD)pThis;

        thunk.m_jmp = 0xe9;

        thunk.m_relproc = (int)proc - ((int)this+sizeof(_YWndProcThunk));



        // write block from data cache and

        // flush from instruction cache

        FlushInstructionCache(GetCurrentProcess(), &thunk, sizeof(thunk));

    }

};



class YBaseWnd

{

public:

    YBaseWnd():m_hWnd(NULL),m_hInst(NULL)

    {}

    virtual ~YBaseWnd(){}



    virtual BOOL ProcessWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult)

    {

        BOOL   bCallDefaultHandle = TRUE;

        return bCallDefaultHandle;

    }



    static void put_Instance(HINSTANCE hInst){g_hInst=hInst;}

    static HINSTANCE get_Instance(){return g_hInst;}



public:

    inline HWND      get_SafeHwnd(){return m_hWnd;}

    inline HINSTANCE get_SafeInstance(){return m_hInst;};

    inline HWND      get_ParentHwnd(){return m_hParent;}
	inline void      put_ParentHwnd(HWND hWnd){m_hParent = hWnd;}



protected:

    virtual BOOL Create(LPCWSTR   wzClassName,

                        LPCWSTR   wzWndName,

                        RECT&     rect,

                        HMENU     hMenu,

                        DWORD     dwExStyle,

                        DWORD     dwStyle,

                        HWND      hParentWnd,

                        UINT      uIcon=0)

    {

        if(NULL==YBaseWnd::g_hInst) YBaseWnd::g_hInst = GetModuleHandle(NULL);

        m_hInst = YBaseWnd::g_hInst;

        if(!RegisterWnd(m_hInst, wzClassName, uIcon)) return FALSE;

        m_hWnd = ::CreateWindowEx(dwExStyle,

            wzClassName,

            wzWndName,

            dwStyle,

            RECT_X(rect),

            RECT_Y(rect),

            RECT_WIDTH(rect),

            RECT_HEIGHT(rect),

            hParentWnd,

            hMenu,

            m_hInst,

            (LPVOID)this);

        if(NULL == m_hWnd) return FALSE;

        m_hParent = hParentWnd;

        return TRUE;

    }



protected:

    /*Following functions implement a full windows circle*/

    static LRESULT WINAPI InitWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)

    {  

        if(WM_CREATE == Msg)

        {

            YBaseWnd* pWnd = (YBaseWnd*)((LPCREATESTRUCT)lParam)->lpCreateParams;



            if(pWnd)

            {

                pWnd->m_hWnd = hWnd;

                pWnd->m_thunk.Init(pWnd->StdWndProc, pWnd);



                WNDPROC pProc = (WNDPROC)&(pWnd->m_thunk.thunk);

                WNDPROC pOldProc = (WNDPROC)::SetWindowLongPtrW(hWnd, GWLP_WNDPROC, (LONG)pProc);


                pOldProc;  // avoid unused warning


                return pProc(hWnd, Msg, wParam, lParam);

            }

        }



        return ::DefWindowProc(hWnd, Msg, wParam, lParam);

    }

    static LRESULT WINAPI StdWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)

    {

        YBaseWnd* pWnd = (YBaseWnd*)hWnd;



        if(pWnd)

            return pWnd->WndProc(Msg, wParam, lParam);

        else

            return ::DefWindowProc(hWnd, Msg, wParam, lParam);

    }

    BOOL    RegisterWnd(HINSTANCE hInstance, LPCWSTR wzClassName, UINT  uIcon=0)

    {

        HINSTANCE hInst = hInstance;

        if(NULL == hInst)

            hInst = GetModuleHandle(NULL);



        WNDCLASS wndcls;

        ZeroMemory(&wndcls,sizeof(WNDCLASS));

        if (!(::GetClassInfo(hInst, wzClassName, &wndcls)))

        {

            wndcls.style            = CS_DBLCLKS;

            wndcls.lpfnWndProc      = YBaseWnd::InitWndProc;

            wndcls.cbClsExtra       = 0;

            wndcls.cbWndExtra       = 0;

            wndcls.hInstance        = hInst;

            wndcls.hIcon            = ::LoadIconW(hInst, MAKEINTRESOURCE(uIcon));

            wndcls.hCursor          = ::LoadCursorW(NULL, IDC_ARROW);

            wndcls.hbrBackground    = (HBRUSH) (COLOR_3DFACE + 1);

            wndcls.lpszMenuName     = NULL;

            wndcls.lpszClassName    = wzClassName;

            return RegisterClass(&wndcls);

        }



        return TRUE;

    }

    LRESULT WndProc(UINT Msg, WPARAM wParam, LPARAM lParam)

    {

        LRESULT lResult = 0;

        BOOL bRet = ProcessWindowMessage(m_hWnd, Msg, wParam, lParam, lResult);

        if(WM_DESTROY == Msg)

        {

            PostQuitMessage(0);

            return 0;

        }

        if(bRet)

            return ::DefWindowProc(m_hWnd, Msg, wParam, lParam);

        return lResult;

    }



protected:

    HWND            m_hParent;

    HWND	        m_hWnd;

    HINSTANCE       m_hInst;



private:

    YWndProcThunk   m_thunk;



private:

    COLORREF        m_clrBkGnd;

    static HINSTANCE g_hInst;

};





#define MESSAGE_PROCESS_START()\
    virtual BOOL ProcessWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult)\
    {

#define HANDLE_MESSAGE(msg, func)\
        if(msg == uMsg)\
        {\
            BOOL   bCallDefaultHandler = TRUE;\
            lResult = func(uMsg, wParam, lParam, bCallDefaultHandler);\
            return bCallDefaultHandler;\
        }

#define HANDLE_COMMAND_MESSAGE(id, code, func)\
        if(WM_COMMAND==uMsg && id==LOWORD(wParam) && code == HIWORD(wParam))\
        {\
            BOOL   bCallDefaultHandler = TRUE;\
            lResult = func(HIWORD(wParam), LOWORD(wParam), (HWND)lParam, bCallDefaultHandler);\
            return bCallDefaultHandler;\
        }

#define HANDLE_CTRL_CLICK_MESSAGE(hwnd, func)\
        if(WM_COMMAND==uMsg && BN_CLICKED==HIWORD(wParam) && hwnd==(HWND)lParam)\
        {\
            BOOL   bCallDefaultHandler = TRUE;\
            lResult = func(bCallDefaultHandler);\
            return bCallDefaultHandler;\
        }

#define HANDLE_CLICK_MESSAGE(func)\
        if(WM_COMMAND==uMsg && BN_CLICKED==HIWORD(wParam))\
        {\
            BOOL   bCallDefaultHandler = TRUE;\
            lResult = func((HWND)lParam, bCallDefaultHandler);\
            return bCallDefaultHandler;\
        }

#define HANDLE_CLICK_HWND_MESSAGE(func)\
        if(WM_COMMAND==uMsg && BN_CLICKED==HIWORD(wParam))\
        {\
            BOOL   bCallDefaultHandler = TRUE;\
            lResult = func((HWND)lParam, bCallDefaultHandler);\
            return bCallDefaultHandler;\
        }

#define HANDLE_SYSCOMMAND_MESSAGE(id, func)\
    if(WM_SYSCOMMAND==uMsg && id==LOWORD(wParam))\
    {\
        BOOL   bCallDefaultHandler = TRUE;\
        lResult = func(bCallDefaultHandler);\
        return bCallDefaultHandler;\
    }

#define HANDLE_RANGE_COMMAND_MESSAGE(idFirst, idLast, code, func)\
        if(WM_COMMAND==uMsg && idFirst<=LOWORD(wParam) && idLast>=LOWORD(wParam) && code == HIWORD(wParam))\
        {\
            BOOL   bCallDefaultHandler = TRUE;\
            lResult = func(LOWORD(wParam), HIWORD(wParam), (HWND)lParam, bCallDefaultHandler);\
            return bCallDefaultHandler;\
        }

#define HANDLE_RANGE_CLICK_MESSAGE(idFirst, idLast, func)\
        if(WM_COMMAND==uMsg && idFirst<=LOWORD(wParam) && idLast>=LOWORD(wParam) && BN_CLICKED == HIWORD(wParam))\
        {\
            BOOL   bCallDefaultHandler = TRUE;\
            lResult = func(LOWORD(wParam), bCallDefaultHandler);\
            return bCallDefaultHandler;\
        }

#define MESSAGE_PROCESS_END()\
        return TRUE;\
    }

#endif