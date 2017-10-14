

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
char* serializarMensaje(uint32_t idMensaje,char *mensajeAEnviar){
	uint32_t largoMensaje= string_length(mensajeAEnviar);
	uint32_t largoStringId=LARGO_STRING_HEADER_ID;
	uint32_t largoStringTamMensaje=LARGO_STRING_TAM_MENSAJE;
	void *mensajeSerializado = malloc(largoStringId+largoStringTamMensaje+(largoMensaje+1));
	uint32_t offsetPuntero=0;

	char *idString=intToArrayZerosLeft(idMensaje,largoStringId);
	memcpy(mensajeSerializado+offsetPuntero,idString,largoStringId);
	offsetPuntero+=largoStringId;

	char *largoMensajeString=intToArrayZerosLeft(largoMensaje,largoStringTamMensaje);
	memcpy(mensajeSerializado+offsetPuntero,largoMensajeString,largoStringTamMensaje);
	offsetPuntero+=largoStringTamMensaje;

	memcpy(mensajeSerializado+offsetPuntero,mensajeAEnviar,largoMensaje+1);

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
