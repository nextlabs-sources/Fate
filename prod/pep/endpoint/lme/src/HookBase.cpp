#include "stdafx.h"
#include "HookBase.h"

#include "ImageHlp.h"




void CHookBase::SubstituteOrgFuncWithNew( void* pObj, UINT nOrgVtableIndex, void* pNewFunc )
{
    if( !pObj )
    {
        return;
    }

    CLock aLock( &m_cs );

    m_mpNewFuncIndex[pNewFunc] = nOrgVtableIndex;

    MAPHookedIt it = m_mpHookedObj.find( pObj );

    if( it == m_mpHookedObj.end() )
    {
        for( MAPHookedIt it_l = m_mpHookedObj.begin(); it_l != m_mpHookedObj.end(); ++it_l )
        {
            MAPHookedSingle& aHookedSingle = (*it_l).second;
            MAPHookedSingleIt itTmp = aHookedSingle.find( nOrgVtableIndex );

            if( itTmp != aHookedSingle.end() )
            {
                if( (*((PVOID**)pObj))[nOrgVtableIndex] == ((*itTmp).second).m_pOrgFuncBak )//(*((PVOID**)((*it).first)))[nOrgVtableIndex] )
                {
                    m_mpHookedObj[pObj][nOrgVtableIndex] = ((*itTmp).second);
                    return;
                }
            }
        }
        FuncHookObj& aFuncHookObj = (m_mpHookedObj[pObj])[nOrgVtableIndex]; //= std::make_pair( (*(PVOID**)pObj)[nOrgVtableIndex], pNewFunc ) ;       
        aFuncHookObj.m_pOrgFunc = (*(PVOID**)pObj)[nOrgVtableIndex];
        aFuncHookObj.m_pOrgFuncBak = aFuncHookObj.m_pOrgFunc;
        aFuncHookObj.m_pNewFunc = pNewFunc;
        return;
    }
    else
    {
        MAPHookedSingle& aHookedFuncMap = (*it).second;
        MAPHookedSingleIt itSingle = aHookedFuncMap.find( nOrgVtableIndex );
        
        if( itSingle != aHookedFuncMap.end() ) 
        {
            FuncHookObj& aFuncHookObj = (*itSingle).second;
            if( !aFuncHookObj.m_bIsHooked )
            {
                //_ASSERT( (*(PVOID**)pObj)[nOrgVtableIndex] == (*itSingle).second.first );
                if( aFuncHookObj.m_pOrgFuncBak != (*(PVOID**)pObj)[nOrgVtableIndex] )
                {
                    aFuncHookObj.m_pOrgFunc = (*(PVOID**)pObj)[nOrgVtableIndex];//std::make_pair( (*(PVOID**)pObj)[nOrgVtableIndex], pNewFunc );
                    aFuncHookObj.m_pOrgFuncBak = aFuncHookObj.m_pOrgFunc;
                    aFuncHookObj.m_bIsHooked = false;
                }
                if( aFuncHookObj.m_pNewFunc != pNewFunc )
                {
                    aFuncHookObj.m_pNewFunc = pNewFunc;
                    aFuncHookObj.m_bIsHooked = false;
                }
            }
        }
        else
        {
            for( MAPHookedIt it_l = m_mpHookedObj.begin(); it_l != m_mpHookedObj.end(); ++it_l )
            {
                MAPHookedSingle& aHookedSingle = (*it_l).second;
                MAPHookedSingleIt itTmp = aHookedSingle.find( nOrgVtableIndex );

                if( itTmp != aHookedSingle.end() )
                {
                    if( (*((PVOID**)pObj))[nOrgVtableIndex] == ((*itTmp).second).m_pOrgFuncBak )//(*((PVOID**)((*it).first)))[nOrgVtableIndex] )
                    {
                        aHookedFuncMap[nOrgVtableIndex] = (*itTmp).second;
                        return;
                    }
                }
            }
            FuncHookObj& aFuncHookObj = aHookedFuncMap[nOrgVtableIndex];
            aFuncHookObj.m_pOrgFunc = (*(PVOID**)pObj)[nOrgVtableIndex];
            aFuncHookObj.m_pOrgFuncBak = aFuncHookObj.m_pOrgFunc;
            aFuncHookObj.m_pNewFunc = pNewFunc;
        }
    }   
}


