#include "ThreadCall.h"
#include <stdlib.h>

typedef struct ThreadCall ThreadCall;
struct ThreadCall
{
    void (*Function)(void *parameters);
    size_t ParametersSize;
};

static inline size_t ThreadCallQueueGetIndex(const size_t offset, const size_t size, size_t index)
{
    return (offset + index) % size;
}

static inline char *ThreadCallQueueGetElement(void *queue, const size_t offset, const size_t size, size_t index)
{
    return ((char *)queue) + ThreadCallQueueGetIndex(offset, size, index);
}

static void SafeMemcopy(const void *origin, void *dest, const size_t size)
{
    char *charOrigin = (char *)origin;
    char *charDest = (char *)dest;

    const int direction = charOrigin > charDest;
    const int indexer = !direction + direction * -1;

    charOrigin += direction * (size - 1);
    charDest += direction * (size - 1);

    for(size_t x = 0; x < size; x++, charOrigin += indexer, charDest += indexer)
        *charDest = *charOrigin;
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

int ThreadCallQueueResize(ThreadCallQueue *callQueue, const size_t newSize)
{
    const size_t preLoopCount = callQueue->Count - ((callQueue->Count + callQueue->Offset) % callQueue->Size) * (callQueue->Count + callQueue->Offset > callQueue->Size);

    if(callQueue->Size > newSize)
        SafeMemcopy(ThreadCallQueueGetElement(callQueue->Queue, callQueue->Offset, callQueue->Size, 0), ThreadCallQueueGetElement(callQueue->Queue, 0, newSize, newSize - preLoopCount), preLoopCount);

    void *newQueue = realloc(callQueue->Queue, newSize);

    if(newQueue == NULL)
    {
        if(callQueue->Size > newSize)
            SafeMemcopy(ThreadCallQueueGetElement(callQueue->Queue, 0, newSize, newSize - preLoopCount), ThreadCallQueueGetElement(callQueue->Queue, callQueue->Offset, callQueue->Size, 0), preLoopCount);

        return errno;
    }

    callQueue->Queue = newQueue;

    if(callQueue->Size < newSize)
        SafeMemcopy(ThreadCallQueueGetElement(callQueue->Queue, callQueue->Offset, callQueue->Size, 0), ThreadCallQueueGetElement(callQueue->Queue, 0, newSize, newSize - preLoopCount), preLoopCount);

    callQueue->Offset = newSize - preLoopCount;
    callQueue->Size = newSize;

    return EXIT_SUCCESS;
}

int ThreadCallQueuePush(ThreadCallQueue *callQueue, void (*function)(void *parameters), const size_t parametersSize, const void *parameters)
{
    ThreadCall threadCall;
    threadCall.Function = function;
    threadCall.ParametersSize = parametersSize;

    while(sizeof(threadCall) + parametersSize + callQueue->Count > callQueue->Size)
    {
        int result = ThreadCallQueueResize(callQueue, (callQueue->Size + 1) * 2);

        if(result)
            return result;
    }

    {
        char *charThreadCall = (char *)&threadCall;
        for(size_t x = 0; x < sizeof(threadCall); x++)
            *ThreadCallQueueGetElement(callQueue->Queue, callQueue->Offset, callQueue->Size, x + callQueue->Count) = charThreadCall[x];
    }

    for(size_t x = 0; x < parametersSize; x++)
        *ThreadCallQueueGetElement(callQueue->Queue, callQueue->Offset, callQueue->Size, x + sizeof(threadCall) + callQueue->Count) = ((char *)parameters)[x];

    callQueue->Count += parametersSize + sizeof(threadCall);

    return 0;
}

int ThreadCallQueuePop(ThreadCallQueue *callQueue)
{
    ThreadCall threadCall;

    {
        char *charThreadCall = (char *)&threadCall;
        for(size_t x = 0; x < sizeof(threadCall); x++)
            charThreadCall[x] = *ThreadCallQueueGetElement(callQueue->Queue, callQueue->Offset, callQueue->Size, x);
    }

    char parameters[threadCall.ParametersSize];
    
    for(size_t x = 0; x < threadCall.ParametersSize; x++)
        parameters[x] = *ThreadCallQueueGetElement(callQueue->Queue, callQueue->Offset, callQueue->Size, x + sizeof(threadCall));

    callQueue->Offset = (callQueue->Offset + sizeof(threadCall) + threadCall.ParametersSize) % callQueue->Size;
    callQueue->Count -= sizeof(threadCall) + threadCall.ParametersSize;

    threadCall.Function(parameters);

    return callQueue->Count <= 0;
} 

void ThreadCallQueueFree(ThreadCallQueue *callQueue)
{
    free(callQueue->Queue);
    pthread_mutex_destroy(&callQueue->Mutex);
}