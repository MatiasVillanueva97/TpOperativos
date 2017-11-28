/*
 ============================================================================
 Name        : master.c
 Author      : Grupo 1234
 Description : Proceso Master
 ============================================================================
 */

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

// ================================================================ //
// Master ejecuta una tarea, sobre un archivo.
// Se conecta a YAMA para pedir instrucciones,
// y a los workers (usando hilos) para mandar instrucciones
// Puede haber varios master corriendo al mismo tiempo.
// ================================================================ //

void* conectarAWorkerTransformacion();

void pruebaEnviarArchivoYama(int socketYama, char *archivoRequerido) {
	int bytesEnviados, i, j, k, h;
	int cantStrings = protocoloCantidadMensajes[TIPO_MSJ_PATH_ARCHIVO_TRANSFORMAR];
	char **arrayMensajes = malloc(cantStrings);
	arrayMensajes[0] = malloc(string_length(archivoRequerido) + 1);
	strcpy(arrayMensajes[0], archivoRequerido);
	//arrayMensajes[0][string_length(archivoRequerido)] = '\0';
	char *mensajeSerializado = serializarMensaje(TIPO_MSJ_PATH_ARCHIVO_TRANSFORMAR, arrayMensajes, cantStrings);
	bytesEnviados = enviarMensaje(socketYama, mensajeSerializado);
	for (i = 0; i < cantStrings; i++) {
		free(arrayMensajes[i]);
	}
	free(arrayMensajes);
	printf("bytesEnviados del archivo a trabajar: %d\n", bytesEnviados);
}

void pruebaRecibirTablaTransformaciones(char **arrayTablaTransformacion, int socketYama, int cantStrings, int cantBloquesArchivo) {
	int bytesEnviados, i, j, k, h;

	printf("cantBloquesArchivo: %d\n", cantBloquesArchivo);
	printf("cantStrings: %d\n", cantStrings);
	//recibir la tabla de transformación
	arrayTablaTransformacion = deserializarMensaje(socketYama, cantStrings);
	puts("datos de la tabla de transformación\n-------------------------------------");
	for (i = 0, j = 0; i < cantBloquesArchivo; i++) {
		printf("nro nodo: %d\n", atoi(arrayTablaTransformacion[j]));
		j++;
		printf("ip nodo: %s\n", arrayTablaTransformacion[j]);
		j++;
		printf("puerto nodo: %d\n", atoi(arrayTablaTransformacion[j]));
		j++;
		printf("nro bloque: %d\n", atoi(arrayTablaTransformacion[j]));
		j++;
		printf("bytes bloque: %d\n", atoi(arrayTablaTransformacion[j]));
		j++;
		printf("temporal: %s\n", arrayTablaTransformacion[j]);
		j++;
		puts("");
	}
	for (i = 0; i < cantStrings; i++) {
		free(arrayTablaTransformacion[i]);
	}
	free(arrayTablaTransformacion);
	puts("tabla de transformación recibida con éxito\n---------------------------------------\n\n");
	puts("presionar ENTER");
	getchar();
}

void pruebaEnvioFinTransformacionOk(int socketYama, int nroNodo, int nroBloque) {
	int cantStrings, bytesEnviados, i, j, k, h;

	puts("\n\nenvío de nodo y bloque que terminó la transformación");
	printf("nroNodo %d - nroBloque %d\n", nroNodo, nroBloque);
	cantStrings = protocoloCantidadMensajes[TIPO_MSJ_TRANSFORMACION_OK];

	char **arrayMensajes = malloc(sizeof(char*) * cantStrings);

	char *nodoString = intToArrayZerosLeft(nroNodo, 4);
	arrayMensajes[0] = malloc(string_length(nodoString) + 1);
	strcpy(arrayMensajes[0], nodoString);

	char *bloqueString = intToArrayZerosLeft(nroBloque, 4);
	arrayMensajes[1] = malloc(string_length(bloqueString) + 1);
	strcpy(arrayMensajes[1], bloqueString);
	char *mensajeSerializado = serializarMensaje(TIPO_MSJ_TRANSFORMACION_OK, arrayMensajes, cantStrings);
	bytesEnviados = enviarMensaje(socketYama, mensajeSerializado);
	for (i = 0; i < cantStrings; i++) {
		free(arrayMensajes[i]);
	}
	free(arrayMensajes);
	puts("presionar ENTER en pruebaEnvioFinTransformacionOk");
	getchar();

}

