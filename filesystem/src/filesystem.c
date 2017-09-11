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
#include "./configuracion.h"
#include "../../headers/configServer.h"
#include "../../headers/configClient.h"
//#include "../../headers/handshake.h"
#include "../../headers/constantes.h"

#define PACKAGESIZE 1024	// Define cual va a ser el size maximo del paquete a enviar

int main(int argc, char *argv[]) {
	datosConfig datosConfig;
	datosConfigServer datosConexionFileSystem;
	char *pathArchivoConfig = "../src/config.txt";
	char message[PACKAGESIZE];	//TODO: definir un protocolo de mensajes para evitar el tamaño fijo de mensajes
	int preparadoRecibir;

	printf("\n*** Proceso FileSystem ***");

	if (!configFileH(pathArchivoConfig, &datosConfig)) {
		printf("Hubo un error al leer el archivo de configuración");
		return EXIT_FAILURE;
	}

	datosConexionFileSystem.puerto = datosConfig.PUERTO;
	if(!initializeServer(&datosConexionFileSystem)){
		puts("No pude iniciar como servidor");
		return EXIT_FAILURE;
	}
	puts("Ya estoy preparado para recibir conexiones\n");
	if(!aceptarConexion(&datosConexionFileSystem)){
		puts("Hubo un error al aceptar conexiones\n");
		return EXIT_FAILURE;
	}
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

	printf("\n");
	return EXIT_SUCCESS;
}
