#include <stdio.h>
#include <stdlib.h>
#include "CallQueue.h"

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

static size_t TestsPassed = 0;
static size_t TestsCount = 0;

struct TestFunctionParameters
{
    size_t *Dest;
    size_t Value;
};

void TestFunction(size_t *dest, size_t value)
{
    *dest += value;
}

void G_TestFunction(void *parameters)
{
    struct TestFunctionParameters *thisParameters = parameters;
    TestFunction(thisParameters->Dest, thisParameters->Value);
}

int main (int argCount, char ** argValues)
{
    int result;
    const size_t callQueueStartingCount = 64;

    CallQueue callQueue;
    TRY(CallQueueInit(callQueueStartingCount, malloc(callQueueStartingCount), &callQueue), return result;)

    const size_t pushAmount = 20;
    size_t testValue = 0;

    for(size_t x = 0; x < pushAmount; x++)
    {
        struct TestFunctionParameters functionParameters;
        functionParameters.Dest = &testValue;
        functionParameters.Value = pushAmount;

        TRY(CallQueuePush(&callQueue, G_TestFunction, sizeof(functionParameters), &functionParameters), return result;)
    }


    printf("Working\n");
    while(!CallQueuePop(&callQueue));

    TEST(testValue == pushAmount * pushAmount, "testValue %llu should have been %llu", testValue, pushAmount * pushAmount);

    TESTEND;

    return EXIT_SUCCESS;
}
