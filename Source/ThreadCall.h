#ifndef __THREAD_COMMUNICATOR__
#define __THREAD_COMMUNICATOR__

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