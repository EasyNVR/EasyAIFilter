#include "EasyQueue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void EasyQueue_Init(EasyQueue* queue)
{
    memset(queue, 0, sizeof(EasyQueue));
    queue->capacity = MAX_QUEUE_BUFF_SIZE;
	EasyMutex_Init(&queue->mutex);
}

void EasyQueue_Destroy(EasyQueue* queue)
{
	if (queue == NULL)
		return;
	EasyQueue_clear(queue);
	EasyMutex_Deinit(&queue->mutex);
}

int EasyQueue_enqueue(EASY_AV_Frame* elem, EasyQueue* queue)
{
	if (queue == NULL)
		return 0;

	EasyMutex_Lock(&queue->mutex);
    if (queue->count>=queue->capacity)
    {
		printf("Data buffer is full!\n");
		EasyMutex_UnLock(&queue->mutex);
        return -1;
    }

    int tail=queue->head+queue->count;
    if (tail>=queue->capacity)
    {
        tail-=queue->capacity;
    }
    queue->data[tail] = elem;
    queue->count++;   

	EasyMutex_UnLock(&queue->mutex);
    return 0;
}

EASY_AV_Frame* EasyQueue_dequeue(EasyQueue* queue)
{
	if (queue == NULL)
		return NULL;
	EasyMutex_Lock(&queue->mutex);
    if (!queue->count)
    {
		EasyMutex_UnLock(&queue->mutex);
        return NULL;
    }

	void* elem = queue->data[queue->head];
    queue->data[queue->head]=0;
    queue->head++;
    if (queue->head==queue->capacity)
    {
        queue->head = 0;
    }
    queue->count--;    
	EasyMutex_UnLock(&queue->mutex);

    return elem;     
}

void EasyQueue_clear(EasyQueue* queue)
{
	if (queue == NULL)
		return;
	EasyMutex_Lock(&queue->mutex);

    while(queue->count > 0)
    {
		EASY_AV_Frame* elem = (EASY_AV_Frame*)queue->data[queue->head];
		if (elem)
		{
			if (elem->pBuffer)
				free(elem->pBuffer);
			free(elem);
		}
        queue->data[queue->head]=0;
        queue->head++;
        if (queue->head==queue->capacity)
        {
            queue->head = 0;
        }
        queue->count--;    
    }

	EasyMutex_UnLock(&queue->mutex);

    return;
}

int EasyQueue_size(EasyQueue* queue)
{
	if (queue == NULL)
		return -1;
	EasyMutex_Lock(&queue->mutex);
    int count = queue->count;    
	EasyMutex_UnLock(&queue->mutex);
    return count;   
}
