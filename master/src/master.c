/*
 ============================================================================
 Name        : master.c
 Author      : Grupo 1234
 Description : Proceso Master
 ============================================================================
 */

// ================================================================ //
// Master ejecuta una tarea, sobre un archivo.
// Se conecta a YAMA para pedir instrucciones,
// y a los workers (usando hilos) para mandar instrucciones
// Puede haber varios master corriendo al mismo tiempo.
// ================================================================ //
#include "../../utils/includes.h"
#include <unistd.h>
#include <time.h>

enum keys {
	YAMA_IP, YAMA_PUERTO, NODO_IP, NODO_PUERTO
};

char* keysConfigMaster[] = { "YAMA_IP", "YAMA_PUERTO", "NODO_IP", "NODO_PUERTO",
NULL };

char* datosConfigMaster[4];

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
	char temporalReduccLocal[LARGO_TEMPORAL];
};

pthread_mutex_t mutexSocketWorker, mutexSocketYama;
//int nroNodoFinalizado, nroBloqueFinalizado;
int32_t headerIdFinalizado = 0;

void enviarArchivoYama(int socketYama, char *archivoRequerido) {
	int bytesEnviados, i, j, k, h;
	int cantStrings = protocoloCantidadMensajes[TIPO_MSJ_PATH_ARCHIVO_TRANSFORMAR];
	char **arrayMensajes = malloc(cantStrings);
	arrayMensajes[0] = malloc(string_length(archivoRequerido) + 1);
	strcpy(arrayMensajes[0], archivoRequerido);
	char *mensajeSerializado = serializarMensaje(TIPO_MSJ_PATH_ARCHIVO_TRANSFORMAR, arrayMensajes, cantStrings);
	bytesEnviados = enviarMensaje(socketYama, mensajeSerializado);
	for (i = 0; i < cantStrings; i++) {
		free(arrayMensajes[i]);
	}
	free(arrayMensajes);
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

	for (i = 0; i < cantStrings; i++) {
		free(arrayTablaTransformacion[i]);
	}
	free(arrayTablaTransformacion);
}

/*
 * headerId: TIPO_MSJ_TRANSFORMACION_OK - TIPO_MSJ_TRANSFORMACION_ERROR
 */
int envioFinTransformacion(int headerId, int nroNodo, int nroBloque) {
	int bytesEnviados, i, j, k, h;
	int cantMensajes = protocoloCantidadMensajes[headerId];

	//arma el array de strings para serializar
	char **arrayMensajes = malloc(sizeof(char*) * cantMensajes);

	char *nodoString = intToArrayZerosLeft(nroNodo, 4);
	arrayMensajes[0] = malloc(string_length(nodoString) + 1);
	strcpy(arrayMensajes[0], nodoString);

	char *bloqueString = intToArrayZerosLeft(nroBloque, 4);
	arrayMensajes[1] = malloc(string_length(bloqueString) + 1);
	strcpy(arrayMensajes[1], bloqueString);

	//serializa los mensajes y los envía
	char *mensajeSerializado = serializarMensaje(headerId, arrayMensajes, cantMensajes);
	pthread_mutex_lock(&mutexSocketYama);
	bytesEnviados = enviarMensaje(socketYama, mensajeSerializado);
	pthread_mutex_unlock(&mutexSocketYama);
	//libera todos los pedidos de malloc
	for (i = 0; i < cantMensajes; i++) {
		free(arrayMensajes[i]);
	}
	free(arrayMensajes);
	return bytesEnviados;
}

