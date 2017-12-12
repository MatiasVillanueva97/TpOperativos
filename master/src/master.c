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

char * datosConfigMaster[4];

char * archivoTransformador;
char * archivoReductor;
char * archivoRequerido;
char * archivoDestino;

//char archivoTransformador[200], archivoReductor[200], archivoRequerido[200],archivoDestino[200];

int masterCorriendo = 0;
int socketYama;

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
	char temporal[LARGO_TEMPORAL];
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
	char temporalReduccionGlobal[LARGO_TEMPORAL];
};

struct filaAlmacenamientoFinal {
	int nodo;
	char ip[LARGO_IP];
	int puerto;
	char archivoReduccionGlobal[LARGO_TEMPORAL];
};

pthread_mutex_t mutexSocketWorker, mutexSocketYama;
//int nroNodoFinalizado, nroBloqueFinalizado;
int32_t headerIdFinalizado = 0;

void conectarAWorkerReduccionLocal(void *arg);

void enviarArchivoYama(int socketYama, char *archivoRequerido) {
	int bytesEnviados, i, j, k, h;
	int cantStrings = protocoloCantidadMensajes[TIPO_MSJ_PATH_ARCHIVO_TRANSFORMAR];
	char **arrayMensajes = malloc(cantStrings);
	arrayMensajes[0] = malloc(string_length(archivoRequerido) + 1);
	strcpy(arrayMensajes[0], archivoRequerido);
	char *mensajeSerializado = serializarMensaje(TIPO_MSJ_PATH_ARCHIVO_TRANSFORMAR, arrayMensajes, cantStrings);
	bytesEnviados = enviarMensaje(socketYama, mensajeSerializado);

	liberar_array(arrayMensajes, cantStrings);
}

void recibirTablaTransformacion(struct filaTransformacion *datosTransformacion, int socketYama, int cantBloquesArchivo) {
	int bytesEnviados, i, j, k, h;
	int cantMensajesXBloqueArchivo = 6;
	int cantStrings = cantMensajesXBloqueArchivo * cantBloquesArchivo;
	char **arrayTablaTransformacion = deserializarMensaje(socketYama, cantStrings);

	//recibir la tabla de transformación
	printf("\n ---------- Tabla de transformación ---------- \n");
	printf("\tNodo\tIP\t\tPuerto\tBloque\tBytes\t\tTemporal\n");
	printf("---------------------------------------------------------------------------------------------\n");
	for (i = 0, j = 0; i < cantBloquesArchivo; i++) {
		// cada msje es una fila de la tabla transformacion
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
		strcpy(datosTransformacion[i].temporal, arrayTablaTransformacion[j]);
		j++;
		printf("\t%d\t%s\t%d\t%d\t%d\t\t%s\n", datosTransformacion[i].nodo, datosTransformacion[i].ip, datosTransformacion[i].puerto, datosTransformacion[i].bloque, datosTransformacion[i].bytes, datosTransformacion[i].temporal);
	}
	printf("\n");

	liberar_array(arrayTablaTransformacion, cantStrings);
}

/*
 * headerId: TIPO_MSJ_TRANSFORMACION_OK - TIPO_MSJ_TRANSFORMACION_ERROR
 */
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

int envioFinReduccionGlobal(int headerId, int nroNodo) {
	int bytesEnviados, i, j, k, h;
	int cantMensajes = protocoloCantidadMensajes[headerId];

	//arma el array de strings para serializar
	char **arrayMensajes = malloc(sizeof(char*) * cantMensajes);

	//colecto nro de nodo
	char *nodoString = intToArrayZerosLeft(nroNodo, 4);
	arrayMensajes[0] = malloc(string_length(nodoString) + 1);
	strcpy(arrayMensajes[0], nodoString);

	//serializa los mensajes y los envía
	printf ("envio a Yama %d", headerId);
	char *mensajeSerializado = serializarMensaje(headerId, arrayMensajes, cantMensajes);
	pthread_mutex_lock(&mutexSocketYama);
	bytesEnviados = enviarMensaje(socketYama, mensajeSerializado);
	pthread_mutex_unlock(&mutexSocketYama);

	//libera todos los pedidos de malloc
	liberar_array(arrayMensajes, cantMensajes);
	return bytesEnviados;

}

