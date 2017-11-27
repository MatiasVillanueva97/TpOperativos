/*
 ============================================================================
 Name        : master.c
 Author      : Grupo 1234
 Description : Proceso Master
 ============================================================================
 */

#include "../../utils/includes.h"

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

void pruebaRecibirTablaTransformaciones(int socketYama) {
	int bytesEnviados, i, j, k, h;
	int32_t headerId = deserializarHeader(socketYama);
	if (headerId == TIPO_MSJ_TABLA_TRANSFORMACION) {
		char **arrayMensajesCantMensajes = deserializarMensaje(socketYama, protocoloCantidadMensajes[TIPO_MSJ_TABLA_TRANSFORMACION]);
		int cantMensajes = atoi(arrayMensajesCantMensajes[0]);
		free(arrayMensajesCantMensajes[0]);
		free(arrayMensajesCantMensajes);
		printf("cantMensajes: %d\n", cantMensajes);
		//recibir la tabla de transformación
		int cantStrings = 6 * cantMensajes;
		char **arrayTablaTransformacion = deserializarMensaje(socketYama, cantStrings);
		for (i = 0, j = 0; i < cantMensajes; i++) {
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
		puts("tabla de transformación recibida\n---------------------------------------\n\n");
		puts("presionar ENTER");
		getchar();
	}
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

	//inicia comunicación con YAMA enviando el HANDSHAKE
	int cantStringsHandshake = protocoloCantidadMensajes[TIPO_MSJ_HANDSHAKE];
	char **arrayMensajesHandshake = malloc(sizeof(char*) * cantStringsHandshake);
	char *mensaje = intToArrayZerosLeft(NUM_PROCESO_MASTER, 4);
	arrayMensajesHandshake[0] = malloc(string_length(mensaje) + 1);
	strcpy(arrayMensajesHandshake[0], mensaje);
	char *mensajeSerializadoHS = serializarMensaje(TIPO_MSJ_HANDSHAKE, arrayMensajesHandshake, cantStringsHandshake);
	enviarMensaje(socketYama, mensajeSerializadoHS);
	free(arrayMensajesHandshake);

	int32_t headerId = deserializarHeader(socketYama);
	if (headerId == TIPO_MSJ_HANDSHAKE_RESPUESTA_OK) {

		//envía a yama el archivo con el que quiere trabajar
		pruebaEnviarArchivoYama(socketYama, archivoRequerido);

		//respuesta con la tabla de transformaciones
		pruebaRecibirTablaTransformaciones(socketYama);

		//envío de fin transformación OK, todos los del nodo 2, 1 del nodo 1 y 1 del nodo 3
		pruebaEnvioFinTransformacionOk(socketYama, 2, 36);
		pruebaEnvioFinTransformacionOk(socketYama, 1, 33);
		pruebaEnvioFinTransformacionOk(socketYama, 3, 55);
		pruebaEnvioFinTransformacionOk(socketYama, 2, 65);

		pruebaRecibirTablaRedLocal(socketYama);//para el nodo 3

		//envío de fin transformación OK nodo 1
		pruebaEnvioFinTransformacionOk(socketYama, 1, 88);
		pruebaEnviarFinReduccLocal(socketYama, 3);
		pruebaEnvioFinTransformacionOk(socketYama, 1, 74);

		pruebaRecibirTablaRedLocal(socketYama);//para el nodo 2

		//envía mensaje de fin de reducción local nodo 2


		pruebaRecibirTablaRedLocal(socketYama);//para el nodo 1
		pruebaEnviarFinReduccLocal(socketYama, 2);
		pruebaEnviarFinReduccLocal(socketYama, 1);

		//recibe la tabla de reducción global

	} else {
		puts("me banneó el hdp!!!!!");
	}
}

int main(int argc, char *argv[]) {
	int i;
	t_log* logMASTER;
	logMASTER = log_create("logMASTER.log", "MASTER", false, LOG_LEVEL_TRACE); //creo el logger, sin mostrar por pantalla
	uint32_t preparadoEnviarYama = 1;

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
		//preparadoEnviarFs = handshakeClient(&datosConexionFileSystem, NUM_PROCESO_KERNEL);
		preparadoEnviarYama = 0;
	}

	/* ******************** SOLO PARA PRUEBAS ******************* */
	pruebas(socketYama, archivoRequerido);

	// conectarse a workers y hacer transformacion

	/* ********************************************************* */
//
//			int j, cantColumnasTabla = 6;
//
//			pthread_t hilosWorker[3];
//			char **arrayMensajesTablaTransformacion;
//			struct datosWorker *datos[3];
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

	struct datosWorker *datos = arg;
	int socketWorker = conectarA(datos->ip, datos->puerto);
//int socketWorker = conectarA("127.0.0.1", "5300");
	/*puts("\nIP y puerto");
	 printf("IP: %s\n", datos->ip);
	 printf("Puerto: %s\n", datos->puerto);*/

	printf("Worker\n");

	enviarMensaje(socketWorker, "seisle");
	sleep(10000);
	return EXIT_SUCCESS;

}
