/*
 ============================================================================
 Name        : master.c
 Author      : Grupo 1234
 Description : Proceso Master
 Resume      : Master ejecuta una tarea, sobre un archivo.
 Se conecta a YAMA para pedir instrucciones,
 y a los workers (usando hilos) para mandar instrucciones
 Puede haber varios master corriendo al mismo tiempo.
 ============================================================================
 */

#include "../../utils/includes.h"
#include <unistd.h>
#include <time.h>

enum keys {
	YAMA_IP, YAMA_PUERTO, NODO_IP, NODO_PUERTO
};

char* keysConfigMaster[] = { "YAMA_IP", "YAMA_PUERTO", "NODO_IP", "NODO_PUERTO",
		NULL };

struct datosWorker {
	char ip[16];
	char puerto[5];
};

struct filaTransformacion {
	int nodo;
	char ip[LARGO_IP];
	int puerto;
	int bloque;
	int bytes;
	char temporalTransformacion[LARGO_TEMPORAL];
};

struct filaReduccionLocal {
	int nodo;
	char ip[LARGO_IP];
	int puerto;
	char *temporalesTransformacion[LARGO_TEMPORAL];
	char temporalReduccionLocal[LARGO_TEMPORAL];
};

struct filaReduccionGlobal {
	int nodo;
	char ip[LARGO_IP];
	int puerto;
	char temporalReduccionLocal[LARGO_TEMPORAL];
};

struct filaAlmacenamientoFinal {
	int nodo;
	char ip[LARGO_IP];
	int puerto;
	char temporalReduccionGlobal[LARGO_TEMPORAL];
};

char * datosConfigMaster[4];
char * archivoTransformador;
char * archivoReductor;
char * archivoRequerido;
char * archivoDestino;

int masterCorriendo = 0;
int socketYama;
pthread_mutex_t mutexSocketYama;
int32_t headerIdFinalizado = 0;

void conectarAWorkerReduccionLocal(void *arg);

//LISTO
void calcularMetricas(float tiempo) {
	printf("El job duró: %f \n", tiempo);
}

//LISTO
int32_t handshake(int socket) {
	//envía el pedido de handshake
	int cantStringsHandshake = protocoloCantidadMensajes[TIPO_MSJ_HANDSHAKE];
	char **arrayMensajesHandshake = malloc(sizeof(char*) * cantStringsHandshake);
	char *mensaje = intToArrayZerosLeft(NUM_PROCESO_MASTER, 4);
	arrayMensajesHandshake[0] = malloc(string_length(mensaje) + 1);
	strcpy(arrayMensajesHandshake[0], mensaje);
	char *mensajeSerializadoHS = serializarMensaje(TIPO_MSJ_HANDSHAKE, arrayMensajesHandshake, cantStringsHandshake);
	enviarMensaje(socket, mensajeSerializadoHS);
	free(arrayMensajesHandshake);

	//recibe la respuesta
	return deserializarHeader(socket);
}

//LISTO
int getCantFilas(int socket, int cantMensajes) {
	char **arrayMensajes = deserializarMensaje(socket, cantMensajes);
	int cantFilas = atoi(arrayMensajes[0]);
	free(arrayMensajes[0]);
	free(arrayMensajes);
	return cantFilas;
}

//LISTO
void enviarArchivoYama(int socket, char *archivo) {
	int bytesEnviados, i, j, k, h;
	int cantStrings = protocoloCantidadMensajes[TIPO_MSJ_PATH_ARCHIVO_TRANSFORMAR];
	char **arrayMensajes = malloc(cantStrings);
	arrayMensajes[0] = malloc(string_length(archivo) + 1);
	strcpy(arrayMensajes[0], archivo);
	char *mensajeSerializado = serializarMensaje(TIPO_MSJ_PATH_ARCHIVO_TRANSFORMAR, arrayMensajes, cantStrings);
	bytesEnviados = enviarMensaje(socket, mensajeSerializado);

	liberar_array(arrayMensajes, cantStrings);
}

//LISTO
char * leerArchivo(char * ubicacionArchivo) {
	FILE *fp;
	// Lee el archivo (transformador o reductor) y lo pasa a string para poder enviarlo al worker
	char *pathArchivo = string_from_format("%s", ubicacionArchivo);
	fp = fopen(pathArchivo, "r"); // read mode
	fseek(fp, 0, SEEK_END);
	long lengthArchivo = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	char *archivoString = malloc(lengthArchivo+1);
	if (archivoString) {
		fread(archivoString, 1, lengthArchivo, fp);
	} else {
		perror("Malloc al leer archivo");
	}
	fclose(fp);
	return archivoString;
}

