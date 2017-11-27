/*
 * comunicacionesFS.c
 *
 *  Created on: 24/11/2017
 *      Author: utnso
 */

#define CANT_MENSAJES_POR_BLOQUE_DE_ARCHIVO 6
#define CANT_MENSAJES_POR_NODO 3

int pedirMetadataArchivoFS(int socketFS, char *archivo) {
	int cantStrings = 1, i;
	char **arrayMensajesSerializar = malloc(sizeof(char*) * cantStrings);
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
	return enviarMensaje(socketFS, mensajeSerializado);
}

int getCantidadPartesArchivoFS(int socketFS, int cantMensajes) {
	char **arrayMensajes = deserializarMensaje(socketFS, cantMensajes);
	return atoi(arrayMensajes[0]);
}

bloqueArchivo* recibirMetadataArchivoFS(int socketFS, int cantPartesArchivo) {
	int i;
	char **arrayMensajes = deserializarMensaje(socketFS, cantPartesArchivo * CANT_MENSAJES_POR_BLOQUE_DE_ARCHIVO);
	bloqueArchivo *bloques = malloc(cantPartesArchivo * sizeof(bloqueArchivo));
	if (!bloques)
		perror("Error de malloc de bloques");
	int j = 0;
	for (i = 0; i < cantPartesArchivo; i++) {
		char **nodo1 = string_split(arrayMensajes[j], "_");
		bloques[i].nodoCopia1 = atoi(nodo1[1]);
		j++;
		bloques[i].bloqueCopia1 = atoi(arrayMensajes[j]);
		j++;
		bloques[i].bytesBloque = atoi(arrayMensajes[j]);
		j++;
		char **nodo2 = string_split(arrayMensajes[j], "_");
		bloques[i].nodoCopia2 = atoi(nodo2[1]);
		j++;
		bloques[i].bloqueCopia2 = atoi(arrayMensajes[j]);
		j++;
		bloques[i].bytesBloque = atoi(arrayMensajes[j]);
		j++;
	}

	/* ***************** datos de bloques y nodos inventados para probar **************** */
//	bloqueArchivo *bloques = malloc(cantPartesArchivo * sizeof(bloqueArchivo));
//	bloques[0].nodoCopia1 = 1;
//	bloques[0].bloqueCopia1 = 33;
//	bloques[0].nodoCopia2 = 3;
//	bloques[0].bloqueCopia2 = 13;
//	bloques[0].bytesBloque = 12564;
//
//	bloques[1].nodoCopia1 = 2;
//	bloques[1].bloqueCopia1 = 36;
//	bloques[1].nodoCopia2 = 3;
//	bloques[1].bloqueCopia2 = 21;
//	bloques[1].bytesBloque = 3264;
//
//	bloques[2].nodoCopia1 = 2;
//	bloques[2].bloqueCopia1 = 12;
//	bloques[2].nodoCopia2 = 3;
//	bloques[2].bloqueCopia2 = 55;
//	bloques[2].bytesBloque = 3264;
//
//	bloques[3].nodoCopia1 = 2;
//	bloques[3].bloqueCopia1 = 65;
//	bloques[3].nodoCopia2 = 3;
//	bloques[3].bloqueCopia2 = 5;
//	bloques[3].bytesBloque = 10264;
//
//	bloques[4].nodoCopia1 = 1;
//	bloques[4].bloqueCopia1 = 88;
//	bloques[4].nodoCopia2 = 3;
//	bloques[4].bloqueCopia2 = 101;
//	bloques[4].bytesBloque = 4264;
//
//	bloques[5].nodoCopia1 = 1;
//	bloques[5].bloqueCopia1 = 74;
//	bloques[5].nodoCopia2 = 2;
//	bloques[5].bloqueCopia2 = 120;
//	bloques[5].bytesBloque = 32334;
	/* **************** fin código de pruebas ************************** */
	return bloques;
}

int getCantidadNodosFS(int socketFS, int cantMensajes) {
	printf("cantMensajes: %d\n",cantMensajes);
	char **arrayMensajes = deserializarMensaje(socketFS, cantMensajes);
	printf("cantMensajesString: %s\n",arrayMensajes[0]);
	return atoi(arrayMensajes[0]);
}

void recibirNodosArchivoFS(int socketFS, int cantNodosArchivo, datosPropiosNodo *nodosParaPlanificar) {
	int nroNodo, i;
	int cantMensajes = cantNodosArchivo * CANT_MENSAJES_POR_NODO;
	char **arrayMensajes = deserializarMensaje(socketFS, cantMensajes);
	int j = 0;
	for (i = 0; i < cantNodosArchivo; i++) {
		char **nodo = string_split(arrayMensajes[j], "_");
		nroNodo = atoi(nodo[1]);
		listaGlobalNodos[nroNodo].numero = nroNodo;
		nodosParaPlanificar[i].numero = nroNodo;
		printf("nodo: %d\n", nroNodo);
		j++;
		strcpy(listaGlobalNodos[nroNodo].ip, arrayMensajes[j]);
		strcpy(nodosParaPlanificar[i].ip, arrayMensajes[j]);
		j++;
		listaGlobalNodos[nroNodo].puerto = atoi(arrayMensajes[j]);
		nodosParaPlanificar[i].puerto = atoi(arrayMensajes[j]);
		j++;
	}
	for (i = 0; i < cantMensajes; i++) {
		free(arrayMensajes[i]);
	}
	free(arrayMensajes);
	return;
	/* ***************** datos de bloques y nodos inventados para probar **************** */
	/*i = 0;
	nodo = string_split("NODO_1", "_");
	nroNodo = atoi(nodo[1]);
	listaGlobalNodos[nroNodo].numero = nroNodo;
	printf("nodo: %d\n", nroNodo);
	strcpy(listaGlobalNodos[nroNodo].ip, "127.0.0.001");
	listaGlobalNodos[nroNodo].puerto = 5300;
	nodosParaPlanificar[i].numero = nroNodo;
	strcpy(nodosParaPlanificar[i].ip, "127.0.0.001");
	nodosParaPlanificar[i].puerto = 5300;
	i++;
	nodo = string_split("NODO_2", "_");
	nroNodo = atoi(nodo[1]);
	printf("nodo: %d\n", nroNodo);
	listaGlobalNodos[nroNodo].numero = nroNodo;
	strcpy(listaGlobalNodos[nroNodo].ip, "127.168.1.10");
	listaGlobalNodos[nroNodo].puerto = 5302;
	nodosParaPlanificar[i].numero = nroNodo;
	strcpy(nodosParaPlanificar[i].ip, "127.168.1.10");
	nodosParaPlanificar[i].puerto = 5302;
	i++;
	nodo = string_split("NODO_3", "_");
	nroNodo = atoi(nodo[1]);
	printf("nodo: %d\n", nroNodo);
	listaGlobalNodos[nroNodo].numero = nroNodo;
	strcpy(listaGlobalNodos[nroNodo].ip, "127.0.0.1");
	listaGlobalNodos[nroNodo].puerto = 5303;
	nodosParaPlanificar[i].numero = nroNodo;
	strcpy(nodosParaPlanificar[i].ip, "127.0.0.1");
	nodosParaPlanificar[i].puerto = 5303;
	i++;*/
	/* **************************************************************** */
}
