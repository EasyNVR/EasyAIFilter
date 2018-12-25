#ifndef __EASY_MUTEX_H__
#define __EASY_MUTEX_H__
#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

#ifndef EasyMutex
#ifdef _WIN32
#define EasyMutex CRITICAL_SECTION
#else
#define EasyMutex pthread_mutex_t
#endif
#endif

#ifdef __cplusplus
extern "C"
{
#endif
extern void EasyMutex_Init(EasyMutex* mutex);

extern void EasyMutex_Deinit(EasyMutex* mutex);

extern void EasyMutex_Lock(EasyMutex* mutex);

extern void EasyMutex_UnLock(EasyMutex* mutex);
#ifdef __cplusplus
};
#endif


#endif //__EASY_MUTEX_H__
