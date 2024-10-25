#include "ThreadCommunicator.h"
#include <stdlib.h>

struct ThreadCall
{
    void (*Function)(void *parameters);
    size_t ParametersSize;
};

inline size_t ThreadCallQueueGetIndex(ThreadCallQueue *callQueue, size_t index)
{
    return (callQueue->Offset + index) % callQueue->Size;
}

inline void *ThreadCallQueueGetElement(ThreadCallQueue *callQueue, size_t index)
{
    return ((char *)callQueue->Queue) + ThreadCallQueueGetIndex(callQueue, index);
}

int ThreadCallQueueInit(const size_t queueStartingSize, void *queue, ThreadCallQueue *callQueueDest)
{
    ThreadCallQueue callQueue;
    callQueue.Mutex = PTHREAD_MUTEX_INITIALIZER;
    callQueue.Offset = 0;
    callQueue.Count = 0;
    callQueue.Size = queueStartingSize;
    callQueue.Queue = queue;

    int result = pthread_mutex_init(&callQueue.Mutex, NULL);

    if(result)
        return result;

    *callQueueDest = callQueue;
    return EXIT_SUCCESS;
}

int ThreadCallQueueUpdateSize(ThreadCallQueue *callQueue, const size_t newSize)
{
    void *begin = ThreadCallQueueGetElement(callQueue, callQueue->Count);
    size_t preloopCount = callQueue->Count - ThreadCallQueueGetIndex(callQueue, callQueue->Count);

    for(size_t x = 0; x < callQueue->Count; x++)
    {

    }
}

int ThreadCallQueueResize(ThreadCallQueue *callQueue, const size_t newSize)
{
    if(callQueue->Size > newSize)
        ThreadCallQueueUpdateSize(callQueue, newSize);

    void *newQueue = realloc(callQueue->Queue, newSize);

    if(newQueue == NULL)
    {
        if(callQueue->Size > newSize)
            ThreadCallQueueUpdateSize(callQueue, callQueue->Size);

        return errno;
    }

    callQueue->Queue = newQueue;

    if(callQueue->Size < newSize)
        ThreadCallQueueUpdateSize(callQueue, newSize);

    callQueue->Size = newSize;
}

int ThreadCallQueuePush(ThreadCallQueue *callQueue, const void (*function)(void *parameters), const size_t parametersSize, const void *parameters)
{

}

int ThreadCallQueuePop(ThreadCallQueue *callQueue)
{
    
}

int ThreadCallQueueFlush(ThreadCallQueue *callQueue)
{

}