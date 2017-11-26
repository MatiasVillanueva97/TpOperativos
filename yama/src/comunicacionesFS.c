/*
 * comunicacionesFS.c
 *
 *  Created on: 24/11/2017
 *      Author: utnso
 */

#define CANT_MENSAJES_POR_BLOQUE_DE_ARCHIVO 5
#define CANT_MENSAJES_POR_NODO 3

int pedirMetadataArchivoFS(int socketFS, char *archivo) {
	int cantStrings = 1, i;
	char **arrayMensajesSerializar = malloc(sizeof(char*) * cantStrings);
	printf("archivo a trabajar: %s\n", archivo);
	printf("largo del archivo a trabajar: %d\n", string_length(archivo));
	arrayMensajesSerializar[0] = malloc(string_length(archivo) + 1);
	if (!arrayMensajesSerializar[0])
		perror("error de malloc");
	strcpy(arrayMensajesSerializar[0], archivo);
	printf("arrayMensajesSerializar[0]: %s\n", arrayMensajesSerializar[0]);
	char *mensajeSerializado = serializarMensaje(TIPO_MSJ_PEDIDO_METADATA_ARCHIVO, arrayMensajesSerializar, cantStrings);
	for (i = 0; i < cantStrings; i++) {
		free(arrayMensajesSerializar[i]);
	}
	free(arrayMensajesSerializar);
	//return enviarMensaje(socketFS, mensajeSerializado);
	return 1;
}

int getCantidadPartesArchivoFS(int socketFS, int cantMensajes) {
	char **arrayMensajes = deserializarMensaje(socketFS, cantMensajes);
	return atoi(arrayMensajes[0]);
}

bloqueArchivo* recibirMetadataArchivoFS(int socketFS, int cantPartesArchivo) {
	/*int i;

	 char **arrayMensajes = deserializarMensaje(socketFS, cantPartesArchivo * CANT_MENSAJES_POR_BLOQUE_DE_ARCHIVO);
	 bloqueArchivo *bloques = malloc(cantPartesArchivo * sizeof(bloqueArchivo));
	 int j = 0;
	 for (i = 0; i < cantPartesArchivo; i++) {
	 bloques[i].nodoCopia1 = atoi(arrayMensajes[j]);
	 j++;
	 bloques[i].bloqueCopia1 = atoi(arrayMensajes[j]);
	 j++;
	 bloques[i].nodoCopia2 = atoi(arrayMensajes[j]);
	 j++;
	 bloques[i].bloqueCopia2 = atoi(arrayMensajes[j]);
	 j++;
	 bloques[i].bytesBloque = atoi(arrayMensajes[j]);

	 j++;
	 }*/

	/* ***************** datos de bloques y nodos inventados para probar **************** */
	bloqueArchivo *bloques = malloc(cantPartesArchivo * sizeof(bloqueArchivo));
	bloques[0].nodoCopia1 = 1;
	bloques[0].bloqueCopia1 = 33;
	bloques[0].nodoCopia2 = 3;
	bloques[0].bloqueCopia2 = 13;
	bloques[0].bytesBloque = 12564;

	bloques[1].nodoCopia1 = 2;
	bloques[1].bloqueCopia1 = 36;
	bloques[1].nodoCopia2 = 3;
	bloques[1].bloqueCopia2 = 21;
	bloques[1].bytesBloque = 3264;

	bloques[2].nodoCopia1 = 2;
	bloques[2].bloqueCopia1 = 12;
	bloques[2].nodoCopia2 = 3;
	bloques[2].bloqueCopia2 = 55;
	bloques[2].bytesBloque = 3264;

	bloques[3].nodoCopia1 = 2;
	bloques[3].bloqueCopia1 = 65;
	bloques[3].nodoCopia2 = 3;
	bloques[3].bloqueCopia2 = 5;
	bloques[3].bytesBloque = 10264;

	bloques[4].nodoCopia1 = 1;
	bloques[4].bloqueCopia1 = 88;
	bloques[4].nodoCopia2 = 3;
	bloques[4].bloqueCopia2 = 101;
	bloques[4].bytesBloque = 4264;

	bloques[5].nodoCopia1 = 1;
	bloques[5].bloqueCopia1 = 74;
	bloques[5].nodoCopia2 = 2;
	bloques[5].bloqueCopia2 = 120;
	bloques[5].bytesBloque = 32334;

	/* **************** fin código de pruebas ************************** */
	return bloques;
}

int getCantidadNodosFS(int socketFS, int cantMensajes) {
	char **arrayMensajes = deserializarMensaje(socketFS, cantMensajes);
	return atoi(arrayMensajes[0]);
}

void recibirNodosArchivoFS(int socketFS, int cantNodosArchivo) {
	int nroNodo, i;
	char **nodo = malloc(sizeof(char*) * 3);	//se usa para el string_split
	int cantMensajes = cantNodosArchivo * CANT_MENSAJES_POR_NODO;
	/*char **arrayMensajes = deserializarMensaje(socketFS, cantMensajes);
	int j = 0;
	for (i = 0; i < cantNodosArchivo; i++) {
		nodo = string_split(arrayMensajes[j], "_");
		nroNodo = atoi(nodo[1]);
		listaGlobalNodos[nroNodo].numero = nroNodo;
		j++;
		strcpy(listaGlobalNodos[nroNodo].ip, arrayMensajes[j]);
		j++;
		listaGlobalNodos[nroNodo].puerto = atoi(arrayMensajes[j]);
		j++;
	}
	for (i = 0; i < cantMensajes; i++) {
		free(arrayMensajes[i]);
	}
	free(arrayMensajes);*/

	/* ***************** datos de bloques y nodos inventados para probar **************** */

	nodo = string_split("NODO_1", "_");
	nroNodo = atoi(nodo[1]);
	listaGlobalNodos[nroNodo].numero = nroNodo;
	printf("nodo: %d\n", nroNodo);
	strcpy(listaGlobalNodos[nroNodo].ip, "127.0.0.001");
	listaGlobalNodos[nroNodo].puerto = 5300;

	nodo = string_split("NODO_2", "_");
	nroNodo = atoi(nodo[1]);
	printf("nodo: %d\n", nroNodo);
	listaGlobalNodos[nroNodo].numero = nroNodo;
	strcpy(listaGlobalNodos[nroNodo].ip, "127.168.1.10");
	listaGlobalNodos[nroNodo].puerto = 5302;

	nodo = string_split("NODO_3", "_");
	nroNodo = atoi(nodo[1]);
	printf("nodo: %d\n", nroNodo);
	listaGlobalNodos[nroNodo].numero = nroNodo;
	strcpy(listaGlobalNodos[nroNodo].ip, "127.0.0.1");
	listaGlobalNodos[nroNodo].puerto = 5303;

	/* **************************************************************** */
}
