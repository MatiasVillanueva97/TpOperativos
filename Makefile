COMPILER=gcc
BIN_PATH=bin
SRC_PATH=src
COMMONS=-lcommons
THREAD=-lpthread
READLINE=-lreadline

FILESYSTEM=filesystem
MASTER=master
YAMA=yama
WORKER=worker
DATANODE=datanode

default: all

all: filesystem master yama worker datanode
.PHONY: filesystem master yama worker datanode

filesystem:
	mkdir -p $(FILESYSTEM)/$(BIN_PATH)
	$(COMPILER) $(FILESYSTEM)/$(SRC_PATH)/$(FILESYSTEM).c -o $(FILESYSTEM)/$(BIN_PATH)/$(FILESYSTEM) $(COMMONS) $(THREAD) $(READLINE)

master:
	mkdir -p $(MASTER)/$(BIN_PATH)
	$(COMPILER) $(MASTER)/$(SRC_PATH)/$(MASTER).c -o $(MASTER)/$(BIN_PATH)/$(MASTER) $(COMMONS) $(THREAD) $(READLINE)

yama:
	mkdir -p $(YAMA)/$(BIN_PATH)
	$(COMPILER) $(YAMA)/$(SRC_PATH)/$(YAMA).c -o $(YAMA)/$(BIN_PATH)/$(YAMA) $(COMMONS) $(THREAD) $(READLINE)

worker:
	mkdir -p $(WORKER)/$(BIN_PATH)
	$(COMPILER) $(WORKER)/$(SRC_PATH)/$(WORKER).c -o $(WORKER)/$(BIN_PATH)/$(WORKER) $(COMMONS) $(THREAD) $(READLINE)

datanode:
	mkdir -p $(DATANODE)/$(BIN_PATH)
	$(COMPILER) $(DATANODE)/$(SRC_PATH)/$(DATANODE).c -o $(DATANODE)/$(BIN_PATH)/$(DATANODE) $(COMMONS) $(THREAD) $(READLINE)

clean:
	rm -rf $(FILESYSTEM)/$(BIN_PATH)
	rm -rf $(MASTER)/$(BIN_PATH)
	rm -rf $(YAMA)/$(BIN_PATH)
	rm -rf $(WORKER)/$(BIN_PATH)
	rm -rf $(DATANODE)/$(BIN_PATH)
