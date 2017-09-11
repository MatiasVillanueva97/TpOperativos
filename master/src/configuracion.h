typedef struct config {
	char *IP_YAMA;
	char *PUERTO_YAMA;
} datosConfig;

int configFileH(char *pathFileConfig, datosConfig *datosConexion) {
	// abro el archivo de configuracion
	t_config *file = config_create(pathFileConfig);
	if (!file) {
		printf("\nError: No se encuentra el archivo\nEjecución abortada\n");
		return 0;
	}

	// busco sus keys
	datosConexion->IP_YAMA = config_get_string_value(file, "IP_YAMA");
	datosConexion->PUERTO_YAMA = config_get_string_value(file, "PUERTO_YAMA");

	printf("\nMis datos de configuración son los siguientes:");
	printf("\nIP_YAMA: %s", datosConexion->IP_YAMA);
	printf("\nPUERTO_YAMA: %s", datosConexion->PUERTO_YAMA);

	printf("\n");
	return 1;
}