void recibirTablaReduccionGlobal(struct filaReduccionGlobal *datosReduccionGlobal, int socketYama, int cantNodos) {
	int bytesEnviados, i, j, k, h;
	int cantMensajesXFila = 4;
	int cantStrings = cantMensajesXFila * cantNodos;
	char **arrayTablaReduccionGlobal = deserializarMensaje(socketYama, cantStrings);
	printf("\nmensaje recibido %s\n", *arrayTablaReduccionGlobal);

	//recibir la tabla de reduccion global
	printf("\n ---------- Tabla de reduccion global ---------- \n");
	printf("\tNodo\tIP\t\tPuerto\t\tTemporal\n");
	printf("---------------------------------------------------------------------------------------------\n");
	for (i = 0, j = 0; i < cantNodos; i++) {
		// cada msje es una fila de la tabla transformacion
		datosReduccionGlobal[i].nodo = atoi(arrayTablaReduccionGlobal[j]);
		j++;
		strcpy(datosReduccionGlobal[i].ip, arrayTablaReduccionGlobal[j]);
		j++;
		datosReduccionGlobal[i].puerto = atoi(arrayTablaReduccionGlobal[j]);
		j++;
		strcpy(datosReduccionGlobal[i].temporalReduccionGlobal, arrayTablaReduccionGlobal[j]);
		j++;
		printf("\t%d\t%s\t%d\t%s\n", datosReduccionGlobal[i].nodo, datosReduccionGlobal[i].ip, datosReduccionGlobal[i].puerto, datosReduccionGlobal[i].temporalReduccionGlobal);
	}
	printf("\n");

	liberar_array(arrayTablaReduccionGlobal, cantStrings);
}

int envioFinAlmacenamientoFinal(int headerId) {

	//serializa los mensajes y los envía
	printf ("envio a Yama %d", headerId);
	pthread_mutex_lock(&mutexSocketYama);
	int bytesEnviados = enviarHeaderSolo(socketYama, headerId);
	pthread_mutex_unlock(&mutexSocketYama);

	return bytesEnviados;

}

void recibirTablaReduccLocalYEnviarAWorker(int socketYama, int headerId) {
	int cantStrings, bytesEnviados, i, j, k, h, cantTemporales;
	char **arrayMensajesCantNodos = deserializarMensaje(socketYama, protocoloCantidadMensajes[headerId]);
	int cantNodos = atoi(arrayMensajesCantNodos[0]);
	free(arrayMensajesCantNodos[0]);
	free(arrayMensajesCantNodos);
	printf("cantNodos: %d\n", cantNodos);
	pthread_t hilosWorker[cantNodos];
	//struct datosWorker datosWorker[cantNodos];
	struct filaReduccionLocal datosReduccionLocal[cantNodos];

	puts("datos para la reducción local");
	for (i = 0; i < cantNodos; i++) {
		cantStrings = 4;
		char **arrayDatosReduccionLocal = deserializarMensaje(socketYama, cantStrings);

		datosReduccionLocal[i].nodo = atoi(arrayDatosReduccionLocal[0]);
		printf("nro nodo: %d\n", atoi(arrayDatosReduccionLocal[0]));

		strcpy(datosReduccionLocal[i].ip, arrayDatosReduccionLocal[1]);
		printf("ip nodo: %s\n", arrayDatosReduccionLocal[1]);

		datosReduccionLocal[i].puerto = atoi(arrayDatosReduccionLocal[2]);
		printf("puerto nodo: %d\n", atoi(arrayDatosReduccionLocal[2]));

		cantTemporales = atoi(arrayDatosReduccionLocal[3]);
		printf("cantidad de temporales: %d\n", cantTemporales);

		//		for (k = 0; k < 3; k++) {
		//			free(arrayDatosReduccionLocal[k]);
		//		}
		//		free(arrayDatosReduccionLocal);

		char **arrayTablaTemporalesTransformacion = deserializarMensaje(socketYama, cantTemporales);
		for (k = 0; k < cantTemporales; k++) {
			printf("temporal %d: %s\n", k, arrayTablaTemporalesTransformacion[k]);
			datosReduccionLocal[i].temporalesTransformacion[k] = malloc(string_length(arrayTablaTemporalesTransformacion[k]) + 1);
			strcpy(datosReduccionLocal[i].temporalesTransformacion[k], arrayTablaTemporalesTransformacion[k]);
		}

		//		for (k = 0; k < cantTemporales; k++) {
		//			free(arrayTablaTemporalesTransformacion[k]);
		//		}
		//		free(arrayTablaTemporalesTransformacion);

		//falta recibir el temporalReduccLocal
		char **arrayTemporalReduccionLocal = deserializarMensaje(socketYama, 1);
		printf("temporal recibido %s", arrayTemporalReduccionLocal[0]);
		strcpy(datosReduccionLocal[i].temporalReduccionLocal, arrayTemporalReduccionLocal[0]);
		printf("temporal guardado %s", datosReduccionLocal[i].temporalReduccionLocal);

		//		free(arrayTemporalReduccioLocal[0]);
		//		free(arrayTemporalReduccioLocal);

		pthread_create(&(hilosWorker[i]), NULL, conectarAWorkerReduccionLocal, &datosReduccionLocal[i]);
	}

	for (i = 0; i < cantNodos; i++) {
		//TODO: está bien hecho así??????????
		//TODO: no se quedaría esperando que terminen todas las transformaciones en vez de seguir??????
		pthread_join(hilosWorker[i], NULL);
	}
}

