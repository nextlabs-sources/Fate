#ifndef __WDE_CONTENTSTORAGE_H__
#define __WDE_CONTENTSTORAGE_H__

#ifdef _MSC_VER
#pragma once
#else
#error "contentstorage.h only supports windows-compile"
#endif // _MSC_VER

#include <windows.h>
#include <string>

#include "nlconfig.hpp"
#include "commonlib_helper.h"

namespace nextlabs
{

class CContextStorage
{
public:
	CContextStorage(){

	}

public:
	bool StoreClipboardInfo(const std::wstring& info);
    bool GetClipboardInfo(std::wstring& info);

    bool StoreDragDropContentFileInfo(const std::wstring& filePath);
    bool GetDragDropContentFileInfo(std::wstring& filePath);

private:
	std::wstring GetCommonComponentsDir();
	bool Init();

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

			if(NULL == pwsz) throw std::string("pwsz == NULL");

			int cchW = static_cast<int>(wcslen(pwsz)) + 1;		

			// calculate the required size 
			int nRequriedBytes = ::WideCharToMultiByte(CP_ACP,0,pwsz,cchW,NULL,0,NULL,NULL);

			if(nRequriedBytes > nDefaultBufLen)
			{
				// allocate at heap
				pstr_ = new char[nRequriedBytes*sizeof(char)];
				if(0==::WideCharToMultiByte(CP_ACP,0,pwsz,cchW,pstr_,nRequriedBytes,NULL,NULL))
				{
					throw std::string("U2A convert failed");
				}
			}
			else
			{
				// no need to use heap, use stack instead
				pstr_ = buffer_;
				if(0==::WideCharToMultiByte(CP_ACP,0,pwsz,cchW,buffer_,nDefaultBufLen,NULL,NULL))
				{
					throw std::string("U2A convert failed");
				}
			}			

		}
	private:
		char* pstr_;
		char  buffer_[nDefaultBufLen];
	};

	typedef CStrU2A<>  U2A;

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
			if(NULL == pstr) throw std::string("pstr is null");

			int cchA = static_cast<int>(strlen(pstr))+1;
			// calculate the required size 
			int nRequiredSize =::MultiByteToWideChar(CP_ACP,0,pstr,cchA,NULL,0)*sizeof(wchar_t);
			if(nRequiredSize > nDefaultBufLen*sizeof(wchar_t))
			{
				// alloc at heap
				pwstr_ = new wchar_t[nRequiredSize/sizeof(wchar_t)+1];
				if(0 == ::MultiByteToWideChar(CP_ACP,0,pstr,cchA,pwstr_,nRequiredSize/sizeof(wchar_t)+1))
				{
					throw std::string("A2U convert failed");
				}
			}
			else
			{
				// no need to use heap, use stack instead
				pwstr_ = buffer_;
				if(0 == ::MultiByteToWideChar(CP_ACP,0,pstr,cchA,pwstr_,nDefaultBufLen*sizeof(wchar_t)))
				{
					throw std::string("A2U convert failed");
				}
			}

		}
	private:
		wchar_t* pwstr_;
		wchar_t  buffer_[nDefaultBufLen];
	};
	typedef CStrA2U<>	A2U;

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

private:
	static HMODULE m_hNLStorage;
	static nextlabs::nl_CacheData m_fnCacheData;
	static nextlabs::nl_GetData m_fnGetData;
	static nextlabs::nl_FreeMem m_fnFreeMem;

    bool Set(const std::string& key, const std::string& value);
    bool Get(const std::string& key, std::string& value);
private:
	static const wchar_t* m_basepepContent;
};

}  // ns nextlabs

#endif //__WDE_CONTENTSTORAGE_H__



