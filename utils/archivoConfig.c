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

int leerArchivoConfig(char *nameArchivoConfig, char **keysConfig, char **datosConfig) {
	char *pathArchivoConfig = string_new();
	string_append_with_format(&pathArchivoConfig, "../../configTxts/%s", nameArchivoConfig);
	t_config *archivoConfig = config_create(pathArchivoConfig);
	if (!archivoConfig) {
		puts("\nError: No se encuentra el archivo\nEjecución abortada\n");
		return EXIT_FAILURE;
	}
	int i=0;
	while(keysConfig[i]){
		datosConfig[i] = config_get_string_value(archivoConfig, keysConfig[i]);
		//printf("%s: %s \n", keysConfig[i],datosConfigMaster[i]);
		i++;
	}
	return EXIT_SUCCESS;
}