//LISTO
void recibirTablaTransformacion(struct filaTransformacion *datosTransformacion, int socket, int cantFilas) {
	int bytesEnviados, i, j, k, h;
	int cantMensajesXFila = 6;
	int cantMensajes = cantMensajesXFila * cantFilas;
	char **arrayTablaTransformacion = deserializarMensaje(socket, cantMensajes);

	// Recibir la tabla de transformación
	printf("\n ---------- Tabla de transformación ---------- \n");
	printf("\tNodo\tIP\tPuerto\tBloque\tBytes\tTemporal\n");
	printf("---------------------------------------------------------------------------------------------\n");
	for (i = 0, j = 0; i < cantFilas; i++) {
		// Por cada fila de la tabla transformación guardo los msjes
		datosTransformacion[i].nodo = atoi(arrayTablaTransformacion[j]);
		j++;
		strcpy(datosTransformacion[i].ip, arrayTablaTransformacion[j]);
		j++;
		datosTransformacion[i].puerto = atoi(arrayTablaTransformacion[j]);
		j++;
		datosTransformacion[i].bloque = atoi(arrayTablaTransformacion[j]);
		j++;
		datosTransformacion[i].bytes = atoi(arrayTablaTransformacion[j]);
		j++;
		strcpy(datosTransformacion[i].temporalTransformacion, arrayTablaTransformacion[j]);
		j++;
		printf("\t%d\t%s\t%d\t%d\t%d\t%s\n", datosTransformacion[i].nodo, datosTransformacion[i].ip, datosTransformacion[i].puerto, datosTransformacion[i].bloque, datosTransformacion[i].bytes, datosTransformacion[i].temporalTransformacion);
	}
	printf("\n");

	liberar_array(arrayTablaTransformacion, cantMensajes);
}

void recibirTablaReduccionGlobal(struct filaReduccionGlobal *datosReduccionGlobal, int socketYama, int cantNodos) {
	int bytesEnviados, i, j, k, h;
	int cantMensajesXFila = 4;
	int cantStrings = cantMensajesXFila * cantNodos;
	char **arrayTablaReduccionGlobal = deserializarMensaje(socketYama, cantStrings);
	printf("\nmensaje recibido %s\n", *arrayTablaReduccionGlobal);

	// Recibir la tabla de reduccion global
	printf("\n ---------- Tabla de reduccion global ---------- \n");
	printf("\tNodo\tIP\tPuerto\t\tTemporal\n");
	printf("---------------------------------------------------------------------------------------------\n");
	for (i = 0, j = 0; i < cantNodos; i++) {
		// Por cada fila de la tabla reduccion global guardo los msjes
		datosReduccionGlobal[i].nodo = atoi(arrayTablaReduccionGlobal[j]);
		j++;
		strcpy(datosReduccionGlobal[i].ip, arrayTablaReduccionGlobal[j]);
		j++;
		datosReduccionGlobal[i].puerto = atoi(arrayTablaReduccionGlobal[j]);
		j++;
		strcpy(datosReduccionGlobal[i].temporalReduccionLocal, arrayTablaReduccionGlobal[j]);
		j++;
		printf("\t%d\t%s\t%d\t%s\n", datosReduccionGlobal[i].nodo, datosReduccionGlobal[i].ip, datosReduccionGlobal[i].puerto, datosReduccionGlobal[i].temporalReduccionLocal);
	}
	printf("\n");

	liberar_array(arrayTablaReduccionGlobal, cantStrings);
}

//LISTO
void recibirTablaAlmacenamientoFinal(struct filaAlmacenamientoFinal *datosAlmacenamientoFinal, int socket, int cantFilas) {
	//int bytesEnviados, i, j, k, h;
	int cantMensajesXFila = 4;
	int cantMensajes = cantMensajesXFila * cantFilas;
	char **arrayTablaAlmacenamientoFinal = deserializarMensaje(socket, cantMensajes);

	// Recibir la tabla de transformación
	printf("\n ---------- Tabla de almacenamiento final ---------- \n");
	printf("\tNodo\tIP\t\tPuerto\t\tTemporal\n");
	printf("---------------------------------------------------------------------------------------------\n");

	datosAlmacenamientoFinal[0].nodo = atoi(arrayTablaAlmacenamientoFinal[0]);
	strcpy(datosAlmacenamientoFinal[0].ip, arrayTablaAlmacenamientoFinal[1]);
	datosAlmacenamientoFinal[0].puerto = atoi(arrayTablaAlmacenamientoFinal[2]);
	strcpy(datosAlmacenamientoFinal[0].temporalReduccionGlobal, arrayTablaAlmacenamientoFinal[3]);
	printf("\t%d\t%s\t%d\t%s\n", datosAlmacenamientoFinal[0].nodo, datosAlmacenamientoFinal[0].ip, datosAlmacenamientoFinal[0].puerto, datosAlmacenamientoFinal[0].temporalReduccionGlobal);

	liberar_array(arrayTablaAlmacenamientoFinal, cantMensajes);
}