void recibirTablaReduccLocalYEnviarAWorker(int socketYama, int headerId) {
//	int cantStrings, bytesEnviados, i, j, k, h, cantTemporales;
//	char **arrayMensajesCantNodos = deserializarMensaje(socketYama, protocoloCantidadMensajes[headerId]);
//	int cantNodos = atoi(arrayMensajesCantNodos[0]);
//	free(arrayMensajesCantNodos[0]);
//	free(arrayMensajesCantNodos);
//	printf("cantNodos: %d\n", cantNodos);
//	pthread_t hilosWorker[cantNodos];
//	//struct datosWorker datosWorker[cantNodos];
//	struct filaReduccionLocal datosReduccLocal[cantNodos];
//
//	puts("datos para la reducción local");
//	for (i = 0; i < cantNodos; i++) {
//		cantStrings = 4;
//		char **arrayDatosNodoReduccLocal = deserializarMensaje(socketYama, cantStrings);
//
//		datosReduccLocal[i].nodo = atoi(arrayDatosNodoReduccLocal[0]);
//		printf("nro nodo: %d\n", atoi(arrayDatosNodoReduccLocal[0]));
//
//		strcpy(datosReduccLocal[i].ip, arrayDatosNodoReduccLocal[1]);
//		printf("ip nodo: %s\n", arrayDatosNodoReduccLocal[1]);
//
//		datosReduccLocal[i].puerto = atoi(arrayDatosNodoReduccLocal[2]);
//		printf("puerto nodo: %d\n", atoi(arrayDatosNodoReduccLocal[2]));
//
//		cantTemporales = atoi(arrayDatosNodoReduccLocal[3]);
//		printf("cantidad de temporales: %d\n", cantTemporales);
//
//		for (k = 0; k < 3; k++) {
//			free(arrayDatosNodoReduccLocal[k]);
//		}
//		free(arrayDatosNodoReduccLocal);
//
//		char **arrayTablaTemporalesTransformacion = deserializarMensaje(socketYama, cantTemporales);
//		for (k = 0; k < cantTemporales; k++) {
//			printf("temporal %d: %s\n", k, arrayTablaTemporalesTransformacion[k]);
//			strcpy(datosReduccLocal[i].temporalesTransformacion[k], arrayTablaTemporalesTransformacion[k]);
//		}
//		for (k = 0; k < cantTemporales; k++) {
//			free(arrayTablaTemporalesTransformacion[k]);
//		}
//		free(arrayTablaTemporalesTransformacion);
//		//falta recibir el temporalReduccLocal
//		char **arrayTemporalReduccLocal = deserializarMensaje(socketYama, 1);
//		strcpy(datosReduccLocal[i].temporalReduccLocal, arrayTemporalReduccLocal[0]);
//		free(arrayTemporalReduccLocal[0]);
//		free(arrayTemporalReduccLocal);
//
//		pthread_create(&(hilosWorker[i]), NULL, conectarAWorkerReduccionLocal, &datosReduccLocal[i]);
//	}
}

void pruebaEnviarFinReduccLocal(int socketYama, int nroNodo) {
	int cantStrings, bytesEnviados, i, j, k, h;

	puts("\n\nenvío de nodo que terminó la reducción local");
	printf("nroNodo: %d\n", nroNodo);
	cantStrings = protocoloCantidadMensajes[TIPO_MSJ_REDUCC_LOCAL_OK];

	char **arrayMensajes = malloc(sizeof(char*) * cantStrings);

	char *nodoString = intToArrayZerosLeft(nroNodo, 4);
	arrayMensajes[0] = malloc(string_length(nodoString) + 1);
	strcpy(arrayMensajes[0], nodoString);

	char *mensajeSerializado = serializarMensaje(TIPO_MSJ_REDUCC_LOCAL_OK, arrayMensajes, cantStrings);
	bytesEnviados = enviarMensaje(socketYama, mensajeSerializado);
	for (i = 0; i < cantStrings; i++) {
		free(arrayMensajes[i]);
	}
	free(arrayMensajes);
	puts("presionar ENTER en pruebaEnviarFinReduccLocal");
	getchar();

}