void conectarAWorkerReduccionLocal(void *arg) {
	FILE *fp;
	int i, j;
	struct filaReduccionLocal *datos = (struct filaReduccionLocal*) arg;
	int cantTemporalesTransformacion = sizeof(datos->temporalesTransformacion) / LARGO_TEMPORAL;

	//pasa el archivo a string para enviarlo al worker
	char *pathArchivo = string_from_format("%s", archivoReductor);
	printf("archivo reductor %s", archivoReductor);
	fp = fopen(pathArchivo, "r"); // read mode
	fseek(fp, 0, SEEK_END);
	long lengthArchivoReductor = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	char *reductorString = malloc(lengthArchivoReductor);
	if (reductorString) {
		fread(reductorString, 1, lengthArchivoReductor, fp);
	}
	fclose(fp);

	//se conecta a worker
	int socketWorker = conectarA(datos->ip, string_itoa(datos->puerto));

	int32_t headerIdWorker = handshakeWorker(socketWorker);
	if (headerIdWorker != TIPO_MSJ_HANDSHAKE_RESPUESTA_OK) {
		puts("Error de handshake con el worker");
	} else {
		puts("Conectado al worker");

		//envio de mensajes: reductor.py | cantidad de temporales transformacion | temp tranf 1 | .... | temp transf N | temp reduccLocal
		// el protocolo son 2 porq solo recibe el script reductor y la cant de temporales, recien ahi sabe cuanto leer despues
		int cantStringsASerializar = 1 + 1 + cantTemporalesTransformacion + 1;
		char **arrayMensajes = malloc(sizeof(char*) * cantStringsASerializar);

		//serializo reductor
		j = 0;
		arrayMensajes[j] = malloc(lengthArchivoReductor);
		strcpy(arrayMensajes[j], reductorString);
		free(reductorString);

		//serializo cantidad de temporales transformacion
		j++;
		char* cantTemporalesTransformacionString = intToArrayZerosLeft(cantTemporalesTransformacion, 4);
		arrayMensajes[j] = malloc(string_length(cantTemporalesTransformacionString) + 1);
		strcpy(arrayMensajes[j], cantTemporalesTransformacionString);

		//serializo temporales transformacion
		j++;
		for (i = 0; i < cantTemporalesTransformacion; i++) {
			arrayMensajes[j] = malloc(string_length(datos->temporalesTransformacion[i]) + 1);
			strcpy(arrayMensajes[j], datos->temporalesTransformacion[i]);
			j++;
		}

		//serializo temporal reduccion local
		arrayMensajes[j] = malloc(string_length(datos->temporalReduccionLocal) + 1);
		strcpy(arrayMensajes[j], datos->temporalReduccionLocal);

		//TIPO_MSJ_DATA_REDUCCION_LOCAL_WORKER: 1 MENSAJE
		char *mensajeSerializado = serializarMensaje(TIPO_MSJ_DATA_REDUCCION_LOCAL_WORKER, arrayMensajes, cantStringsASerializar);

		for (j = 0; j < cantStringsASerializar; j++) {
			free(arrayMensajes[j]);
		}
		free(arrayMensajes);

		// envio mensaje serializado
		printf("\nmensaje serializado: \n%s\n", mensajeSerializado);
		int temp = enviarMensaje(socketWorker, mensajeSerializado);
		printf ("bytes enviados a worker msje serializado %d", temp);

		// recibo rta. del worker
		int32_t headerIdWorker = deserializarHeader(socketWorker);
		printf ("header recibido de worker %d %s", headerIdWorker, protocoloMensajesPredefinidos[headerIdWorker]);
		int nroNodoFinalizado = datos->nodo;

		sleep(3);
		// Avisar a YAMA
		int bytesEnviadosMensaje = envioFinReduccionLocal(headerIdWorker, nroNodoFinalizado);
		pthread_t idHilo = pthread_self();

		//printf("Resultado transformación hilo %lu en nodo %d sobre bloque %d es: %s\n", idHilo, datosEnHilo->nodo, datosEnHilo->bloque, protocoloMensajesPredefinidos[headerId]);
		printf("Datos al final del hilo %lu: nodo %d, ip %s, puerto %d", idHilo, nroNodoFinalizado, datos->ip, datos->puerto);
		//printf("Bytes enviados mensaje en el hilo %lu: %d\n\n", idHilo, bytesEnviadosMensaje);
		puts("");
	}
}