// LISTO
void conectarAWorkerTransformacion(void *arg) {
	int i, j;
	pthread_t idHilo = pthread_self();
	struct filaTransformacion *datosEnHilo = (struct filaTransformacion*) arg;

	// Leo el archivo transformador
	char * transformadorString = leerArchivo(archivoTransformador);

	// Abrir conexión a Worker
	int socketWorker = conectarA(datosEnHilo->ip, string_itoa(datosEnHilo->puerto));
	if (socketWorker < 0) {
		puts("No se pudo conectar al worker");
	}

	int32_t headerIdWorker = handshake(socketWorker);
	if (headerIdWorker != TIPO_MSJ_HANDSHAKE_RESPUESTA_OK) {
		puts("Error de handshake con el worker");
	} else {
		puts("Conectado al worker");

		int cantStringsASerializar = 4;
		char **arrayMensajes = malloc(sizeof(char*) * cantStringsASerializar);
		j = 0;

		// Serializo script transformacion
		arrayMensajes[j] = malloc(string_length(transformadorString) + 1);
		strcpy(arrayMensajes[j], transformadorString);
		j++;

		// Serializo bloque
		arrayMensajes[j] = malloc(4 + 1);
		strcpy(arrayMensajes[j], intToArrayZerosLeft(datosEnHilo->bloque, 4));
		j++;

		// Serializo bytes
		arrayMensajes[j] = malloc(8 + 1);
		strcpy(arrayMensajes[j], intToArrayZerosLeft(datosEnHilo->bytes, 8));
		j++;

		// Serializo temporal transformacion
		arrayMensajes[j] = malloc(string_length(datosEnHilo->temporalTransformacion) + 1);
		strcpy(arrayMensajes[j], datosEnHilo->temporalTransformacion);

		// TIPO_MSJ_DATA_TRANSFORMACION_WORKER: 4 MENSAJES -> script de transformación, bloque, bytes y temporal
		char *mensajeSerializado = serializarMensaje(TIPO_MSJ_DATA_TRANSFORMACION_WORKER, arrayMensajes, cantStringsASerializar);
		liberar_array(arrayMensajes, cantStringsASerializar);
		int cantBytesEnviados = enviarMensaje(socketWorker, mensajeSerializado);

		// Recibo resultado del Worker y cierro conexión
		int32_t headerId = deserializarHeader(socketWorker);
		printf("Header recibido por parte del worker en hilo %lu: %d\n", idHilo, headerId);
		cerrarCliente(socketWorker);

		// Aviso a YAMA
		int bytesEnviadosMensaje = envioFinTransformacion(headerId, datosEnHilo->nodo, datosEnHilo->bloque);
		printf("Datos al final del hilo %lu: nodo %d, bloque %d, ip %s, puerto %d, temporal %s \n", idHilo, datosEnHilo->nodo, datosEnHilo->bloque, datosEnHilo->ip, datosEnHilo->puerto, datosEnHilo->temporalTransformacion);
	}
}

//LISTO
void recibirTablaReduccionLocal(struct filaReduccionLocal *datosReduccionLocal, int socketYama, int cantNodos) {
		int cantStrings, bytesEnviados, i, j, k, h, cantTemporales;

		// Recibir la tabla de reduccion local
		printf("\n ---------- Tabla de reducción local ---------- \n");
		printf("\tNodo\tIP\t\t\tPuerto\t\tTemporales Transf.\t\tTemporal Red. Local\n");
		printf("---------------------------------------------------------------------------------------------\n");
		for (i = 0, j = 0; i < cantNodos; i++) {
			// Leo los primeros 4 campos, hasta cant. de temporales
			cantStrings = 4;
			char **arrayTablaReduccionLocal = deserializarMensaje(socketYama, cantStrings);

			datosReduccionLocal[i].nodo = atoi(arrayTablaReduccionLocal[j]);
			j++;

			strcpy(datosReduccionLocal[i].ip, arrayTablaReduccionLocal[j]);
			j++;

			datosReduccionLocal[i].puerto = atoi(arrayTablaReduccionLocal[j]);
			j++;

			cantTemporales = atoi(arrayTablaReduccionLocal[j]);
			j++;

			printf("\t%d\t%s\t%d\t\t", datosReduccionLocal[i].nodo, datosReduccionLocal[i].ip, datosReduccionLocal[i].puerto);
			// Leo los temporales de transformación
			char **arrayTemporalesTransformacion = deserializarMensaje(socketYama, cantTemporales);
			for (k = 0; k < cantTemporales; k++) {
				printf("%s - ", arrayTemporalesTransformacion[k]);
				datosReduccionLocal[i].temporalesTransformacion[k] = malloc(string_length(arrayTemporalesTransformacion[k]) + 1);
				strcpy(datosReduccionLocal[i].temporalesTransformacion[k], arrayTemporalesTransformacion[k]);
			}

			// Leo el temporal de reducción local
			char **arrayTemporalReduccionLocal = deserializarMensaje(socketYama, 1);
			printf("\t\t%s\n", arrayTemporalReduccionLocal[0]);
			strcpy(datosReduccionLocal[i].temporalReduccionLocal, arrayTemporalReduccionLocal[0]);

		}
}

