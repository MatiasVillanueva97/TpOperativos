/*
 * comunicacionesFS.c
 *
 *  Created on: 24/11/2017
 *      Author: utnso
 */

#define CANT_MENSAJES_POR_BLOQUE_DE_ARCHIVO 5
#define CANT_MENSAJES_POR_NODO 3

typedef struct {
	int nodoCopia1;
	int bloqueCopia1;
	int nodoCopia2;
	int bloqueCopia2;
	int bytesBloque;
} bloqueArchivo;

#define LARGO_IP 16

typedef struct {
	int numero;
	char ip[LARGO_IP];
	int puerto;
} datosConexionNodo;

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
	return enviarMensaje(socketFS, mensajeSerializado);
}

bloqueArchivo* recibirMetadataArchivoFS(int socketFS) {
	/*int i;
	 int32_t headerId = deserializarHeader(socketFS);
	 if (headerId != TIPO_MSJ_METADATA_ARCHIVO) {
	 perror("El FS no mandó los bloques");
	 bloqueArchivo *bloquesError = malloc(sizeof(bloqueArchivo));
	 bloquesError[0].nodoCopia1 = 0;
	 bloquesError[0].bloqueCopia1 = 0;
	 bloquesError[0].nodoCopia2 = 0;
	 bloquesError[0].bloqueCopia2 = 0;
	 bloquesError[0].bytesBloque = 0;
	 return bloquesError;
	 }
	 int cantidadMensajes = protocoloCantidadMensajes[headerId];
	 char **arrayMensajes = deserializarMensaje(socketFS, cantidadMensajes);
	 //guardar la data en algún lado
	 cantPartesArchivo = atoi(arrayMensajes[0]);
	 printf("cantPartesArchivo: %d\n", cantPartesArchivo);

	 arrayMensajes = deserializarMensaje(socketFS, cantPartesArchivo * CANT_MENSAJES_POR_BLOQUE_DE_ARCHIVO);
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
	cantPartesArchivo = 6;
	bloqueArchivo *bloques = malloc(cantPartesArchivo * sizeof(bloqueArchivo));
	//bloqueArchivo bloquesPrueba[cantPartesArchivo];
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

datosConexionNodo * recibirNodosArchivoFS(int socketFS) {
	/*int i;
	 int32_t headerId = deserializarHeader(socketFS);
	 printf("headerId nodos: %d\n", headerId);
	 if (headerId != TIPO_MSJ_DATOS_CONEXION_NODOS) {
	 printf("El FS no mandó los nodos\n");
	 datosConexionNodo *nodosError = malloc(sizeof(bloqueArchivo));
	 nodosError[0].numero = 0;
	 strcpy(nodosError[0].ip, "");
	 nodosError[0].puerto = 0;
	 return nodosError;
	 }
	 int cantidadMensajes = protocoloCantidadMensajes[headerId];
	 char **arrayMensajes = deserializarMensaje(socketFS, cantidadMensajes);
	 //guardar la data en algún lado
	 cantNodosArchivo = atoi(arrayMensajes[0]);
	 printf("cantNodos: %d\n", cantNodosArchivo);
	 arrayMensajes = deserializarMensaje(socketFS, cantNodosArchivo * CANT_MENSAJES_POR_NODO);
	 datosConexionNodo *nodos = malloc(cantNodosArchivo * sizeof(datosConexionNodo));
	 int j = 0;
	 for (i = 0; i < cantNodosArchivo; i++) {
	 nodos[i].numero = atoi(arrayMensajes[j]);
	 j++;
	 strcpy(nodos[i].ip, arrayMensajes[j]);
	 j++;
	 nodos[i].puerto = atoi(arrayMensajes[j]);
	 j++;
	 }*/

	/* ***************** datos de bloques y nodos inventados para probar **************** */
	cantNodosArchivo = 3;
	datosConexionNodo *nodos = malloc(cantNodosArchivo * sizeof(datosConexionNodo));
	nodos[0].numero = 1;
	strcpy(nodos[0].ip, "127.0.0.001");
	nodos[0].puerto = 5300;

	nodos[1].numero = 2;
	strcpy(nodos[1].ip, "127.168.1.10");
	nodos[1].puerto = 5302;

	nodos[2].numero = 3;
	strcpy(nodos[2].ip, "127.0.0.1");
	nodos[2].puerto = 5303;

	/* **************************************************************** */
	return nodos;
}
