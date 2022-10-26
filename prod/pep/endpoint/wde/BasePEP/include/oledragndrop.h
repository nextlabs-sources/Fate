#ifndef _OLE_DRAG_N_DROP
#define _OLE_DRAG_N_DROP

#include "commonutils.hpp"

#include <windows.h>
#include <string>

namespace nextlabs
{
    typedef struct 
    {
        std::wstring className;
        policyengine::WdeAction action;
        BOOL shouldTriggleEvent;
    } DNDWinClassAction;

    typedef struct 
    {
        std::wstring processName;
        DNDWinClassAction *winClassActions;
    } DNDAppWinClassAction;
    
    //Wordpad
    static DNDWinClassAction WORDPAD_CLASS_ACTIONS[] = {
        {L"RICHEDIT50W", policyengine::WdeActionCopy, TRUE},
        {L"", policyengine::WdeActionMax, FALSE}
    };

    //Explorer
    static DNDWinClassAction EXPLORER_CLASS_ACTIONS[] = {
        {L"*", nextlabs::policyengine::WdeActionCopy, FALSE},
        {L"", nextlabs::policyengine::WdeActionMax, FALSE}
    };

    //WinWord
    static DNDWinClassAction WORD_CLASS_ACTIONS[] = {
        {L"_WwG", nextlabs::policyengine::WdeActionCopy, TRUE},
        {L"", nextlabs::policyengine::WdeActionMax, FALSE} 
    };

    static DNDAppWinClassAction DRAG_N_DROP_APP_CLASS_ACTIONS[] = {
        {L"wordpad.exe", WORDPAD_CLASS_ACTIONS},
        {L"explorer.exe", EXPLORER_CLASS_ACTIONS},
        {L"winword.exe", WORD_CLASS_ACTIONS},
    };
}

#endif