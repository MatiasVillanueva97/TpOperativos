/*
 ============================================================================
 Name        : filesystem.c
 Author      : Grupo 1234
 Description : Proceso FileSystem
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/log.h>
#include "../../utils/conexionesSocket.h"
#include "../../utils/archivoConfig.h"

#define PACKAGESIZE 1024	// Define cual va a ser el size maximo del paquete a enviar

enum keys {IP_PROPIA,PUERTO_PROPIO};
char* keysConfigFS[]={"IP_PROPIA", "PUERTO_PROPIO", NULL};
char* datosConfigFS[2];

int main(int argc, char *argv[]) {
	char message[PACKAGESIZE];
    t_log* logFileSystem;
    logFileSystem = log_create("logFile.log", "FILESYSTEM", false, LOG_LEVEL_TRACE); //creo el logger, sin mostrar por pantalla

    log_info(logFileSystem,"Iniciando FileSystem");
   	printf("\n*** Proceso FileSystem ***");

   	char *nameArchivoConfig = "configFilesystem.txt";

	// 1º) leer archivo de config.
	if (leerArchivoConfig(nameArchivoConfig, keysConfigFS, datosConfigFS)) {	//leerArchivoConfig devuelve 1 si hay error
		printf("Hubo un error al leer el archivo de configuración\n");
		return EXIT_FAILURE;
	}

	// 2º) inicializar server y aguardar conexiones
	int listenningSocket=inicializarServer((char*) IP_PROPIA, (char*) PUERTO_PROPIO);
	if(listenningSocket<0){
		log_error(logFileSystem,"No pude iniciar como servidor");
		puts("No pude iniciar como servidor");
		return EXIT_FAILURE;
	}
	puts("Ya estoy preparado para recibir conexiones\n");

	int socketCliente=aceptarConexion(listenningSocket);
	if(socketCliente<0){
		log_error(logFileSystem,"Hubo un error al aceptar conexiones");
		puts("Hubo un error al aceptar conexiones\n");
		return EXIT_FAILURE;
	}
	log_info(logFileSystem,"FileSystem conectado, esperando conexiones");
	puts("Ya me conecté, ahora estoy esperando mensajes\n");

    // 3º) si estadoEstable -> FORMATEAR
   // int socketWorker = inicializarClient(datosConfigMaster.WORKER_IP, datosConfigMaster->WORKER_PUERTO);
    //conectarseA(socketWorker);

    // Etapa de Transformación: crear hilo, conectarse al worker, esperar y notificar a YAMA
    // Etapa de Reducción Local: crear hilo, conectarse al worker, esperar y notificar a YAMA
    // Etapa de Reducción Global: crear hilo, conectarse al worker, esperar y notificar a YAMA





	if (recv(socketCliente,(void*) message, PACKAGESIZE, 0)) {
		puts("Impresión por pantalla del contenido del archivo recibido");
		puts("/* **************************************** */");
		//imprime por pantalla el mensaje recibido
		printf("Mensaje entrante: %s\n", message);
	}

	cerrarServer(listenningSocket);
	log_info(logFileSystem,"Server cerrado");

	log_destroy(logFileSystem);
	return EXIT_SUCCESS;
}
