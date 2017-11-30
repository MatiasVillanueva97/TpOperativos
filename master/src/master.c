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

enum keys {
	YAMA_IP, YAMA_PUERTO, NODO_IP, NODO_PUERTO
};
char* keysConfigMaster[] = { "YAMA_IP", "YAMA_PUERTO", "NODO_IP", "NODO_PUERTO",
NULL };

char* datosConfigMaster[4];

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

struct filaReduccLocal {
	int nodo;
	char ip[LARGO_IP];
	int puerto;
	char *temporalesTransformacion[LARGO_TEMPORAL];
	char temporalReduccLocal[LARGO_TEMPORAL];
};

pthread_mutex_t mutex;
int nroNodoFinalizado, nroBloqueFinalizado;
int32_t headerIdFinalizado = EN_DESUSO;

void* conectarAWorkerTransformacion(void *arg);
void* conectarAWorkerReduccLocal(void *arg);

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

void recibirTablaTransformaciones(struct filaTransformacion *datosTransformacion, int socketYama, int cantBloquesArchivo) {
	int bytesEnviados, i, j, k, h;
	const int cantMensajesXBloqueArchivo = 6;
	int cantStrings = cantMensajesXBloqueArchivo * cantBloquesArchivo;
	char **arrayTablaTransformacion = deserializarMensaje(socketYama, cantStrings);

	printf("cantBloquesArchivo: %d\n", cantBloquesArchivo);
	printf("cantStrings: %d\n", cantStrings);
	//recibir la tabla de transformación
	puts("datos de la tabla de transformación\n-------------------------------------");
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
		puts("");
		printf("nodo %d - ip %s - puerto %d - bloque %d - bytes %d - temporal %s\n", datosTransformacion[i].nodo, datosTransformacion[i].ip, datosTransformacion[i].puerto, datosTransformacion[i].bloque, datosTransformacion[i].bytes, datosTransformacion[i].temporal);
	}

	for (i = 0; i < cantStrings; i++) {
		free(arrayTablaTransformacion[i]);
	}
	free(arrayTablaTransformacion);
	puts("tabla de transformación recibida con éxito\n---------------------------------------\n\n");
}

/*
 * headerId: TIPO_MSJ_TRANSFORMACION_OK - TIPO_MSJ_TRANSFORMACION_ERROR
 */
