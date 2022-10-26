


#ifndef _UI_GLOBAL_H_
#define _UI_GLOBAL_H_

#include <string>
#include <vector>
typedef std::vector<std::wstring>   STRINGVECTOR;
typedef std::vector<std::pair<std::wstring, std::wstring>>  STRINGPAIRVECTOR;


#define WINCX		    465
#define WINCY		    400
#define MYMARGIN	    10
#define BUTTONCX	    75
#define BUTTONCY	    20
#define BIGICON		    32
#define SMALLICON	    16
#define LINESPACE	    2
#define DBLINESPACE	    4
#define CHECKBOXCX      10
#define HIDESHOWBTNCX   45
#define HIDESHOWBTNCY   17
#define CMBOHEIGHT      22
#define CMBOWIDTH       130
#define FONTCY          12
#define WIDTH(x)	(x.right - x.left)
#define HEIGHT(x)	(x.bottom - x.top)

#define DM_DLG_INFO     L"Verify the following external recipients. Select the checkbox for each recipient confirmed."
#define DM_DLG_INFO_CRL     L"Following recipient(s) do not have license to receive one or more attachments. Remove these recipients and try again."
#define TAG_DLG_INFO    L"Sending documents to external recipients requires associating a client to the documents. Select a client from the list or Not Client related."
#define INTER_DLG_INFO  L"The following attachments are for INTERNAL USE ONLY. Click on OK to return to the email."
#define TAG_NOT_CLIENT_RELATED      L"Not Client Related"


#define WM_ITEMREMOVE       (WM_USER+100)
#define WM_ITEMCHECK        (WM_USER+101)
#define WM_ITEMCBNSELECT    (WM_USER+102)
#define WM_ITEMHIDESHOW     (WM_USER+103)


extern HINSTANCE   g_hInstance;
extern HFONT       g_fntNormal;
extern HFONT       g_fntBold;
extern HFONT       g_fntSmallBold;


#endif