void* CHookBase::GetOrgFunc( void* pObj, void* pNewFunc )
{
    if( !pObj || ! pNewFunc )
    {
        return 0;
    }

    CLock aLock( &m_cs );

    MAPHookedIt it = m_mpHookedObj.find( pObj );

    if( it == m_mpHookedObj.end() )
    {
        return 0;
    }
    else
    {
        MAPNewFuncIndexIt itNewFuncIdx = m_mpNewFuncIndex.find( pNewFunc );

        if( itNewFuncIdx == m_mpNewFuncIndex.end() )
        {
            return 0;
        }

        MAPHookedSingle& aHookedFuncMap = (*it).second;
        MAPHookedSingleIt itSingle = aHookedFuncMap.find( (*itNewFuncIdx).second );
        if( itSingle != aHookedFuncMap.end() ) 
        {
            return (*itSingle).second.m_pNextFunc;
            //return (*itSingle).second.m_pOrgFuncBak;
        }
        else
        {
            return 0;
        }
    }   
}

void* CHookBase::GetOrgFunc( void* pObj, UINT nFuncIndex )
{
    if( !pObj )
    {
        return 0;
    }

    CLock aLock( &m_cs );

    MAPHookedIt it = m_mpHookedObj.find( pObj );

    if( it == m_mpHookedObj.end() )
    {
        return 0;
    }
    else
    {
        MAPHookedSingle& aHookedFuncMap = (*it).second;
        MAPHookedSingleIt itSingle = aHookedFuncMap.find( nFuncIndex );
        if( itSingle != aHookedFuncMap.end() ) 
        {
          //  return (*itSingle).second.m_pOrgFunc;
			return (*itSingle).second.m_pNextFunc;
        }
        else
        {
            return 0;
        }
    }   
}

void CHookBase::DoHook( void* pObj )
{
    if( !pObj )
    {
        return;
    }

    CLock aLock( &m_cs );

    MAPHookedIt it = m_mpHookedObj.find( pObj );

    if( it == m_mpHookedObj.end() )
    {
        return;
    }
    
    MAPHookedSingle& aHookedFuncMap = (*it).second;
	BOOL bRet = false ;
    //gDetourTransactionBegin();
    //gDetourUpdateThread(GetCurrentThread());
	//  OutputDebugString( TEXT("Do  com hook") );
    for( MAPHookedSingleIt itSingle = aHookedFuncMap.begin(); itSingle != aHookedFuncMap.end(); ++itSingle )
    {
        FuncHookObj& aFuncHookObj = (*itSingle).second;
    /*    void*& pOrgFunc = aFuncHookObj.m_pOrgFunc;
        void*& pNewFunc = aFuncHookObj.m_pNewFunc;
		void*& pNextFunc =	aFuncHookObj.m_pNextFunc ;*/
		try
		{
			if( aFuncHookObj.m_pOrgFunc &&  aFuncHookObj.m_pNewFunc && !aFuncHookObj.m_bIsHooked )
			{

				// gDetourAttach( &(PVOID&)pOrgFunc, pNewFunc );
				bRet = HookCode(aFuncHookObj.m_pOrgFunc , aFuncHookObj.m_pNewFunc, &aFuncHookObj.m_pNextFunc ) ;
				if( bRet == false )
				{
				//	OutputDebugString( TEXT("Hook Code Failure") );
				}
				aFuncHookObj.m_bIsHooked = true;
			}
		}catch( ... )
		{
			   OutputDebugString( TEXT("Hook Code Met exception") );
		}
    }       
	//DP((L"Hook Finsihed")) ;
  //  gDetourTransactionCommit();
}