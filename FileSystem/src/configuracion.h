typedef struct config {
	char *PUERTO;
	char *IP;
} datosConfig;

int configFileH(char *pathFileConfig, datosConfig *datosConexion) {
	// abro el archivo de configuracion
	t_config *file = config_create(pathFileConfig);
	if (!file) {
		printf("Error: No se encuentra el archivo\nEjecución abortada\n");
		return 0;
	}

	// busco sus keys
	datosConexion->PUERTO = config_get_string_value(file, "PUERTO");
	datosConexion->IP = config_get_string_value(file, "IP");

	printf("\nMis datos de configuración son los siguientes:");
	printf("\nPUERTO: %s", datosConexion->PUERTO);
	printf("\nIP: %s", datosConexion->IP);

	printf("\n");
	return 1;
}
