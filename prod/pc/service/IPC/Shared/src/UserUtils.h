
#ifndef _USERUTILS_H_
#define _USERUTILS_H_

class UserUtils
{
public:
    UserUtils(void);
    ~UserUtils(void);

    //////////////////////////////////////////////////////////////////////////////////
    // Return the SID of the specified user. pszSID must be deallocated by caller
    //
    //////////////////////////////////////////////////////////////////////////////////
	static void GetUserSID(TCHAR* pszUserName, TCHAR **ppszSID);

    static void GetLoggedInUsers (StringVector& loggedInUsers);

};

#endif