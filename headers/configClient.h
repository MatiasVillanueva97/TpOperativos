#include <netdb.h>
#include <string.h>


typedef struct configClient {
	char *ip;
	char *puerto;
	int serverSocket;
} datosConfigClient;

int initializeClient(datosConfigClient *datosConexionClient) {
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
	}

	// Me conecto al server usando el file descriptor del socket previo.
	if(connect(datosConexionClient->serverSocket, serverInfo->ai_addr, serverInfo->ai_addrlen)<0){
		perror("connect");
		return -1;
	}
	freeaddrinfo(serverInfo);	// No lo necesitamos mas

	return 0;
}

void cerrarClient(datosConfigClient *datosConexionClient) {

	// Cierro la conexión y libero el socket
	close(datosConexionClient->serverSocket);

	return;
}
