#pragma once

bool PathName2Path(char* szPathName,char Path[MAX_PATH]);
bool PathName2Path(wchar_t* wszPathName,wchar_t Path[MAX_PATH]);

class Mutex 
{
	friend class MUTEX;
public:
	Mutex()
	{
		InitializeCriticalSection(&cs);
	}
	~Mutex()
	{
		DeleteCriticalSection(&cs);
	}
public:
	void lock()
	{
		EnterCriticalSection(&cs);
	}
	void unlock()
	{
		LeaveCriticalSection(&cs);
	}
private:
	Mutex(const Mutex& mutex);
	void operator=(const Mutex& mutex);
private:	
	CRITICAL_SECTION cs;
};

class MUTEX
{
public:
	MUTEX(Mutex *mutex):_mutex(mutex)
	{
		_mutex->lock();
	}
	~MUTEX()
	{
		_mutex->unlock();
	}
private:
	Mutex* _mutex;
};


void GetSessionTypeString(WCHAR SessionType[256],enum UCC_SESSION_TYPE enSessionType);

BOOL InitLog();
