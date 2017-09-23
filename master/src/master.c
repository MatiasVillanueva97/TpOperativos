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
#include "../../utils/conexionesSocket.h"
#include "../../utils/archivoConfig.h"
#include "../../utils/comunicacion.h"

//#define PACKAGESIZE 1024	// Define cual va a ser el size maximo del paquete a enviar

enum keys {YAMA_IP, YAMA_PUERTO, WORKER_IP, WORKER_PUERTO};
char* keysConfigMaster[]={"YAMA_IP", "YAMA_PUERTO", "WORKER_IP", "WORKER_PUERTO", NULL};
char* datosConfigMaster[4];

// ================================================================ //
// Master ejecuta una tarea, sobre un archivo.
// Se conecta a YAMA para pedir instrucciones,
// y a los workers (usando hilos) para mandar instrucciones
// Puede haber varios master corriendo al mismo tiempo.
// ================================================================ //

int main(int argc, char *argv[]) {
	struct headerProtocolo headerComunicacion;
	printf("\n*** Proceso Master ***\n");

	if(argc<5){
		puts("Error. Faltan parámetros en la ejecución del proceso\n");
		return EXIT_FAILURE;
	}
	char *transformador=argv[1];
	char *reductor=argv[2];
	char *archivoRequerido=argv[3];
	char *archivoDestino=argv[4];

	printf("%s - %s - %s - %s\n",transformador,reductor,archivoRequerido,archivoDestino);

	char *nameArchivoConfig = "configMaster.txt";

	// 1º) leer archivo de config.
	if (leerArchivoConfig(nameArchivoConfig, keysConfigMaster, datosConfigMaster)) {	//leerArchivoConfig devuelve 1 si hay error
		printf("Hubo un error al leer el archivo de configuración");
		return 0;
	}

    // 2º) conectarse a YAMA y aguardar instrucciones
    //int serverSocket=conectarA(datosConfigTxt.YAMA_IP, datosConfigTxt.YAMA_PUERTO);

    //envía a yama el archivo con el que quiere trabajar
	headerComunicacion=armarHeader(11,strlen(archivoRequerido) + 1);
	char *headerSerializado=serializarHeader(headerComunicacion);
	printf("Header serializado: %s\n",headerSerializado);
    //int cantBytesEnviados=send(serverSocket, archivoRequerido, 1, 0); 	// Solo envio si el usuario no quiere salir.
	//printf("\n%d\n",cantBytesEnviados);


	// 3º) conectarse a un worker y pasarle instrucciones (pasar a HILOS!)
    //int socketWorker = inicializarClient(datosConfigTxt.WORKER_IP, datosConfigTxt.WORKER_PUERTO);
    //conectarA(socketWorker);

    // Etapa de Transformación: crear hilo, conectarse al worker, esperar y notificar a YAMA
    // Etapa de Reducción Local: crear hilo, conectarse al worker, esperar y notificar a YAMA
    // Etapa de Reducción Global: crear hilo, conectarse al worker, esperar y notificar a YAMA
	//close(serverSocket);
	return EXIT_SUCCESS;
}
