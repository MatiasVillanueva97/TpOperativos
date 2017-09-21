/*
 * archivoConfig.c
 *
 *  Created on: 18/9/2017
 *      Author: utnso
 */
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>
#include <commons/string.h>
#include "constantes.h"

/* ***************************** config Worker ****************************** */
struct datosConfigWorker{
	char *PUERTO_PROPIO;
	char *IP_PROPIA;
};

int leerArchivoConfigWorker(char *nameArchivoConfig, struct datosConfigWorker *datosConfigTxt) {
	char *pathArchivoConfig = string_new();
	string_append_with_format(&pathArchivoConfig, "../../configTxts/%s", nameArchivoConfig);
	t_config *archivoConfig = config_create(pathArchivoConfig);
	if (!archivoConfig) {
		puts("\nError: No se encuentra el archivo\nEjecución abortada\n");
		return EXIT_FAILURE;
	}
	datosConfigTxt->IP_PROPIA = config_get_string_value(archivoConfig, "IP_PROPIA");
	datosConfigTxt->PUERTO_PROPIO = config_get_string_value(archivoConfig, "PUERTO_PROPIO");
	/*while(keysConfig[i]){
		datosConfig[i] = config_get_string_value(archivoConfig, keysConfig[i]);
		//printf("%s: %s \n", keysConfig[i],datosConfigMaster[i]);
		i++;
	}*/
	return EXIT_SUCCESS;
}

/* ********************************** config master ********************************** */
struct datosConfigMaster{
	char *WORKER_PUERTO;
	char *WORKER_IP;
	char *YAMA_PUERTO;
	char *YAMA_IP;
};

int leerArchivoConfigMaster(char *nameArchivoConfig, struct datosConfigMaster *datosConfigTxt) {
	char *pathArchivoConfig = string_new();
	string_append_with_format(&pathArchivoConfig, "../../configTxts/%s", nameArchivoConfig);
	t_config *archivoConfig = config_create(pathArchivoConfig);
	if (!archivoConfig) {
		puts("\nError: No se encuentra el archivo\nEjecución abortada\n");
		return EXIT_FAILURE;
	}
	datosConfigTxt->WORKER_PUERTO = config_get_string_value(archivoConfig, "WORKER_PUERTO");
	datosConfigTxt->WORKER_IP = config_get_string_value(archivoConfig, "WORKER_IP");
	datosConfigTxt->YAMA_PUERTO = config_get_string_value(archivoConfig, "YAMA_PUERTO");
	datosConfigTxt->YAMA_IP = config_get_string_value(archivoConfig, "YAMA_IP");

	return EXIT_SUCCESS;
}

/* ********************************** config yama ********************************** */
struct datosConfigYama{
	char *PUERTO_PROPIO;
	char *IP_PROPIA;
	char *FS_PUERTO;
	char *FS_IP;
};

int leerArchivoConfigYama(char *nameArchivoConfig, struct datosConfigYama *datosConfigTxt) {
	char *pathArchivoConfig = string_new();
	string_append_with_format(&pathArchivoConfig, "../../configTxts/%s", nameArchivoConfig);
	t_config *archivoConfig = config_create(pathArchivoConfig);
	if (!archivoConfig) {
		puts("\nError: No se encuentra el archivo\nEjecución abortada\n");
		return EXIT_FAILURE;
	}
	datosConfigTxt->PUERTO_PROPIO = config_get_string_value(archivoConfig, "PUERTO_PROPIO");
	datosConfigTxt->IP_PROPIA = config_get_string_value(archivoConfig, "IP_PROPIA");
	datosConfigTxt->FS_PUERTO = config_get_string_value(archivoConfig, "FS_PUERTO");
	datosConfigTxt->FS_IP = config_get_string_value(archivoConfig, "FS_IP");

	return EXIT_SUCCESS;
}

/* ********************************** config FS ********************************** */
struct datosConfigFS{
	char *PUERTO_PROPIO;
	char *IP_PROPIA;
};

int leerArchivoConfigFS(char *nameArchivoConfig, struct datosConfigFS *datosConfigTxt) {
	char *pathArchivoConfig = string_new();
	string_append_with_format(&pathArchivoConfig, "../../configTxts/%s", nameArchivoConfig);
	t_config *archivoConfig = config_create(pathArchivoConfig);
	if (!archivoConfig) {
		puts("\nError: No se encuentra el archivo\nEjecución abortada\n");
		return EXIT_FAILURE;
	}
	datosConfigTxt->PUERTO_PROPIO = config_get_string_value(archivoConfig, "PUERTO_PROPIO");
	datosConfigTxt->IP_PROPIA = config_get_string_value(archivoConfig, "IP_PROPIA");

	return EXIT_SUCCESS;
}
