#include <windows.h>
#include "ipcproxy.h"

// There is no "do heartbeat" event.  However, the "new user has logged on" event
// will automatically heartbeat and, as a bonus, doesn't actually require a valid
// user name/id to work correctly.
const WCHAR *LOGONEVENT = L"handleLogonEvent";
const int IPCPROXY_REQUEST_TIMEOUT = 5000;
const WCHAR *IPC_CM_REQUEST_HANDLER = L"com.bluejungle.destiny.agent.controlmanager.CMRequestHandler";

#if 1
int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow)
#else 
    int main(int argc, char *argv[])
#endif
{
    BOOL bRet = FALSE;
    IPCProxy* pIPCProxy = new IPCProxy();
    if (pIPCProxy != NULL)
    {
        WCHAR* pszSID = NULL;
        if (pIPCProxy->Init(IPC_CM_REQUEST_HANDLER)) {
            IPCREQUEST request, response;
            memset (&request, 0, sizeof (IPCREQUEST));
            request.ulSize = sizeof (IPCREQUEST);

            
            wcsncpy_s (request.methodName, _countof(request.methodName), LOGONEVENT, _TRUNCATE);
            wcsncpy_s (request.params[0], _countof(request.params[0]), L"Nobody\\ForcingHeartbeat", _TRUNCATE);

            memset (&response, 0, sizeof (IPCREQUEST));
            bRet = pIPCProxy->Invoke (&request, &response, IPCPROXY_REQUEST_TIMEOUT);
        }
    }
    
    delete pIPCProxy;
    
    return EXIT_SUCCESS;
}
