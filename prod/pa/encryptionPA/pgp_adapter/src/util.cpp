#include "stdafx.h"

#include "util.h"



LockHandle GsmLockAlloc(void)
{
#ifdef WIN32
	HANDLE mutex = NULL;
	SECURITY_ATTRIBUTES sa;

	memset (&sa, 0, sizeof sa);
	sa.bInheritHandle = FALSE;
	sa.lpSecurityDescriptor = NULL;
	sa.nLength = sizeof sa;

	mutex = CreateMutex (&sa, FALSE, NULL);
	return (LockHandle)mutex;
#else
#endif
}

void GsmLockFree(LockHandle hLock)
{
#ifdef WIN32
	if (NULL != hLock)
	{
		CloseHandle((HANDLE)hLock);
	}
#else
#endif
}

/* Returns 0 on success. */
int GsmLock(LockHandle hLock)
{
#ifdef WIN32
	if (hLock)
	{
		int code = WaitForSingleObject ((HANDLE)hLock, INFINITE);
		return code != WAIT_OBJECT_0;
	}
	else
	{
		return 1;
	}
#else
#endif
}

void GsmUnlock(LockHandle hLock)
{
#ifdef WIN32
	if (hLock)
	{
		ReleaseMutex ((HANDLE)hLock);
	}
#else
#endif
}
