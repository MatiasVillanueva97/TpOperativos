/*
 ============================================================================
 Name        : datanode.c
 Author      : Grupo 1234
 Description : Proceso DataNode
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <utils/utils.h>
#include <commons/config.h>
#include <commons/string.h>

#define PACKAGESIZE 1024	// Define cual va a ser el size maximo del paquete a enviar

struct datosConfig {
	char *YAMA_IP;
	int YAMA_PUERTO;
	char *WORKER_IP;
	int WORKER_PUERTO;
};

int main(int argc, char *argv[]) {

	// 1º leer archivo de config.
	//int archivoConfigOK = leerArchivoConfig(pathArchivoConfig, &datosConfigDatanode);

    //if (!archivoConfigOK) {
	//	printf("Hubo un error al leer el archivo de configuración");
	//	return 0;
	}

    // 2º) conectarse a FileSystem
   // int socketFilesystem = inicializarClient(datosConfigDatanode.YAMA_IP, datosConfigDatanode->YAMA_PUERTO);
    conectarseA(socketFilesystem);

	return 0;
}
