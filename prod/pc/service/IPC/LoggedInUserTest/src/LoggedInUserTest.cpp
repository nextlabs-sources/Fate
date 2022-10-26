// LoggedInUserTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "..\..\shared\src\globals.h"
#include "..\..\shared\src\userutils.h"

int _tmain(int argc, _TCHAR* argv[])
{
    StringVector users;
    UserUtils::GetLoggedInUsers (users);

    for (unsigned int i = 0; i < users.size (); i++) 
    {
        _tprintf (_T("%d. %s\n"), i, users[i]);
    }
    return 0;
}

