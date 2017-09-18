/*
 ============================================================================
 Name        : worker.c
 Author      : Grupo 1234
 Description : Proceso Worker
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
//#include <filesystem.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/log.h>
#include "../../utils/conexionesSocket.h"
#include "../../utils/archivoConfig.h"

//#define PACKAGESIZE 1024	// Define cual va a ser el size maximo del paquete a enviar

enum keys {IP_PROPIA,PUERTO_PROPIO};
char* keysConfigMaster[]={"IP_PROPIA", "PUERTO_PROPIO", NULL};
char* datosConfigMaster[2];

int main(int argc, char *argv[]) {
	t_log* logWorker;
	logWorker = log_create("logFile.log", "WORKER", false, LOG_LEVEL_TRACE); //creo el logger, sin mostrar por pantalla
	datosConfigServer datosConexionComoServer;

	log_info(logWorker,"Iniciando Worker");
	printf("\n*** Proceso worker ***");

	char *nameArchivoConfig = "configWorker.txt";
	// 1º) leer archivo de config.
	int archivoConfigOK = leerArchivoConfig(nameArchivoConfig, keysConfigMaster, datosConfigMaster);
	if (!archivoConfigOK) {
		printf("Hubo un error al leer el archivo de configuración");
		return 0;
	}

	int listenningSocket=inicializarServer(IP_PROPIA, PUERTO_PROPIO);
	if(listenningSocket<0){
		log_error(logWorker,"No pude iniciar como servidor");
		puts("No pude iniciar como servidor");
		return EXIT_FAILURE;
	}
	puts("Ya estoy preparado para recibir conexiones\n");

	int socketCliente=aceptarConexion(listenningSocket);
	if(socketCliente<0){
		log_error(logWorker,"Hubo un error al aceptar conexiones");
		puts("Hubo un error al aceptar conexiones\n");
		return EXIT_FAILURE;
	}

	log_info(logWorker,"Worker conectado, esperando conexiones");
	puts("Ya me conecté, ahora estoy esperando mensajes\n");

	return EXIT_SUCCESS;
}
