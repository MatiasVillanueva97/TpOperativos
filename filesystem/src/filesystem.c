/*
 ============================================================================
 Name        : filesystem.c
 Author      : Grupo 1234
 Description : Proceso FileSystem
 ============================================================================
 */

#include "filesystem.h"

enum keys {
	IP_PROPIA, PUERTO_PROPIO
};
char* keysConfigFS[] = { "IP_PROPIA", "PUERTO_PROPIO", NULL };
char* datosConfigFS[2];

// ================================================================ //
// FileSystem sabe qué está guardado y dónde.
// Recibe conexiones de DataNodes hasta alcanzar "Estado Estable".
// Se conecta a YAMA.
// Sólo hay un FileSystem corriendo al mismo tiempo.
// ================================================================ //

/* **************************************************** */
/*
 * función para simluar el envío de la información de los bloques de un archivo
 * se debe modificar cuando se haga bien la recolección esos datos
 */
void envirInfoBloques(int socketCliente) {
	uint32_t headerId = deserializarHeader(socketCliente);
	printf("id: %d\n", headerId);
	printf("mensaje predefinido: %s\n", protocoloMensajesPredefinidos[headerId]);
	int cantidadMensajes = protocoloCantidadMensajes[headerId];
	char **arrayMensajes = deserializarMensaje(socketCliente, cantidadMensajes);
	char *archivo = malloc(string_length(arrayMensajes[0]) + 1);
	strcpy(archivo, arrayMensajes[0]);
	free(arrayMensajes);
	printf("archivo: %s\n", archivo);

	char *cantBloquesFile = "0002";
	char *bloque0NodoCopia0 = "0003";
	char *bloque0BloqueCopia0 = "0033";
	char *bloque0NodoCopia1 = "0001";
	char *bloque0BloqueCopia1 = "0013";
	char *bytesBloque0 = "00012564";
	char *bloque1NodoCopia0 = "0005";
	char *bloque1BloqueCopia0 = "0020";
	char *bloque1NodoCopia1 = "0004";
	char *bloque1BloqueCopia1 = "0025";
	char *bytesBloque1 = "00003264";

	int cantStrings = 11, i=0;
	char **arrayMensajesSerializar = malloc(cantStrings);


	arrayMensajesSerializar[i] = malloc(string_length(cantBloquesFile) + 1);
	strcpy(arrayMensajesSerializar[i], cantBloquesFile);
	arrayMensajesSerializar[i][string_length(cantBloquesFile)] = '\0';

	arrayMensajesSerializar[i+1] = malloc(string_length(bloque0NodoCopia0) + 1);
	strcpy(arrayMensajesSerializar[i+1], bloque0NodoCopia0);
	arrayMensajesSerializar[i+1][string_length(bloque0NodoCopia0)] = '\0';

	arrayMensajesSerializar[i+2] = malloc(string_length(bloque0BloqueCopia0) + 1);
	strcpy(arrayMensajesSerializar[i+2], bloque0BloqueCopia0);
	arrayMensajesSerializar[i+2][string_length(bloque0BloqueCopia0)] = '\0';

	arrayMensajesSerializar[i+3] = malloc(string_length(bloque0NodoCopia1) + 1);
	strcpy(arrayMensajesSerializar[i+3], bloque0NodoCopia1);
	arrayMensajesSerializar[i+3][string_length(bloque0NodoCopia1)] = '\0';

	arrayMensajesSerializar[i+4] = malloc(string_length(bloque0BloqueCopia1) + 1);
	strcpy(arrayMensajesSerializar[i+4], bloque0BloqueCopia1);
	arrayMensajesSerializar[i+4][string_length(bloque0BloqueCopia1)] = '\0';

	arrayMensajesSerializar[i+5] = malloc(string_length(bytesBloque0) + 1);
	strcpy(arrayMensajesSerializar[i+5], bytesBloque0);
	arrayMensajesSerializar[i+5][string_length(bytesBloque0)] = '\0';

	arrayMensajesSerializar[i+6] = malloc(string_length(bloque1NodoCopia0) + 1);
	strcpy(arrayMensajesSerializar[i+6], bloque1NodoCopia0);
	arrayMensajesSerializar[i+6][string_length(bloque1NodoCopia0)] = '\0';

	arrayMensajesSerializar[i+7] = malloc(string_length(bloque1BloqueCopia0) + 1);
	strcpy(arrayMensajesSerializar[i+7], bloque1BloqueCopia0);
	arrayMensajesSerializar[i+7][string_length(bloque1BloqueCopia0)] = '\0';

	arrayMensajesSerializar[i+8] = malloc(string_length(bloque1NodoCopia1) + 1);
	strcpy(arrayMensajesSerializar[i+8], bloque1NodoCopia1);
	arrayMensajesSerializar[i+8][string_length(bloque1NodoCopia1)] = '\0';

	arrayMensajesSerializar[i+9] = malloc(string_length(bloque1BloqueCopia1) + 1);
	strcpy(arrayMensajesSerializar[i+9], bloque1BloqueCopia1);
	arrayMensajesSerializar[i+9][string_length(bloque1BloqueCopia1)] = '\0';

	arrayMensajesSerializar[i+10] = malloc(string_length(bytesBloque1) + 1);
	strcpy(arrayMensajesSerializar[i+10], bytesBloque1);
	arrayMensajesSerializar[i+10][string_length(bytesBloque1)] = '\0';

	char *mensajeSerializado = serializarMensaje(TIPO_MSJ_METADATA_ARCHIVO, arrayMensajesSerializar, cantStrings);
	printf("mensaje serializado: %s\n", mensajeSerializado);
	enviarMensaje(socketCliente, mensajeSerializado);
}

