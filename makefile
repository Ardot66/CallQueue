SOURCE = Source

CALLQUEUENAME = CallQueue
CALLQUEUEDLL := $(BIN)/lib$(CALLQUEUENAME).dll

LOCAL_HEADERS := $(abspath $(wildcard */*.h))
LOCAL_INCLUDE := $(dir $(addprefix -I,$(LOCAL_HEADERS)))

All: $(CALLQUEUEDLL)

$(CALLQUEUEDLL) : $(LOCAL_HEADERS) $(SOURCE)/$(CALLQUEUENAME).c
	gcc -c -fPIC $(SOURCE)/$(CALLQUEUENAME).c -o $(TEMP)/$(CALLQUEUENAME).o
	gcc -shared $(TEMP)/$(CALLQUEUENAME).o -lpthread -o $(CALLQUEUEDLL)