void envioFinTransformacion(int socketYama, int headerId, int nroNodo, int nroBloque) {
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
	bytesEnviados = enviarMensaje(socketYama, mensajeSerializado);

	//libera todos los pedidos de malloc
	for (i = 0; i < cantMensajes; i++) {
		free(arrayMensajes[i]);
	}
	free(arrayMensajes);
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
	struct filaReduccLocal datosReduccLocal[cantNodos];

	puts("datos para la reducción local");
	for (i = 0; i < cantNodos; i++) {
		cantStrings = 4;
		char **arrayDatosNodoReduccLocal = deserializarMensaje(socketYama, cantStrings);

		datosReduccLocal[i].nodo = atoi(arrayDatosNodoReduccLocal[0]);
		printf("nro nodo: %d\n", atoi(arrayDatosNodoReduccLocal[0]));

		strcpy(datosReduccLocal[i].ip, arrayDatosNodoReduccLocal[1]);
		printf("ip nodo: %s\n", arrayDatosNodoReduccLocal[1]);

		datosReduccLocal[i].puerto = atoi(arrayDatosNodoReduccLocal[2]);
		printf("puerto nodo: %d\n", atoi(arrayDatosNodoReduccLocal[2]));

		cantTemporales = atoi(arrayDatosNodoReduccLocal[3]);
		printf("cantidad de temporales: %d\n", cantTemporales);

		for (k = 0; k < 3; k++) {
			free(arrayDatosNodoReduccLocal[k]);
		}
		free(arrayDatosNodoReduccLocal);

		char **arrayTablaTemporalesTransformacion = deserializarMensaje(socketYama, cantTemporales);
		for (k = 0; k < cantTemporales; k++) {
			printf("temporal %d: %s\n", k, arrayTablaTemporalesTransformacion[k]);
			strcpy(datosReduccLocal[i].temporalesTransformacion[k], arrayTablaTemporalesTransformacion[k]);
		}
		for (k = 0; k < cantTemporales; k++) {
			free(arrayTablaTemporalesTransformacion[k]);
		}
		free(arrayTablaTemporalesTransformacion);
		//falta recibir el temporalReduccLocal
		char **arrayTemporalReduccLocal = deserializarMensaje(socketYama, 1);
		strcpy(datosReduccLocal[i].temporalReduccLocal, arrayTemporalReduccLocal[0]);
		free(arrayTemporalReduccLocal[0]);
		free(arrayTemporalReduccLocal);

		pthread_create(&(hilosWorker[i]), NULL, conectarAWorkerReduccLocal, &datosReduccLocal[i]);

	}
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

int main(int argc, char *argv[]) {
	int i, j, k, h;
	t_log* logMASTER;
	logMASTER = log_create("logMASTER.log", "MASTER", false, LOG_LEVEL_TRACE); //creo el logger, sin mostrar por pantalla
	uint32_t preparadoEnviarYama = 1;
	int32_t headerIdYama;

	log_info(logMASTER, "Iniciando proceso MASTER");
	printf("\n*** Proceso Master ***\n");

	if (argc < 5) {
		puts("Error. Faltan parámetros en la ejecución del proceso.\n");
		return EXIT_FAILURE;
	}
	char *transformador = argv[1];
	char *reductor = argv[2];
	char *archivoRequerido = argv[3];
	char *archivoDestino = argv[4];

// 1º) leer archivo de config.
	char *nameArchivoConfig = "configMaster.txt";
	if (leerArchivoConfig(nameArchivoConfig, keysConfigMaster, datosConfigMaster)) { //leerArchivoConfig devuelve 1 si hay error
		printf("Hubo un error al leer el archivo de configuración");
		return EXIT_FAILURE;
	}

// 2º) conectarse a YAMA y aguardar instrucciones
	log_info(logMASTER, "Conexión a Yama, IP: %s, Puerto: %s", datosConfigMaster[YAMA_IP], datosConfigMaster[YAMA_PUERTO]);
	int socketYama = conectarA(datosConfigMaster[YAMA_IP], datosConfigMaster[YAMA_PUERTO]);
	if (socketYama < 0) {
		//preparadoEnviarYama = 0;
		puts("No se pudo conectar a YAMA");
		return EXIT_FAILURE;
	}

	//inicia comunicación con YAMA enviando el HANDSHAKE
	headerIdYama = handshakeYama(socketYama);
	if (headerIdYama == TIPO_MSJ_HANDSHAKE_RESPUESTA_OK) {
		//envía a yama el archivo con el que quiere trabajar
		enviarArchivoYama(socketYama, archivoRequerido);
		fcntl(socketYama, F_SETFL, O_NONBLOCK);

		for (j = 0;; j++) {
			printf("pasó %d vez\n", j);
			sleep(2);
			headerIdYama = deserializarHeader(socketYama);
			printf("headerIdPrincipal: %d - %d - %s\n", headerIdYama, protocoloCantidadMensajes[headerIdYama], protocoloMensajesPredefinidos[headerIdYama]);
			if (headerIdYama == TIPO_MSJ_TABLA_TRANSFORMACION) { //recibe la tabla de transformaciones
				int cantBloquesArchivo = getCantBloquesArchivo(socketYama, protocoloCantidadMensajes[headerIdYama]);
				pthread_t hilosWorker[cantBloquesArchivo];
				struct datosWorker datos[cantBloquesArchivo];
				struct filaTransformacion datosTransformacion[cantBloquesArchivo];
				puts("pasóooossso");
				recibirTablaTransformaciones(datosTransformacion, socketYama, cantBloquesArchivo);
				for (i = 0; i < cantBloquesArchivo; i++) {
					printf("se creó el hilo %d\n", i);
					// por cada tarea se crea un hilo para conectarse al worker q corresponda, se le pasa la fila de la tabla recibida
					pthread_create(&(hilosWorker[i]), NULL, conectarAWorkerTransformacion, &datosTransformacion[i]);
				}

				for (i = 0; i < cantBloquesArchivo; i++) {
					//genera el join de cada hilo creado
					//TODO: está bien hecho así??????????
					//TODO: no se quedaría esperando que terminen todas las transformaciones en vez de seguir??????
					pthread_join(hilosWorker[i], NULL);
				}
				puts("pasóoooo");
//			pthread_create(&(hilosWorker[0]), NULL, conectarAWorkerTransformacion, datos);
//			pthread_join(hilosWorker[0], NULL);
//			free(arrayMensajesTablaTransformacion);
//			arrayMensajesTablaTransformacion = deserializarMensaje(socketYama, cantColumnasTabla);
//			struct datosWorker *datos2 = malloc(sizeof(struct datosWorker) + 1);
//			strcpy(datos2->ip, arrayMensajesTablaTransformacion[1]);
//			strcpy(datos2->puerto, arrayMensajesTablaTransformacion[2]);
//			pthread_create(&hiloWorker2, NULL, conectarAWorkerTransformacion, (void*) datos2);
//			free(arrayMensajesTablaTransformacion);
			}

			if (headerIdYama == TIPO_MSJ_TABLA_REDUCCION_LOCAL) {
				recibirTablaReduccLocalYEnviarAWorker(socketYama, headerIdYama);
			}

			//luego de fijarse el header recibido del yama y hacer lo que deba
			//se fija en el headerIdFinalizado que es el que completan los hilos
			//si ese header es != es porque algún hilo terminó alguna etapa y hay que enviar mensaje a yama
			if (headerIdFinalizado == TIPO_MSJ_TRANSFORMACION_OK) {
				//enviar a YAMA fin ok
				int nodo = nroNodoFinalizado;
				int bloque = nroBloqueFinalizado;
				headerIdFinalizado = 0;	//lo pone en 0 así en la próxima vuelta no hace nada si ningún hilo lo modificó
				pthread_mutex_unlock(&mutex);//libera el mutex para que otro hilo modifiqué las variables compartidas
				//enviar a yama el mensaje serializado
			}
			if (headerIdFinalizado == TIPO_MSJ_TRANSFORMACION_ERROR) {
				//enviar a YAMA fin ok
				int nodo = nroNodoFinalizado;
				int bloque = nroBloqueFinalizado;
				headerIdFinalizado = 0;	//lo pone en 0 así en la próxima vuelta no hace nada si ningún hilo lo modificó
				pthread_mutex_unlock(&mutex);//libera el mutex para que otro hilo modifiqué las variables compartidas
				//enviar a yama el mensaje serializado
			}
			if (headerIdFinalizado == TIPO_MSJ_REDUCC_LOCAL_OK) {
				//enviar a YAMA fin ok
				int nodo = nroNodoFinalizado;
				headerIdFinalizado = 0;	//lo pone en 0 así en la próxima vuelta no hace nada si ningún hilo lo modificó
				pthread_mutex_unlock(&mutex);//libera el mutex para que otro hilo modifiqué las variables compartidas
				//enviar a yama el mensaje serializado
			}
			if (headerIdFinalizado == TIPO_MSJ_REDUCC_LOCAL_ERROR) {
				//enviar a YAMA fin ok
				int nodo = nroNodoFinalizado;
				headerIdFinalizado = 0;	//lo pone en 0 así en la próxima vuelta no hace nada si ningún hilo lo modificó
				pthread_mutex_unlock(&mutex);//libera el mutex para que otro hilo modifiqué las variables compartidas
				//enviar a yama el mensaje serializado
			}
		}

	} else {
		puts("me banneó el hdp!!!!!");
	}

	/* ********************************************************* */

	/* ************************************************************** */

// 3º) Etapa de Transformación: crear hilo, conectarse al worker, esperar y notificar a YAMA
//      A qué procesos Worker deberá conectarse con su IP y Puerto
//      Sobre qué bloque de cada Worker debe aplicar el programa de Transformación.
//      El nombre de archivo temporal donde deberá almacenar el resultado del script de Transformación.
//      El proceso Master deberá entonces:
//      Iniciar un hilo por cada etapa de Transformación indicada por el proceso YAMA.
//      Cada hilo se conectará al correspondiente Worker, le enviará el programa de Transformación y le indicará el bloque sobre el cuál quiere ejecutar el programa, la cantidad de bytes ocupados en dicho bloque y el nombre del archivo temporal donde guardará el resultado. Quedará a la espera de la confirmación por parte del proceso Worker.
//      Notificara del éxito o fracaso de la operación al proceso YAMA.
// 4º) Etapa de Reducción Local: crear hilo, conectarse al worker, esperar y notificar a YAMA
// 5º) Etapa de Reducción Global: crear hilo, conectarse al worker, esperar y notificar a YAMA
// 6º) Desconectar Yama
	cerrarCliente(socketYama);
	return EXIT_SUCCESS;
}

void* conectarAWorkerReduccLocal(void *arg) {
	FILE *fp;
	int i, j;
	struct filaReduccLocal *datos = (struct filaReduccLocal*) arg;
	int cantTemporalesTransformacion = sizeof(datos->temporalesTransformacion) / LARGO_TEMPORAL;

//pasa el archivo a string para enviarlo al worker
	char *archivo = "reductor.py";
	char *pathArchivo = string_from_format("../../scripts/%s", archivo);
	fp = fopen(pathArchivo, "r"); // read mode
	fseek(fp, 0, SEEK_END);
	long length = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	char *reductorString = malloc(length);
	if (reductorString) {
		fread(reductorString, 1, length, fp);
	}
	fclose(fp);

//int socketWorker = conectarA(datos->ip, string_itoa(datos->puerto));
	int socketWorker = conectarA("127.0.0.1", "5300");

//reductor.py|cantidad de temporales transformacion | temp tranf 1 | .... | temp transf N | temp reduccLocal
	int cantStringsASerializar = 1 + 1 + cantTemporalesTransformacion + 1;
	char **arrayMensajes = malloc(sizeof(char*) * cantStringsASerializar);

	j = 0;
	arrayMensajes[j] = malloc(string_length(reductorString) + 1);
	strcpy(arrayMensajes[j], reductorString);
	free(reductorString);
	j++;
	char* cantTemporalesTransformacionString = intToArrayZerosLeft(cantTemporalesTransformacion, 4);
	arrayMensajes[j] = malloc(string_length(cantTemporalesTransformacionString) + 1);
	strcpy(arrayMensajes[j], cantTemporalesTransformacionString);
	j++;
	for (i = 0; i < cantTemporalesTransformacion; i++) {
		arrayMensajes[j] = malloc(string_length(datos->temporalesTransformacion[i]) + 1);
		strcpy(arrayMensajes[j], datos->temporalesTransformacion[i]);
		j++;
	}
	arrayMensajes[j] = malloc(string_length(datos->temporalReduccLocal) + 1);
	strcpy(arrayMensajes[j], datos->temporalReduccLocal);

//TIPO_MSJ_DATA_REDUCCION_LOCAL_WORKER: 1 MENSAJE
	char *mensajeSerializado = serializarMensaje(TIPO_MSJ_DATA_REDUCCION_LOCAL_WORKER, arrayMensajes, cantStringsASerializar);
	for (j = 0; j < cantStringsASerializar; j++) {
		free(arrayMensajes[j]);
	}
	free(arrayMensajes);
//printf("\nmensaje serializado: \n%s\n", mensajeSerializado);
	enviarMensaje(socketWorker, mensajeSerializado);

	int32_t headerIdWorker = deserializarHeader(socketWorker);
	pthread_mutex_lock(&mutex);
	nroNodoFinalizado = datos->nodo;
	headerIdFinalizado = headerIdWorker;
	//pthread_mutex_unlock(&mutex);
}

void* conectarAWorkerTransformacion(void *arg) {
	FILE *fp;
	int i, j;

//pasa el archivo a string para enviarlo al worker
	char *archivo = "transformador.py";
	char *pathArchivo = string_from_format("../../scripts/%s", archivo);
	fp = fopen(pathArchivo, "r"); // read mode
	fseek(fp, 0, SEEK_END);
	long length = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	char *transformadorString = malloc(length);
	if (transformadorString) {
		fread(transformadorString, 1, length, fp);
	}
	fclose(fp);

	struct filaTransformacion *datos = (struct filaTransformacion*) arg;

	int socketWorker = conectarA(datos->ip, string_itoa(datos->puerto));
	//int socketWorker = conectarA("127.0.0.1", "5300");

	sleep(2 * datos->nodo);
	int cantStringsASerializar = 4;	//código de transformación, bloque, bytes y temporal
	char **arrayMensajes = malloc(sizeof(char*) * cantStringsASerializar);
	j = 0;
	arrayMensajes[j] = malloc(string_length(transformadorString) + 1);
	strcpy(arrayMensajes[j], transformadorString);
	j++;
	arrayMensajes[j] = malloc(4 + 1);
	strcpy(arrayMensajes[j], intToArrayZerosLeft(datos->bloque, 4));
	j++;
	arrayMensajes[j] = malloc(8 + 1);
	strcpy(arrayMensajes[j], intToArrayZerosLeft(datos->bytes, 8));
	j++;
	arrayMensajes[j] = malloc(string_length(datos->temporal) + 1);
	strcpy(arrayMensajes[j], datos->temporal);

//TIPO_MSJ_DATA_TRANSFORMACION_WORKER: 4 MENSAJES
	char *mensajeSerializado = serializarMensaje(TIPO_MSJ_DATA_TRANSFORMACION_WORKER, arrayMensajes, cantStringsASerializar);
	for (j = 0; j < cantStringsASerializar; j++) {
		free(arrayMensajes[j]);
	}
	free(arrayMensajes);
//printf("\nmensaje serializado: \n%s\n", mensajeSerializado);
	enviarMensaje(socketWorker, mensajeSerializado);

//respuesta con la tabla de transformaciones
	int32_t headerIdWorker = deserializarHeader(socketWorker);
	printf("headerIdWorker: %d\n", headerIdWorker);
	pthread_mutex_lock(&mutex);
	nroNodoFinalizado = datos->nodo;
	nroBloqueFinalizado = datos->bloque;
	headerIdFinalizado = headerIdWorker;
	//pthread_mutex_unlock(&mutex);

}
