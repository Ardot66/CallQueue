#include <stdio.h>
#include <stdlib.h>
#include "ThreadCall.h"

#define TEST(expression, onFail, ...) \
{\
    const char *expressionString = #expression; \
    TestsCount++; \
    if(expression) \
    {\
        printf("Test Passed: %s\n", expressionString); \
        TestsPassed ++;\
    } \
    else\
        printf("Test Failed: %s; " onFail " at %s line %d\n", expressionString, ## __VA_ARGS__, __FILE__, __LINE__);\
}

#define TESTEND printf("%llu out of %llu tests passed", TestsPassed, TestsCount)

#define TRY(function, ...) if(result = (function)) {__VA_ARGS__}

typedef struct ThreadData ThreadData;
struct ThreadData
{
    ThreadCallQueue CallQueue;
    pthread_cond_t Finished;
    pthread_mutex_t FinishedMutex;
    size_t Result;
};

static size_t TestsPassed = 0;
static size_t TestsCount = 0;

struct ThreadTestParameters
{
    ThreadData *ThreadData;
    size_t Value;
};

void ThreadTest(void *parameters)
{
    struct ThreadTestParameters *thisParameters = parameters;
    thisParameters->ThreadData->Result += thisParameters->Value;
}

void ThreadCleanup(void *parameters)
{
    int result;

    ThreadData *threadData = (ThreadData *)parameters;
    ThreadCallQueueFree(&threadData->CallQueue);
    pthread_cond_destroy(&threadData->Finished);
    pthread_mutex_destroy(&threadData->FinishedMutex);
    free(threadData);
}

void *ThreadMain(void *parameters)
{
    ThreadData *threadData = (ThreadData *)parameters;

    int result;
    pthread_cleanup_push(ThreadCleanup, threadData);

    while(1)
    {
        pthread_testcancel();

        if(threadData->CallQueue.Count <= 0)
            continue;

        TRY(pthread_mutex_lock(&threadData->CallQueue.Mutex), return NULL;)

        while(ThreadCallQueuePop(&threadData->CallQueue) == EXIT_SUCCESS);

        TRY(pthread_mutex_lock(&threadData->FinishedMutex), return NULL;)
        TRY(pthread_cond_signal(&threadData->Finished), return NULL;)
        TRY(pthread_mutex_unlock(&threadData->FinishedMutex), return NULL;)

        TRY(pthread_mutex_unlock(&threadData->CallQueue.Mutex), return NULL;)
    }

    pthread_cleanup_pop(1);

    return NULL;
}

int main(int argCount, char **argValues)
{
    const size_t callQueueStartingSize = 64;
    int result;

    ThreadData *threadData;
    TRY(!(threadData = malloc(sizeof(*threadData))), return result;)
    
    threadData->Finished = PTHREAD_COND_INITIALIZER;
    threadData->FinishedMutex = PTHREAD_MUTEX_INITIALIZER;
    threadData->Result = 0;

    printf("Working\n");
    TRY(pthread_mutex_init(&threadData->FinishedMutex, NULL), return result;)
    TRY(pthread_cond_init(&threadData->Finished, NULL), return result;)
    TRY(ThreadCallQueueInit(callQueueStartingSize, malloc(callQueueStartingSize), &threadData->CallQueue), return result;)

    pthread_t thread;
    TRY(pthread_create(&thread, NULL, ThreadMain, threadData), return result;)

    const size_t callCount = 10;
    const size_t threadTestResult = 80085;

    TRY(pthread_mutex_lock(&threadData->CallQueue.Mutex), return result;)   

    for(size_t x = 0; x < callCount; x++)
    {
        struct ThreadTestParameters parameters;
        parameters.ThreadData = threadData;
        parameters.Value = threadTestResult;

        TRY(ThreadCallQueuePush(&threadData->CallQueue, ThreadTest, sizeof(parameters), &parameters), return result;)
    }
    
    TRY(pthread_mutex_unlock(&threadData->CallQueue.Mutex), return result;)

    TRY(pthread_mutex_lock(&threadData->FinishedMutex), return result;)
    while(threadData->Result == 0)
        TRY(pthread_cond_wait(&threadData->Finished, &threadData->FinishedMutex), return result;)
    TRY(pthread_mutex_unlock(&threadData->FinishedMutex), return result;)

    TEST(threadTestResult * callCount == threadData->Result, "test result was %llu, should have been %llu", threadData->Result, threadTestResult * callCount);

    TRY(pthread_cancel(thread), return result;)

    void *threadResult;
    TRY(pthread_join(thread, &threadResult), return result;)
    TESTEND;
}
