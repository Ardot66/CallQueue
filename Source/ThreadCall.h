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

#endif