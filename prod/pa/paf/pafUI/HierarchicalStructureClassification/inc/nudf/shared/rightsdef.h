

#ifndef __NUDF_SHARE_RIGHTS_DEF_H__
#define __NUDF_SHARE_RIGHTS_DEF_H__

#ifdef __cplusplus
extern "C" {
#endif


//
//  Define Internal Rights
//

// NXRMSERV Rights
#define INTERNAL_RIGHT_SERV_CTL                 0x0000000000000001ULL
#define INTERNAL_RIGHT_SERV_DEBUG               0x0000000000000002ULL
#define INTERNAL_RIGHT_SERV_CONFIG_AUTHN        0x0000000000000004ULL
#define INTERNAL_RIGHT_SERV_CONFIG_AUTHZ        0x0000000000000008ULL
#define INTERNAL_RIGHT_SERV_CONFIG_KEYMGN       0x0000000000000010ULL
#define INTERNAL_RIGHT_SERV_CONFIG_AUDIT        0x0000000000000020ULL
#define INTERNAL_RIGHT_SERV_CONFIG_VHD          0x0000000000000040ULL
#define INTERNAL_RIGHT_SERV_CONFIG_FS           0x0000000000000080ULL

// KEY MANAGEMENT Rights
#define INTERNAL_RIGHT_KEY_GET                  0x0000000000001000ULL
#define INTERNAL_RIGHT_KEY_REVOKE               0x0000000000002000ULL
#define INTERNAL_RIGHT_KEY_DISPATCH             0x0000000000004000ULL

// AUDIT Rights
#define INTERNAL_RIGHT_AUDIT                    0x0000000000010000ULL

// VHD Rights
#define INTERNAL_RIGHT_VHD_QUERY                0x0000000001000000ULL
#define INTERNAL_RIGHT_VHD_DISPLAY              0x0000000002000000ULL


//
//
//
#define RIGHTS_NOT_CACHE                        0x8000000000000000ULL


//
//  Define Built-in Rights
//
#define BUILTIN_RIGHT_VIEW                      0x0000000000000001ULL
#define BUILTIN_RIGHT_EDIT                      0x0000000000000002ULL
#define BUILTIN_RIGHT_PRINT                     0x0000000000000004ULL
#define BUILTIN_RIGHT_CLIPBOARD                 0x0000000000000008ULL
#define BUILTIN_RIGHT_SAVEAS					0x0000000000000010ULL
#define BUILTIN_RIGHT_DECRYPT                   0x0000000000000020ULL
#define BUILTIN_RIGHT_SCREENCAP                 0x0000000000000040ULL
#define BUILTIN_RIGHT_SEND                      0x0000000000000080ULL
#define BUILTIN_RIGHT_CLASSIFY                  0x0000000000000100ULL

#define BUILTIN_RIGHT_ALL                       (BUILTIN_RIGHT_VIEW \
                                                 | BUILTIN_RIGHT_EDIT \
                                                 | BUILTIN_RIGHT_PRINT \
                                                 | BUILTIN_RIGHT_CLIPBOARD \
                                                 | BUILTIN_RIGHT_SAVEAS \
                                                 | BUILTIN_RIGHT_DECRYPT \
                                                 | BUILTIN_RIGHT_SCREENCAP \
                                                 | BUILTIN_RIGHT_SEND \
                                                 | BUILTIN_RIGHT_CLASSIFY)
    
#define RIGHT_ACTION_VIEW                       L"RIGHT_VIEW"
#define RIGHT_ACTION_EDIT                       L"RIGHT_EDIT"
#define RIGHT_ACTION_PRINT                      L"RIGHT_PRINT"
#define RIGHT_ACTION_CLIPBOARD                  L"RIGHT_CLIPBOARD"
#define RIGHT_ACTION_SAVEAS					    L"RIGHT_SAVEAS"
#define RIGHT_ACTION_DECRYPT                    L"RIGHT_DECRYPT"
#define RIGHT_ACTION_SCREENCAP                  L"RIGHT_SCREENCAP"
#define RIGHT_ACTION_SEND                       L"RIGHT_SEND"
#define RIGHT_ACTION_CLASSIFY                   L"RIGHT_CLASSIFY"

