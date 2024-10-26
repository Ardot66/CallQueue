DIRECTORY := $(patsubst %/,%,$(dir $(realpath $(lastword $(MAKEFILE_LIST)))))
BIN = Bin
SOURCE := $(DIRECTORY)/Source
TEMP = Temp

CALLQUEUENAME = CallQueue
CALLQUEUEDLL := $(BIN)/lib$(CALLQUEUENAME).dll

All: $(CALLQUEUEDLL)

$(CALLQUEUEDLL) : $(SOURCE)/$(CALLQUEUENAME).c
	gcc -c -fPIC $(SOURCE)/$(CALLQUEUENAME).c -o $(TEMP)/$(CALLQUEUENAME).o
	gcc -shared $(TEMP)/$(CALLQUEUENAME).o -lpthread -o $(CALLQUEUEDLL)

Update:
	git pull origin export