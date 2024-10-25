#include "ThreadCall.h"
#include <stdlib.h>

typedef struct ThreadCall ThreadCall;
struct ThreadCall
{
    void (*Function)(void *parameters);
    size_t ParametersSize;
};

inline size_t ThreadCallQueueGetIndex(ThreadCallQueue *callQueue, size_t index)
{
    return (callQueue->Offset + index) % callQueue->Size;
}

inline char *ThreadCallQueueGetElement(ThreadCallQueue *callQueue, size_t index)
{
    return ((char *)callQueue->Queue) + ThreadCallQueueGetIndex(callQueue, index);
}

void SafeMemcopy(const void *origin, void *dest, const size_t size)
{
    char *charOrigin = (char *)origin;
    char *charDest = (char *)dest;

    int direction = charOrigin < charDest;
    int indexer = !direction + direction * -1;

    charOrigin += direction * (size - 1);
    charDest += direction * (size - 1);

    for(size_t x = 0; x < size; x++, charOrigin += indexer, charDest += indexer)
        charDest[x] = charOrigin[x];
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
        SafeMemcopy(ThreadCallQueueGetElement(callQueue, 0), ThreadCallQueueGetElement(callQueue, newSize - preLoopCount), preLoopCount);

    void *newQueue = realloc(callQueue->Queue, newSize);

    if(newQueue == NULL)
    {
        if(callQueue->Size > newSize)
            SafeMemcopy(ThreadCallQueueGetElement(callQueue, newSize - preLoopCount), ThreadCallQueueGetElement(callQueue, 0), preLoopCount);

        return errno;
    }

    callQueue->Queue = newQueue;

    if(callQueue->Size < newSize)
        SafeMemcopy(ThreadCallQueueGetElement(callQueue, 0), ThreadCallQueueGetElement(callQueue, newSize - preLoopCount), preLoopCount);

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
            *ThreadCallQueueGetElement(callQueue, x) = charThreadCall[x];
    }

    for(size_t x = 0; x < parametersSize; x++)
        *ThreadCallQueueGetElement(callQueue, x + sizeof(threadCall)) = *(char *)parameters;

    callQueue->Count += parametersSize + sizeof(threadCall);

    return 0;
}

int ThreadCallQueuePop(ThreadCallQueue *callQueue)
{
    ThreadCall threadCall;

    {
        char *charThreadCall = (char *)&threadCall;
        for(size_t x = 0; x < sizeof(threadCall); x++)
            charThreadCall[x] = *ThreadCallQueueGetElement(callQueue, x);
    }

    char parameters[threadCall.ParametersSize];
    
    for(size_t x = 0; x < threadCall.ParametersSize; x++)
        parameters[x] = *ThreadCallQueueGetElement(callQueue, x + sizeof(threadCall));

    callQueue->Offset = (callQueue->Offset + sizeof(threadCall) + threadCall.ParametersSize) % callQueue->Size;
    callQueue->Count -= sizeof(threadCall) + threadCall.ParametersSize;

    threadCall.Function(parameters);

    return callQueue->Count <= 0;
}  