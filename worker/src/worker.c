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
#include <commons/config.h>
#include <commons/string.h>
#include <commons/log.h>
#include "../../utils/conexionesSocket.h"
#include "../../utils/archivoConfig.h"

//#define PACKAGESIZE 1024	// Define cual va a ser el size maximo del paquete a enviar

enum keys {IP_PROPIA,PUERTO_PROPIO};
char* keysConfigWorker[]={"IP_PROPIA", "PUERTO_PROPIO", NULL};
char* datosConfigWorker[2];

// ================================================================ //
// Worker es el que realiza las operaciones que le pide el Master.
// 1) Recibe orden del Master
// 2) Se forkea
// 3) El principal sigue escuchando, el fork ejecuta la orden
// 4) Termina la orden y el fork muere
// Puede haber varios Worker corriendo al mismo tiempo.
// ================================================================ //

int main(int argc, char *argv[]) {
	t_log* logWorker;
	logWorker = log_create("logFile.log", "WORKER", false, LOG_LEVEL_TRACE); //creo el logger, sin mostrar por pantalla

	log_info(logWorker, "Iniciando Worker");
	printf("\n*** Proceso worker ***\n");

	// 1º) leer archivo de config.
	char *nameArchivoConfig = "configWorker.txt";
	if (leerArchivoConfig(nameArchivoConfig, keysConfigWorker, datosConfigWorker)) {	//leerArchivoConfig devuelve 1 si hay error
		printf("Hubo un error al leer el archivo de configuración");
		return 0;
	}

	// 2º) inicializar server y aguardar conexiones (de master)
   	int listenningSocket = inicializarServer(datosConfigWorker[IP_PROPIA], datosConfigWorker[PUERTO_PROPIO]);
	if(listenningSocket < 0){
		log_error(logWorker, "No pude iniciar como servidor");
		puts("No pude iniciar como servidor");
		return EXIT_FAILURE;
	}
	while(1){//inicio bucle para forkear
		puts("Ya estoy preparado para recibir conexiones\n");

		int socketCliente = aceptarConexion(listenningSocket);
		if(socketCliente < 0){
			log_error(logWorker, "Hubo un error al aceptar conexiones");
			puts("Hubo un error al aceptar conexiones\n");
			return EXIT_FAILURE;
		}

		log_info(logWorker, "Worker conectado, esperando conexiones");
		puts("Ya me conecté, ahora estoy esperando mensajes\n");

		// 3º) abro forks para ejecutar instrucciones de master
		int pid = fork();
		if(pid == -1){
			log_error(logWorker, "Error al forkear");
			return EXIT_FAILURE;
		} else if(pid == 0){
			log_info(logWorker, "Hijo creado");
			printf("Soy el hijo y mi PID es %d\n", getpid());
			// Acá debería ejecutar lo q me pidió el Master
		} else {
			log_info(logWorker, "Hijo creado");
			printf("Soy el padre y mi PID sigue siendo %d\n",getpid());
			printf("El PID de mi hijo es %d\n", pid);
			// Acá debería seguir escuchando conexiones
			close(socketCliente);//cierro el socket en el padre para poder conectarlo con otro master
		}
	}
	return EXIT_SUCCESS;
}
