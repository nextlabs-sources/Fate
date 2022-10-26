#pragma once

class CLock 
{
public:
    CLock( CRITICAL_SECTION* pCs ):m_pCs( pCs ) { EnterCriticalSection( m_pCs ); }
    ~CLock(){ LeaveCriticalSection( m_pCs ); }
private:
    CRITICAL_SECTION* m_pCs;
};

class CHookBase
{
private:

    typedef struct tagFuncHook
    {
        void* m_pOrgFuncBak;
        void* m_pOrgFunc;
        void* m_pNewFunc;
		void* m_pNextFunc ;
        bool  m_bIsHooked;
        tagFuncHook():m_pOrgFuncBak(0),m_pOrgFunc(0),m_pNewFunc(0),m_pNextFunc(0),m_bIsHooked(false){}
    }FuncHookObj;

    typedef std::map< UINT, FuncHookObj > MAPHookedSingle;
    typedef MAPHookedSingle::iterator MAPHookedSingleIt;

    typedef std::map< void*, MAPHookedSingle > MAPHooked;
    typedef MAPHooked::iterator MAPHookedIt;

    MAPHooked   m_mpHookedObj;

    typedef std::map< void*, UINT > MAPNewFuncIndex;
    typedef MAPNewFuncIndex::iterator MAPNewFuncIndexIt;
    MAPNewFuncIndex m_mpNewFuncIndex;
    //void*       m_pObjBase;
    //void**      m_pVtable;    

protected:

    CHookBase( )//void* pObjBase ):m_pObjBase(pObjBase)
    { 
        //m_pVtable = *(PVOID**)pObjBase;
        InitializeCriticalSection( &m_cs ); 
    }
    ~CHookBase(){ DeleteCriticalSection(&m_cs); }

    CRITICAL_SECTION m_cs;

private:


protected:

    void SubstituteOrgFuncWithNew( void* pObj, UINT nOrgVtableIndex, void* pNewFunc );

    void DoHook( void* pObj );

    void* GetOrgFunc( void* pObj, void* pNewFunc );

    void* GetOrgFunc( void* pObj, UINT nFuncIndex );

protected:

};

//ClassName(){};\
//~##ClassName();
#define INSTANCE_DECLARE( ClassName ) private:\
    static ClassName* m_pInstance;\
public:\
       static ClassName* GetInstance()\
{\
if( !m_pInstance )\
{\
m_pInstance = new ClassName();\
}\
return m_pInstance;\
}\

#define INSTANCE_DEFINE( ClassName ) ClassName* ClassName::m_pInstance = 0

//#include "evaluator.h"
//class CPDPEvaluator
//{
//private:
//    
//    CPDPEvaluator(){ extern CEvaluator gEva;gEva.EvaInitialize(0); }
//    ~CPDPEvaluator(){ extern CEvaluator gEva;gEva.EvaUninitialize(); }
//    static CPDPEvaluator* m_pInstance;
//public:
//    static CPDPEvaluator* GetInstance()
//    {
//        if( !m_pInstance )
//        {
//        m_pInstance = new CPDPEvaluator();
//        }
//        return m_pInstance;
//    }
//};