void conectarAWorkerReduccionGlobal(void *arg) {
	FILE *fp;
	int i, j;
	pthread_t idHilo = pthread_self();
	struct filaReduccionGlobal *datos = (struct filaReduccionGlobal*) arg;
	printf("Se creó el hilo %lu\n", idHilo);

	//printf("Datos adentro del hilo %lu: nodo %d, bloque %d, ip %s, puerto %d, temporal %s \n", idHilo, datosEnHilo->nodo, datosEnHilo->bloque, datosEnHilo->ip, datosEnHilo->puerto, datosEnHilo->temporal);

	//pasa el archivo a string para enviarlo al worker
	char *pathArchivo = string_from_format("%s", archivoReductor);
	printf("archivo reductor %s", archivoReductor);
	fp = fopen(pathArchivo, "r"); // read mode
	fseek(fp, 0, SEEK_END);
	long lengthArchivoReductor = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	char *reductorString = malloc(lengthArchivoReductor);
	if (reductorString) {
		fread(reductorString, 1, lengthArchivoReductor, fp);
	}
	fclose(fp);

	//se conecta a worker
	int socketWorker = conectarA(datos->ip, string_itoa(datos->puerto));

	int32_t headerIdWorker = handshakeWorker(socketWorker);
	if (headerIdWorker != TIPO_MSJ_HANDSHAKE_RESPUESTA_OK) {
		puts("Error de handshake con el worker");
	} else {
		puts("Conectado al worker");

		//envio de mensajes: reductor.py | cantidad de temporales transformacion | temp tranf 1 | .... | temp transf N | temp reduccLocal
		// el protocolo son 2 porq solo recibe el script reductor y la cant de temporales, recien ahi sabe cuanto leer despues

		int cantNodos = 2; //REVISAR
		int cantStringsASerializar = 1 + 1 + cantNodos + 1;
		char **arrayMensajes = malloc(sizeof(char*) * cantStringsASerializar);

		//serializo reductor
		j = 0;
		arrayMensajes[j] = malloc(lengthArchivoReductor);
		strcpy(arrayMensajes[j], reductorString);
		free(reductorString);

		//serializo cantidad de nodos
		j++;
		char* cantNodosReduccionGlobal = intToArrayZerosLeft(cantNodos, 4);
		arrayMensajes[j] = malloc(string_length(cantNodosReduccionGlobal) + 1);
		strcpy(arrayMensajes[j], cantNodosReduccionGlobal);

		for (i = 0; i < cantNodos; i++) {
			//nroNodo = filasReduccGlobal[i].nodo;
			//printf("Nodo a serializar\nnroNodo %d\n", nroNodo);
			//printf("nombre %s - número %d - ip %s - puerto %d\n", getDatosGlobalesNodo(nroNodo)->nombre, getDatosGlobalesNodo(nroNodo)->numero, getDatosGlobalesNodo(nroNodo)->ip, getDatosGlobalesNodo(nroNodo)->puerto);

			//nro de nodo
			arrayMensajes[j] = malloc(4 + 1);
			strcpy(arrayMensajes[j], intToArrayZerosLeft(datos->nodo, 4));
			j++;

			//IP del nodo
			arrayMensajes[j] = malloc(string_length(datos->ip) + 1);
			strcpy(arrayMensajes[j], datos->ip);
			j++;

			//puerto del nodo
			arrayMensajes[j] = malloc(LARGO_PUERTO + 1);
			strcpy(arrayMensajes[j], intToArrayZerosLeft(datos->puerto, LARGO_PUERTO));
			j++;

			//temporal de la fila
			arrayMensajes[j] = malloc(string_length(datos->temporalReduccionGlobal) + 1);
			strcpy(arrayMensajes[j], datos->temporalReduccionGlobal);
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
		int temp = enviarMensaje(socketWorker, mensajeSerializado);
		printf ("bytes enviados a worker msje serializado %d", temp);

		// recibo rta. del worker
		int32_t headerIdWorker = deserializarHeader(socketWorker);
		printf ("header recibido de worker %d %s", headerIdWorker, protocoloMensajesPredefinidos[headerIdWorker]);
		int nroNodoFinalizado = datos->nodo;

		sleep(3);
		// Avisar a YAMA
		int bytesEnviadosMensaje = envioFinReduccionLocal(headerIdWorker, nroNodoFinalizado);
		pthread_t idHilo = pthread_self();

		//printf("Resultado transformación hilo %lu en nodo %d sobre bloque %d es: %s\n", idHilo, datosEnHilo->nodo, datosEnHilo->bloque, protocoloMensajesPredefinidos[headerId]);
		printf("Datos al final del hilo %lu: nodo %d, ip %s, puerto %d", idHilo, nroNodoFinalizado, datos->ip, datos->puerto);
		//printf("Bytes enviados mensaje en el hilo %lu: %d\n\n", idHilo, bytesEnviadosMensaje);
		puts("");
	}
}

int envioFinReduccionLocal(int headerId, int nroNodo) {
	int bytesEnviados, i, j, k, h;
	int cantMensajes = protocoloCantidadMensajes[headerId];

	//arma el array de strings para serializar
	char **arrayMensajes = malloc(sizeof(char*) * cantMensajes);

	//colecto nro de nodo
	char *nodoString = intToArrayZerosLeft(nroNodo, 4);
	arrayMensajes[0] = malloc(string_length(nodoString) + 1);
	strcpy(arrayMensajes[0], nodoString);

	//serializa los mensajes y los envía
	printf ("envio a Yama %d", headerId);
	char *mensajeSerializado = serializarMensaje(headerId, arrayMensajes, cantMensajes);
	pthread_mutex_lock(&mutexSocketYama);
	bytesEnviados = enviarMensaje(socketYama, mensajeSerializado);
	pthread_mutex_unlock(&mutexSocketYama);

	//libera todos los pedidos de malloc
	liberar_array(arrayMensajes, cantMensajes);
	return bytesEnviados;

}

void calcularMetricas(float tiempo) {
	printf("El job duró: %f \n", tiempo);
}

int32_t handshakeYama(int socketYama) {
	//envía el pedido de handshake al yama
	int cantStringsHandshake = protocoloCantidadMensajes[TIPO_MSJ_HANDSHAKE];
	char **arrayMensajesHandshake = malloc(sizeof(char*) * cantStringsHandshake);
	char *mensaje = intToArrayZerosLeft(NUM_PROCESO_MASTER, 4);
	arrayMensajesHandshake[0] = malloc(string_length(mensaje) + 1);
	strcpy(arrayMensajesHandshake[0], mensaje);
	char *mensajeSerializadoHS = serializarMensaje(TIPO_MSJ_HANDSHAKE, arrayMensajesHandshake, cantStringsHandshake);
	enviarMensaje(socketYama, mensajeSerializadoHS);
	free(arrayMensajesHandshake);

	//recibe la respuesta del yama
	return deserializarHeader(socketYama);

}

int32_t handshakeWorker(int socketWorker) {
	int cantStringsHandshake = protocoloCantidadMensajes[TIPO_MSJ_HANDSHAKE];
	char **arrayMensajesHandshake = malloc(sizeof(char*) * cantStringsHandshake);
	char *mensaje = intToArrayZerosLeft(NUM_PROCESO_MASTER, 4);
	arrayMensajesHandshake[0] = malloc(string_length(mensaje) + 1);
	strcpy(arrayMensajesHandshake[0], mensaje);
	char *mensajeSerializadoHS = serializarMensaje(TIPO_MSJ_HANDSHAKE, arrayMensajesHandshake, cantStringsHandshake);
	enviarMensaje(socketWorker, mensajeSerializadoHS);

	liberar_array(arrayMensajesHandshake, cantStringsHandshake);
	free(mensajeSerializadoHS);

	//recibe y devuelve la respuesta del worker
	return deserializarHeader(socketWorker);

}

int getCantBloquesArchivo(int socketYama, int cantMensajes) {
	char **arrayMensajes = deserializarMensaje(socketYama, cantMensajes);
	int cantBloquesArchivo = atoi(arrayMensajes[0]);
	free(arrayMensajes[0]);
	free(arrayMensajes);
	return cantBloquesArchivo;
}

int getCantNodos(int socketYama, int cantMensajes) {
	char **arrayMensajes = deserializarMensaje(socketYama, 1);
	int cantNodos = atoi(arrayMensajes[0]);
	free(arrayMensajes[0]);
	free(arrayMensajes);
	return cantNodos;
}

void conectarAWorkerTransformacion(void *arg) {
	FILE *fp;
	int i, j;
	pthread_t idHilo = pthread_self();
	struct filaTransformacion *datosEnHilo = (struct filaTransformacion*) arg;
	printf("Se creó el hilo %lu\n", idHilo);

	printf("Datos adentro del hilo %lu: nodo %d, bloque %d, ip %s, puerto %d, temporal %s \n", idHilo, datosEnHilo->nodo, datosEnHilo->bloque, datosEnHilo->ip, datosEnHilo->puerto, datosEnHilo->temporal);

	//pasa el archivo a string para enviarlo al worker
	char *pathArchivo = string_from_format("%s", archivoTransformador);
	fp = fopen(pathArchivo, "r"); // read mode
	fseek(fp, 0, SEEK_END);
	long lengthArchivoTransformador = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	char *transformadorString = malloc(lengthArchivoTransformador);
	if (transformadorString) {
		fread(transformadorString, 1, lengthArchivoTransformador, fp);
	} else {
		perror("Malloc del transformador");
	}
	fclose(fp);
	//	if (strcmp(datosEnHilo->ip, "127.0.0.1") || datosEnHilo->bloque == 76)
	//	printf("datos -1 adentro del hilo: nodo %d, bloque %d, ip %s, puerto %d, temporal %s \n", datosEnHilo->nodo, datosEnHilo->bloque, datosEnHilo->ip, datosEnHilo->puerto, datosEnHilo->temporal);

	// Abrir conexión a Worker
	//	pthread_mutex_lock(&mutexSocketWorker);
	int socketWorker = conectarA(datosEnHilo->ip, string_itoa(datosEnHilo->puerto));
	if (socketWorker < 0) {
		puts("No se pudo conectar al worker");
	}
	//	pthread_mutex_unlock(&mutexSocketWorker);

	printf("socket worker hilo %lu: %d\n", idHilo, socketWorker);
	//	if (strcmp(datosEnHilo->ip, "127.0.0.1") || datosEnHilo->bloque == 76)
	//	printf("datos 0 adentro del hilo: nodo %d, bloque %d, ip %s, puerto %d, temporal %s \n", datosEnHilo->nodo, datosEnHilo->bloque, datosEnHilo->ip, datosEnHilo->puerto, datosEnHilo->temporal);

	int32_t headerIdWorker = handshakeWorker(socketWorker);
	if (headerIdWorker != TIPO_MSJ_HANDSHAKE_RESPUESTA_OK) {
		puts("Error de handshake con el worker");
	} else {
		puts("Conectado al worker");


		int cantStringsASerializar = 4;	//script de transformación, bloque, bytes y temporal

		//serializo script transformacion
		char **arrayMensajes = malloc(sizeof(char*) * cantStringsASerializar);
		j = 0;
		arrayMensajes[j] = malloc(string_length(transformadorString) + 1);
		strcpy(arrayMensajes[j], transformadorString);
		j++;

		arrayMensajes[j] = malloc(4 + 1);
		strcpy(arrayMensajes[j], intToArrayZerosLeft(datosEnHilo->bloque, 4));
		j++;

		arrayMensajes[j] = malloc(8 + 1);
		strcpy(arrayMensajes[j], intToArrayZerosLeft(datosEnHilo->bytes, 8));
		j++;

		arrayMensajes[j] = malloc(string_length(datosEnHilo->temporal) + 1);
		strcpy(arrayMensajes[j], datosEnHilo->temporal);

		//TIPO_MSJ_DATA_TRANSFORMACION_WORKER: 4 MENSAJES
		char *mensajeSerializado = serializarMensaje(TIPO_MSJ_DATA_TRANSFORMACION_WORKER, arrayMensajes, cantStringsASerializar);
		liberar_array(arrayMensajes, cantStringsASerializar);
		int cantBytesEnviados = enviarMensaje(socketWorker, mensajeSerializado);

		// Recibir resultado del Worker
		int32_t headerId = deserializarHeader(socketWorker);
		printf("header recibido por parte del worker en hilo %lu: %d\n", idHilo, headerId);
		// Cerrar conexión a Worker
		cerrarCliente(socketWorker);

		// Avisar a YAMA
		int bytesEnviadosMensaje = envioFinTransformacion(headerId, datosEnHilo->nodo, datosEnHilo->bloque);
		//printf("Resultado transformación hilo %lu en nodo %d sobre bloque %d es: %s\n", idHilo, datosEnHilo->nodo, datosEnHilo->bloque, protocoloMensajesPredefinidos[headerId]);
		printf("Datos al final del hilo %lu: nodo %d, bloque %d, ip %s, puerto %d, temporal %s \n", idHilo, datosEnHilo->nodo, datosEnHilo->bloque, datosEnHilo->ip, datosEnHilo->puerto, datosEnHilo->temporal);
		//printf("Bytes enviados mensaje en el hilo %lu: %d\n\n", idHilo, bytesEnviadosMensaje);
		puts("");
	}
}

void conectarAWorkerAlmacenamientoFinal (void *arg) {
	pthread_t idHilo = pthread_self();
	struct filaTransformacion *datosEnHilo = (struct filaTransformacion*) arg;

	// Abrir conexión a Worker
		//	pthread_mutex_lock(&mutexSocketWorker);
		int socketWorker = conectarA(datosEnHilo->ip, string_itoa(datosEnHilo->puerto));
		if (socketWorker < 0) {
			puts("No se pudo conectar al worker");
		}
		//	pthread_mutex_unlock(&mutexSocketWorker);

		printf("socket worker hilo %lu: %d\n", idHilo, socketWorker);
		//	if (strcmp(datosEnHilo->ip, "127.0.0.1") || datosEnHilo->bloque == 76)
		//	printf("datos 0 adentro del hilo: nodo %d, bloque %d, ip %s, puerto %d, temporal %s \n", datosEnHilo->nodo, datosEnHilo->bloque, datosEnHilo->ip, datosEnHilo->puerto, datosEnHilo->temporal);

		int32_t headerIdWorker = handshakeWorker(socketWorker);
		if (headerIdWorker != TIPO_MSJ_HANDSHAKE_RESPUESTA_OK) {
			puts("Error de handshake con el worker");
		} else {
			puts("Conectado al worker");
			//enviar datos
		}

		// Avisar a Yama
		envioFinAlmacenamientoFinal(headerIdWorker);
}

int main(int argc, char *argv[]) {
	clock_t start = clock();
	int i, j, k, h;
	t_log* logMASTER;
	logMASTER = log_create("logMASTER.log", "MASTER", true, LOG_LEVEL_TRACE); //creo el logger, mostrando por pantalla
	uint32_t preparadoEnviarYama = 1;
	int32_t headerIdYama;

	//TODO !!!!!!!!! Creo que esto está mal, no se está haciendo un malloc para los char*
	//por ende está usando cualquier parte de la memoria
	//probable segmentation fault
	archivoTransformador = argv[1];
	archivoReductor = argv[2];
	archivoRequerido = argv[3];
	archivoDestino = argv[4];
	//	strcpy(archivoTransformador, argv[1]);
	//	strcpy(archivoReductor, argv[2]);
	//	strcpy(archivoRequerido, argv[3]);
	//	strcpy(archivoDestino, argv[4]);

	log_info(logMASTER, "Iniciando proceso MASTER");
	printf("\n*** Proceso Master ***\n");

	if (pthread_mutex_init(&mutexSocketYama, NULL) != 0) {
		printf("\n mutex init failed\n");
		return 1;
	}
	if (pthread_mutex_init(&mutexSocketWorker, NULL) != 0) {
		printf("\n mutex init failed\n");
		return 1;
	}

	if (argc < 5) {
		puts("Error. Faltan parámetros en la ejecución del proceso.\n");
		return EXIT_FAILURE;
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
		//preparadoEnviarYama = 0;
		puts("No se pudo conectar a YAMA");
		return EXIT_FAILURE;
	}

	// Inicia comunicación con YAMA enviando el HANDSHAKE
	headerIdYama = handshakeYama(socketYama);
	if (headerIdYama != TIPO_MSJ_HANDSHAKE_RESPUESTA_OK) {
		puts("Error de handshake con YAMA");
	} else {
		puts("Conectado a YAMA");
		//envía a yama el archivo con el que quiere trabajar
		enviarArchivoYama(socketYama, archivoRequerido);
		//fcntl(socketYama, F_SETFL, O_NONBLOCK);
		char idString[LARGO_STRING_HEADER_ID + 1];
		int cantBytesRecibidos, cantBloquesArchivo, cantNodos;

		while (masterCorriendo == 0) {
			headerIdYama = deserializarHeader(socketYama);
			printf("\nheaderId recibido: %d - ", headerIdYama);
			printf("headerId msje recibido: %s\n", protocoloMensajesPredefinidos[headerIdYama]);

			//			if (recv(socketYama, idString, LARGO_STRING_HEADER_ID, 0) == -1) {
			//				puts("No se recibió el header YAMA");
			//				sleep(2);
			//				headerIdYama = 0;
			//			} else {
			//				idString[LARGO_STRING_HEADER_ID] = '\0';
			//				headerIdYama = atoi(idString);
			//				printf("headerIdYama: %d - %s\n\n", headerIdYama, protocoloMensajesPredefinidos[headerIdYama]);
			//			}

			switch (headerIdYama) {

			case TIPO_MSJ_TABLA_TRANSFORMACION: {
				cantBloquesArchivo = getCantBloquesArchivo(socketYama, protocoloCantidadMensajes[headerIdYama]);
				pthread_t hilosWorker[cantBloquesArchivo];
				//struct datosWorker datos[cantBloquesArchivo];
				struct filaTransformacion datosTransformacion[cantBloquesArchivo];
				recibirTablaTransformacion(datosTransformacion, socketYama, cantBloquesArchivo);
				for (i = 0; i < cantBloquesArchivo; i++) {
					sleep(2);
					// por cada tarea se crea un hilo para conectarse al worker q corresponda, se le pasa la fila de la tabla recibida
					pthread_create(&(hilosWorker[i]), NULL, conectarAWorkerTransformacion, &datosTransformacion[i]);
				}

				for (i = 0; i < cantBloquesArchivo; i++) {
					//TODO: está bien hecho así??????????
					//TODO: no se quedaría esperando que terminen todas las transformaciones en vez de seguir??????
					pthread_join(hilosWorker[i], NULL);
				}
			}
			break;

			// 4º) Etapa de Reducción Local: crear hilo, conectarse al worker, esperar y notificar a YAMA
			case TIPO_MSJ_TABLA_REDUCCION_LOCAL:
				recibirTablaReduccLocalYEnviarAWorker(socketYama, headerIdYama);
				break;

			// 5º) Etapa de Reducción Global: crear hilo, conectarse al worker, esperar y notificar a YAMA
			case TIPO_MSJ_TABLA_REDUCCION_GLOBAL:{
				cantNodos = getCantNodos(socketYama, protocoloCantidadMensajes[headerIdYama]);
				printf ("cantNodoS %d", cantNodos);
				pthread_t hiloWorkerReduccionGlobal;
				//struct datosWorkerReduccionGlobal datos1[cantNodos];
				struct filaReduccionGlobal datosReduccionGlobal[cantNodos];

				recibirTablaReduccionGlobal(datosReduccionGlobal, socketYama, cantNodos);

				char **arrayDatosReduccionGlobal = deserializarMensaje(socketYama, 1);
			char * temporalGlobal;
			strcpy(temporalGlobal, arrayDatosReduccionGlobal[0]);
			free(arrayDatosReduccionGlobal[0]);
			free(arrayDatosReduccionGlobal);
			printf("temporal global: %s\n", temporalGlobal);

			pthread_create(&hiloWorkerReduccionGlobal, NULL, conectarAWorkerReduccionGlobal, &datosReduccionGlobal);
					pthread_join(hiloWorkerReduccionGlobal, NULL);


			}
			break;

			// 6º) Etapa de Almacenado Final: crear hilo, conectarse al worker, esperar y notificar a YAMA
			case TIPO_MSJ_TABLA_ALMACENAMIENTO_FINAL: {
				pthread_t hiloWorkerAlmacenamientoFinal;
				struct filaAlmacenamientoFinal datosAlmacenamientoFinal;

				pthread_create(&hiloWorkerAlmacenamientoFinal, NULL, conectarAWorkerAlmacenamientoFinal, &datosAlmacenamientoFinal);
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
