#include "EasyMutex.h"

void EasyMutex_Init(EasyMutex* mutex)
{
#ifdef _WIN32
	InitializeCriticalSection(mutex);
#else
	//*mutex = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_init(mutex, NULL);
#endif
}

void EasyMutex_Deinit(EasyMutex* mutex)
{
#ifdef _WIN32
	DeleteCriticalSection(mutex);
#else
	pthread_mutex_destroy(mutex);
#endif
}

void EasyMutex_Lock(EasyMutex* mutex)
{
#ifdef _WIN32
	EnterCriticalSection(mutex);
#else
	pthread_mutex_lock(mutex);
#endif
}

void EasyMutex_UnLock(EasyMutex* mutex)
{
#ifdef _WIN32
	LeaveCriticalSection(mutex);
#else
	pthread_mutex_unlock(mutex);
#endif
}