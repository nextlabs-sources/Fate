#ifndef __ADOBEXI_H__
#define __ADOBEAI_H__

// Acrobat Headers.
#ifndef MAC_PLATFORM
#include "PIHeaders.h"
#endif

#pragma warning( push )
#pragma warning( disable : 4512 )

#include <windows.h>
#include <map>
#include <string>
using namespace std;
#include "policy.h"
#include <boost/algorithm/string.hpp>


#define  ADOBEXI_SEND_RIGHT		1
#define  ADOBEXI_CONVERT_RIGHT  2
#define  ADOBEXI_COPY_RIGHT     4

namespace AdobeXI
{

	typedef  void (* _AVMenuAddMenuItem)(AVMenu menu, AVMenuItem menuItem, AVMenuIndex menuItemIndex);
	
	typedef void (* _SendMail)(DWORD dwP1,DWORD dwP2);

	typedef struct
	{
		DWORD dwRight;
		PDDoc  pPDFDoc;
	}FILESTOREINFOR;
	

	class SWRLock
	{
	public:
		SWRLock(void);
		~SWRLock();
		void Lock(bool write);
		void Unlock(bool write);	
	private:
		SRWLOCK lock_;
	};

	class AutoSWRLocker
	{
	public:
		AutoSWRLocker(SWRLock& lock, bool write = true);		
		~AutoSWRLocker();
	private:
		SWRLock& lock_;
		bool write_;
	};

	
	class CAdobeXITool
	{
	public:
		/*
		File have right to do action on line, like "copy" "send" "convert"
		parameter action name:
		return true allow, false deny
		*/
		static bool DoActionOnLine(_In_ const string& strAction,_In_ CENoiseLevel_t EmNosie = CE_NOISE_LEVEL_MIN,_In_ const string& strFilePath = "",_In_ PDDoc pdDoc = NULL);
		
		/*
		The Call back of avmenuaddmenuitem
		*/

		static void myAVMenuAddMenuItem(AVMenu menu, AVMenuItem menuItem, AVMenuIndex menuItemIndex);
		
		/*
		clear globe parameter of file right
		*/
		static void DelAllFileRightOnline();

		/*
		Test File have right.
		parameter 
		wstrFilePath: file path
		wstrAction: File right eg "copy" "convert" "send"
		bAllow: true allow false deny.
		return true success it can find in cache, false: fail 
		*/

		static bool IsAllowActionOnline(_In_ const string& wstrFilePath, _In_ string& strAction, _Out_ bool& bAllow);
		
		/*
		Get corresponding value
		parameter 
		wstrAction: File right eg "copy" "convert" "send"
		bAllow: true allow false deny.
		return the value of right 
		*/

		static DWORD GetRightValueOnline(_In_ string& wstrAction,_In_ bool bAllow);

		/*
		Set the right value to globe
		parameter 
		wstrFilePath: file path
		dwSend: send value
		dwConvert: convert value
		*/

		static void SetFileOnLineRightValue(_In_ const string& wstrFilePath, _In_ DWORD dwSend, _In_ DWORD dwConvert,_In_ PDDoc pDdoc);

		/*
		Query policy to get file right
		parameter 
		wstrFilePath: file path
		*/
		static void QueryFileOnlineRight(_In_ const string& strFilePath, _In_ PDDoc pdDoc = NULL);
		
		/*
		The Call back of sendmail
		*/
		static void mySendMail(DWORD dwP1, DWORD dwP2);

		/*
		Get send mail function address
		*/
		static char* CheckEmailFuncAddress();

		/*
		check cache exist deny right path.
		strAction: File right eg "convert" "send"
		bAllow: true allow false deny.
		*/
		static void IsExistOneDenyActionPath(_In_ string& strAction,_Out_ bool& bAllow,_Out_ string& strFilePath,_Out_ PDDoc &pDoc);

		/*
		 *	check the iprotectview if more than 0
		 */
		static bool IsiProtectedViewValueGreaterThanZero(AVTVersionNumPart major, bool bAcrobat = false);
       	/*
		 *	set current save as path
		 */
        static void SetCurrentSaveAsPath(_In_ const string& strFilePath);
       	/*
		 *	get current save as path
		 */
        static string GetCurrentSaveAsPath();
        /*
		 *	verify save as dialog is grey or not
		 */
        static bool IsGreyOnlineWndOnSaveAsDlg();

		static PDDoc GetCurrentFilePDDoc(_In_ const string& strFilePath);

	public:
		static _AVMenuAddMenuItem next_AVMenuAddMenuItem;
		static _SendMail next_SendMail;

	private:
		/*
		Get right value from globe
		parameter 
		wstrFilePath: file path
		dwRight: right value
		*/
		static void SetFileRightOnlineToCache(_In_ const string& wstrFilePath,_In_ DWORD dwRight,_In_ PDDoc pDoc = NULL);

		/*
		set right value to globe
		parameter 
		wstrFilePath: file path
		dwRight: right value
		*/

		static bool GetFileRightOnlineFromCache(_In_ const string& wstrFilePath,_Out_ DWORD& dwRight);

		/*
		the call back of SaveToAcrobatEnabled, it control menu show or grey.
		*/

		static ACCB1 ASBool ACCB2 MenuSaveToAcrobatEnabled(void  *data);
		
		/*
		 *	Get current user SID
		 */
		static void GetCurrentUserID(char **UserSID);
		
	private:
		static map<string,FILESTOREINFOR> m_mapFileRight;
        static string m_CurrentSaveAsPath;
	};
}

#endif