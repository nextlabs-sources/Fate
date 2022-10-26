// dllmain.h : Declaration of module class.

class CenhancementModule : public CAtlDllModuleT< CenhancementModule >
{
public :
	DECLARE_LIBID(LIBID_enhancementLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_ENHANCEMENT, "{2A9EAF67-12C2-4655-A2DD-E7D988A3AE59}")
};

extern class CenhancementModule _AtlModule;
