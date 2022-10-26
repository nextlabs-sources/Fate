// dllmain.h : Declaration of module class.

class CIconBadgingModule : public CAtlDllModuleT< CIconBadgingModule >
{
public :
	DECLARE_LIBID(LIBID_IconBadgingLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_ICONBADGING, "{A824802A-669B-4EC4-81DD-F9444C3EE137}")
};

extern class CIconBadgingModule _AtlModule;
