/*
 ============================================================================
 Name        : yama.c
 Author      : Grupo 1234
 Description : Proceso YAMA
 ============================================================================
 */
// ================================================================ //
// YAMA coordina con Master donde correr los jobs.
// Se conecta a FileSystem. Única instancia.
// Solo hay un YAMA corriendo al mismo tiempo.
// ================================================================ //
#include "../../utils/includes.h"

// ================================================================ //
// enum y vectores para los datos de configuración levantados del archivo config
// ================================================================ //
enum keys {
	IP_PROPIA, PUERTO_PROPIO, FS_IP, FS_PUERTO
};
char* keysConfigYama[] = { "IP_PROPIA", "PUERTO_PROPIO", "FS_IP", "FS_PUERTO", NULL };
char* datosConfigYama[4];

// ================================================================ //
// tabla de estados de YAMA
// ================================================================ //
enum etapasTablaEstados {
	TRANSFORMACION, REDUCC_LOCAL, REDUCC_GLOBAL
};
enum estadoTablaEstados {
	EN_PROCESO, ERROR, FIN_OK
};
struct filaTablaEstados {
	int job;
	int master;
	int nodo;
	int bloque;
	int etapa;
	int estado;
	struct filaTablaEstados *siguiente;
};

//struct filaTablaEstados tablaEstados[100];

int getDatosConfiguracion() {
	char *nameArchivoConfig = "configYama.txt";
	if (leerArchivoConfig(nameArchivoConfig, keysConfigYama, datosConfigYama)) { //leerArchivoConfig devuelve 1 si hay error
		printf("Hubo un error al leer el archivo de configuración");
		return 0;
	}
	return 1;
}

int conexionAFileSystem(t_log* logYAMA) {
	log_info(logYAMA, "Conexión a FileSystem, IP: %s, Puerto: %s", datosConfigYama[FS_IP], datosConfigYama[FS_PUERTO]);
	int socketFS = conectarA(datosConfigYama[FS_IP], datosConfigYama[FS_PUERTO]);
	if (socketFS < 0) {
		puts("Filesystem not ready\n");
		//preparadoEnviarFs = handshakeClient(&datosConexionFileSystem, NUM_PROCESO_KERNEL);
	}
	return socketFS;
}

int inicializoComoServidor(t_log* logYAMA) {
	int listenningSocket = inicializarServer(datosConfigYama[IP_PROPIA], datosConfigYama[PUERTO_PROPIO]);
	if (listenningSocket < 0) {
		log_error(logYAMA, "No pude iniciar como servidor");
		puts("No pude iniciar como servidor");
	}
	return listenningSocket;
}

int recibirConexion(t_log* logYAMA, int listenningSocket) {
	int socketCliente = aceptarConexion(listenningSocket);
	if (socketCliente < 0) {
		log_error(logYAMA, "Hubo un error al aceptar conexiones");
		puts("Hubo un error al aceptar conexiones\n");
	} else {
		log_info(logYAMA, "Cliente conectado, esperando mensajes");
		puts("Esperando mensajes\n");
	}
	return socketCliente;
}

int main(int argc, char *argv[]) {

	t_log* logYAMA;
	logYAMA = log_create("logYAMA.log", "YAMA", false, LOG_LEVEL_TRACE); //creo el logger, sin mostrar por pantalla
	int preparadoEnviarFs = 1, i;

	log_info(logYAMA, "Iniciando proceso YAMA");
	printf("\n*** Proceso Yama ***\n");

	// 1º) leer archivo de config.
	if (!getDatosConfiguracion()) {
		return EXIT_FAILURE;
	}
	/* ************** conexión como cliente al FS *************** */
	int socketFS;
	if ((socketFS = conexionAFileSystem(logYAMA)) < 0) {
		preparadoEnviarFs = 0;
	}
	/* ************** inicialización como server ************ */
	int listenningSocket;
	if ((listenningSocket = inicializoComoServidor(logYAMA)) < 0) {
		return EXIT_FAILURE;
	}

	int socketCliente;
	if ((socketCliente = recibirConexion(logYAMA, listenningSocket)) < 0) {
		return EXIT_FAILURE;
	}

	/* *************************** espera recepción de un mensaje ****************************/
	uint32_t headerId = deserializarHeader(socketCliente);
	printf("id: %d\n", headerId);
	printf("mensaje predefinido: %s\n", protocoloMensajesPredefinidos[headerId]);
	int cantidadMensajes = protocoloCantidadMensajes[headerId];
	char **arrayMensajes = deserializarMensaje(socketCliente, cantidadMensajes);
	switch (headerId) {
	case TIPO_MSJ_HANDSHAKE:
		;
		free(arrayMensajes);
		break;
	case TIPO_MSJ_PATH_ARCHIVO:
		;
		char *archivo = malloc(string_length(arrayMensajes[0]) + 1);
		strcpy(archivo, arrayMensajes[0]);
		printf("archivo: %s\n", archivo);
		free(arrayMensajes[0]);
		break;
	case TIPO_MSJ_CUATRO_MENSAJES:
		;
		for (i = 0; i < cantidadMensajes; i++) {
			printf("mensajeRecibido %d: %s\n", i, arrayMensajes[i]);
			free(arrayMensajes[i]);
		}
		free(arrayMensajes);
		break;
	default:
		;
		free(arrayMensajes);
		break;
	}
	//

	if (preparadoEnviarFs) {
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
	log_info(logYAMA, "Server cerrado");

	log_destroy(logYAMA);
	return EXIT_SUCCESS;
}
