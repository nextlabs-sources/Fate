/*
 * DestinyNotifyRequestHandler.h 
 * Author: Fuad Rashid
 * All sources, binaries and HTML pages (C) copyright 2004 by Blue Jungle Inc., 
 * Redwood City CA, Ownership remains with Blue Jungle Inc, 
 * All rights reserved worldwide. 
 */

#ifndef _DESTINYNOTIFYREQUESTHANDLER_H_
#define _DESTINYNOTIFYREQUESTHANDLER_H_

#include <string>
#include "nltypes.h"
#include <map>

using std::wstring;

class IIPCRequestHandler;

class DestinyNotifyRequestHandler :
    public IIPCRequestHandler
{
public:
    DestinyNotifyRequestHandler(HWND hMainWnd, NotificationVector* pArray, NOTIFYICONDATA* pNID, HMODULE hResourceModule);
    ~DestinyNotifyRequestHandler(void);

    virtual bool Invoke(IPCREQUEST& request, IPCREQUEST* pResponse);

    /**
    * @return name of request handler to use for generating 
    * unique names for OS objects
    */
    virtual TCHAR* GetName ();

    /**
    * Set whether or not to display notifications.
    * 
    */
    void SetDisplayActivity (BOOL bDisplayActivity) {m_bDisplayActivity = bDisplayActivity; }

    /**
    * Set whether or not to display notifications.
    * 
    */
    void SetDisplayAllowActivity (BOOL bDisplayAllowActivity) {m_bDisplayAllowActivity = bDisplayAllowActivity; }

    /* Determine if any UI should be shown. */
    bool IsDisplayUIEnabled(void)
    {
      return m_bDisplayUI;
    }

    /* Set UI display state - true will prevent any UI from being displayed. */
    void SetDisplayUI( bool enable )
    {
      m_bDisplayUI = enable;
    }

private:
    BOOL GetUserSID (TCHAR **pszUserName);

private:
    HWND m_hWnd;
    NotificationVector* m_pNotificationArray;
    NOTIFYICONDATA* m_pNID;
    HMODULE m_hResourceModule;
    std::map<nlstring, nlstring> m_actionTitleMap;
    tstring m_singleMessageSuffix;
    tstring m_multipleDenyMsg;
    BOOL m_bDisplayActivity;
    BOOL m_bDisplayAllowActivity;
    bool m_bDisplayUI;
    TCHAR m_szName [256];
};
#endif
