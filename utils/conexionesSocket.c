/*
 * conexionesSocket.c
 *
 *  Created on: 18/9/2017
 *      Author: utnso
 */

#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <commons/string.h>

#define MAX_CONEXIONES 10

/* ******************** funciones para clientes de conexiones ************************ */
int conectarA(char *ipServer, char *puertoServer){
	// Uso getaddrinfo() para obtener los datos de la direccion de red y guardarlos en serverInfo.
	struct addrinfo hints;
	struct addrinfo *serverInfo;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC; // Permite que la maquina se encargue de verificar si usamos IPv4 o IPv6
	hints.ai_socktype = SOCK_STREAM; // Indica que usaremos el protocolo TCP

	getaddrinfo(ipServer, puertoServer, &hints, &serverInfo);	// Carga en serverInfo los datos de la conexion

	// Obtengo un socket, utilizando la estructura serverInfo ya generada.
	int socketServer = socket(serverInfo->ai_family,serverInfo->ai_socktype, serverInfo->ai_protocol);

	if(socketServer<0){
		perror("socket");
		return -1;
	}

	// Me conecto al server usando el file descriptor del socket previo.
	if(connect(socketServer, serverInfo->ai_addr, serverInfo->ai_addrlen)<0){
		perror("connect");
		return -1;
	}
	freeaddrinfo(serverInfo);	// No lo necesitamos mas

	return socketServer;
}

void cerrarCliente(int socketServer){
	// Cierro la conexión y libero el socket
	close(socketServer);
	return;
}

/* ******************** funciones para servers de conexiones ************************ */
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
	int listenningSocket = socket(serverInfo->ai_family,serverInfo->ai_socktype, serverInfo->ai_protocol);

	if(listenningSocket<0){
		perror("socket");
		return -1;
	}

	// Uso el file descriptor previo para decirle al sistema por donde voy a escuchar las conexiones
	if(bind(listenningSocket, serverInfo->ai_addr,serverInfo->ai_addrlen)!=0){
		perror("bind");
		return -1;
	}
	freeaddrinfo(serverInfo); // Ya no lo vamos a necesitar

	// Empiezo a escuchar las conexiones. IMPORTANTE: listen() es una syscall BLOQUEANTE.
	if(listen(listenningSocket, MAX_CONEXIONES)!=0){
		perror("listen");
		return -1;
	}

	return listenningSocket;
}

int aceptarConexion(int listenningSocket) {
	int socketCliente;
	// Acepto la conexión del cliente y abro el *nuevo* socket
	struct sockaddr_in addr; // Esta estructura contendra los datos de la conexion del cliente. IP, puerto, etc.
	socklen_t addrlen = sizeof(addr);
	if((socketCliente = accept(listenningSocket, (struct sockaddr *) &addr,&addrlen))<0){
		perror("accept");
		return -1;
	}
	return socketCliente;
}

void cerrarServer(int listenningSocket) {
	// Cierro la conexión y libero el socket
	//close(socketCliente);
	close(listenningSocket);
	printf("\nCliente desconectado.\n");
	return;
}
