/*
 ============================================================================
 Name        : yama.c
 Author      : Grupo 1234
 Description : Proceso Yama
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
	datosConfigServer datosConexionYama;
	datosConfigClient datosConexionFileSystem;
	int preparadoEnviarFs = 0;
	char *pathArchivoConfig = "../src/config.txt";
	char message[PACKAGESIZE];	//TODO: definir un protocolo de mensajes para evitar el tamaño fijo de mensajes
	int preparadoRecibir;

	printf("\n*** Proceso Yama ***");

	if (!configFileH(pathArchivoConfig, &datosConfig)) {
		printf("Hubo un error al leer el archivo de configuración");
		return EXIT_FAILURE;
	}

	// Conexión a FileSystem
	datosConexionFileSystem.ip = datosConfig.IP_FS;
	datosConexionFileSystem.puerto = datosConfig.PUERTO_FS;
	if (!initializeClient(&datosConexionFileSystem)) {
		//preparadoEnviarFs = handshakeClient(&datosConexionFileSystem, NUM_PROCESO_KERNEL);
		preparadoEnviarFs=1;
	}


	datosConexionYama.puerto = datosConfig.PUERTO;
	if(initializeServer(&datosConexionYama)<0){
		puts("No pude iniciar como servidor");
		return EXIT_FAILURE;
	}
	puts("Ya estoy preparado para recibir conexiones\n");
	if(aceptarConexion(&datosConexionYama)<0){
		puts("Hubo un error al aceptar conexiones\n");
		return EXIT_FAILURE;
	}
	puts("Ya me conecté, ahora estoy esperando mensajes\n");
	//preparadoRecibir=handshakeServer(&datosConexionYama,NUM_PROCESO_FS);
	preparadoRecibir=1;
	//while (preparadoRecibir) {
		preparadoRecibir = recv(datosConexionYama.socketCliente,(void*) message, PACKAGESIZE, 0);
		puts("Impresión por pantalla del contenido del archivo recibido");
		puts("/* **************************************** */");
		if (preparadoRecibir) {
			//imprime por pantalla el mensaje recibido
			printf("Mensaje entrante: %s\n", message);
		}
	//}

	if(preparadoEnviarFs) {
		// Envia el mensaje a la FileSystem
		send(datosConexionFileSystem.serverSocket, message,strlen(message) + 1, 0);
	}



	cerrarServer(&datosConexionYama);

	printf("\n");
	return EXIT_SUCCESS;
}
