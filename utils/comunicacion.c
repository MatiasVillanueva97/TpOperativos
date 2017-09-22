
#include <commons/string.h>

struct headerProtocolo{
	int id;
	int tamPayload;
	//int origen;	//redundante porque el server debería conocer quién se le conecta de acuerdo al socket, la primera vez que se le conecta alguien le pregunta quién es y ahí ya memoriza de quién es el socket
};

//el send no manda siempre todos los bytes que le pongo por el protocolo IP
//leer la cantidad de bytes enviados que es lo que devuelve
int enviarMensaje(int serverSocket,char *message){
	return send(serverSocket, message, strlen(message) + 1, 0);
}

struct headerProtocolo armarHeader(int idMensaje, int tamMensaje){
	struct headerProtocolo header;
	header.id=idMensaje;
	header.tamPayload=tamMensaje;
	return header;
}

char* serializarHeader(struct headerProtocolo header){
	char *headerSerializado= malloc(sizeof(struct headerProtocolo));
	memcpy(headerSerializado, string_itoa(header.id),sizeof(int));
	printf("print id: %s\n",headerSerializado);
	memcpy(headerSerializado,string_itoa(header.tamPayload),sizeof(int));
	printf("print tamaño: %s\n",headerSerializado);
	return headerSerializado;
}

char* recibirHeader(int socketCliente,char *message){
	if(recv(socketCliente,(void*) message, (sizeof(struct headerProtocolo)), 0)<0){
		perror("Recepción Header");
		return -1;
	}
	return message;
}

char* recibirMensaje(int socketCliente,char *message,int packageSize){
	if(recv(socketCliente,(void*) message, packageSize, 0)<0){
		perror("Recepción Mensaje");
		return -1;
	}
	return message;
}
