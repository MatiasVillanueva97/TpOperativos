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

#define PACKAGESIZE 1024	// Define cual va a ser el size maximo del paquete a enviar

int main(int argc, char *argv[]) {
    t_log* logFileSystem;
    logFileSystem = log_create("logFile.log", "FILESYSTEM", false, LOG_LEVEL_TRACE); //creo el logger, sin mostrar por pantalla

    log_info(logFileSystem,"Iniciando FileSystem");
   	printf("\n*** Proceso FileSystem ***");

	// 1º) leer archivo de config.
	char *pathArchivoConfig = "../src/config.txt";
    //struct datosConfig datosConfigFilesystem;
	//int archivoConfigOK = leerArchivoConfig(pathArchivoConfig, &datosConfigMaster);

//    if (!archivoConfigOK) {
//		printf("Hubo un error al leer el archivo de configuración");
//		return 0;
//	}

    // 2º) inicializar server y aguardar conexiones
   // int socketYama = inicializarClient(datosConfigMaster.YAMA_IP, datosConfigMaster->YAMA_PUERTO);
    //conectarseA(socketYama);

    // 3º) si estadoEstable -> FORMATEAR
   // int socketWorker = inicializarClient(datosConfigMaster.WORKER_IP, datosConfigMaster->WORKER_PUERTO);
    //conectarseA(socketWorker);

    // Etapa de Transformación: crear hilo, conectarse al worker, esperar y notificar a YAMA
    // Etapa de Reducción Local: crear hilo, conectarse al worker, esperar y notificar a YAMA
    // Etapa de Reducción Global: crear hilo, conectarse al worker, esperar y notificar a YAMA


	datosConfigServer datosConexionFileSystem;

	datosConexionFileSystem.puerto = datosConfig.PUERTO;
	int inicializarServerOK = (&datosConexionFileSystem)
	if inicializarServerOK <0){
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
		preparadoRecibir = recv(datosConexionFileSystem.socketCliente,(void*) message, PACKAGESIZE, 0);
		puts("Impresión por pantalla del contenido del archivo recibido");
		puts("/* **************************************** */");
		if (preparadoRecibir) {
			//imprime por pantalla el mensaje recibido
			printf("Mensaje entrante: %s\n", message);
		}

	cerrarServer(&datosConexionFileSystem);
	log_info(logFileSystem,"Server cerrado");

	log_destroy(logFileSystem);
	return EXIT_SUCCESS;
}
