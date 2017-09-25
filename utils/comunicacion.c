

struct headerProtocolo{
	int id;
	int tamPayload;
	//int origen;	//redundante porque el server debería conocer quién se le conecta de acuerdo al socket, la primera vez que se le conecta alguien le pregunta quién es y ahí ya memoriza de quién es el socket
};

/* *************************** funciones para el header ********************************* */
struct headerProtocolo armarHeader(int idMensaje, int tamMensaje){
	struct headerProtocolo header;
	header.id=idMensaje;
	header.tamPayload=tamMensaje;
	return header;
}

char* serializarHeader(struct headerProtocolo header){
	void *headerSerializado= malloc(sizeof(struct headerProtocolo));
	printf("tamaño header serializado: %d - %s\n",string_length(headerSerializado),(char *) headerSerializado);

	memcpy(headerSerializado, &(header.id),sizeof(header.id));
	memcpy(headerSerializado+sizeof(int),&(header.tamPayload),sizeof(header.tamPayload));
	//string_append_with_format(&headerSerializado, "%s", headerSerializado+sizeof(int));
	printf("tamaño header serializado: %d - %s\n",string_length(headerSerializado),(char *) headerSerializado);

	return headerSerializado;
}

struct headerProtocolo deserializarHeader(char * headerSerializado){
	struct headerProtocolo header;
	header.id=*headerSerializado;
	header.tamPayload=*(headerSerializado+sizeof(header.id));
	return header;
}

/* ****************************** funciones para enviar mensajes ******************************/
int enviarHeader(int serverSocket,struct headerProtocolo header){
	int cantBytesEnviados;
	char *idString=intToArrayZerosLeft(header.id,sizeof(header.id));
	char *tamPayloadString=intToArrayZerosLeft(header.tamPayload,sizeof(header.tamPayload));
	cantBytesEnviados = send(serverSocket,idString, string_length(idString)+1, 0);
	//printf("String Length del id del header: %d\n",string_length(idString));
	//printf("Bytes enviados del id del header: %d\n",cantBytesEnviados);
	if(cantBytesEnviados!=( string_length(idString)+1)){
		puts("Error. No se enviaron todos los bytes del id del header\n");
		return 0;
	}
	cantBytesEnviados = send(serverSocket,tamPayloadString, string_length(tamPayloadString)+1, 0);
	//printf("String Length del tamaño del header: %d\n",string_length(tamPayloadString));
	//printf("Bytes enviados del tamaño del header: %d\n",cantBytesEnviados);
	if(cantBytesEnviados!=(string_length(tamPayloadString) + 1)){
		puts("Error. No se enviaron todos los bytes del tamaño del header\n");
		return 0;
	}
	return 1;
}
//el send no manda siempre todos los bytes que le pongo por el protocolo IP
//leer la cantidad de bytes enviados que es lo que devuelve
int enviarMensaje(int serverSocket,char *message){
	int cantBytesEnviados = send(serverSocket, message, string_length(message)+1, 0);
	//printf("String Length del mensaje: %d\n",string_length(message));
	//printf("Bytes enviados del mensaje: %d\n",cantBytesEnviados);
	if(cantBytesEnviados!=( string_length(message)+1)){
		puts("Error. No se enviaron todos los bytes del mensaje\n");
		return 0;
	}
	return 1;
}


/* ******************************** funciones para recibir mensajes ********************************/
struct headerProtocolo recibirHeader(int socketCliente){
	int idEntero,tamEntero,packageSizeId=5,packageSizeTam=5;		// 4+1 hardcodeado a revisar
	char id[packageSizeId],tamPayload[packageSizeTam];
	if(recv(socketCliente,(void*) id, packageSizeId, 0)<0){
		perror("Recepción Id Header");
		struct headerProtocolo header=armarHeader(-1,tamEntero);
		return header;
	}
	if(recv(socketCliente,(void*) tamPayload, packageSizeTam, 0)<0){
		perror("Recepción Tamaño Payload Header");
		struct headerProtocolo header=armarHeader(-1,tamEntero);
		return header;
	}
	//printf("recepción en char *: %s - %s\n",id,tamPayload);
	sscanf(id, "%d", &idEntero);
	sscanf(tamPayload, "%d", &tamEntero);
	struct headerProtocolo header=armarHeader(idEntero,tamEntero);
	return header;
}

/*
 * recibe por socket un mensaje
 * parámetros: socket del cliente y el largo del string (string_length)
 * devuelve un mensaje como char*
 */
char* recibirMensaje(int socketCliente,int packageSize){
	char *message=malloc(packageSize+1);
	if(recv(socketCliente,(void*) message,packageSize+1, 0)<0){
		perror("Recepción Mensaje");
		return (char*) -1;
	}
	return message;
}