//void recibirTablaReduccLocalYEnviarAWorker(int socketYama, int headerId) {
//	int cantStrings, bytesEnviados, i, j, k, h, cantTemporales;
//
//	char **arrayMensajesCantNodos = deserializarMensaje(socketYama, protocoloCantidadMensajes[headerId]);
//	int cantNodos = atoi(arrayMensajesCantNodos[0]);
//	free(arrayMensajesCantNodos[0]);
//	free(arrayMensajesCantNodos);
//
//	pthread_t hilosWorker[cantNodos];
//	//struct datosWorker datosWorker[cantNodos];
//	struct filaReduccionLocal datosReduccionLocal[cantNodos];
//
//	puts("datos para la reducción local");
//	for (i = 0; i < cantNodos; i++) {
//		cantStrings = 4;
//		char **arrayDatosReduccionLocal = deserializarMensaje(socketYama, cantStrings);
//
//		datosReduccionLocal[i].nodo = atoi(arrayDatosReduccionLocal[0]);
//		printf("nro nodo: %d\n", atoi(arrayDatosReduccionLocal[0]));
//
//		strcpy(datosReduccionLocal[i].ip, arrayDatosReduccionLocal[1]);
//		printf("ip nodo: %s\n", arrayDatosReduccionLocal[1]);
//
//		datosReduccionLocal[i].puerto = atoi(arrayDatosReduccionLocal[2]);
//		printf("puerto nodo: %d\n", atoi(arrayDatosReduccionLocal[2]));
//
//		cantTemporales = atoi(arrayDatosReduccionLocal[3]);
//		printf("cantidad de temporales: %d\n", cantTemporales);
//
//		//		for (k = 0; k < 3; k++) {
//		//			free(arrayDatosReduccionLocal[k]);
//		//		}
//		//		free(arrayDatosReduccionLocal);
//
//		//TODO REVISAR ESTO
//		char **arrayTablaTemporalesTransformacion = deserializarMensaje(socketYama, cantTemporales);
//		for (k = 0; k < cantTemporales; k++) {
//			printf("temporal %d: %s\n", k, arrayTablaTemporalesTransformacion[k]);
//			datosReduccionLocal[i].temporalesTransformacion[k] = malloc(string_length(arrayTablaTemporalesTransformacion[k]) + 1);
//			strcpy(datosReduccionLocal[i].temporalesTransformacion[k], arrayTablaTemporalesTransformacion[k]);
//		}
//
//		//		for (k = 0; k < cantTemporales; k++) {
//		//			free(arrayTablaTemporalesTransformacion[k]);
//		//		}
//		//		free(arrayTablaTemporalesTransformacion);
//
//		//falta recibir el temporalReduccLocal
//		char **arrayTemporalReduccionLocal = deserializarMensaje(socketYama, 1);
//		printf("temporal recibido %s", arrayTemporalReduccionLocal[0]);
//		strcpy(datosReduccionLocal[i].temporalReduccionLocal, arrayTemporalReduccionLocal[0]);
//		printf("temporal guardado %s", datosReduccionLocal[i].temporalReduccionLocal);
//
//		//		free(arrayTemporalReduccioLocal[0]);
//		//		free(arrayTemporalReduccioLocal);
//
//		pthread_create(&(hilosWorker[i]), NULL, conectarAWorkerReduccionLocal, &datosReduccionLocal[i]);
//	}
//
//	for (i = 0; i < cantNodos; i++) {
//		//TODO: está bien hecho así??????????
//		//TODO: no se quedaría esperando que terminen todas las transformaciones en vez de seguir??????
//		pthread_join(hilosWorker[i], NULL);
//	}
//}

//LISTO
void conectarAWorkerReduccionLocal(void *arg) {
	int i, j;
	pthread_t idHilo = pthread_self();
	struct filaReduccionLocal *datosEnHilo = (struct filaReduccionLocal*) arg;
	int cantTemporalesTransformacion = sizeof(datosEnHilo->temporalesTransformacion) / LARGO_TEMPORAL;

	// Leo el archivo reductor
	char * reductorString = leerArchivo(archivoReductor);

	// Abrir conexión a Worker
	printf("me conecto a %s : %d", datosEnHilo->ip, datosEnHilo->puerto);
	int socketWorker = conectarA(datosEnHilo->ip, string_itoa(datosEnHilo->puerto));
	if (socketWorker < 0) {
		puts("No se pudo conectar al worker");
	}

	// Handshake con Worker
	int32_t headerIdWorker = handshake(socketWorker);
	if (headerIdWorker != TIPO_MSJ_HANDSHAKE_RESPUESTA_OK) {
		puts("Error de handshake con el worker");
	} else {
		puts("Conectado al worker");

		int cantStringsASerializar = 1 + 1 + cantTemporalesTransformacion + 1;
		char **arrayMensajes = malloc(sizeof(char*) * cantStringsASerializar);
		j = 0;

		// Serializo script reductor
		arrayMensajes[j] = malloc(string_length(reductorString) + 1);
		strcpy(arrayMensajes[j], reductorString);
		free(reductorString);
		j++;

		// Serializo cantidad de temporales transformacion
		char* cantTemporalesTransformacionString = intToArrayZerosLeft(cantTemporalesTransformacion, 4);
		arrayMensajes[j] = malloc(string_length(cantTemporalesTransformacionString) + 1);
		strcpy(arrayMensajes[j], cantTemporalesTransformacionString);
		j++;

		// Serializo temporales transformacion
		for (i = 0; i < cantTemporalesTransformacion; i++) {
			arrayMensajes[j] = malloc(string_length(datosEnHilo->temporalesTransformacion[i]) + 1);
			strcpy(arrayMensajes[j], datosEnHilo->temporalesTransformacion[i]);
			j++;
		}

		// Serializo temporal reduccion local
		arrayMensajes[j] = malloc(string_length(datosEnHilo->temporalReduccionLocal) + 1);
		strcpy(arrayMensajes[j], datosEnHilo->temporalReduccionLocal);

		//TIPO_MSJ_DATA_REDUCCION_LOCAL_WORKER: 1 MENSAJE -> reductor.py | cantidad de temporales transformacion | temp tranf 1 | .... | temp transf N | temp reduccLocal
		char *mensajeSerializado = serializarMensaje(TIPO_MSJ_DATA_REDUCCION_LOCAL_WORKER, arrayMensajes, cantStringsASerializar);
		liberar_array(arrayMensajes, cantStringsASerializar);
		int cantBytesEnviados = enviarMensaje(socketWorker, mensajeSerializado);

		// Recibo resultado del Worker y cierro conexión
		int32_t headerIdWorker = deserializarHeader(socketWorker);
		printf("Header recibido de worker %d %s", headerIdWorker, protocoloMensajesPredefinidos[headerIdWorker]);
		cerrarCliente(socketWorker);

		// Aviso a YAMA
		int bytesEnviadosMensaje = envioFinReduccion(headerIdWorker, datosEnHilo->nodo);
		printf("Datos al final del hilo %lu: nodo %d, ip %s, puerto %d", idHilo, datosEnHilo->nodo, datosEnHilo->ip, datosEnHilo->puerto);
	}
}

