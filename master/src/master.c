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
char* keysConfigMaster[] = { "YAMA_IP", "YAMA_PUERTO", "NODO_IP", "NODO_PUERTO", NULL };
char* datosConfigMaster[4];

// ================================================================ //
// Master ejecuta una tarea, sobre un archivo.
// Se conecta a YAMA para pedir instrucciones,
// y a los workers (usando hilos) para mandar instrucciones
// Puede haber varios master corriendo al mismo tiempo.
// ================================================================ //

void* printDataAndWait();

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

	 //envía a yama el archivo con el que quiere trabajar
	 //hago un paquete serializado con el mensaje a enviar
	 int cantStrings = 1;
	 char **arrayMensajes = malloc(cantStrings);
	 arrayMensajes[0] = malloc(string_length(archivoRequerido) + 1);
	 strcpy(arrayMensajes[0], archivoRequerido);
	 arrayMensajes[0][string_length(archivoRequerido)] = '\0';
	 //	for (i = 0; i < cantStrings; i++) {
	 //	 arrayMensajes[i] = malloc(string_length(argv[i + 1]) + 1);
	 //	 strcpy(arrayMensajes[i], argv[i + 1]);
	 //	 arrayMensajes[i][string_length(argv[i + 1])] = '\0';
	 //	 }
	 char *mensajeSerializado = serializarMensaje(TIPO_MSJ_PATH_ARCHIVO, arrayMensajes, cantStrings);
	 enviarMensaje(socketYama, mensajeSerializado);
	 for (i = 0; i < cantStrings; i++) {
	 free(arrayMensajes[i]);
	 }
	 free(arrayMensajes);


	// 3º) conectarse a un worker y pasarle instrucciones (pasar a HILOS!)
	int socketWorker = conectarA(datosConfigMaster[NODO_IP], datosConfigMaster[NODO_PUERTO]);

	pthread_t hiloWorker;

	pthread_create(&hiloWorker, NULL, printDataAndWait, NULL);

	pthread_join(hiloWorker, NULL);

	// Etapa de Transformación: crear hilo, conectarse al worker, esperar y notificar a YAMA
	// Etapa de Reducción Local: crear hilo, conectarse al worker, esperar y notificar a YAMA
	// Etapa de Reducción Global: crear hilo, conectarse al worker, esperar y notificar a YAMA

	//cerrarCliente(socketYama);
	return EXIT_SUCCESS;
}

void* printDataAndWait() {

	printf("hilo creado\n");
	sleep(10000);
	return EXIT_SUCCESS;
}
