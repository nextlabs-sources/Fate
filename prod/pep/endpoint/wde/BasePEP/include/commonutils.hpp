#ifndef __WDE_COMMON_UTILS__
#define __WDE_COMMON_UTILS__

#ifdef _MSC_VER
#pragma once
#else
#error "commonutils.hpp only supports windows-compile"
#endif // _MSC_VER

#include "eframework/policy/comm_helper.hpp"
#include "PAMngr.h"
#include <windows.h>
#include <shtypes.h>
#include <stdio.h>
#include <exception>
#include <stdexcept>
#include <map>
#include <vector>
#include <set>
#include <Winternl.h>
#include "madCHook_helper.h"

#pragma warning(push)
#pragma warning(disable: 6387 6011) 
#include <atlbase.h>
#include <atlcom.h>
#include <atlctl.h>
#include <ShObjIdl.h>
#include <Shlwapi.h>
#include <Shellapi.h>
#include <ShlObj.h>
#pragma warning(pop)

#include <boost/algorithm/string.hpp>
#include <Sddl.h>
#include <AccCtrl.h>
#include <Aclapi.h>
#include <TlHelp32.h>

namespace nextlabs

{
	#define WDE_OBLIGATION_ENCRYPT_NAME       L"EFS_ENCRYPTION"        // EFS Encryption
	#define WDE_OBLIGATION_RICH_USER_MESSAGE  L"RICH_USER_MESSAGE"	   // Rich User Message

    typedef enum EventResult{
        kEventDeny,				// most used by events-notifier like OnBefore*, denied to called real api
        kEventAllow,
        kEventReturnDirectly	// return this , it's the derived own responsibility to make target logic-flow integrity
    };

    typedef HRESULT(WINAPI *f_SHCreateShellItemArrayFromDataObject)(IDataObject *pdo,
        REFIID riid,
        void **ppv
        );

	/************************************************************************/
	/* Nextlabs Exception                                                   */
	/************************************************************************/
	class exeception : public std::exception
	{
	public:
		exeception() throw() : std::exception(), 
			file_(NULL), func_(NULL) , line_(-1),lasterr_(0)
		{

		}
		exeception(	const char* file,
			const char* func,
			int			line,
			const char* what,
			int			lasterr): std::exception(what), 
			file_(file),func_(func),line_(line),lasterr_(lasterr)
		{

		}
		virtual ~exeception() throw()
		{

		}

		exeception& operator= (const nextlabs::exeception& e) throw()
		{
			if( (&e) == this )
			{
				return *this;  // avoid self-assign
			}

			((std::exception&)(*this)) = ((const std::exception&)e);
			file_ = e.file();
			func_ = e.func();
			line_ = e.line();
			lasterr_ = e.lasterr();
		}

		inline const char* file() const throw() {return file_;}
		inline const char* func() const throw() {return func_;}
		inline		 int   line() const throw() {return line_;}
		inline	   int  lasterr() const throw() {return lasterr_;}

	protected:
		char const* file_; 
		char const* func_;
		int			line_;
		int			lasterr_;
	};

#define NLEXCEPTION(what)		nextlabs::exeception(__FILE__,__FUNCTION__,__LINE__,what,0);
#define NLWIN32EXCPTION(what)	nextlabs::exeception(__FILE__,__FUNCTION__,__LINE__,what,::GetLastError());