void conectarAWorkerReduccionGlobal(void *arg) {
	FILE *fp;
	int i, j;
	pthread_t idHilo = pthread_self();
	struct filaReduccionGlobal *datosEnHilo = (struct filaReduccionGlobal*) arg;
	printf("Se creó el hilo %lu\n", idHilo);

	// Leo el archivo reductor
	char * reductorString = leerArchivo(archivoReductor);

	// Abrir conexión a Worker
	int socketWorker = conectarA(datosEnHilo->ip, string_itoa(datosEnHilo->puerto));
	if (socketWorker < 0) {
		puts("No se pudo conectar al worker");
	}

	// Handshake con Worker
	int32_t headerIdWorker = handshake(socketWorker);
	if (headerIdWorker != TIPO_MSJ_HANDSHAKE_RESPUESTA_OK) {
		puts("Error de handshake con el worker");
	} else {
		puts("Conectado al worker");

		//envio de mensajes: reductor.py | cantidad de temporales transformacion | temp tranf 1 | .... | temp transf N | temp reduccLocal
		// el protocolo son 2 porq solo recibe el script reductor y la cant de temporales, recien ahi sabe cuanto leer despues

		int cantNodos = 2; //REVISAR
		int cantStringsASerializar = 1 + 1 + cantNodos + 1;
		char **arrayMensajes = malloc(sizeof(char*) * cantStringsASerializar);
		j = 0;



		// Serializo script reductor
		arrayMensajes[j] = malloc(string_length(reductorString) + 1);
		strcpy(arrayMensajes[j], reductorString);
		free(reductorString);
		j++;

		// Serializo cantidad de nodos
		char* cantNodosReduccionGlobalString = intToArrayZerosLeft(cantNodos, 4);
		arrayMensajes[j] = malloc(string_length(cantNodosReduccionGlobalString) + 1);
		strcpy(arrayMensajes[j], cantNodosReduccionGlobalString);
		j++;

		for (i = 0; i < cantNodos; i++) {
			// Serializo nro de nodo
			arrayMensajes[j] = malloc(4 + 1);
			strcpy(arrayMensajes[j], intToArrayZerosLeft(datosEnHilo->nodo, 4));
			j++;

			// Serializo IP
			arrayMensajes[j] = malloc(string_length(datosEnHilo->ip) + 1);
			strcpy(arrayMensajes[j], datosEnHilo->ip);
			j++;

			// Serializo Puerto
			arrayMensajes[j] = malloc(LARGO_PUERTO + 1);
			strcpy(arrayMensajes[j], intToArrayZerosLeft(datosEnHilo->puerto, LARGO_PUERTO));
			j++;

			// Serializo temporal reduccion
			arrayMensajes[j] = malloc(string_length(datosEnHilo->temporalReduccionLocal) + 1);
			strcpy(arrayMensajes[j], datosEnHilo->temporalReduccionLocal);
			j++;
		}

		//		j++;
		//		char* cantNodosReduccionGlobal = intToArrayZerosLeft(cantNodos, 4);
		//		arrayMensajes[j] = malloc(string_length(cantNodosReduccionGlobal) + 1);
		//		strcpy(arrayMensajes[j], cantNodosReduccionGlobal);

		//serializo temporales
		//		j++;
		//		char* cantNodosReduccionGlobal = intToArrayZerosLeft(cantNodos, 4);
		//		arrayMensajes[j] = malloc(string_length(cantNodosReduccionGlobal) + 1);
		//		strcpy(arrayMensajes[j], cantNodosReduccionGlobal);

		//serializo temporal reduccion global
		//		arrayMensajes[j] = malloc(string_length(datos->temporalReduccionLocal) + 1);
		//		strcpy(arrayMensajes[j], datos->temporalReduccionLocal);

		//TIPO_MSJ_DATA_REDUCCION_LOCAL_WORKER: 1 MENSAJE
		char *mensajeSerializado = serializarMensaje(TIPO_MSJ_DATA_REDUCCION_GLOBAL_WORKER, arrayMensajes, cantStringsASerializar);

		for (j = 0; j < cantStringsASerializar; j++) {
			free(arrayMensajes[j]);
		}
		free(arrayMensajes);

		// envio mensaje serializado
		printf("\nmensaje serializado: \n%s\n", mensajeSerializado);
		//int temp = enviarMensaje(socketWorker, mensajeSerializado);
		//printf("bytes enviados a worker msje serializado %d", temp);

		// recibo rta. del worker
		//int32_t headerIdWorker = deserializarHeader(socketWorker);
		printf("header recibido de worker %d %s", headerIdWorker, protocoloMensajesPredefinidos[headerIdWorker]);
		int nroNodoFinalizado = datosEnHilo->nodo;

		sleep(3);
		// Avisar a YAMA
		int bytesEnviadosMensaje = envioFinReduccion(TIPO_MSJ_REDUCC_GLOBAL_OK, nroNodoFinalizado);
		pthread_t idHilo = pthread_self();

		//printf("Resultado transformación hilo %lu en nodo %d sobre bloque %d es: %s\n", idHilo, datosEnHilo->nodo, datosEnHilo->bloque, protocoloMensajesPredefinidos[headerId]);
		printf("Datos al final del hilo %lu: nodo %d, ip %s, puerto %d", idHilo, nroNodoFinalizado, datosEnHilo->ip, datosEnHilo->puerto);
		//printf("Bytes enviados mensaje en el hilo %lu: %d\n\n", idHilo, bytesEnviadosMensaje);
		puts("");
	}
}

