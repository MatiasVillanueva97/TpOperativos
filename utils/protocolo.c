

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
char* serializarHeader(struct headerProtocolo header){
	void *headerSerializado= malloc(sizeof(struct headerProtocolo));
	//printf("tamaño header serializado: %d - %s\n",string_length(headerSerializado),(char *) headerSerializado);

	memcpy(headerSerializado, &(header.id),sizeof(header.id));
	memcpy(headerSerializado+sizeof(int),&(header.tamPayload),sizeof(header.tamPayload));
	//string_append_with_format(&headerSerializado, "%s", headerSerializado+sizeof(int));
	//printf("tamaño header serializado: %d - %s\n",string_length(headerSerializado),(char *) headerSerializado);

	return headerSerializado;
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
