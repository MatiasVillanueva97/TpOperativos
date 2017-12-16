COMPILER=gcc
BIN_PATH=bin
SRC_PATH=src
LOG_PATH=log
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

export LC_ALL=C

.PHONY: all help
.PHONY: clean-filesystem clean-datanode clean-yama clean-worker clean-master
.PHONY: compile-filesystem compile-datanode compile-yama compile-worker compile-master
.PHONY: run-filesystem run-datanode run-yama run-worker run-master run-master-arg
.PHONY: debug-filesystem debug-datanode debug-yama debug-worker debug-master

test: 
	/bin/bash

# FILESYSTEM #
clean-filesystem:
	@echo "Limpiando FILESYSTEM..."
	@rm -rf $(FILESYSTEM)/$(BIN_PATH) && rm -rf $(FILESYSTEM)/${LOG_PATH}/*.log

compile-filesystem: clean-filesystem
	@echo "Compilando FILESYSTEM..."
	@mkdir -p $(FILESYSTEM)/$(BIN_PATH) && $(COMPILER) $(FILESYSTEM)/$(SRC_PATH)/$(FILESYSTEM).c -g -o $(FILESYSTEM)/$(BIN_PATH)/$(FILESYSTEM) $(COMMONS) $(THREAD) $(READLINE) $(MATH)

run-filesystem: compile-filesystem
	@echo "Ejecutando FILESYSTEM..."
	@clear
	@cd $(FILESYSTEM)/$(BIN_PATH) && ./$(FILESYSTEM) ${ARG}

debug-filesystem: compile-filesystem
	@echo "Ejecutando FILESYSTEM en modo DEBUG..."
	cd $(FILESYSTEM)/$(BIN_PATH) && valgrind --leak-check=full ./$(FILESYSTEM)

# DATANODE #
clean-datanode:
	@echo "Limpiando DATANODE..."
	@rm -rf $(DATANODE)/$(BIN_PATH) && rm -rf $(DATANODE)/*.log

compile-datanode: clean-datanode
	@echo "Compilando DATANODE..."
	@mkdir -p $(DATANODE)/$(BIN_PATH) && $(COMPILER) $(DATANODE)/$(SRC_PATH)/$(DATANODE).c -g -o $(DATANODE)/$(BIN_PATH)/$(DATANODE) $(COMMONS) $(THREAD) $(READLINE)

run-datanode: compile-datanode
	@echo "Ejecutando DATANODE..."
	@clear
	@cd $(DATANODE)/$(BIN_PATH) && ./$(DATANODE)

debug-datanode: compile-datanode
	@echo "Ejecutando DATANODE en modo DEBUG..."
	@cd $(DATANODE)/$(BIN_PATH) && valgrind --leak-check=full ./$(DATANODE)

# YAMA #
clean-yama:
	@echo "Limpiando YAMA..."
	@rm -rf $(YAMA)/$(BIN_PATH) && rm -rf $(YAMA)/*.log

compile-yama: clean-yama
	@echo "Compilando YAMA..."
	@mkdir -p $(YAMA)/$(BIN_PATH) && $(COMPILER) $(YAMA)/$(SRC_PATH)/$(YAMA).c -g -o $(YAMA)/$(BIN_PATH)/$(YAMA) $(COMMONS) $(THREAD) $(READLINE)

run-yama: compile-yama
	@echo "Ejecutando YAMA..."
	@clear
	@cd $(YAMA)/$(BIN_PATH) && ./$(YAMA)

debug-yama: compile-yama
	@echo "Ejecutando YAMA en modo DEBUG..."
	@cd $(YAMA)/$(BIN_PATH) && valgrind --leak-check=full ./$(YAMA)

# WORKER #
clean-worker:
	@echo "Limpiando WORKER..."
	@rm -rf $(WORKER)/$(BIN_PATH) && rm -rf $(WORKER)/*.log

compile-worker: clean-worker
	@echo "Compilando WORKER..."
	@mkdir -p $(WORKER)/$(BIN_PATH) && $(COMPILER) $(WORKER)/$(SRC_PATH)/$(WORKER).c -g -o $(WORKER)/$(BIN_PATH)/$(WORKER) $(COMMONS) $(THREAD) $(READLINE)

run-worker: compile-worker
	@echo "Ejecutando WORKER..."
	@clear
	@cd $(WORKER)/$(BIN_PATH) && ./$(WORKER)

debug-worker: compile-worker
	@echo "Ejecutando WORKER en modo DEBUG..."
	@cd $(WORKER)/$(BIN_PATH) && valgrind --leak-check=full ./$(WORKER)

# MASTER #
clean-master:
	@echo "Limpiando MASTER..."
	@rm -rf $(MASTER)/$(BIN_PATH) && rm -rf $(MASTER)/*.log

compile-master: clean-master
	@echo "Compilando MASTER..."
	@mkdir -p $(MASTER)/$(BIN_PATH) && $(COMPILER) $(MASTER)/$(SRC_PATH)/$(MASTER).c -g -o $(MASTER)/$(BIN_PATH)/$(MASTER) $(COMMONS) $(THREAD) $(READLINE)

run-master: compile-master
	@echo "Ejecutando MASTER..."
	@cd $(MASTER)/$(BIN_PATH) &&  ./$(MASTER) ../../scripts/transformador.py ../../scripts/reductor.py yamafs:/nombres.csv yamafs:/analisis/resultado.json

run-master-arg: compile-master
	@echo "Ejecutando MASTER..."
	@clear
	@cd $(MASTER)/$(BIN_PATH) &&  ./$(MASTER) ${ARG}

debug-master: compile-master
	@echo "Ejecutando MASTER en modo DEBUG..."
	@cd $(MASTER)/$(BIN_PATH) &&  valgrind --leak-check=full ./$(MASTER) ../../scripts/transformador.py ../../scripts/reductor.py yamafs:/nombres.csv yamafs:/analisis/resultado.json

all: compile-filesystem compile-datanode compile-worker compile-yama compile-master

help:
	@echo ""
	@echo "*** TP 2017 2ºC - Mi Grupo 1234 - UTN FRBA ***"
	@echo ""
	@echo "Comandos disponibles:"
	@echo "- make clean-(PROCESO): borra el PROCESO. Ej.: make clean-filesystem"
	@echo "- make compile-(PROCESO): borra y compila el PROCESO. Ej.: make compile-master"
	@echo "- make run-(PROCESO): borra, compila y ejecuta el PROCESO. Ej.: make run-worker"
	@echo "- make debug-(PROCESO): borra, compila y ejecuta el PROCESO usando Valgrind. Ej.: make debug-worker"
	@echo ""
	@echo "- make all: borra y compila todos los PROCESOS. Es el default para el Build ("martillito") de Eclipse."
	@echo "(PROCESO) = filesystem / datanode / yama / worker / master"
	@echo ""