//LISTO
void conectarAWorkerAlmacenamientoFinal(void *arg) {
	int i, j;
	pthread_t idHilo = pthread_self();
	struct filaAlmacenamientoFinal *datosEnHilo = (struct filaAlmacenamientoFinal*) arg;

	// Abrir conexión a Worker
	int socketWorker = conectarA(datosEnHilo->ip, string_itoa(datosEnHilo->puerto));
	if (socketWorker < 0) {
		puts("No se pudo conectar al worker");
	}

	// Handshake con Worker
	int32_t headerIdWorker = handshake(socketWorker);
	if (headerIdWorker != TIPO_MSJ_HANDSHAKE_RESPUESTA_OK) {
		puts("Error de handshake con el worker");
	} else {
		puts("Conectado al worker");

		int cantStringsASerializar = 2;
		char **arrayMensajes = malloc(sizeof(char*) * cantStringsASerializar);
		j = 0;

		// Serializo path reduccion global
		arrayMensajes[j] = malloc(string_length(datosEnHilo->temporalReduccionGlobal) + 1);
		strcpy(arrayMensajes[j], datosEnHilo->temporalReduccionGlobal);

		// Serializo path almacenado final
		arrayMensajes[j] = malloc(string_length(archivoDestino));
		strcpy(arrayMensajes[j], archivoDestino);
		j++;

		// TIPO_MSJ_DATA_ALMACENAMIENTO_FINAL_WORKER: 2 MENSAJES -> path de reduccion global y path de almacenado final
		char *mensajeSerializado = serializarMensaje(TIPO_MSJ_DATA_ALMACENAMIENTO_FINAL_WORKER, arrayMensajes, cantStringsASerializar);
		liberar_array(arrayMensajes, cantStringsASerializar);
		int cantBytesEnviados = enviarMensaje(socketWorker, mensajeSerializado);

		// Recibo resultado del Worker y cierro conexión
		int32_t headerId = deserializarHeader(socketWorker);
		printf("Header recibido por parte del worker en hilo %lu: %d\n", idHilo, headerId);
		cerrarCliente(socketWorker);

		// Aviso a YAMA
		int bytesEnviadosMensaje = envioFinAlmacenamientoFinal(headerIdWorker);
		printf("Datos al final del hilo %lu: nodo %d, ip %s, puerto %d, temporal %s \n", idHilo, datosEnHilo->nodo, datosEnHilo->ip, datosEnHilo->puerto, datosEnHilo->temporalReduccionGlobal);
	}
}

//LISTO
int envioFinTransformacion(int headerId, int nroNodo, int nroBloque) {
	int bytesEnviados, i, j, k, h;
	int cantMensajes = protocoloCantidadMensajes[headerId];

	//arma el array de strings para serializar
	char **arrayMensajes = malloc(sizeof(char*) * cantMensajes);

	//colecto nro de nodo
	char *nodoString = intToArrayZerosLeft(nroNodo, 4);
	arrayMensajes[0] = malloc(string_length(nodoString) + 1);
	strcpy(arrayMensajes[0], nodoString);

	//colecto nro de bloque
	char *bloqueString = intToArrayZerosLeft(nroBloque, 4);
	arrayMensajes[1] = malloc(string_length(bloqueString) + 1);
	strcpy(arrayMensajes[1], bloqueString);

	//serializa los mensajes y los envía
	char *mensajeSerializado = serializarMensaje(headerId, arrayMensajes, cantMensajes);
	pthread_mutex_lock(&mutexSocketYama);
	bytesEnviados = enviarMensaje(socketYama, mensajeSerializado);
	pthread_mutex_unlock(&mutexSocketYama);

	//libera todos los pedidos de malloc
	liberar_array(arrayMensajes, cantMensajes);
	return bytesEnviados;
}

