// dllmain.h : Declaration of module class.

class COutlookAddinModule : public CAtlDllModuleT< COutlookAddinModule >
{
public :
	DECLARE_LIBID(LIBID_OutlookAddinLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_OUTLOOKADDIN, "{24723DC1-D4DD-44A4-AA20-384DCB29E44E}")
};

extern class COutlookAddinModule _AtlModule;
