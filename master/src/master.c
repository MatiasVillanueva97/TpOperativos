/*
 ============================================================================
 Name        : master.c
 Author      : Grupo 1234
 Description : Proceso Master
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <commons/config.h>
#include <commons/string.h>
#include "../../utils/conexionesSocket.h"
#include "../../utils/archivoConfig.h"

#define PACKAGESIZE 1024	// Define cual va a ser el size maximo del paquete a enviar

/*enum keys {YAMA_IP, YAMA_PUERTO, WORKER_IP, WORKER_PUERTO};
char* keysConfigMaster[]={"YAMA_IP", "YAMA_PUERTO", "WORKER_IP", "WORKER_PUERTO", NULL};
char* datosConfigMaster[4];*/

int main(int argc, char *argv[]) {
	struct datosConfigMaster datosConfigTxt;
	char message[PACKAGESIZE];
	printf("\n*** Proceso Master ***");

	char *nameArchivoConfig = "configMaster.txt";

	// 1º) leer archivo de config.
	if (leerArchivoConfigMaster(nameArchivoConfig, &datosConfigTxt)) {	//leerArchivoConfig devuelve 1 si hay error
			printf("Hubo un error al leer el archivo de configuración\n");
			return EXIT_FAILURE;
		}

    // 2º) conectarse a YAMA y aguardar instrucciones
    int serverSocket=conectarA(datosConfigTxt.YAMA_IP, datosConfigTxt.YAMA_PUERTO);
    int enviar = 1;
    while(enviar){
    		fgets(message, PACKAGESIZE, stdin);			// Lee una linea en el stdin (lo que escribimos en la consola) hasta encontrar un \n (y lo incluye) o llegar a PACKAGESIZE.
    		if (!strcmp(message,"exit\n")) enviar = 0;			// Chequeo que el usuario no quiera salir
    		if (enviar) send(serverSocket, message, strlen(message) + 1, 0); 	// Solo envio si el usuario no quiere salir.
    	}
    // 3º) conectarse a un worker y pasarle instrucciones (pasar a HILOS!)
    //int socketWorker = inicializarClient(datosConfigTxt.WORKER_IP, datosConfigTxt.WORKER_PUERTO);
    //conectarA(socketWorker);

    // Etapa de Transformación: crear hilo, conectarse al worker, esperar y notificar a YAMA
    // Etapa de Reducción Local: crear hilo, conectarse al worker, esperar y notificar a YAMA
    // Etapa de Reducción Global: crear hilo, conectarse al worker, esperar y notificar a YAMA

	return EXIT_SUCCESS;
}
