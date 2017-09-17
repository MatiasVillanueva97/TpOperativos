#include <netdb.h>
#include <string.h>

#define MAX_CONEXIONES 30			// Define cuantas conexiones vamos a mantener pendientes al mismo tiempo

typedef struct configServer {
	int socketCliente;
	char *puerto;
	int listenningSocket;
} datosConfigServer;

int initializeServer(datosConfigServer *datosConexionServer) {
	// Uso getaddrinfo() para obtener los datos de la direccion de red y guardarlos en serverInfo.
	struct addrinfo hints;
	struct addrinfo *serverInfo;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC; // No importa si uso IPv4 o IPv6
	hints.ai_flags = AI_PASSIVE; // Asigna el address del localhost: 127.0.0.1
	hints.ai_socktype = SOCK_STREAM; // Indica que usaremos el protocolo TCP
	getaddrinfo(NULL, datosConexionServer->puerto, &hints, &serverInfo); // Notar que le pasamos NULL como IP, ya que le indicamos que use localhost en AI_PASSIVE

	// Obtengo un socket, utilizando la estructura serverInfo ya generada.
	if((datosConexionServer->listenningSocket = socket(serverInfo->ai_family,serverInfo->ai_socktype, serverInfo->ai_protocol))<0){
		perror("socket");
		return -1;
	}

	// Uso el file descriptor previo para decirle al sistema por donde voy a escuchar las conexiones
	if(bind(datosConexionServer->listenningSocket, serverInfo->ai_addr,serverInfo->ai_addrlen)!=0){
		perror("bind");
		return -1;
	}
	freeaddrinfo(serverInfo); // Ya no lo vamos a necesitar

	// Empiezo a escuchar las conexiones. IMPORTANTE: listen() es una syscall BLOQUEANTE.
	if(listen(datosConexionServer->listenningSocket, MAX_CONEXIONES)!=0){
		perror("listen");
		return -1;
	}

	return 0;
}

int aceptarConexion(datosConfigServer *datosConexionServer) {
	// Acepto la conexión del cliente y abro el *nuevo* socket
	struct sockaddr_in addr; // Esta estructura contendra los datos de la conexion del cliente. IP, puerto, etc.
	socklen_t addrlen = sizeof(addr);
	if((datosConexionServer->socketCliente = accept(datosConexionServer->listenningSocket, (struct sockaddr *) &addr,&addrlen))<0){
		perror("accept");
		return -1;
	}

	return 0;
}

void cerrarServer(datosConfigServer *datosConexionServer) {

	// Cierro la conexión y libero el socket
	close(datosConexionServer->socketCliente);
	close(datosConexionServer->listenningSocket);
	printf("\nCliente desconectado.");
	return;
}
