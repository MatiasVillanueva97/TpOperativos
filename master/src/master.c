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
#include "./configuracion.h"
#include "../../headers/configServer.h"
#include "../../headers/configClient.h"
//#include "../../headers/handshake.h"
#include "../../headers/constantes.h"

//#define PACKAGESIZE 1024	// Define cual va a ser el size maximo del paquete a enviar



void enviarArchivo(FILE *fp) {
	int i;
	char ch, *message = 0;
	long length;
	datosConfigClient datosConexionMaster;

	if (!conectarseAYama(&datosConexionMaster)) {
		//error, tengo que eliminar el thread y retornar
		puts("error al conectarse a YAMA");
	}

	//paso el contenido del archivo a una variable tipo string
	fseek(fp, 0, SEEK_END);
	length = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	message = malloc(length);
	if (message) {
		fread(message, 1, length, fp);
	}
	fclose(fp);

	/*printf("contenido del archivo:\n");
	for (i = 0; i < 10; i++) {
	 printf("%s", message);
	 sleep(5);
	 }*/

	send(datosConexionMaster.serverSocket, message, strlen(message) + 1, 0);

	return;

}

int conectarseAYama(datosConfigClient *datosConexionMaster) {
	datosConfig datosConfig;

	char *pathArchivoConfig = "../src/config.txt";
	int preparadoEnviar;

	if (!configFileH(pathArchivoConfig, &datosConfig)) {
		printf("Hubo un error al leer el archivo de configuración");
		return 0;
	}
	datosConexionMaster->ip = datosConfig.YAMA_IP;
	datosConexionMaster->puerto = datosConfig.YAMA_PUERTO;
	if (!initializeClient(datosConexionMaster)) {
		printf("Error al inicializar el cliente.\n");
		return 0;
	}
	//preparadoEnviar = handshakeClient(datosConexionMaster,	NUM_PROCESO_CONSOLA);
	preparadoEnviar =1;
	return preparadoEnviar;
}


void iniciarPrograma() {
	char nombreArchivo[30];
	char *pathArchivo = string_new();
	FILE *fp;
	pthread_t hiloPrograma;
	int iRetHiloPrograma;

	printf("\nIniciar Programa\n");
	printf("Ingrese el nombre del archivo fuente\n");
	//fgets(nombreArchivo, sizeof(nombreArchivo), stdin);
	scanf("%s", nombreArchivo);
	string_append_with_format(&pathArchivo, "../../%s", nombreArchivo);

	fp = fopen(pathArchivo, "r"); // read mode

	if (fp) {
		//crea un nuevo hilo de programa
		//iRetHiloPrograma = pthread_create(&hiloPrograma, NULL,(void*) enviarArchivo, (void*) fp);

		enviarArchivo(fp);	//no me funca el hilo, por ahora lo pruebo así, la posta es la línea de arriba que crea un hilo
	}else{
		printf("Ocurrió un error al intentar abrir el archivo\n");
	}

}

int main(int argc, char *argv[]) {
	datosConfig datosConfig;

	char *pathArchivoConfig = "../src/config.txt";
	int preparadoEnviar;

	printf("\n*** Proceso Master ***");

	if (!configFileH(pathArchivoConfig, &datosConfig)) {
		printf("Hubo un error al leer el archivo de configuración");
		return 0;
	}
	iniciarPrograma();

	printf("\n");
	return EXIT_SUCCESS;
}
