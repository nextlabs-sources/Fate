#pragma once

class CPrivateLoader
{
public:

	typedef CEResult_t (*CEP_StopPDP_t)(CEHandle handle, 
		CEString password,
		CEint32 timeout_in_seconds);


	CEP_StopPDP_t f_CEP_StopPDP_t;

	CPrivateLoader()
	{
		m_hlib = NULL;
		f_CEP_StopPDP_t = NULL;
		m_loaded = FALSE;
	}

	~CPrivateLoader()
	{

	}


	BOOL Load(_In_ const wchar_t* path)
	{
		assert( path != NULL );
		if( path == NULL )
		{
			return false;
		}

#ifdef _WIN64
		wchar_t sdk_lib[] = {L"ceprivate.dll"};
#else
		wchar_t sdk_lib[] = {L"ceprivate32.dll"};
#endif

		
		bool status = false;
		wchar_t sdk_path[1024] = {0};

		DWORD dwPathFlag = GetDllDirectory(1024, sdk_path);
		if((dwPathFlag != 0) || (wcsncmp(sdk_path, L"", 1) == 0))
		{
			 if(SetDllDirectory(path) == 0)
			 {
				status = false;
				goto Load_done;
			 }
		}
		else
		{
			status = false;
			goto Load_done;
		}
		WCHAR libpath[MAX_PATH] = {0};
		_snwprintf_s(libpath, _countof(libpath), _TRUNCATE, L"%s\\%s", path, sdk_lib);
		
		m_hlib = LoadLibraryW(libpath);
		SetDllDirectory(sdk_path);
		if( m_hlib == NULL )
		{
			status = false;
			goto Load_done;
		}
		f_CEP_StopPDP_t = (CEP_StopPDP_t)GetProcAddress(m_hlib,"CEP_StopPDP");

		if (!f_CEP_StopPDP_t)
		{
			status = false;
			goto Load_done;
		}

		m_loaded = true; /* load successful */
		status = true;

Load_done:
		/* If any of the functions cannot be found the SDK cannot be used. */
		if( status == false )
		{
			Unload();
		}/* if failed */

		return status;
	}

	BOOL IsLoaded()
	{
		return m_loaded;
	}

	BOOL Unload()
	{
		if (m_hlib)
		{
			FreeLibrary(m_hlib);
		}
		m_hlib = NULL;
		m_loaded = FALSE;
		f_CEP_StopPDP_t = NULL;

		return TRUE;
	}
	

	

private:



	

	HMODULE m_hlib;

	BOOL m_loaded;

};
