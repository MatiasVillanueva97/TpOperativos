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
	char *ip[15];
	char *puerto[4];
};

// ================================================================ //
// Master ejecuta una tarea, sobre un archivo.
// Se conecta a YAMA para pedir instrucciones,
// y a los workers (usando hilos) para mandar instrucciones
// Puede haber varios master corriendo al mismo tiempo.
// ================================================================ //

void* conectarAWorkerTransformacion();

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
	int bytesEnviados = 0,
			cantStringsHandshake = protocoloCantidadMensajes[TIPO_MSJ_HANDSHAKE];
	char **arrayMensajesHandshake = malloc(cantStringsHandshake);
	char *mensaje = intToArrayZerosLeft(NUM_PROCESO_MASTER, 4);
	arrayMensajesHandshake[0] = malloc(string_length(mensaje) + 1);
	strcpy(arrayMensajesHandshake[0], mensaje);
	arrayMensajesHandshake[0][string_length(mensaje)] = '\0';
	char *mensajeSerializadoHS = serializarMensaje(TIPO_MSJ_HANDSHAKE, arrayMensajesHandshake, cantStringsHandshake);
	enviarMensaje(socketYama, mensajeSerializadoHS);
	free(arrayMensajesHandshake);

	uint32_t headerId = deserializarHeader(socketYama);
	if (headerId == TIPO_MSJ_HANDSHAKE_RESPUESTA_OK) {

		//envía a yama el archivo con el que quiere trabajar
		//hago un paquete serializado con el mensaje a enviar
		int cantStrings = protocoloCantidadMensajes[TIPO_MSJ_PATH_ARCHIVO_TRANSFORMAR];
		char **arrayMensajes = malloc(cantStrings);
		arrayMensajes[0] = malloc(string_length(archivoRequerido) + 1);
		strcpy(arrayMensajes[0], archivoRequerido);
		arrayMensajes[0][string_length(archivoRequerido)] = '\0';
		char *mensajeSerializado = serializarMensaje(TIPO_MSJ_PATH_ARCHIVO_TRANSFORMAR, arrayMensajes, cantStrings);
		bytesEnviados += enviarMensaje(socketYama, mensajeSerializado);
		for (i = 0; i < cantStrings; i++) {
			free(arrayMensajes[i]);
		}
		free(arrayMensajes);
		printf("bytesEnviados: %d\n", bytesEnviados);
		headerId = deserializarHeader(socketYama);
		if (headerId == TIPO_MSJ_TABLA_TRANSFORMACION) {
			// conectarse a workers y hacer transformacion

			char **arrayMensajesCantMensajes = deserializarMensaje(socketYama, 1);
			int cantMensajes = atoi(arrayMensajes[0]);
			printf("cantMensajes: %d\n", cantMensajes);
			free(arrayMensajesCantMensajes);

			int j, cantColumnasTabla = 6;
			//for (i = 0; i < cantMensajes; i++) {

			// cada msje es una fila de la tabla transformacion
			j = 0;
			char **arrayMensajesTablaTransformacion = deserializarMensaje(socketYama, cantColumnasTabla);
			// crear hilo y conectar a worker i

			pthread_t hiloWorker1;

			//char ** conexion = malloc(2);
			//conexion[0] = malloc(12);
			//conexion[1] = malloc(4);

			struct datosWorker *datos = malloc(sizeof(struct datosWorker) + 1);
			strcpy(datos->ip, arrayMensajesTablaTransformacion[1]);
			strcpy(datos->puerto, arrayMensajesTablaTransformacion[2]);

			pthread_create(&hiloWorker1, NULL, conectarAWorkerTransformacion, (void*) datos);

			pthread_join(hiloWorker1, NULL);

			//printf("nodo: %d\n", atoi(arrayMensajesTablaTransformacion[j]));
			j++;
			//printf("IP: %s\n", arrayMensajesTablaTransformacion[j]);
			j++;
			//printf("puerto: %d\n", atoi(arrayMensajesTablaTransformacion[j]));
			j++;
			//printf("bloque: %d\n", atoi(arrayMensajesTablaTransformacion[j]));
			j++;
			//printf("bytes: %d\n", atoi(arrayMensajesTablaTransformacion[j]));
			j++;
			//printf("temporal: %s\n", arrayMensajesTablaTransformacion[j]);
			j++;
			free(arrayMensajesTablaTransformacion);
			//}
		} else {
			puts("error de lo que me mandaron");
		}

	} else {
		puts("me banneó el hdp!!!!!");
	}
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
	//cerrarCliente(socketYama);
	return EXIT_SUCCESS;
}

void* conectarAWorkerTransformacion(void *arg) {

	struct datosWorker *datos = arg;
	int socketWorker = conectarA(datos->ip, datos->puerto);
//	int socketWorker = conectarA("127.0.0.1", "5300");

	printf(datos->ip, "\n");
	printf(datos->puerto, "\n");

	printf("Worker 1\n");
	sleep(10000);
	return EXIT_SUCCESS;

}
