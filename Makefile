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

default: error

.PHONY: filesystem master yama worker datanode error

filesystem:
	@echo "Limpiando FILESYSTEM..."
	rm -rf $(FILESYSTEM)/$(BIN_PATH) && rm -rf $(FILESYSTEM)/*.log
	@echo "Compilando FILESYSTEM..."
	mkdir -p $(FILESYSTEM)/$(BIN_PATH) && $(COMPILER) $(FILESYSTEM)/$(SRC_PATH)/$(FILESYSTEM).c -o $(FILESYSTEM)/$(BIN_PATH)/$(FILESYSTEM) $(COMMONS) $(THREAD) $(READLINE) $(MATH)
	@echo "Ejecutando FILESYSTEM..."
	cd $(FILESYSTEM)/$(BIN_PATH) && ./$(FILESYSTEM)

datanode:
	@echo "Limpiando DATANODE..."
	rm -rf $(DATANODE)/$(BIN_PATH) && rm -rf $(DATANODE)/*.log
	@echo "Compilando DATANODE..."
	mkdir -p $(DATANODE)/$(BIN_PATH) && $(COMPILER) $(DATANODE)/$(SRC_PATH)/$(DATANODE).c -o $(DATANODE)/$(BIN_PATH)/$(DATANODE) $(COMMONS) $(THREAD) $(READLINE)
	@echo "Ejecutando DATANODE..."
	cd $(DATANODE)/$(BIN_PATH) && ./$(DATANODE)

yama:
	@echo "Limpiando YAMA..."
	rm -rf $(YAMA)/$(BIN_PATH) && rm -rf $(YAMA)/*.log
	@echo "Compilando YAMA..."
	mkdir -p $(YAMA)/$(BIN_PATH) && $(COMPILER) $(YAMA)/$(SRC_PATH)/$(YAMA).c -o $(YAMA)/$(BIN_PATH)/$(YAMA) $(COMMONS) $(THREAD) $(READLINE)
	@echo "Ejecutando YAMA..."
	cd $(YAMA)/$(BIN_PATH) && ./$(YAMA)
      
worker: 
	@echo "Limpiando WORKER..."
	rm -rf $(WORKER)/$(BIN_PATH) && rm -rf $(WORKER)/*.log
	@echo "Compilando WORKER..."
	mkdir -p $(WORKER)/$(BIN_PATH) && $(COMPILER) $(WORKER)/$(SRC_PATH)/$(WORKER).c -o $(WORKER)/$(BIN_PATH)/$(WORKER) $(COMMONS) $(THREAD) $(READLINE)
	@echo "Ejecutando WORKER..."	
	cd $(WORKER)/$(BIN_PATH) && ./$(WORKER) 

master:
	@echo "Limpiando MASTER..." 
	rm -rf $(MASTER)/$(BIN_PATH) && rm -rf $(MASTER)/*.log
	mkdir -p $(MASTER)/$(BIN_PATH)
	@echo "Compilando MASTER..."
	$(COMPILER) $(MASTER)/$(SRC_PATH)/$(MASTER).c -o $(MASTER)/$(BIN_PATH)/$(MASTER) $(COMMONS) $(THREAD) $(READLINE)
	@echo "Ejecutando MASTER..."
	cd $(MASTER)/$(BIN_PATH) && ./$(MASTER) ../../scripts/transformador.py ../../scripts/reductor.py yamafs:/nombres.csv yamafs:/analisis/resultado.json

error: 
	@echo "[ERROR] Pasar como argumento el proceso: make filesystem/datanode/yama/worker/master"
