

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

/*
 * EN DESUSO
 * PARA CORREGIR
 */
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
 * EN DESUSO
 * PARA CORREGIR
 */
struct headerProtocolo deserializarHeader(char * headerSerializado){
	struct headerProtocolo header;
	header.id=*headerSerializado;
	header.tamPayload=*(headerSerializado+sizeof(header.id));
	return header;
}