void pruebas(int socketYama, char *archivoRequerido) {
	int bytesEnviados = 0, i, j, k, h;

	//envío de fin transformación OK, todos los del nodo 2, 1 del nodo 1 y 1 del nodo 3
	//	pruebaEnvioFinTransformacionOk(socketYama, 2, 36);
	//	pruebaEnvioFinTransformacionOk(socketYama, 1, 33);
	//	pruebaEnvioFinTransformacionOk(socketYama, 3, 55);
	//	pruebaEnvioFinTransformacionOk(socketYama, 2, 65);
	//
	//	pruebaRecibirTablaRedLocal(socketYama); //para el nodo 3
	//
	//	//envío de fin transformación OK nodo 1
	//	pruebaEnvioFinTransformacionOk(socketYama, 1, 88);
	//	pruebaEnviarFinReduccLocal(socketYama, 3);
	//	pruebaEnvioFinTransformacionOk(socketYama, 1, 74);
	//
	//	pruebaRecibirTablaRedLocal(socketYama); //para el nodo 2
	//
	//	//envía mensaje de fin de reducción local nodo 2
	//
	//	pruebaRecibirTablaRedLocal(socketYama); //para el nodo 1
	//	pruebaEnviarFinReduccLocal(socketYama, 2);
	//	pruebaEnviarFinReduccLocal(socketYama, 1);

	//recibe la tabla de reducción global

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

int getCantBloquesArchivo(int socketYama, int cantMensajes) {
	char **arrayMensajes = deserializarMensaje(socketYama, cantMensajes);
	int cantBloquesArchivo = atoi(arrayMensajes[0]);
	free(arrayMensajes[0]);
	free(arrayMensajes);
	return cantBloquesArchivo;
}

int getCantNodos(int socketYama, int cantMensajes) {
	char **arrayMensajes = deserializarMensaje(socketYama, cantMensajes);
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
	long length = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	char *transformadorString = malloc(length);
	if (transformadorString) {
		fread(transformadorString, 1, length, fp);
	} else {
		perror("Malloc del transformador");
	}
	fclose(fp);
	//	if (strcmp(datosEnHilo->ip, "127.0.0.1") || datosEnHilo->bloque == 76)
	//	printf("datos -1 adentro del hilo: nodo %d, bloque %d, ip %s, puerto %d, temporal %s \n", datosEnHilo->nodo, datosEnHilo->bloque, datosEnHilo->ip, datosEnHilo->puerto, datosEnHilo->temporal);

	// Abrir conexión a Worker
//	pthread_mutex_lock(&mutexSocketWorker);
	int socketWorker = conectarA(datosEnHilo->ip, string_itoa(datosEnHilo->puerto));
//	pthread_mutex_unlock(&mutexSocketWorker);

	printf("socket worker hilo %lu: %d\n", idHilo, socketWorker);
	//	if (strcmp(datosEnHilo->ip, "127.0.0.1") || datosEnHilo->bloque == 76)
	//	printf("datos 0 adentro del hilo: nodo %d, bloque %d, ip %s, puerto %d, temporal %s \n", datosEnHilo->nodo, datosEnHilo->bloque, datosEnHilo->ip, datosEnHilo->puerto, datosEnHilo->temporal);

	int cantStringsASerializar = 4;	//código de transformación, bloque, bytes y temporal
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
	for (j = 0; j < cantStringsASerializar; j++) {
		free(arrayMensajes[j]);
	}
	free(arrayMensajes);
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

void conectarAWorkerReduccionLocal(void *arg) {
	//	FILE *fp;
	//	int i, j;
	//	struct filaReduccLocal *datos = (struct filaReduccLocal*) arg;
	//	int cantTemporalesTransformacion = sizeof(datos->temporalesTransformacion) / LARGO_TEMPORAL;
	//
	////pasa el archivo a string para enviarlo al worker
	////char *archivo = "reductor.py";
	//	char *pathArchivo = string_from_format("../..%s", archivoReductor);
	//	fp = fopen(pathArchivo, "r"); // read mode
	//	fseek(fp, 0, SEEK_END);
	//	long length = ftell(fp);
	//	fseek(fp, 0, SEEK_SET);
	//	char *reductorString = malloc(length);
	//	if (reductorString) {
	//		fread(reductorString, 1, length, fp);
	//	}
	//	fclose(fp);
	//
	////int socketWorker = conectarA(datos->ip, string_itoa(datos->puerto));
	//	int socketWorker = conectarA("127.0.0.1", "5300");
	//
	////reductor.py|cantidad de temporales transformacion | temp tranf 1 | .... | temp transf N | temp reduccLocal
	//	int cantStringsASerializar = 1 + 1 + cantTemporalesTransformacion + 1;
	//	char **arrayMensajes = malloc(sizeof(char*) * cantStringsASerializar);
	//
	//	j = 0;
	//	arrayMensajes[j] = malloc(string_length(reductorString) + 1);
	//	strcpy(arrayMensajes[j], reductorString);
	//	free(reductorString);
	//	j++;
	//	char* cantTemporalesTransformacionString = intToArrayZerosLeft(cantTemporalesTransformacion, 4);
	//	arrayMensajes[j] = malloc(string_length(cantTemporalesTransformacionString) + 1);
	//	strcpy(arrayMensajes[j], cantTemporalesTransformacionString);
	//	j++;
	//	for (i = 0; i < cantTemporalesTransformacion; i++) {
	//		arrayMensajes[j] = malloc(string_length(datos->temporalesTransformacion[i]) + 1);
	//		strcpy(arrayMensajes[j], datos->temporalesTransformacion[i]);
	//		j++;
	//	}
	//	arrayMensajes[j] = malloc(string_length(datos->temporalReduccLocal) + 1);
	//	strcpy(arrayMensajes[j], datos->temporalReduccLocal);
	//
	////TIPO_MSJ_DATA_REDUCCION_LOCAL_WORKER: 1 MENSAJE
	//	char *mensajeSerializado = serializarMensaje(TIPO_MSJ_DATA_REDUCCION_LOCAL_WORKER, arrayMensajes, cantStringsASerializar);
	//	for (j = 0; j < cantStringsASerializar; j++) {
	//		free(arrayMensajes[j]);
	//	}
	//	free(arrayMensajes);
	////printf("\nmensaje serializado: \n%s\n", mensajeSerializado);
	//	enviarMensaje(socketWorker, mensajeSerializado);
	//
	//	int32_t headerIdWorker = deserializarHeader(socketWorker);
	//	nroNodoFinalizado = datos->nodo;
	//	headerIdFinalizado = headerIdWorker;
	// Avisar a Yama
	//TIPO_MSJ_REDUCC_LOCAL_OK,
	//TIPO_MSJ_REDUCC_LOCAL_ERROR,
	printf("worker");
}

void conectarAWorkerReduccionGlobal(void *arg) {

	// Avisar a Yama
	//TIPO_MSJ_REDUCC_GLOBAL_OK,
	//TIPO_MSJ_REDUCC_GLOBAL_ERROR,

}

void conectarAWorkerAlmacenadoFinal(void *arg) {

	// Avisar a Yama
	//TIPO_MSJ_ALM_FINAL_OK,
	//TIPO_MSJ_ALM_FINAL_ERROR,

}

int main(int argc, char *argv[]) {
	clock_t start = clock();
	int i, j, k, h;
	t_log* logMASTER;
	logMASTER = log_create("logMASTER.log", "MASTER", false, LOG_LEVEL_TRACE); //creo el logger, sin mostrar por pantalla
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

	//	strcpy(archivoTransformador, argv[1]);
	//	strcpy(archivoReductor, argv[2]);
	//	strcpy(archivoRequerido, argv[3]);
	//	strcpy(archivoDestino, argv[4]);

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
		int cantBytesRecibidos, cantBloquesArchivo;

		while (masterCorriendo == 0) {
			headerIdYama = deserializarHeader(socketYama);

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
				struct datosWorker datos[cantBloquesArchivo];
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
				masterCorriendo = 1;
			}
				break;
			case TIPO_MSJ_TABLA_REDUCCION_LOCAL:
				//recibirTablaReduccLocalYEnviarAWorker(socketYama, headerIdYama);
				break;
			case TIPO_MSJ_TABLA_REDUCCION_GLOBAL:
				break;
			case TIPO_MSJ_TABLA_ALMACENAMIENTO_FINAL:
				break;
			} // fin switch

			/*
			 int nodo = nroNodoFinalizado;
			 int bloque = nroBloqueFinalizado;
			 headerIdFinalizado = 0;	//lo pone en 0 así en la próxima vuelta no hace nada si ningún hilo lo modificó
			 printf("nodo %d finalizado, bloque %d - %s\n", nroNodoFinalizado, nroBloqueFinalizado, protocoloMensajesPredefinidos[headerIdFinalizado]);
			 enviar a yama el mensaje serializado
			 char **arrayMensajes = malloc(sizeof(char*) * protocoloCantidadMensajes[headerIdFinalizado]);
			 char *nodoString = intToArrayZerosLeft(nodo, 4);
			 arrayMensajes[0] = malloc(string_length(nodoString) + 1);
			 strcpy(arrayMensajes[0], nodoString);
			 char *bloqueString = intToArrayZerosLeft(bloque, 4);
			 arrayMensajes[1] = malloc(string_length(bloqueString) + 1);
			 strcpy(arrayMensajes[1], bloqueString);
			 char *mensajeSerializado = serializarMensaje(headerIdFinalizado, arrayMensajes, protocoloCantidadMensajes[headerIdFinalizado]);
			 printf("mensajes serializado: %s\n", mensajeSerializado);
			 enviarMensaje(socketYama, mensajeSerializado);
			 */
		} // fin while

	} // fin if

	// 4º) Etapa de Reducción Local: crear hilo, conectarse al worker, esperar y notificar a YAMA
	// 5º) Etapa de Reducción Global: crear hilo, conectarse al worker, esperar y notificar a YAMA

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
