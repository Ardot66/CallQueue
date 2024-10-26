#ifndef __CALL_QUEUE__
#define __CALL_QUEUE__

#include <stddef.h>

typedef struct CallQueue CallQueue;
struct CallQueue
{
    void *Queue;
    size_t Size;
    size_t Count;
    size_t Offset;
};

int CallQueueInit(const size_t queueStartingSize, void *queue, CallQueue *callQueueDest);
int CallQueueResize(CallQueue *callQueue, const size_t newSize);
int CallQueuePush(CallQueue *callQueue, void (*function)(void *parameters), const size_t parametersSize, const void *parameters);
int CallQueuePop(CallQueue *callQueue);
void CallQueueFree(CallQueue *callQueue);

#endif