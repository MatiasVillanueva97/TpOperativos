COMPILER=gcc
BIN_PATH=bin
SRC_PATH=src
COMMONS=-lcommons
THREAD=-lpthread

default: all

all: filesystem master yama
.PHONY: all filesystem master yama

filesystem:
	mkdir -p FileSystem/$(BIN_PATH)
	$(COMPILER) FileSystem/$(SRC_PATH)/filesystem.c -o FileSystem/$(BIN_PATH)/filesystem $(COMMONS) $(THREAD)

master:
	mkdir -p Master/$(BIN_PATH)
	$(COMPILER) Master/$(SRC_PATH)/master.c -o Master/$(BIN_PATH)/master $(COMMONS) $(THREAD)
	
yama:
	mkdir -p Yama/$(BIN_PATH)
	$(COMPILER) Yama/$(SRC_PATH)/yama.c -o Yama/$(BIN_PATH)/yama $(COMMONS) $(THREAD)


clean: 
	rm -rf FileSystem/$(BIN_PATH)
	rm -rf Master/$(BIN_PATH)
	rm -rf Yama/$(BIN_PATH)
