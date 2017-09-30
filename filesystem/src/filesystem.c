/*
 ============================================================================
 Name        : filesystem.c
 Author      : Grupo 1234
 Description : Proceso FileSystem
 ============================================================================
 */

#include "filesystem.h"

enum keys {IP_PROPIA,PUERTO_PROPIO};
char* keysConfigFS[]={"IP_PROPIA", "PUERTO_PROPIO", NULL};
char* datosConfigFS[2];

// ================================================================ //
// FileSystem sabe qué está guardado y dónde.
// Recibe conexiones de DataNodes hasta alcanzar "Estado Estable".
// Se conecta a YAMA.
// Sólo hay un FileSystem corriendo al mismo tiempo.
// ================================================================ //

int main(int argc, char *argv[]) {
    t_log* logFileSystem;
    logFileSystem = log_create("logFile.log", "FILESYSTEM", false, LOG_LEVEL_TRACE); //creo el logger, sin mostrar por pantalla

    log_info(logFileSystem,"Iniciando FileSystem");
   	printf("\n*** Proceso FileSystem ***\n");

	// 1º) leer archivo de config.
   	char *nameArchivoConfig = "configFilesystem.txt";
   	if (leerArchivoConfig(nameArchivoConfig, keysConfigFS, datosConfigFS)) {	//leerArchivoConfig devuelve 1 si hay error
   			printf("Hubo un error al leer el archivo de configuración");
   			return 0;
   		}

	// 2º) inicializar server y aguardar conexiones (de datanode)
   	int listenningSocket=inicializarServer(datosConfigFS[IP_PROPIA],datosConfigFS[PUERTO_PROPIO]);
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


//	consola();
//	while (consola()!=1){
//		consola();
//	}


    // 3º) si estadoEstable -> FORMATEAR
    //int socketWorker = inicializarClient(datosConfigMaster.WORKER_IP, datosConfigMaster->WORKER_PUERTO);
    //conectarseA(socketWorker);

    // Etapa de Transformación: crear hilo, conectarse al worker, esperar y notificar a YAMA
    // Etapa de Reducción Local: crear hilo, conectarse al worker, esperar y notificar a YAMA
    // Etapa de Reducción Global: crear hilo, conectarse al worker, esperar y notificar a YAMA

	/* *************************** espera recepción de un mensaje ****************************/
	/* ********* espera el header ********* */
	struct headerProtocolo header = recibirHeader(socketCliente);
	char *mensaje=recibirMensaje(socketCliente,header.tamPayload);

	puts("Impresión por pantalla del contenido del archivo recibido");
	puts("/* **************************************** */");
	printf("tamaño del mensaje: %d\n",header.tamPayload);
	printf("mensaje: %s\n",mensaje);


	cerrarServer(listenningSocket);
//	if (recv(socketCliente,(void*) message, PACKAGESIZE, 0)) {
//		puts("Impresión por pantalla del contenido del archivo recibido");
//		puts("/* **************************************** */");
//		//imprime por pantalla el mensaje recibido
//		printf("Mensaje entrante: %s\n", message);
//	}
//
//	cerrarServer(listenningSocket);
	log_info(logFileSystem,"Server cerrado");

	log_destroy(logFileSystem);
	return EXIT_SUCCESS;
}
