#pragma once



enum EM_FLAG{ EF_ADD, EF_SUB };

typedef std::vector<std::pair<std::wstring,std::wstring>> TAG;

class TalkWithSCE : private boost::noncopyable
{
public:
	static TalkWithSCE& GetInstance()
	{
		return sm_ThisInstance;
	}

private:
	TalkWithSCE(void){	}

	~TalkWithSCE(){	}

public:
	void StartServerThread();

	void UnregisterPEPCLient() const;

	void CacheOpenedFile(_In_ const std::wstring& fileName, _In_ EM_FLAG flag , _In_ const TAG& vecTag, _In_opt_ PVOID pObj = NULL);

private:
	void RegisterPEPCLient() const;

	_Check_return_ bool QueryScreenCapture(_Out_ std::string& DisplayText);

private:
	static TalkWithSCE sm_ThisInstance;

private:
	struct OpenedFilesStruct
	{
		int Count;
		PVOID pObj;
		TAG tag;

		OpenedFilesStruct() : Count(0), pObj(NULL)
		{

		}

		OpenedFilesStruct(int iCount, PVOID in_pObj, const TAG& itag) : Count(iCount), pObj(in_pObj), tag(itag)
		{

		}
	};

private:
	boost::shared_mutex m_mutex;

	std::map<std::wstring, OpenedFilesStruct>  m_AllOpenedFiles;
};
