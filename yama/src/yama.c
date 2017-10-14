/*
 ============================================================================
 Name        : yama.c
 Author      : Grupo 1234
 Description : Proceso YAMA
 ============================================================================
 */

#include "../../utils/includes.h"


#define PACKAGESIZE 1024	// Define cual va a ser el size maximo del paquete a enviar

enum keys {IP_PROPIA,PUERTO_PROPIO, FS_IP,FS_PUERTO};
char* keysConfigYama[]={"IP_PROPIA", "PUERTO_PROPIO","FS_IP","FS_PUERTO", NULL};
char* datosConfigYama[4];

// ================================================================ //
// YAMA coordina con Master donde correr los jobs.
// Se conecta a FileSystem. Única instancia.
// Solo hay un YAMA corriendo al mismo tiempo.
// ================================================================ //

int main(int argc, char *argv[]) {
    t_log* logYAMA;
    logYAMA = log_create("logYAMA.log", "YAMA", false, LOG_LEVEL_TRACE); //creo el logger, sin mostrar por pantalla
	int preparadoEnviarFs = 1;

	log_info(logYAMA,"Iniciando proceso YAMA");
	printf("\n*** Proceso Yama ***\n");

	// 1º) leer archivo de config.
	char *nameArchivoConfig = "configYama.txt";
	if (leerArchivoConfig(nameArchivoConfig, keysConfigYama, datosConfigYama)) {	//leerArchivoConfig devuelve 1 si hay error
		printf("Hubo un error al leer el archivo de configuración");
		return 0;
	}

	/* ************** conexión como cliente al FS *************** */
	log_info(logYAMA,"Conexión a FileSystem, IP: %s, Puerto: %s",datosConfigYama[FS_IP],datosConfigYama[FS_PUERTO]);
	int socketFS = conectarA(datosConfigYama[FS_IP],datosConfigYama[FS_PUERTO]);
	if (socketFS<0) {
		puts("Filesystem not ready\n");
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

	int socketCliente=aceptarConexion(listenningSocket);
	if(socketCliente<0){
		log_error(logYAMA,"Hubo un error al aceptar conexiones");
		puts("Hubo un error al aceptar conexiones\n");
		return EXIT_FAILURE;
	}
	log_info(logYAMA,"FileSystem conectado, esperando conexiones");
	puts("Esperando mensajes\n");

	/* *************************** espera recepción de un mensaje ****************************/
	/* ********* espera el header ********* */
	int i;

	char idString[LARGO_STRING_HEADER_ID+1];
	recv(socketCliente,idString,LARGO_STRING_HEADER_ID, 0);
	idString[LARGO_STRING_HEADER_ID]='\0';
	printf("id: %d\n",atoi(idString));

	for(i=0;i<4;i++){	//el 4 después va a salir del id (son 4 mensajes)
		char tamMensajeString[LARGO_STRING_TAM_MENSAJE+1];
		recv(socketCliente,tamMensajeString,LARGO_STRING_TAM_MENSAJE, 0);
		tamMensajeString[LARGO_STRING_TAM_MENSAJE]='\0';
		printf("tamMensaje: %d\n",atoi(tamMensajeString));

		char *mensajeRecibido=malloc(atoi(tamMensajeString)+1);
		recv(socketCliente,mensajeRecibido,atoi(tamMensajeString), 0);
		mensajeRecibido[atoi(tamMensajeString)]='\0';
		printf("mensajeRecibido: %s\n",mensajeRecibido);
	}
	if(preparadoEnviarFs) {
		// Envia el mensaje a la FileSystem
		//printf("%d - %d\n",header.id,header.tamPayload);

		/*if(!enviarHeader(socketFS,header)){
			puts("Error. No se enviaron todos los bytes del header");
		}
		if(!enviarMensaje(socketFS,mensaje)){
			puts("Error. No se enviaron todos los bytes del mensaje");
		}*/
	}

	cerrarServer(listenningSocket);
	//cerrarServer(socketCliente);
	log_info(logYAMA,"Server cerrado");

	log_destroy(logYAMA);
	return EXIT_SUCCESS;
}
