#ifndef __THREAD_CALL__
#define __THREAD_CALL__

#include <pthread.h>

typedef struct ThreadCallQueue ThreadCallQueue;

struct ThreadCallQueue
{
    void *Queue;
    size_t Size;
    size_t Count;
    size_t Offset;
    pthread_mutex_t Mutex;
};

int ThreadCallQueueInit(const size_t queueStartingSize, void *queue, ThreadCallQueue *callQueueDest);
int ThreadCallQueueResize(ThreadCallQueue *callQueue, const size_t newSize);
int ThreadCallQueuePush(ThreadCallQueue *callQueue, void (*function)(void *parameters), const size_t parametersSize, const void *parameters);
int ThreadCallQueuePop(ThreadCallQueue *callQueue);
void ThreadCallQueueFree(ThreadCallQueue *callQueue);

#endif