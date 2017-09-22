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
#include <commons/log.h>
#include "../../utils/conexionesSocket.h"
#include "../../utils/archivoConfig.h"

#define PACKAGESIZE 1024	// Define cual va a ser el size maximo del paquete a enviar

enum keys {IP_PROPIA,PUERTO_PROPIO, FS_IP,FS_PUERTO};
char* keysConfigYama[]={"IP_PROPIA", "PUERTO_PROPIO","FS_IP","FS_PUERTO", NULL};
char* datosConfigYama[4];

int main(int argc, char *argv[]) {
    t_log* logYAMA;
    logYAMA = log_create("logYAMA.log", "YAMA", false, LOG_LEVEL_TRACE); //creo el logger, sin mostrar por pantalla
	int preparadoEnviarFs = 1;
	char message[PACKAGESIZE];	//TODO: definir un protocolo de mensajes para evitar el tamaño fijo de mensajes
	int preparadoRecibir=0;

	log_info(logYAMA,"Iniciando proceso YAMA");
	printf("\n*** Proceso Yama ***\n");

	char *nameArchivoConfig = "configYama.txt";
	// 1º) leer archivo de config.
	if (leerArchivoConfig(nameArchivoConfig, keysConfigYama, datosConfigYama)) {	//leerArchivoConfig devuelve 1 si hay error
		printf("Hubo un error al leer el archivo de configuración");
		return 0;
	}

	/* ************** conexión como cliente al FS *************** */
	log_info(logYAMA,"Conexión a FileSystem, IP: %s, Puerto: %s",datosConfigYama[FS_IP],datosConfigYama[FS_PUERTO]);
	int socketFS = conectarA(datosConfigYama[FS_IP],datosConfigYama[FS_PUERTO]);
	if (!socketFS) {
		//preparadoEnviarFs = handshakeClient(&datosConexionFileSystem, NUM_PROCESO_KERNEL);
		preparadoEnviarFs=0;
	}

	/* ************** inicialización como server ************ */
	int listenningSocket=inicializarServer(datosConfigYama[IP_PROPIA],datosConfigYama[PUERTO_PROPIO]);
	if(listenningSocket<0){
		log_error(logYAMA,"No pude iniciar como servidor");
		puts("No pude iniciar como servidor");
		return EXIT_FAILURE;
	}
	puts("Ya estoy preparado para recibir conexiones\n");

	int socketCliente=aceptarConexion(listenningSocket);
	if(socketCliente<0){
		log_error(logYAMA,"Hubo un error al aceptar conexiones");
		puts("Hubo un error al aceptar conexiones\n");
		return EXIT_FAILURE;
	}
	log_info(logYAMA,"FileSystem conectado, esperando conexiones");
	puts("Ya me conecté, ahora estoy esperando mensajes\n");


	preparadoRecibir=1;
	//while (preparadoRecibir) {
		preparadoRecibir = recv(socketCliente,(void*) message, PACKAGESIZE, 0);
		puts("Impresión por pantalla del contenido del archivo recibido");
		//puts("/* **************************************** */");
		if (preparadoRecibir) {
			//imprime por pantalla el mensaje recibido
			printf("Mensaje entrante: %s\n", message);
		}
	//}

	if(preparadoEnviarFs) {
		// Envia el mensaje a la FileSystem
		send(socketFS, message,strlen(message) + 1, 0);
	}

	cerrarServer(listenningSocket);
	log_info(logYAMA,"Server cerrado");

	printf("\n");
	log_destroy(logYAMA);
	return EXIT_SUCCESS;
}