int main(int argc, char *argv[]) {
	t_log* logFileSystem;
	logFileSystem = log_create("logFile.log", "FILESYSTEM", false, LOG_LEVEL_TRACE); //creo el logger, sin mostrar por pantalla

	log_info(logFileSystem, "Iniciando FileSystem");
	printf("\n*** Proceso FileSystem ***\n");

	// 1º) leer archivo de config.
	char *nameArchivoConfig = "configFilesystem.txt";
	if (leerArchivoConfig(nameArchivoConfig, keysConfigFS, datosConfigFS)) { //leerArchivoConfig devuelve 1 si hay error
		printf("Hubo un error al leer el archivo de configuración");
		return 0;
	}

	// 2º) inicializar server y aguardar conexiones (de datanode)
	int listenningSocket = inicializarServer(datosConfigFS[IP_PROPIA], datosConfigFS[PUERTO_PROPIO]);
	if (listenningSocket < 0) {
		log_error(logFileSystem, "No pude iniciar como servidor");
		puts("No pude iniciar como servidor");
		return EXIT_FAILURE;
	}
	puts("Ya estoy preparado para recibir conexiones\n");

	int socketCliente = aceptarConexion(listenningSocket);
	if (socketCliente < 0) {
		log_error(logFileSystem, "Hubo un error al aceptar conexiones");
		puts("Hubo un error al aceptar conexiones\n");
		return EXIT_FAILURE;
	}
	log_info(logFileSystem, "FileSystem conectado, esperando conexiones");
	puts("Ya me conecté, ahora estoy esperando mensajes\n");

	//función que hice para simular la recepción por parte de YAMA de un pedido de información
	//sobre un archivo y el envío de la ubicación de los bloques de dicho archivo
	envirInfoBloques(socketCliente);

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
	//struct headerProtocolo header = recibirHeader(socketCliente);
	//char *mensaje=recibirMensaje(socketCliente,header.tamPayload);
	/*puts("Impresión por pantalla del contenido del archivo recibido");
	 printf("tamaño del mensaje: %d\n",header.tamPayload);
	 printf("mensaje: %s\n",mensaje);*/

	cerrarServer(listenningSocket);
//	if (recv(socketCliente,(void*) message, PACKAGESIZE, 0)) {
//		puts("Impresión por pantalla del contenido del archivo recibido");
//		puts("/* **************************************** */");
//		//imprime por pantalla el mensaje recibido
//		printf("Mensaje entrante: %s\n", message);
//	}
//
//	cerrarServer(listenningSocket);
	log_info(logFileSystem, "Server cerrado");

	log_destroy(logFileSystem);
	return EXIT_SUCCESS;
}
