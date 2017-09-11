typedef struct config {
	char *YAMA_IP;
	char *YAMA_PUERTO;
} datosConfig;

int configFileH(char *pathFileConfig, datosConfig *datosConexion) {
	// abro el archivo de configuracion
	t_config *file = config_create(pathFileConfig);
	if (!file) {
		printf("\nError: No se encuentra el archivo\nEjecución abortada\n");
		return 0;
	}

	// busco sus keys
	datosConexion->YAMA_IP = config_get_string_value(file, "YAMA_IP");
	datosConexion->YAMA_PUERTO = config_get_string_value(file, "YAMA_PUERTO");

	printf("\nMis datos de configuración son los siguientes:");
	printf("\nYAMA_IP: %s", datosConexion->YAMA_IP);
	printf("\nYAMA_PUERTO: %s", datosConexion->YAMA_PUERTO);

	printf("\n");
	return 1;
}
