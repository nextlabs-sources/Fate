// dllmain.h : Declaration of module class.

class CcbPepModule : public CAtlDllModuleT< CcbPepModule >
{
public :
	DECLARE_LIBID(LIBID_cbPepLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_CBPEP, "{54CE3B8A-A8D6-468C-8848-F8567FE4F25A}")
};

extern class CcbPepModule _AtlModule;
