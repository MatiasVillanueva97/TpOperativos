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

int leerArchivoConfig(char *nameArchivoConfig, char **keysConfig, char **datosConfig) {
	int i;
	char *pathArchivoConfig= string_new();
	string_append_with_format(&pathArchivoConfig, "../../config/%s", nameArchivoConfig);
	t_config *archivoConfig = config_create(pathArchivoConfig);
	free(pathArchivoConfig);
	if (!archivoConfig) {
		printf("Error: No se encuentra el archivo\nEjecución abortada\n");
		free(archivoConfig);
		return EXIT_FAILURE;
	}
	i=0;
	while(keysConfig[i]){
		datosConfig[i] = config_get_string_value(archivoConfig, keysConfig[i]);
		i++;
	}
	free(archivoConfig);
	return EXIT_SUCCESS;
}