//LISTO
int envioFinReduccion(int headerId, int nroNodo) {
	int bytesEnviados, i, j, k, h;
	int cantMensajes = protocoloCantidadMensajes[headerId];

	//arma el array de strings para serializar
	char **arrayMensajes = malloc(sizeof(char*) * cantMensajes);

	//colecto nro de nodo
	char *nodoString = intToArrayZerosLeft(nroNodo, 4);
	arrayMensajes[0] = malloc(string_length(nodoString) + 1);
	strcpy(arrayMensajes[0], nodoString);

	//serializa los mensajes y los envía
	printf("envio a Yama %d", headerId);
	char *mensajeSerializado = serializarMensaje(headerId, arrayMensajes, cantMensajes);
	pthread_mutex_lock(&mutexSocketYama);
	bytesEnviados = enviarMensaje(socketYama, mensajeSerializado);
	pthread_mutex_unlock(&mutexSocketYama);

	//libera todos los pedidos de malloc
	liberar_array(arrayMensajes, cantMensajes);
	return bytesEnviados;
}

//LISTO
int envioFinAlmacenamientoFinal(int headerId) {
	//serializa los mensajes y los envía
	printf("envio a Yama %d", headerId);
	pthread_mutex_lock(&mutexSocketYama);
	int bytesEnviados = enviarHeaderSolo(socketYama, headerId);
	pthread_mutex_unlock(&mutexSocketYama);
	return bytesEnviados;
}

