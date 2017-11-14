COMPILER=gcc
BIN_PATH=bin
SRC_PATH=src
COMMONS=-lcommons
THREAD=-lpthread
READLINE=-lreadline
MATH=-lm
DIR=$(pwd)

FILESYSTEM=filesystem
MASTER=master
YAMA=yama
WORKER=worker
DATANODE=datanode

default: all

all: filesystem worker datanode yama master
.PHONY: filesystem master yama worker datanode

filesystem:
	mkdir -p $(FILESYSTEM)/$(BIN_PATH)
	$(COMPILER) $(FILESYSTEM)/$(SRC_PATH)/$(FILESYSTEM).c -o $(FILESYSTEM)/$(BIN_PATH)/$(FILESYSTEM) $(COMMONS) $(THREAD) $(READLINE) $(MATH)

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
	rm -rf $(FILESYSTEM)/$(BIN_PATH) && rm -rf $(FILESYSTEM)/*.log
	rm -rf $(MASTER)/$(BIN_PATH) && rm -rf $(MASTER)/*.log
	rm -rf $(YAMA)/$(BIN_PATH) && rm -rf $(YAMA)/*.log
	rm -rf $(WORKER)/$(BIN_PATH) && rm -rf $(WORKER)/*.log
	rm -rf $(DATANODE)/$(BIN_PATH) && rm -rf $(DATANODE)/*.log

run:
	terminator --working-directory="$(DIR)" --title "FileSystem" --command="cd filesystem/bin/; ./filesystem; bash" --geometry=640x400+0+0 &
	sleep 1
	terminator --working-directory="$(DIR)" --title "Worker" --command="cd worker/bin/; ./worker; bash" --geometry=640x400-0+0 &
	sleep 1
	terminator --working-directory="$(DIR)" --title "YAMA" --command="cd yama/bin/; ./yama; bash" --geometry=640x400+0-0 &
	sleep 1
	terminator --working-directory="$(DIR)" --title "Master" --command="cd master/bin/; ./master ../../scripts/transformador.sh ../../scripts/reductor.rb yamafs:/datos.csv yamafs:/analisis/resultado.json; bash" --geometry=640x400-0-0 &
