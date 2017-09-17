#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>
#include <commons/string.h>

#define MAX_CONEXIONES 10

int leerArchivoConfig(char *pathArchivoConfig, char **keysConfig, char **datosConfig) {
	int i;
	t_config *archivoConfig = config_create(pathArchivoConfig);
	if (!archivoConfig) {
		printf("Error: No se encuentra el archivo\nEjecución abortada\n");
		return EXIT_FAILURE;
	}
	i=0;
	while(keysConfig[i]){
		datosConfig[i] = config_get_string_value(archivoConfig, keysConfig[i]);
		//printf("%s: %s \n", keysConfig[i],datosConfigMaster[i]);
		i++;
	}
	return EXIT_SUCCESS;
}

int conectarA(char *IP, char *puerto){
	// Uso getaddrinfo() para obtener los datos de la direccion de red y guardarlos en serverInfo.
	struct addrinfo hints;
	struct addrinfo *serverInfo;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC; // Permite que la maquina se encargue de verificar si usamos IPv4 o IPv6
	hints.ai_socktype = SOCK_STREAM; // Indica que usaremos el protocolo TCP

	getaddrinfo(IP, puerto, &hints, &serverInfo);	// Carga en serverInfo los datos de la conexion

	// Obtengo un socket, utilizando la estructura serverInfo ya generada.
	int socketClient = socket(serverInfo->ai_family,serverInfo->ai_socktype, serverInfo->ai_protocol);

	if(socketClient<0){
		perror("socket");
		return -1;
	}

	// Me conecto al server usando el file descriptor del socket previo.
	if(connect(socketClient, serverInfo->ai_addr, serverInfo->ai_addrlen)<0){
		perror("connect");
		return -1;
	}
	freeaddrinfo(serverInfo);	// No lo necesitamos mas

	return 0;
}

int inicializarServer(char *IP, char *puerto) {
	// Uso getaddrinfo() para obtener los datos de la direccion de red y guardarlos en serverInfo.
	struct addrinfo hints;
	struct addrinfo *serverInfo;
	memset(&hints, 0, sizeof(hints));

	hints.ai_family = AF_UNSPEC; // No importa si uso IPv4 o IPv6
	hints.ai_flags = AI_PASSIVE; // Asigna el address del localhost: 127.0.0.1
	hints.ai_socktype = SOCK_STREAM; // Indica que usaremos el protocolo TCP
	getaddrinfo(NULL, puerto, &hints, &serverInfo); // Notar que le pasamos NULL como IP, ya que le indicamos que use localhost en AI_PASSIVE

	// Obtengo un socket, utilizando la estructura serverInfo ya generada.
	int socketServer = socket(serverInfo->ai_family,serverInfo->ai_socktype, serverInfo->ai_protocol);

	if(socketServer<0){
		perror("socket");
		return -1;
	}

	// Uso el file descriptor previo para decirle al sistema por donde voy a escuchar las conexiones
	if(bind(socketServer, serverInfo->ai_addr,serverInfo->ai_addrlen)!=0){
		perror("bind");
		return -1;
	}
	freeaddrinfo(serverInfo); // Ya no lo vamos a necesitar

	// Empiezo a escuchar las conexiones. IMPORTANTE: listen() es una syscall BLOQUEANTE.
	if(listen(socketServer, MAX_CONEXIONES)!=0){
		perror("listen");
		return -1;
	}

	return 0;
}

void cerrarClient(int socket){
	// Cierro la conexión y libero el socket
	close(socket);
	return;
}

//int aceptarConexion(datosConfigServer *datosConexionServer) {
//	// Acepto la conexión del cliente y abro el *nuevo* socket
//	struct sockaddr_in addr; // Esta estructura contendra los datos de la conexion del cliente. IP, puerto, etc.
//	socklen_t addrlen = sizeof(addr);
//	if((datosConexionServer->socketCliente = accept(datosConexionServer->listenningSocket, (struct sockaddr *) &addr,&addrlen))<0){
//		perror("accept");
//		return -1;
//	}
//	return 0;
//}

void cerrarServer(int socket) {

	// Cierro la conexión y libero el socket
	close(socket);
	//close(datosConexionServer->listenningSocket);
	printf("\nCliente desconectado.");
	return;
}
