
/*
 * EN DESUSO
 */
/*struct headerProtocolo{
	int id;
	int tamPayload;
	//int origen;	//redundante porque el server debería conocer quién se le conecta de acuerdo al socket, la primera vez que se le conecta alguien le pregunta quién es y ahí ya memoriza de quién es el socket
};*/

/* *************************** funciones para el header ********************************* */
/*
 * EN DESUSO
 */
/*struct headerProtocolo armarHeader(int idMensaje, int tamMensaje){
	struct headerProtocolo header;
	header.id=idMensaje;
	header.tamPayload=tamMensaje;
	return header;
}*/

char* serializarMensaje(uint32_t idMensaje,char **arrayMensajes){
	int i;
	uint32_t offsetPuntero=0;
	uint32_t largoStringId=LARGO_STRING_HEADER_ID;
	uint32_t largoStringTamMensaje=LARGO_STRING_TAM_MENSAJE;

	//pido memoria para el mensaje serializado calcuando el tamaño a poner en el malloc
	int largoMensajeSerializado=largoStringId;
	for(i=0;i<sizeof(arrayMensajes);i++){
		largoMensajeSerializado+=largoStringTamMensaje;
		largoMensajeSerializado+=string_length(arrayMensajes[i]);
	}

	void *mensajeSerializado = malloc(largoMensajeSerializado);

	//agrego el id del mensaje a la serialización
	char *idString=intToArrayZerosLeft(idMensaje,largoStringId);
	memcpy(mensajeSerializado+offsetPuntero,idString,largoStringId);
	offsetPuntero+=largoStringId;

	//cada vuelta del for agrega el tamaño del mensaje y el mensaje
	for(i=0;i<sizeof(arrayMensajes);i++){

		uint32_t largoMensaje= string_length(arrayMensajes[i]);

		char *largoMensajeString=intToArrayZerosLeft(largoMensaje,largoStringTamMensaje);
		memcpy(mensajeSerializado+offsetPuntero,largoMensajeString,largoStringTamMensaje);
		offsetPuntero+=largoStringTamMensaje;

		memcpy(mensajeSerializado+offsetPuntero,arrayMensajes[i],largoMensaje);
		offsetPuntero+=largoMensaje;
	}
	return mensajeSerializado;
}

/*
 * hace un recv del tamaño del header para extraer el mismo del mensaje serializado
 * devuelve el id del header como int
 */
int deserializarHeader(int socketCliente){
	char* idString=malloc(LARGO_STRING_HEADER_ID+1);
	recibirMensaje(idString,socketCliente,LARGO_STRING_HEADER_ID);
	if(idString<0){
		return -1;
	}
	idString[LARGO_STRING_HEADER_ID]='\0';
	int headerId=atoi(idString);
	free(idString);
	return headerId;
}

/*
 * hace un recv con el tamaño del tamMensaje para recibir el tamaño del mensaje posta
 * hace un recv con el tamaño de mensaje leído para recibir el mensaje posta
 * recibe el socket del cliente
 * devuelve el mensaje
 */
int deserializarTamMensaje(int socketCliente){
	char *tamMensajeString=malloc(LARGO_STRING_TAM_MENSAJE+1);
	recibirMensaje(tamMensajeString, socketCliente,LARGO_STRING_TAM_MENSAJE);
	if(tamMensajeString<0){
		perror("Recibir tamaño de mensaje");
		return -1;
	}
	tamMensajeString[LARGO_STRING_TAM_MENSAJE]='\0';
	int tamMensaje=atoi(tamMensajeString);
	free(tamMensajeString);
	return tamMensaje;
}
void deserializarMensaje(char *mensajeRecibido,int socketCliente,int tamMensaje){
	recibirMensaje(mensajeRecibido,socketCliente,tamMensaje);
	if(mensajeRecibido<0){
		perror("Recepción del mensaje");
	}
	mensajeRecibido[tamMensaje]='\0';

}

/*
 * EN DESUSO
 */
/*struct headerProtocolo deserializarHeader(char * headerSerializado){
	struct headerProtocolo header;
	header.id=*headerSerializado;
	header.tamPayload=*(headerSerializado+sizeof(header.id));
	return header;
}*/
