#include <netdb.h>
#include <string.h>

#define BACKLOG 5			// Define cuantas conexiones vamos a mantener pendientes al mismo tiempo

typedef struct configServer {
	int socketCliente;
	char *puerto;
	int listenningSocket;
} datosConfigServer;

int initializeServer(datosConfigServer *datosConexionServer) {
	int response=0;
	// Uso getaddrinfo() para obtener los datos de la direccion de red y guardarlos en serverInfo.
	struct addrinfo hints;
	struct addrinfo *serverInfo;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC; // No importa si uso IPv4 o IPv6
	hints.ai_flags = AI_PASSIVE; // Asigna el address del localhost: 127.0.0.1
	hints.ai_socktype = SOCK_STREAM; // Indica que usaremos el protocolo TCP
	getaddrinfo(NULL, datosConexionServer->puerto, &hints, &serverInfo); // Notar que le pasamos NULL como IP, ya que le indicamos que use localhost en AI_PASSIVE

	// Obtengo un socket, utilizando la estructura serverInfo ya generada.
	datosConexionServer->listenningSocket = socket(serverInfo->ai_family,serverInfo->ai_socktype, serverInfo->ai_protocol);

	// Uso el file descriptor previo para decirle al sistema por donde voy a escuchar las conexiones
	bind(datosConexionServer->listenningSocket, serverInfo->ai_addr,serverInfo->ai_addrlen);
	freeaddrinfo(serverInfo); // Ya no lo vamos a necesitar

	// Empiezo a escuchar las conexiones. IMPORTANTE: listen() es una syscall BLOQUEANTE.
	listen(datosConexionServer->listenningSocket, BACKLOG);	//0:ok - (-1):error

	return !response;
}

int aceptarConexion(datosConfigServer *datosConexionServer) {
	int response=0;

	// Acepto la conexión del cliente y abro el *nuevo* socket
	struct sockaddr_in addr; // Esta estructura contendra los datos de la conexion del cliente. IP, puerto, etc.
	socklen_t addrlen = sizeof(addr);
	datosConexionServer->socketCliente = accept(datosConexionServer->listenningSocket, (struct sockaddr *) &addr,&addrlen);
	if(datosConexionServer->socketCliente==(-1)){	//-1:error
		response=1;
	}

	return !response;	// Niego para que devuelva <> 0 en SUCCESS
}

void cerrarServer(datosConfigServer *datosConexionServer) {

	// Cierro la conexión y libero el socket
	close(datosConexionServer->socketCliente);
	close(datosConexionServer->listenningSocket);
	printf("\nCliente desconectado.");
	return;
}
