SHELL = cmd

BIN = Bin
SOURCE = Source
TEMP = Temp
TESTS = Tests

CALLQUEUENAME = CallQueue
TESTSNAME = Tests

CALLQUEUEDLL = $(BIN)/lib$(CALLQUEUENAME).dll
TESTSEXE = $(BIN)/$(TESTSNAME).exe

LOCAL_HEADERS := $(abspath $(wildcard */*.h))
LOCAL_INCLUDE := $(dir $(addprefix -I,$(LOCAL_HEADERS)))

All: $(CALLQUEUEDLL) $(TESTSEXE)
	$(TESTSEXE)

$(CALLQUEUEDLL) : $(LOCAL_HEADERS) $(SOURCE)/$(CALLQUEUENAME).c
	gcc -c -fPIC $(SOURCE)/$(CALLQUEUENAME).c -o $(TEMP)/$(CALLQUEUENAME).o
	gcc -shared $(TEMP)/$(CALLQUEUENAME).o -lpthread -o $(CALLQUEUEDLL)

$(TESTSEXE) : $(LOCAL_HEADERS) $(TESTS)/$(TESTSNAME).c
	gcc -c $(LOCAL_INCLUDE) $(TESTS)/$(TESTSNAME).c -o $(TEMP)/$(TESTSNAME).o
	gcc -L$(BIN) $(TEMP)/$(TESTSNAME).o -l$(CALLQUEUENAME) -lpthread -o $(TESTSEXE)

Clean:
	DEL /Q $(BIN)\*.dll $(BIN)\*.exe
	DEL /Q $(TEMP)\*.o