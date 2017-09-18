/*
 ============================================================================
 Name        : master.c
 Author      : Grupo 1234
 Description : Proceso Master
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <commons/config.h>
#include <commons/string.h>
#include "../../utils/utils.h"

//#define PACKAGESIZE 1024	// Define cual va a ser el size maximo del paquete a enviar

enum keys {YAMA_IP, YAMA_PUERTO, WORKER_IP, WORKER_PUERTO};
char* keysConfigMaster[]={"YAMA_IP", "YAMA_PUERTO", "WORKER_IP", "WORKER_PUERTO", NULL};
char* datosConfigMaster[4];

int main(int argc, char *argv[]) {

	printf("\n*** Proceso Master ***");

	char *nameArchivoConfig = "configMaster.txt";

	// 1º) leer archivo de config.
    int archivoConfigOK = leerArchivoConfig(nameArchivoConfig, keysConfigMaster, datosConfigMaster);

    if (!archivoConfigOK) {
		printf("Hubo un error al leer el archivo de configuración");
		return 0;
	}

    // 2º) conectarse a YAMA y aguardar instrucciones
    //conectarA(datosConfigMaster[YAMA_IP], datosConfigMaster[YAMA_PUERTO]);

    // 3º) conectarse a un worker y pasarle instrucciones (pasar a HILOS!)
    //int socketWorker = inicializarClient(datosConfigMaster[WORKER_IP], datosConfigMaster[WORKER_PUERTO]);
    //conectarA(socketWorker);

    // Etapa de Transformación: crear hilo, conectarse al worker, esperar y notificar a YAMA
    // Etapa de Reducción Local: crear hilo, conectarse al worker, esperar y notificar a YAMA
    // Etapa de Reducción Global: crear hilo, conectarse al worker, esperar y notificar a YAMA

	return EXIT_SUCCESS;
}

//void enviarArchivo(FILE *fp) {
//	int i;
//	char ch, *message = 0;
//	long length;
//	datosConfigClient datosConexionMaster;
//
//	if (!conectarseAYama(&datosConexionMaster)) {
//		//error, tengo que eliminar el thread y retornar
//		puts("error al conectarse a YAMA");
//	}
//
//	//paso el contenido del archivo a una variable tipo string
//	fseek(fp, 0, SEEK_END);
//	length = ftell(fp);
//	fseek(fp, 0, SEEK_SET);
//	message = malloc(length);
//	if (message) {
//		fread(message, 1, length, fp);
//	}
//	fclose(fp);
//
//	/*printf("contenido del archivo:\n");
//	for (i = 0; i < 10; i++) {
//	 printf("%s", message);
//	 sleep(5);
//	 }*/
//
//	send(datosConexionMaster.serverSocket, message, strlen(message) + 1, 0);
//
//	return;
//
//}

//void iniciarPrograma() {
//	char nombreArchivo[30];
//	char *pathArchivo = string_new();
//	FILE *fp;
//	pthread_t hiloPrograma;
//	int iRetHiloPrograma;
//
//	printf("\nIniciar Programa\n");
//	printf("Ingrese el nombre del archivo fuente\n");
//	//fgets(nombreArchivo, sizeof(nombreArchivo), stdin);
//	scanf("%s", nombreArchivo);
//	string_append_with_format(&pathArchivo, "../../%s", nombreArchivo);
//
//	fp = fopen(pathArchivo, "r"); // read mode
//
//	if (fp) {
//		//crea un nuevo hilo de programa
//		//iRetHiloPrograma = pthread_create(&hiloPrograma, NULL,(void*) enviarArchivo, (void*) fp);
//
//		//enviarArchivo(fp);	//no me funca el hilo, por ahora lo pruebo así, la posta es la línea de arriba que crea un hilo
//	}else{
//		printf("Ocurrió un error al intentar abrir el archivo\n");
//	}
