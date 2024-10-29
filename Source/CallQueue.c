#include "CallQueue.h"
#include <stdlib.h>

typedef struct CallData CallData;
struct CallData
{
    void (*Function)(void *parameters);
    size_t ParametersSize;
};

static inline size_t CallQueueGetIndex(const size_t offset, const size_t size, size_t index)
{
    return (offset + index) % size;
}

static inline char *CallQueueGetElement(void *queue, const size_t offset, const size_t size, size_t index)
{
    return ((char *)queue) + CallQueueGetIndex(offset, size, index);
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

int CallQueueInit(const size_t queueStartingSize, void *queue, CallQueue *callQueueDest)
{
    if(queue == NULL)
        return errno;

    CallQueue callQueue;
    callQueue.Offset = 0;
    callQueue.Count = 0;
    callQueue.Size = queueStartingSize;
    callQueue.Queue = queue;

    *callQueueDest = callQueue;
    return EXIT_SUCCESS;
}

int CallQueueResize(CallQueue *callQueue, const size_t newSize)
{
    const size_t preLoopCount = callQueue->Count - ((callQueue->Count + callQueue->Offset) % callQueue->Size) * (callQueue->Count + callQueue->Offset > callQueue->Size);

    if(callQueue->Size > newSize)
        SafeMemcopy(CallQueueGetElement(callQueue->Queue, callQueue->Offset, callQueue->Size, 0), CallQueueGetElement(callQueue->Queue, 0, newSize, newSize - preLoopCount), preLoopCount);

    void *newQueue = realloc(callQueue->Queue, newSize);

    if(newQueue == NULL)
    {
        if(callQueue->Size > newSize)
            SafeMemcopy(CallQueueGetElement(callQueue->Queue, 0, newSize, newSize - preLoopCount), CallQueueGetElement(callQueue->Queue, callQueue->Offset, callQueue->Size, 0), preLoopCount);

        return errno;
    }

    callQueue->Queue = newQueue;

    if(callQueue->Size < newSize)
        SafeMemcopy(CallQueueGetElement(callQueue->Queue, callQueue->Offset, callQueue->Size, 0), CallQueueGetElement(callQueue->Queue, 0, newSize, newSize - preLoopCount), preLoopCount);

    callQueue->Offset = newSize - preLoopCount;
    callQueue->Size = newSize;

    return EXIT_SUCCESS;
}

int CallQueuePush(CallQueue *callQueue, void (*function)(void *parameters), const size_t parametersSize, const void *parameters)
{
    CallData callData;
    callData.Function = function;
    callData.ParametersSize = parametersSize;

    while(sizeof(callData) + parametersSize + callQueue->Count > callQueue->Size)
    {
        int result = CallQueueResize(callQueue, (callQueue->Size + 1) * 2);

        if(result)
            return result;
    }

    {
        char *charCallData = (char *)&callData;
        for(size_t x = 0; x < sizeof(callData); x++)
            *CallQueueGetElement(callQueue->Queue, callQueue->Offset, callQueue->Size, x + callQueue->Count) = charCallData[x];
    }

    for(size_t x = 0; x < parametersSize; x++)
        *CallQueueGetElement(callQueue->Queue, callQueue->Offset, callQueue->Size, x + sizeof(callData) + callQueue->Count) = ((char *)parameters)[x];

    callQueue->Count += parametersSize + sizeof(callData);

    return 0;
}

int CallQueuePop(CallQueue *callQueue)
{
    if(callQueue->Count <= 0)
        return 1;

    CallData callData;

    {
        char *charCallData = (char *)&callData;
        for(size_t x = 0; x < sizeof(callData); x++)
            charCallData[x] = *CallQueueGetElement(callQueue->Queue, callQueue->Offset, callQueue->Size, x);
    }

    char parameters[callData.ParametersSize];
    
    for(size_t x = 0; x < callData.ParametersSize; x++)
        parameters[x] = *CallQueueGetElement(callQueue->Queue, callQueue->Offset, callQueue->Size, x + sizeof(callData));

    callQueue->Offset = (callQueue->Offset + sizeof(callData) + callData.ParametersSize) % callQueue->Size;
    callQueue->Count -= sizeof(callData) + callData.ParametersSize;

    callData.Function(parameters);

    return callQueue->Count <= 0;
} 

void CallQueueFree(CallQueue *callQueue)
{
    free(callQueue->Queue);
}