typedef struct config {
	char *PUERTO;
	char *IP;
	char *PUERTO_FS;
	char *IP_FS;
} datosConfig;

int configFileH(char *pathFileConfig, datosConfig *datosConexion) {
	// abro el archivo de configuracion
	t_config *file = config_create(pathFileConfig);
	if (!file) {
		printf("Error: No se encuentra el archivo\nEjecución abortada\n");
		return 0;
	}

	// busco sus keys
	datosConexion->PUERTO = config_get_string_value(file, "PUERTO_PROPIO");
	datosConexion->IP = config_get_string_value(file, "IP_PROPIA");
	datosConexion->PUERTO_FS = config_get_string_value(file, "PUERTO_FS");
	datosConexion->IP_FS = config_get_string_value(file, "IP_FS");

	printf("\nMis datos de configuración son los siguientes:");
	printf("\nPUERTO: %s", datosConexion->PUERTO);
	printf("\nIP: %s", datosConexion->IP);
	printf("\nPUERTO_FS: %s", datosConexion->PUERTO_FS);
	printf("\nIP_FS: %s", datosConexion->IP_FS);

	printf("\n");
	return 1;
}
