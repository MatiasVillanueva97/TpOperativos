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
//#include <filesystem.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/log.h>
#include "./configuracion.h"
#include "../../headers/configServer.h"
#include "../../headers/configClient.h"
//#include "../../headers/handshake.h"
#include "../../headers/constantes.h"

#define PACKAGESIZE 1024	// Define cual va a ser el size maximo del paquete a enviar

int main(int argc, char *argv[]) {
    //crearLogger();
    t_log* logFileSystem;
    logFileSystem = log_create("logFile.log", "FILESYSTEM", false, LOG_LEVEL_TRACE); //creo el logger, sin mostrar por pantalla
	datosConfig datosConfig;
	datosConfigServer datosConexionFileSystem;
	char *pathArchivoConfig = "../src/config.txt";
	char message[PACKAGESIZE];	//TODO: definir un protocolo de mensajes para evitar el tamaño fijo de mensajes
	int preparadoRecibir;

	log_info(logFileSystem,"Iniciando FileSystem");
	printf("\n*** Proceso FileSystem ***");

	if (!configFileH(pathArchivoConfig, &datosConfig)) {
	    log_error(logFileSystem,"Error al leer archivo de configuracion");
		printf("Hubo un error al leer el archivo de configuración");
		return EXIT_FAILURE;
	}

	datosConexionFileSystem.puerto = datosConfig.PUERTO;
	if(initializeServer(&datosConexionFileSystem)<0){
	    log_error(logFileSystem,"No pude iniciar como servidor");
		puts("No pude iniciar como servidor");
		return EXIT_FAILURE;
	}
	puts("Ya estoy preparado para recibir conexiones\n");
	if(aceptarConexion(&datosConexionFileSystem)<0){
	    log_error(logFileSystem,"Hubo un error al aceptar conexiones");
		puts("Hubo un error al aceptar conexiones\n");
		return EXIT_FAILURE;
	}
	log_info(logFileSystem,"FileSystem conectado, esperando conexiones");
	puts("Ya me conecté, ahora estoy esperando mensajes\n");
	//preparadoRecibir=handshakeServer(&datosConexionFileSystem,NUM_PROCESO_FS);
	preparadoRecibir=1;
	//while (preparadoRecibir) {
		preparadoRecibir = recv(datosConexionFileSystem.socketCliente,(void*) message, PACKAGESIZE, 0);
		puts("Impresión por pantalla del contenido del archivo recibido");
		puts("/* **************************************** */");
		if (preparadoRecibir) {
			//imprime por pantalla el mensaje recibido
			printf("Mensaje entrante: %s\n", message);
		}

	//}
	cerrarServer(&datosConexionFileSystem);
	log_info(logFileSystem,"Server cerrado");

	printf("\n");

	log_destroy(logFileSystem);
	return EXIT_SUCCESS;
}
