#pragma once

//////////////////////////////////////////////////////////////////////////
class CHookedCollManager: public CHookBase
{
    INSTANCE_DECLARE( CHookedCollManager );

public:

    void Hook( void* pManager );

public:

    typedef HRESULT (__stdcall* Old_get_Desktop)(ICollaborateManager* pThis,struct ICollaborateDesktop * * varDesktop );
    static HRESULT __stdcall New_get_Desktop (ICollaborateManager* pThis,struct ICollaborateDesktop * * varDesktop ) ;
    typedef HRESULT (__stdcall* Old_get_Frames) (ICollaborateManager* pThis,struct ICollaborateFrames * * varFrames ) ;
    static HRESULT __stdcall New_get_Frames (ICollaborateManager* pThis,struct ICollaborateFrames * * varFrames ) ;
    typedef HRESULT (__stdcall* Old_get_Simple )(ICollaborateManager* pThis,struct ICollaborateSimple * * varSimple ) ;
    static HRESULT __stdcall New_get_Simple (ICollaborateManager* pThis,struct ICollaborateSimple * * varSimple ) ;



	
    static HRESULT __stdcall my_get_Desktop (ICollaborateManager* pThis,struct ICollaborateDesktop * * varDesktop ) ;
    
    static HRESULT __stdcall my_get_Frames (ICollaborateManager* pThis,struct ICollaborateFrames * * varFrames ) ;
    
    static HRESULT __stdcall my_get_Simple (ICollaborateManager* pThis,struct ICollaborateSimple * * varSimple ) ;
};

//////////////////////////////////////////////////////////////////////////