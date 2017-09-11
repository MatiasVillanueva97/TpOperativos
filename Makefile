COMPILER=gcc
BIN_PATH=bin
SRC_PATH=src
COMMONS=-lcommons
THREAD=-lpthread

default: all

all: filesystem master yama
.PHONY: all filesystem master yama

filesystem:
	mkdir -p filesystem/$(BIN_PATH)
	$(COMPILER) filesystem/$(SRC_PATH)/filesystem.c -o filesystem/$(BIN_PATH)/filesystem $(COMMONS) $(THREAD)

master:
	mkdir -p master/$(BIN_PATH)
	$(COMPILER) master/$(SRC_PATH)/master.c -o master/$(BIN_PATH)/master $(COMMONS) $(THREAD)
	
yama:
	mkdir -p yama/$(BIN_PATH)
	$(COMPILER) yama/$(SRC_PATH)/yama.c -o yama/$(BIN_PATH)/yama $(COMMONS) $(THREAD)

clean: 
	rm -rf fileSystem/$(BIN_PATH)
	rm -rf master/$(BIN_PATH)
	rm -rf yama/$(BIN_PATH)
