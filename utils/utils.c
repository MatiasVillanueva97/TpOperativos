int leerArchivoConfig(char *pathArchivoConfig, datosConfig *datosConfig) {
	// abro el archivo de configuracion
	t_config *archivoConfig = config_create(pathArchivoConfig);

	// busco sus keys
	//datosConfigMaster-> YAMA_IP = config_get_string_value(archivoConfig, "YAMA_IP");
	//datosConfigMaster->YAMA_PUERTO =  config_get_int_value(archivoConfig, "YAMA_PUERTO");
	datosConfigMaster->WORKER_IP = config_get_string_value(archivoConfig, "WORKER_IP);
	datosConfigMaster->WORKER_PUERTO = config_int_string_value(archivoConfig, "WORKER_PUERTO");

	config_get_string_value()

	return 1;
}

int inicializarClient(char *IP, int puerto) {
	// Uso getaddrinfo() para obtener los datos de la direccion de red y guardarlos en serverInfo.
	struct addrinfo hints;
	struct addrinfo *serverInfo;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC; // Permite que la maquina se encargue de verificar si usamos IPv4 o IPv6
	hints.ai_socktype = SOCK_STREAM; // Indica que usaremos el protocolo TCP

	getaddrinfo(datosConexionClient->ip, datosConexionClient->puerto, &hints, &serverInfo);	// Carga en serverInfo los datos de la conexion

	// Obtengo un socket, utilizando la estructura serverInfo ya generada.
	if((datosConexionClient->serverSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol))<0){
		perror("socket");
		return -1;

	return 0;
}

int conectarA(int socket)
	// Me conecto al server usando el file descriptor del socket previo.
	if(connect(datosConexionClient->serverSocket, serverInfo->ai_addr, serverInfo->ai_addrlen)<0){
		perror("connect");
		return -1;
	}
	freeaddrinfo(serverInfo);	// No lo necesitamos mas

	return 0;
}
