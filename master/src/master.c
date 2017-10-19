/*
 ============================================================================
 Name        : master.c
 Author      : Grupo 1234
 Description : Proceso Master
 ============================================================================
 */

#include "../../utils/includes.h"

enum keys {
	YAMA_IP, YAMA_PUERTO, WORKER_IP, WORKER_PUERTO
};
char* keysConfigMaster[] = { "YAMA_IP", "YAMA_PUERTO", "WORKER_IP", "WORKER_PUERTO", NULL };
char* datosConfigMaster[4];

// ================================================================ //
// Master ejecuta una tarea, sobre un archivo.
// Se conecta a YAMA para pedir instrucciones,
// y a los workers (usando hilos) para mandar instrucciones
// Puede haber varios master corriendo al mismo tiempo.
// ================================================================ //

int main(int argc, char *argv[]) {
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

	//printf("%s - %s - %s - %s\n", transformador, reductor, archivoRequerido, archivoDestino);

	// 1º) leer archivo de config.
	char *nameArchivoConfig = "configMaster.txt";
	if (leerArchivoConfig(nameArchivoConfig, keysConfigMaster, datosConfigMaster)) {//leerArchivoConfig devuelve 1 si hay error
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
	char *arrayMensajes[4];
	arrayMensajes[0] = argv[1];
	arrayMensajes[1] = argv[2];
	arrayMensajes[2] = argv[3];
	arrayMensajes[3] = argv[4];
	//cuidado con el segmentation fault al armar el array de strings!!!!!!!!!!!!!!!!!!!
	//creo que está mal hacerlo así
	//habría que meter algún malloc en algún lado porque son char *

	char *mensajeSerializado = serializarMensaje(PATH_ARCHIVO, arrayMensajes);
	enviarMensaje(socketYama, mensajeSerializado);

	// 3º) conectarse a un worker y pasarle instrucciones (pasar a HILOS!)
	//int socketWorker = inicializarClient(datosConfigTxt.WORKER_IP, datosConfigTxt.WORKER_PUERTO);
	//conectarA(socketWorker);

	// Etapa de Transformación: crear hilo, conectarse al worker, esperar y notificar a YAMA
	// Etapa de Reducción Local: crear hilo, conectarse al worker, esperar y notificar a YAMA
	// Etapa de Reducción Global: crear hilo, conectarse al worker, esperar y notificar a YAMA
	//cerrarCliente(socketYama);
	return EXIT_SUCCESS;
}