int main(int argc, char *argv[]) {
	clock_t start = clock();
	int i, j, k, h;
	uint32_t preparadoEnviarYama = 1;
	int32_t headerIdYama;

	t_log* logMASTER;
	logMASTER = log_create("logMASTER.log", "MASTER", true, LOG_LEVEL_TRACE); //creo el logger, mostrando por pantalla

	printf("\n*** Proceso Master ***\n");
	log_info(logMASTER, "Iniciando proceso MASTER");

	//TODO !!!!!!!!! Creo que esto está mal, no se está haciendo un malloc para los char*, por ende está usando cualquier parte de la memoria,
	//probable SEGFAULT
	//	archivoTransformador = argv[1];
	//	archivoReductor = argv[2];
	//	archivoRequerido = argv[3];
	//	archivoDestino = argv[4];

	if (argc < 5) {
		puts("Error. Faltan parámetros en la ejecución del proceso.\n");
		return EXIT_FAILURE;
	}

	archivoTransformador = malloc(string_length(argv[1])+1);
	strcpy(archivoTransformador, argv[1]);

	archivoReductor = malloc(string_length(argv[2])+1);
	strcpy(archivoReductor, argv[2]);

	archivoRequerido = malloc(string_length(argv[3])+1);
	strcpy(archivoRequerido, argv[3]);

	archivoDestino = malloc(string_length(argv[4])+1);
	strcpy(archivoDestino, argv[4]);

	if (pthread_mutex_init(&mutexSocketYama, NULL) != 0) {
		printf("\n Mutex init failed\n");
		return 1;
	}

	// 1º) leer archivo de config.
	char *nameArchivoConfig = "configMaster.txt";
	if (leerArchivoConfig(nameArchivoConfig, keysConfigMaster, datosConfigMaster)) { //leerArchivoConfig devuelve 1 si hay error
		printf("Hubo un error al leer el archivo de configuración");
		return EXIT_FAILURE;
	}

	// 2º) conectarse a YAMA y aguardar instrucciones
	log_info(logMASTER, "Conexión a Yama, IP: %s, Puerto: %s", datosConfigMaster[YAMA_IP], datosConfigMaster[YAMA_PUERTO]);
	socketYama = conectarA(datosConfigMaster[YAMA_IP], datosConfigMaster[YAMA_PUERTO]);
	if (socketYama < 0) {
		puts("No se pudo conectar a YAMA");
		return EXIT_FAILURE;
	}

	// 3º) Inicia comunicación con YAMA enviando el HANDSHAKE
	headerIdYama = handshake(socketYama);
	if (headerIdYama != TIPO_MSJ_HANDSHAKE_RESPUESTA_OK) {
		puts("Error de handshake con YAMA");
	} else {
		puts("Conectado a YAMA");

		// Envía a Yama el archivo con el que quiere trabajar
		enviarArchivoYama(socketYama, archivoRequerido);
		char idString[LARGO_STRING_HEADER_ID + 1];
		int cantBytesRecibidos, cantBloquesTransformacion, cantNodosReduccionLocal, cantNodosReduccionGlobal, cantNodosAlmacenamientoFinal;

		// Escucha respuestas de Yama
		while (masterCorriendo == 0) {
			headerIdYama = deserializarHeader(socketYama);
			printf("\nHeaderId recibido: %d - ", headerIdYama);
			printf("HeaderId msje recibido: %s\n", protocoloMensajesPredefinidos[headerIdYama]);

			switch (headerIdYama) {

			case TIPO_MSJ_TABLA_TRANSFORMACION: {
				// Leo cantidad de filas (bloques) que voy a recibir
				cantBloquesTransformacion = getCantFilas(socketYama, protocoloCantidadMensajes[headerIdYama]);
				pthread_t hilosWorkerTransformacion[cantBloquesTransformacion];

				// Creo y recibo tablaTransformacion, que es un array de structs filaTransformacion
				struct filaTransformacion tablaTransformacion[cantBloquesTransformacion];
				recibirTablaTransformacion(tablaTransformacion, socketYama, cantBloquesTransformacion);

				for (i = 0; i < cantBloquesTransformacion; i++) {
					// Por cada tarea se crea un hilo para conectarse al worker q corresponda y se le pasa la fila de la tabla recibida
					pthread_create(&hilosWorkerTransformacion[i], NULL, conectarAWorkerTransformacion, &tablaTransformacion[i]);
					sleep(1);
				}

				for (i = 0; i < cantBloquesTransformacion; i++) {
					//TODO: está bien hecho así? no se quedaría esperando que terminen todas las transformaciones en vez de seguir??????
					pthread_join(hilosWorkerTransformacion[i], NULL);
				}
			}
			break;

			// 4º) Etapa de Reducción Local: crear hilo, conectarse al worker, esperar y notificar a YAMA
			case TIPO_MSJ_TABLA_REDUCCION_LOCAL: {
				// Leo cantidad de filas (nodos) que voy a recibir
				cantNodosReduccionLocal = getCantFilas(socketYama, protocoloCantidadMensajes[headerIdYama]);
				pthread_t hilosWorkerReduccionLocal[cantNodosReduccionLocal];

				// Creo y recibo tablaReduccionLocal, que es un array de structs filaReduccionLocal
				struct filaReduccionLocal tablaReduccionLocal[cantNodosReduccionLocal];
				recibirTablaReduccionLocal(tablaReduccionLocal, socketYama, cantNodosReduccionLocal);

				for (i = 0; i < cantNodosReduccionLocal; i++) {
					// Por cada tarea se crea un hilo para conectarse al worker q corresponda y se le pasa la fila de la tabla recibida
					pthread_create(&hilosWorkerReduccionLocal[i], NULL, conectarAWorkerReduccionLocal, &tablaReduccionLocal[i]);
				}

				for (i = 0; i < cantNodosReduccionLocal; i++) {
					//TODO: está bien hecho así? no se quedaría esperando que terminen todas las transformaciones en vez de seguir??????
					pthread_join(hilosWorkerReduccionLocal[i], NULL);
				}

				//recibirTablaReduccLocalYEnviarAWorker(socketYama, headerIdYama);
				//sleep(5);
			}
			break;

			// 5º) Etapa de Reducción Global: crear hilo, conectarse al worker, esperar y notificar a YAMA
			case TIPO_MSJ_TABLA_REDUCCION_GLOBAL: {
				// Leo cantidad de filas (nodos) que voy a recibir
				cantNodosReduccionGlobal = getCantFilas(socketYama, protocoloCantidadMensajes[headerIdYama]);
				pthread_t hiloWorkerReduccionGlobal;

				// Creo y recibo tablaReduccionGlobal, que es un array de structs filaReduccionGlobal
				struct filaReduccionGlobal tablaReduccionGlobal[cantNodosReduccionGlobal];
				recibirTablaReduccionGlobal(tablaReduccionGlobal, socketYama, cantNodosReduccionGlobal);

				// Recibo el temporal global
				char **arrayDatosReduccionGlobal = deserializarMensaje(socketYama, 1);
				char * temporalGlobal[LARGO_TEMPORAL];
				strcpy(temporalGlobal, arrayDatosReduccionGlobal[0]);
				free(arrayDatosReduccionGlobal[0]);
				free(arrayDatosReduccionGlobal);

				pthread_create(&hiloWorkerReduccionGlobal, NULL, conectarAWorkerReduccionLocal, &tablaReduccionGlobal);
				pthread_join(hiloWorkerReduccionGlobal, NULL);
			}
			break;

			// 6º) Etapa de Almacenamiento Final: crear hilo, conectarse al worker, esperar y notificar a YAMA
			case TIPO_MSJ_TABLA_ALMACENAMIENTO_FINAL: {
				cantNodosAlmacenamientoFinal = 1;
				pthread_t hiloWorkerAlmacenamientoFinal;

				// Creo y recibo tablaAlmacenamientoFinal, que es un array de structs filaAlmacenamientoFinal
				struct filaAlmacenamientoFinal tablaAlmacenamientoFinal[cantNodosAlmacenamientoFinal];
				recibirTablaAlmacenamientoFinal(tablaAlmacenamientoFinal, socketYama, cantNodosAlmacenamientoFinal);

				pthread_create(&hiloWorkerAlmacenamientoFinal, NULL, conectarAWorkerAlmacenamientoFinal, &tablaAlmacenamientoFinal);
				pthread_join(hiloWorkerAlmacenamientoFinal, NULL);

				break;
				masterCorriendo = 1;
			}
			} // fin switch

		} // fin while

	} // fin if

	// 6º) Calcular métricas, finalizar Master y desconectar Yama
	clock_t end = clock();
	float tiempoTranscurrido = (float) (end - start) / CLOCKS_PER_SEC;
	calcularMetricas(tiempoTranscurrido);
	log_info(logMASTER, "Master finalizado.");
	printf("Master finalizado.\n");
	log_destroy(logMASTER);
	cerrarCliente(socketYama);
	return EXIT_SUCCESS;

}