	inline void dumpLastError(DWORD errorCode){
		wchar_t path[MAX_PATH]={0};
		::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS ,
			NULL,
			errorCode,
			MAKELANGID(LANG_NEUTRAL,SUBLANG_NEUTRAL),
			path,
			MAX_PATH,
			NULL);
		printf("Error code:%d\nError message:%ws",errorCode,path);
	}


	namespace utils
	{	
        // Some logs print in DebugView
        static void DebugViewLog(char* fmt, ...)
        {
            char buff[1024] = { 0 };

            va_list args;
            va_start(args, fmt);
            vsnprintf_s(buff, 1023, _TRUNCATE, fmt, args);
            va_end(args);

            OutputDebugStringA(buff);
        }

		//
		// String conversion
		// - in order to minify heap operations, 
		//   if required buf length less than nDefaultBufLen, use stack buffer
		//	 else use heap buffer
		// - most Windows file path's length is less than 128 , so it can cover 95% situations. for the rest , just heap
		//
		template < int nDefaultBufLen = 128 >
		class CStrU2A  // UTF-16  -> ANSI
		{
		public:
			CStrU2A ( const wchar_t* pwsz) : pstr_(NULL)
			{
				try{
					_init(pwsz);
				}catch(...){
					pstr_ = NULL;
					// ignored
				}
			}			
			~CStrU2A()
			{
				if(pstr_ && (pstr_!= buffer_) )
				{					
					delete [] pstr_;
					pstr_=NULL;
				}
#ifdef _DEBUG
				// release res to prevent accidental use
				::ZeroMemory(buffer_,nDefaultBufLen*sizeof(char));
#endif // _DEBUG
				
			}
			inline operator char*() const  {return pstr_;}
			inline operator const char*() const {return pstr_;}
		private:
			void _init(const wchar_t* pwsz)
			{
				pstr_= NULL;
				
				if(NULL == pwsz) throw NLEXCEPTION("pwsz == NULL");
		
				int cchW = static_cast<int>(wcslen(pwsz)) + 1;		
				
				// calculate the required size 
				int nRequriedBytes = ::WideCharToMultiByte(CP_ACP,0,pwsz,cchW,NULL,0,NULL,NULL);
				
				if(nRequriedBytes > nDefaultBufLen)
				{
					// allocate at heap
					pstr_ = new char[nRequriedBytes*sizeof(char)];
					if(0==::WideCharToMultiByte(CP_ACP,0,pwsz,cchW,pstr_,nRequriedBytes,NULL,NULL))
					{
						throw NLWIN32EXCPTION("U2A convert failed");
					}
				}
				else
				{
					// no need to use heap, use stack instead
					pstr_ = buffer_;
					if(0==::WideCharToMultiByte(CP_ACP,0,pwsz,cchW,buffer_,nDefaultBufLen,NULL,NULL))
					{
						throw NLWIN32EXCPTION("U2A convert failed");
					}
				}			

			}
		private:
			char* pstr_;
			char  buffer_[nDefaultBufLen];
		};

typedef nextlabs::utils::CStrU2A<>  U2A;

		template < int nDefaultBufLen = 128 >
		class CStrA2U  // ANSI -> UTF-16
		{
		public:
			CStrA2U( const char* pstr) : pwstr_(NULL)
			{
				try{
					_init(pstr);
				}catch(...){
					pwstr_ = NULL;
					// ignored
				}
			}
			~CStrA2U()
			{
				if(pwstr_ && (pwstr_!= buffer_)){
					delete [] pwstr_;
					pwstr_= NULL;
				}
#ifdef _DEBUG
				// release res to prevent accidental use
				::ZeroMemory(buffer_,nDefaultBufLen*sizeof(wchar_t));
#endif // _DEBUG
			}
			inline operator wchar_t*() const {return pwstr_;}
			inline operator const wchar_t*() const {return pwstr_;}
		private:
			void _init(const char* pstr)
			{
				pwstr_ = NULL;
				if(NULL == pstr) throw NLEXCEPTION("pstr is null");

				int cchA = static_cast<int>(strlen(pstr))+1;
				// calculate the required size 
				int nRequiredSize =::MultiByteToWideChar(CP_ACP,0,pstr,cchA,NULL,0)*sizeof(wchar_t);
				if(nRequiredSize > nDefaultBufLen*sizeof(wchar_t))
				{
					// alloc at heap
					pwstr_ = new wchar_t[nRequiredSize/sizeof(wchar_t)+1];
					if(0 == ::MultiByteToWideChar(CP_ACP,0,pstr,cchA,pwstr_,nRequiredSize/sizeof(wchar_t)+1))
					{
						throw NLWIN32EXCPTION("A2U convert failed");
					}
				}
				else
				{
					// no need to use heap, use stack instead
					pwstr_ = buffer_;
					if(0 == ::MultiByteToWideChar(CP_ACP,0,pstr,cchA,pwstr_,nDefaultBufLen*sizeof(wchar_t)))
					{
						throw NLWIN32EXCPTION("A2U convert failed");
					}
				}

			}
		private:
			wchar_t* pwstr_;
			wchar_t  buffer_[nDefaultBufLen];
		};
typedef nextlabs::utils::CStrA2U<>	A2U;

		inline bool str2wstr(const std::string& str, std::wstring& wstr)
		{
			if(str.empty())
			{
				wstr.clear();
				return true;
			}
			wstr.assign(A2U(str.c_str())); // A2U is vulnerable, leaving scope, become invalid
			return !wstr.empty()?true:false;
		
		}
		inline bool wstr2str(const std::wstring& wstr, std::string& str)
		{
			if(wstr.empty())
			{
				str.clear();
				return true;
			}
			str.assign(U2A(wstr.c_str())); // U2A is vulnerable, leaving scope, become invalid
			
			return !str.empty()? true: false;
		}
		inline std::string from_wstr(const std::wstring& wstr)
		{
			std::string rt;
			return wstr2str(wstr,rt)?rt:(rt.clear(),rt);
		}
		inline std::wstring from_str(const std::string str)
		{
			std::wstring rt;
			return str2wstr(str,rt)?rt:(rt.clear(),rt);
		}

		typedef enum {
			kUri_Unknown =0,
			kUri_URL,
			kUri_UNC,
			kUri_FsPath,
			kUri_Device
		} UriType;

		class CUri
		{
		public:
			CUri() throw() : type_(kUri_Unknown) {}
			CUri(const wchar_t* pw) throw() : uri_(pw? pw:L""),type_(kUri_Unknown){Normalize();}
			virtual ~CUri() throw() {}
			virtual void Normalize();
        private:
            void ParseCmdFilePath();
            std::wstring MyGetLongPath(const std::wstring& path);
		public:
			inline const std::wstring& GetUri() const throw() {return uri_;}
			inline  UriType GetType() const throw() {return type_;}
			inline CUri& operator= (const CUri& rh) throw()
			{
				uri_ = rh.uri_;
				type_ = rh.type_;
				return *this;
			}
			inline BOOL isTypeUnknown() const throw() { return kUri_Unknown == type_;}
			inline BOOL isTypeURL() const throw() {return kUri_URL == type_;}
			inline BOOL isTypeUNC() const throw() {return kUri_UNC == type_;}
			inline BOOL isTypeFsPath() const throw() {return kUri_FsPath == type_;}
			inline BOOL isTypeDevice() const throw() {return kUri_Device == type_;}
		protected:
			std::wstring uri_;
			UriType type_;
		};

        
        std::wstring FormatPath(const std::wstring& path);
        std::wstring GetFilePathFromName(const std::wstring& fullPath);
        BOOL IsSameDirectory(const std::wstring& path1, const std::wstring& path2);
        BOOL IsDestinationRecycleBin(const std::wstring& lpNewFileName);
        BOOL GetFullFilePathFromTitle(const std::wstring& title, __out std::set<std::wstring>& outFiles, const std::set<std::wstring> files);
		
        BOOL isContentData(IDataObject *pDataObject);
        BOOL GetClipboardContentDataSource(IDataObject *pDataObject, const std::wstring& clipboardInfo, __out std::wstring& srcFilePath);
        BOOL EnumClipboardToGetContentResource(__out std::wstring& srcFilePath);
        BOOL GetOleContentClipboardDataSource(IDataObject *pDataObject, const std::wstring& clipboardInfo, __out std::wstring& srcFilePath);

        BOOL GetFilesFromHDROP(HDROP hDrop, __out std::list<std::wstring>& files);

		std::wstring GetCommonComponentsDir();
		BOOL BJAnsiToUnicode (LPCSTR  pszAnsiBuff, LPWSTR lpWideCharStr, int cchDest);

        inline bool IsLocalDriver(const std::wstring& strPath)
		{			
			if (1 !=strPath.find(L":")){
				return false;		// the path should be like C: or D:
			}
			std::wstring strSub = strPath.substr(0, 2) + L"\\";				
			return (DRIVE_FIXED == GetDriveTypeW(strSub.c_str())) ? true : false;
		}

        BOOL CanIgnoreFile(const std::wstring& path);

        BOOL GetDenyImageFilePath(std::wstring& imagefile);
	} // ns utils

	/************************************************************************/
	/* Wrapper Win32 HMODULE                                                */
	/************************************************************************/
	class CModule 
	{
	public:
		CModule() :h_(NULL){_init();}
		CModule(HMODULE h): h_(h)	{_init();}

		inline operator HMODULE() const throw()	{return( h_ );}
		inline const wchar_t* Path(){return path_;	}
		inline const wchar_t* Parent() {return parent_;}
		inline const wchar_t* Name() {return name_;}

	private:
		inline void _init()
		{
			// init
			::ZeroMemory(path_,MAX_PATH*sizeof(wchar_t));
			::ZeroMemory(parent_,MAX_PATH*sizeof(wchar_t));
			::ZeroMemory(name_,MAX_PATH*sizeof(wchar_t));
			// path
			::GetModuleFileNameW(h_,path_,MAX_PATH);
			wchar_t* pname= NULL;
			// parent
			pname=::PathFindFileNameW(path_);
			wcsncpy_s(parent_,MAX_PATH,path_,pname-path_);
			// name
			wcsncpy_s(name_,MAX_PATH,pname,wcslen(pname));

		}
	private:
		HMODULE h_;
		wchar_t path_[MAX_PATH];
		wchar_t parent_[MAX_PATH];
		wchar_t name_[MAX_PATH];
	};

	/************************************************************************/
	/* Represent as Win32-process, Provide information about process        */
	/************************************************************************/
	class CEXEModule : public CModule
	{
	public:
		inline CEXEModule() : CModule(NULL)	{_init();}
	public:
		inline DWORD PID(){return id_;}
		inline BOOL IsWin7() {return isWin7_;}
		inline BOOL IsWin7Above() 	{return  isWin7Above_ ;	}
		// Parse command line
	private:
		inline void _init(){
			// get os version
			osver_.dwOSVersionInfoSize = sizeof(OSVERSIONINFOW);
			::GetVersionExW(&osver_);
			isWin7_ = (osver_.dwMajorVersion == 6 && osver_.dwMinorVersion >= 1);
			isWin7Above_ = (osver_.dwMajorVersion >6 || isWin7_ );
			// process id
			id_=::GetCurrentProcessId();
		}
	private:
		DWORD id_;
		// for win7		dwMajorVersion == 6 && dwMinorVersion >= 1
		// for xp,		5.1
		// for win2k	5.0
		OSVERSIONINFOW osver_;	
		BOOL isWin7_;
		BOOL isWin7Above_;
	};

	/************************************************************************/
	/* Represent a DLL module                                               */
	/************************************************************************/
	class CDllModule : public CModule
	{
	public:
		CDllModule(HMODULE h): CModule(h){	}

		BOOL DispatchEvent(DWORD reason_for_call)
		{
			switch (reason_for_call)
			{
			case DLL_PROCESS_ATTACH:
				// Initialize once for each new process.
				// Return FALSE to fail DLL load.
				return OnProcessAttach();

			case DLL_THREAD_ATTACH:
				// Do thread-specific initialization.
				return OnThreadAttach();

			case DLL_THREAD_DETACH:
				// Do thread-specific cleanup.
				return OnThreadDetach();
			case DLL_PROCESS_DETACH:
				// Perform any necessary cleanup.
				return OnProcessDetach();
			default:
				// ignored
				return FALSE;
				break;
			}
		}
	protected:
		virtual BOOL OnProcessAttach(){
			::DisableThreadLibraryCalls(this->operator HMODULE());
			return TRUE;
		}
		virtual BOOL OnThreadAttach(){
			return TRUE;
		}
		virtual BOOL OnThreadDetach(){
			return TRUE;
		}
		virtual BOOL OnProcessDetach(){
			return TRUE;
		}

	};
	/************************************************************************/
	/* Wrapper Win32 HANDLE                                                 */
	/************************************************************************/
	class CHandle
	{
	public:
		inline CHandle() throw() :	h_( NULL )	{}
		inline CHandle(_In_ HANDLE h) throw() :	h_( h ){}
		inline explicit CHandle(_Inout_ CHandle& h) throw() :h_( NULL )	
		{
			Attach( h.Detach() );
		}
		inline ~CHandle() throw()
		{
			if( h_ != NULL )
			{
				Close();
			}
		}

		inline CHandle& operator=(_Inout_ CHandle& h) throw()
		{
			if( this != &h )
			{
				if( h_ != NULL )
				{
					Close();
				}
				Attach( h.Detach() );
			}

			return( *this );
		}

		inline operator HANDLE() const throw()
		{
			return( h_ );
		}

		// Attach to an existing handle (takes ownership).
		inline void Attach(_In_ HANDLE h) throw()
		{
			h_ = h;  // Take ownership
		}

		// Detach the handle from the object (releases ownership).
		inline HANDLE Detach() throw()
		{
			HANDLE h;

			h = h_;  // Release ownership
			h_ = NULL;

			return( h );
		}

		// Close the handle.
		inline void Close() throw()
		{
			if( h_ != NULL )
			{
				::CloseHandle( h_ );
				h_ = NULL;
			}
		}

	public:
		HANDLE h_;
	};

	namespace policyengine
	{
		extern nextlabs::cesdk_context gCesdkContext;

        enum PolicyResult
        {
            PolicyResultAllow,
            PolicyResultDeny
        };

		enum WdeAction
		{
			WdeActionRead,
			WdeActionWrite,
			WdeActionDelete,
			WdeActionPrint,
			WdeActionRun,
			WdeActionEMail,
			WdeActionIM,
			WdeActionChangeAttribute,
			WdeActionChangeSecurityAttribute,
			WdeActionCopyContent,
			WdeActionCopy,
			WdeActionMove,
			WdeActionRename,
			WdeActionUpload,                     // SharePoint Upload
			WdeActionDownload,                   // SharePoint Download
			WdeActionConvert,                    // Adobe Convert
			WdeActionSend,                        // IEPep upload file.
            WdeActionMax
		};
		inline const wchar_t* WdeActionMap( WdeAction action )
		{
			static const wchar_t* action_name[WdeActionMax] =
			{
				L"OPEN",
				L"EDIT",
				L"DELETE",
				L"PRINT",
				L"RUN",
				L"EMAIL",
				L"IM",
				L"CHANGE_ATTRIBUTES",
				L"CHANGE_SECURITY",
				L"PASTE",
				L"COPY",
				L"MOVE",
				L"RENAME",
				L"UPLOAD",
				L"DOWNLOAD",
				L"CONVERT",
                L"SEND"
			};
			if( action < 0 || action >= WdeActionMax )
			{
				return NULL;
			}	
			return action_name[action];
		}

		struct strCEApplication
		{
			std::wstring appName;
			std::wstring appPath;
			std::wstring appUrl;
		};

        PolicyResult DoEvaluation(WdeAction action, const std::wstring& strSourcePath);
        PolicyResult DoEvaluation(WdeAction action, const std::wstring& strSourcePath, nextlabs::Obligations& obligations);
        PolicyResult DoEvaluation(WdeAction action, const std::wstring& strSourcePath, const std::wstring& strDestPath);
        PolicyResult DoEvaluation(WdeAction action, const std::wstring& strSourcePath, const std::wstring& strDestPath, nextlabs::Obligations& obligations);
		PolicyResult DoEvaluation(WdeAction action, const std::wstring& strSourcePath, const std::wstring& strDestPath, nextlabs::Obligations& obligations, strCEApplication* pceapp);
		PolicyResult DoEvaluation(WdeAction action, const std::wstring& strSourcePath, const std::wstring& strDestPath, nextlabs::Obligations& obligations, strCEApplication* pceapp, ATTRS* psourceAttributes);

		BOOL QueryPolicy(boost::shared_ptr<nextlabs::comm_base>& evaluationPtr, nextlabs::eval_parms& parm);
	}  // ns policyengine

	namespace policyassistant
	{
		
		// for this version we do not care about SE&auto_wrap		
		bool PAHelper(PABase::ACTION action, nextlabs::Obligations& obs, std::wstring& src, 
			std::wstring& dst, DWORD dwLastError, bool bAllow);

	}

    namespace comhelper
    {
        std::vector<std::pair<std::wstring, nextlabs::Obligations>> GetFilePathsFromObject(IUnknown* pObj);
        std::vector<std::pair<std::wstring, nextlabs::Obligations>> GetFilePathsFromObject(IUnknown* pObj, std::wstring& strSrcRoot);
        HRESULT CreateShellItemArrayFromPaths(UINT ct, std::vector<std::pair<std::wstring, nextlabs::Obligations>>& rgt, IShellItemArray **ppsia);
        HRESULT GetPathByShellItem(std::wstring& path, IShellItem * pItem);
		void GetNewCreatedFileName(const std::wstring& strOrigPath, std::wstring& strRealPath);
        // this struct used for file copy\move obligation
        typedef struct _FILEOP
        {
            std::wstring strSrc;
            std::wstring strDst; 
            std::wstring strConstSrc; 
            std::wstring strConstDst; // used to save the original path, maybe strDst will be changed after pa.
            nextlabs::Obligations obligations;  
        }FILEOP; 

        // recursively add the files of a subdirectory, the para strPath should be C:\test 
		BOOL HandleEFSObligation(LPCWSTR lpFileName);
        VOID ParseFolder(const std::wstring& strPath, std::vector<std::pair<std::wstring, nextlabs::Obligations>>& vecFile);
        BOOL SHFileOperation(std::vector<nextlabs::comhelper::FILEOP>& vecFileOp, bool bIsCopy);
        BOOL DoesFileExist(const std::wstring& file);
        VOID GenerateDestPath(const std::vector<std::wstring>& vecFiles, const std::wstring& strDestFolder, const std::wstring& strSrcRoot, std::vector<nextlabs::comhelper::FILEOP>& vecFileOP, bool bIsCopy); 
        VOID WrapMoveFileOperation(const std::wstring& strExistingFileName, const std::wstring& strNewFileName, std::vector<nextlabs::comhelper::FILEOP>& vecFileOP); 
        VOID ParseDeleteFolder(const std::wstring& strExistingFileName, std::vector<std::pair<std::wstring, nextlabs::Obligations>>& vecFile);

        BOOL GetExplorerPath(std::wstring& destPath);
        BOOL DataObj_CanGoAsync(IDataObject *pdtobj);
        BOOL DataObj_GoAsyncForCompat(IDataObject *pdtobj);

        inline void GetRelativeFilePath(const std::wstring& strPath, const std::wstring& strSrcRoot, std::wstring& strRelativePath)
        {
            //OutputDebugStringW(L" -------access GetRelativeFilePath========="); 
            //OutputDebugStringW(strPath.c_str()); 
            //OutputDebugStringW(strSrcRoot.c_str()); 

            strRelativePath = strPath;
            
            if(boost::istarts_with(strRelativePath, strSrcRoot))
            {
                strRelativePath.erase(0,strSrcRoot.length()+1); 
            }
            //OutputDebugStringW(strRelativePath.c_str()); 
        }

		struct HANDLETOMAPPINGSW 
		{
			UINT              uNumberOfMappings;  
			LPSHNAMEMAPPINGW   lpSHNameMapping;    
		};
    }  // ns comhelper 

	namespace hyperlinkbubble
	{
		struct BubbleStruct
		{
			const wchar_t* pHyperLink;
			const wchar_t* pAction;
		};

		void ShowBubble(const wchar_t* pHyperLink, int Timeout, const wchar_t* pAction);

		DWORD WINAPI BubbleThreadProc(LPVOID lpParameter);

		LRESULT CALLBACK BubbleWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

		typedef struct  
		{
			ULONG ulSize;
			WCHAR methodName [64];
			WCHAR params [7][256];
		}NOTIFY_INFO;

		typedef void (__stdcall* type_notify2)(NOTIFY_INFO* pInfo, int nDuration, HWND hCallingWnd, void* pReserved, const WCHAR* pHyperLink);
	}  // ns hyperlinkbubble

	namespace SaveAsInfo
	{
		struct SaveAsStruct
		{
			std::wstring strDestinationPath;
			nextlabs::Obligations obs;
			BOOL bPolicyAllow;
		};
	}  // ns SaveAsInfo

    namespace priorityHelper
    {
        bool SetObjectToLowIntegrity (HANDLE hObject, SE_OBJECT_TYPE type = SE_KERNEL_OBJECT);
    }

}  // ns nextlabs


#endif // __WDE_COMMON_UTILS__
