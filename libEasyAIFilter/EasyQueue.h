#ifndef __EASY_QUEUE_H__
#define __EASY_QUEUE_H__

#include "EasyMutex.h"
#include "EasyTypes.h"


//缓存个数
#define MAX_QUEUE_BUFF_SIZE 20


typedef struct _EasyQueue_T{
    void* data[MAX_QUEUE_BUFF_SIZE];
    int head;
    int count;
    int capacity;
    EasyMutex mutex;
} EasyQueue;

#ifdef __cplusplus
extern "C"
{
#endif
extern  void EasyQueue_Init(EasyQueue* queue);
extern  void EasyQueue_Destroy(EasyQueue* queue);
extern  int EasyQueue_size(EasyQueue* queue);
extern  int EasyQueue_enqueue(EASY_AV_Frame* elem, EasyQueue* queue);
/*
 *Need do free by userself
 */
extern  EASY_AV_Frame* EasyQueue_dequeue(EasyQueue* queue);
extern	void EasyQueue_clear(EasyQueue* queue);
#ifdef __cplusplus
};
#endif



#endif //__EASY_QUEUE_H__