void pruebaRecibirTablaRedLocal(int socketYama) {
	int cantStrings, bytesEnviados, i, j, k, h;

	int32_t headerId = deserializarHeader(socketYama);
	if (headerId == TIPO_MSJ_TABLA_REDUCCION_LOCAL) {
		char **arrayMensajesCantNodos = deserializarMensaje(socketYama, protocoloCantidadMensajes[TIPO_MSJ_TABLA_TRANSFORMACION]);
		int cantNodos = atoi(arrayMensajesCantNodos[0]);
		free(arrayMensajesCantNodos[0]);
		free(arrayMensajesCantNodos);
		printf("cantNodos: %d\n", cantNodos);

		puts("datos para la reducción local");
		for (i = 0, j = 0; i < cantNodos; i++) {
			cantStrings = 4;
			char **arrayTablaReduccLocal = deserializarMensaje(socketYama, cantStrings);

			printf("nro nodo: %d\n", atoi(arrayTablaReduccLocal[j]));
			j++;
			printf("ip nodo: %s\n", arrayTablaReduccLocal[j]);
			j++;
			printf("puerto nodo: %d\n", atoi(arrayTablaReduccLocal[j]));
			j++;
			int cantTemporales = atoi(arrayTablaReduccLocal[j]);
			printf("cantidad de temporales: %d\n", cantTemporales);
			j++;
			char **arrayTablaReduccLocalTemporales = deserializarMensaje(socketYama, cantTemporales);
			for (k = 0; k < cantTemporales; k++) {
				printf("temporal %d: %s\n", k, arrayTablaReduccLocalTemporales[k]);

			}
			free(arrayTablaReduccLocal);
		}
		char **arrayMensajesTemporalReduccLocal = deserializarMensaje(socketYama, 1);
		printf("temporal reducción local: %s\n", arrayMensajesTemporalReduccLocal[0]);
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
	pruebaEnvioFinTransformacionOk(socketYama, 2, 36);
	pruebaEnvioFinTransformacionOk(socketYama, 1, 33);
	pruebaEnvioFinTransformacionOk(socketYama, 3, 55);
	pruebaEnvioFinTransformacionOk(socketYama, 2, 65);

	pruebaRecibirTablaRedLocal(socketYama); //para el nodo 3

	//envío de fin transformación OK nodo 1
	pruebaEnvioFinTransformacionOk(socketYama, 1, 88);
	pruebaEnviarFinReduccLocal(socketYama, 3);
	pruebaEnvioFinTransformacionOk(socketYama, 1, 74);

	pruebaRecibirTablaRedLocal(socketYama); //para el nodo 2

	//envía mensaje de fin de reducción local nodo 2

	pruebaRecibirTablaRedLocal(socketYama); //para el nodo 1
	pruebaEnviarFinReduccLocal(socketYama, 2);
	pruebaEnviarFinReduccLocal(socketYama, 1);

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

int main(int argc, char *argv[]) {
	int i, j, k, h;
	t_log* logMASTER;
	logMASTER = log_create("logMASTER.log", "MASTER", false, LOG_LEVEL_TRACE); //creo el logger, sin mostrar por pantalla
	uint32_t preparadoEnviarYama = 1;
	int32_t headerId;

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
		return 0;
	}

// 2º) conectarse a YAMA y aguardar instrucciones
	log_info(logMASTER, "Conexión a Yama, IP: %s, Puerto: %s", datosConfigMaster[YAMA_IP], datosConfigMaster[YAMA_PUERTO]);
	int socketYama = conectarA(datosConfigMaster[YAMA_IP], datosConfigMaster[YAMA_PUERTO]);
	if (!socketYama) {
		preparadoEnviarYama = 0;
	}

	/* ******************** SOLO PARA PRUEBAS ******************* */
	//inicia comunicación con YAMA enviando el HANDSHAKE
	headerId = handshakeYama(socketYama);
	if (headerId == TIPO_MSJ_HANDSHAKE_RESPUESTA_OK) {
		//envía a yama el archivo con el que quiere trabajar
		pruebaEnviarArchivoYama(socketYama, archivoRequerido);

		//respuesta con la tabla de transformaciones
		headerId = deserializarHeader(socketYama);
		if (headerId == TIPO_MSJ_TABLA_TRANSFORMACION) {
			//getCantidadMensajes(socketYama,headerId);
			int cantBloquesArchivo = getCantBloquesArchivo(socketYama, protocoloCantidadMensajes[TIPO_MSJ_TABLA_TRANSFORMACION]);
			pthread_t hilosWorker[cantBloquesArchivo];
			struct datosWorker datos[cantBloquesArchivo];
			struct filaTransformacion datosTransformacion[cantBloquesArchivo];

			const int cantMensajesXBloqueArchivo = 6;
			int cantStrings = cantMensajesXBloqueArchivo * cantBloquesArchivo;
			char **arrayTablaTransformacion = deserializarMensaje(socketYama, cantStrings);
			//pruebaRecibirTablaTransformaciones(arrayTablaTransformacion, socketYama, cantStrings, cantBloquesArchivo);

			for (i = 0, j = 0; i < cantBloquesArchivo; i++) {
				// por cada tarea leo la fila de la tabla y creo un hilo para conectarme al worker q corresponda
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
				pthread_create(&(hilosWorker[i]), NULL, conectarAWorkerTransformacion, &datosTransformacion[i]);
//							pthread_join(hilosWorker[0], NULL);
			}
			for (i = 0; i < cantStrings; i++) {
				free(arrayTablaTransformacion[i]);
			}
			free(arrayTablaTransformacion);
			for (i = 0; i < cantBloquesArchivo; i++) {
				pthread_join(hilosWorker[i], NULL);
			}
			puts("tabla de transformación recibida con éxito\n---------------------------------------\n\n");
			puts("presionar ENTER");
			getchar();

			//			for (i = 0; i < 3; i++) {
			//				// por cada tarea leo la fila de la tabla y creo un hilo para conectarme al worker q corresponda
			//				// cada msje es una fila de la tabla transformacion
			//				j = 0;
			//
			//				arrayMensajesTablaTransformacion = deserializarMensaje(socketYama, cantColumnasTabla);
			//
			//				//char ** conexion = malloc(2);
			//				//conexion[0] = malloc(12);
			//				//conexion[1] = malloc(4);
			//
			//				datos[i] = malloc(sizeof(struct datosWorker) + 1);
			//				strcpy(datos[i]->ip, arrayMensajesTablaTransformacion[1]);
			//				strcpy(datos[i]->puerto, arrayMensajesTablaTransformacion[2]);
			//				puts("\nIP y puerto");
			//				printf("IP: %s\n", datos[i]->ip);
			//				printf("Puerto: %s\n", datos[i]->puerto);
			//				/*pthread_t hilosWorker[3];
			//				 struct datosWorker datos;
			//				 strcpy(datos.ip, "127.0.0.1");
			//				 strcpy(datos.puerto, "5300");*/
			//
			//				//pthread_create(&(hilosWorker[0]), NULL, conectarAWorkerTransformacion, datos);
			//				//pthread_join(hilosWorker[0], NULL);
			//				//free(arrayMensajesTablaTransformacion);
			//				//arrayMensajesTablaTransformacion = deserializarMensaje(socketYama, cantColumnasTabla);
			//				//struct datosWorker *datos2 = malloc(sizeof(struct datosWorker) + 1);
			//				//strcpy(datos2->ip, arrayMensajesTablaTransformacion[1]);
			//				//strcpy(datos2->puerto, arrayMensajesTablaTransformacion[2]);
			//				//pthread_create(&hiloWorker2, NULL, conectarAWorkerTransformacion, (void*) datos2);
			//				free(arrayMensajesTablaTransformacion);
			//			}
			/*pthread_join(hilosWorker[0], NULL);
			 pthread_join(hilosWorker[1], NULL);
			 pthread_join(hilosWorker[2], NULL);*/

		}

		//pruebas(socketYama, archivoRequerido);

		// conectarse a workers y hacer transformacion
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

void* conectarAWorkerTransformacion(void *arg) {
	FILE *fp;
	int i, j;

	//pasa el archivo a string para enviarlo al worker
	char *archivo = "transformador.sh";
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
	//sleep(200*datos->nodo);
	//printf("conexión a Worker ip %s - puerto %s - nodo %d - bloque %d - temporal %s\n", datos->ip, string_itoa(datos->puerto), datos->nodo, datos->bloque, datos->temporal);
	//int socketWorker = conectarA(datos->ip, string_itoa(datos->puerto));
	int socketWorker = conectarA("127.0.0.1", "5300");

	//char* message = string_from_format("Bloques %s - Bytes %s - Temporal %s", string_itoa(datos->bloque), string_itoa(datos->bytes), datos->temporal);
	//printf("mensaje: %s\n", message);
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
	printf("\nmensaje serializado: \n%s\n", mensajeSerializado);
	enviarMensaje(socketWorker, mensajeSerializado);

	return EXIT_SUCCESS;

}