#define RIGHT_DISP_VIEW                         L"View"
#define RIGHT_DISP_EDIT                         L"Edit"
#define RIGHT_DISP_PRINT                        L"Print"
#define RIGHT_DISP_CLIPBOARD                    L"Access Clipboard"
#define RIGHT_DISP_SAVEAS					    L"Save As"
#define RIGHT_DISP_DECRYPT                      L"Decrypt"
#define RIGHT_DISP_SCREENCAP                    L"Capture Screen"
#define RIGHT_DISP_SEND                         L"Send"
#define RIGHT_DISP_CLASSIFY                     L"Classify"


    
__forceinline
ULONGLONG ActionToRights(_In_ LPCWSTR action)
{
    if(0 == _wcsicmp(RIGHT_ACTION_VIEW, action)) {
        return BUILTIN_RIGHT_VIEW;
    }
    else if(0 == _wcsicmp(RIGHT_ACTION_EDIT, action)) {
        return BUILTIN_RIGHT_EDIT;
    }
    else if(0 == _wcsicmp(RIGHT_ACTION_PRINT, action)) {
        return BUILTIN_RIGHT_PRINT;
    }
    else if(0 == _wcsicmp(RIGHT_ACTION_CLIPBOARD, action)) {
        return BUILTIN_RIGHT_CLIPBOARD;
    }
    else if(0 == _wcsicmp(RIGHT_ACTION_SAVEAS, action)) {
        return BUILTIN_RIGHT_SAVEAS;
    }
    else if(0 == _wcsicmp(RIGHT_ACTION_DECRYPT, action)) {
        return BUILTIN_RIGHT_DECRYPT;
    }
    else if(0 == _wcsicmp(RIGHT_ACTION_SCREENCAP, action)) {
        return BUILTIN_RIGHT_SCREENCAP;
    }
    else if(0 == _wcsicmp(RIGHT_ACTION_SEND, action)) {
        return BUILTIN_RIGHT_SEND;
    }
    else if(0 == _wcsicmp(RIGHT_ACTION_CLASSIFY, action)) {
        return BUILTIN_RIGHT_CLASSIFY;
    }
    else {
        return 0ULL;
    }
}

__forceinline
LPCWSTR RightToDisplayName(const ULONGLONG right)
{
    if(BUILTIN_RIGHT_VIEW == right) {
        return RIGHT_DISP_VIEW;
    }
    else if(BUILTIN_RIGHT_EDIT == right) {
        return RIGHT_DISP_EDIT;
    }
    else if(BUILTIN_RIGHT_PRINT == right) {
        return RIGHT_DISP_PRINT;
    }
    else if(BUILTIN_RIGHT_CLIPBOARD == right) {
        return RIGHT_DISP_CLIPBOARD;
    }
    else if(BUILTIN_RIGHT_SAVEAS == right) {
        return RIGHT_DISP_SAVEAS;
    }
    else if(BUILTIN_RIGHT_DECRYPT == right) {
        return RIGHT_DISP_DECRYPT;
    }
    else if(BUILTIN_RIGHT_SCREENCAP == right) {
        return RIGHT_DISP_SCREENCAP;
    }
    else if(BUILTIN_RIGHT_SEND == right) {
        return RIGHT_DISP_SEND;
    }
    else if(BUILTIN_RIGHT_CLASSIFY == right) {
        return RIGHT_DISP_CLASSIFY;
    }
    else {
        return L"";
    }
}


#ifdef __cplusplus
}
#endif


#endif  // #ifndef __NUDF_SHARE_RIGHTS_DEF_H__