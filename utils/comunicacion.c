#include "constantes.h"
#include <stdint.h>

/* ****************************** funciones para enviar mensajes ******************************/
//el send no manda siempre todos los bytes que le pongo por el protocolo IP
//leer la cantidad de bytes enviados que es lo que devuelve
int enviarMensaje(int serverSocket, char *message) {
	int cantBytesEnviados = send(serverSocket, message, string_length(message), 0);
	if (cantBytesEnviados != string_length(message)) {
		puts("Error. No se enviaron todos los bytes del mensaje\n");
		return 0;
	}
	return cantBytesEnviados;
}

int enviarHeaderSolo(int serverSocket, uint32_t headerId) {
	uint32_t largoStringId = LARGO_STRING_HEADER_ID;
	char idString[largoStringId];
	strcpy(idString, intToArrayZerosLeft(headerId, 4));
	printf("mensajeHeaderSolo: %s\n", idString);
	return enviarMensaje(serverSocket, idString);
}

/* ******************************** funciones para recibir mensajes ********************************/
/*
 * recibe por socket un mensaje
 * parámetros: el puntero donde guardar el mensaje, socket del cliente y el largo del string (string_length)
 */
void recibirMensaje(char *message, int socketCliente, int packageSize) {
	//char *message=malloc(packageSize);
	if (recv(socketCliente, message, packageSize, 0) < 0) {
		perror("Recepción Mensaje");
		strcpy(message, "-1");
	}
}

/* **************** funciones para serializar y deserializar mensajes ************ */
char* serializarMensaje(uint32_t idMensaje, char **arrayMensajes, int cantStrings) {
	int i;
	uint32_t offsetPuntero = 0;
	uint32_t largoStringId = LARGO_STRING_HEADER_ID;
	uint32_t largoStringTamMensaje = LARGO_STRING_TAM_MENSAJE;

	//pido memoria para el mensaje serializado calculando el tamaño a poner en el malloc
	int largoMensajeSerializado = largoStringId;
	for (i = 0; i < cantStrings; i++) {
		largoMensajeSerializado += largoStringTamMensaje;
		largoMensajeSerializado += string_length(arrayMensajes[i]);
	}
	char *mensajeSerializado = malloc(largoMensajeSerializado+1);

	//agrego el id del mensaje a la serialización
	char *idString = intToArrayZerosLeft(idMensaje, largoStringId);
	memcpy(mensajeSerializado + offsetPuntero, idString, largoStringId);
	offsetPuntero += largoStringId;
	//cada vuelta del for agrega el tamaño del mensaje y el mensaje
	for (i = 0; i < cantStrings; i++) {
		uint32_t largoMensaje = string_length(arrayMensajes[i]);

		char *largoMensajeString = intToArrayZerosLeft(largoMensaje, largoStringTamMensaje);
		memcpy(mensajeSerializado + offsetPuntero, largoMensajeString, largoStringTamMensaje);
		offsetPuntero += largoStringTamMensaje;
		memcpy(mensajeSerializado + offsetPuntero, arrayMensajes[i], largoMensaje);
		offsetPuntero += largoMensaje;
	}
	mensajeSerializado[largoMensajeSerializado]='\0';
	return mensajeSerializado;
}

/*
 * hace un recv del tamaño del header para extraer el mismo del mensaje serializado
 * devuelve el id del header como int
 */
uint32_t deserializarHeader(int socketCliente) {
	char idString[LARGO_STRING_HEADER_ID + 1];
	recibirMensaje(idString, socketCliente, LARGO_STRING_HEADER_ID);
	if (idString < 0) {
		return -1;
	}
	idString[LARGO_STRING_HEADER_ID] = '\0';
	uint32_t headerId = atoi(idString);
	return headerId;
}

/*
 * hace un recv con el tamaño del tamMensaje para recibir el tamaño del mensaje posta
 * hace un recv con el tamaño de mensaje leído para recibir el mensaje posta
 * recibe el socket del cliente
 * devuelve el mensaje
 */
char** deserializarMensaje(int socketCliente, int cantMensajes) {
	int i;
	char **arrayMensajes = malloc(cantMensajes * sizeof(char*));
	for (i = 0; i < cantMensajes; i++) {
		char tamMensajeString[LARGO_STRING_TAM_MENSAJE + 1];
		recibirMensaje(tamMensajeString, socketCliente, LARGO_STRING_TAM_MENSAJE);
		tamMensajeString[LARGO_STRING_TAM_MENSAJE] = '\0';
		int tamMensaje = atoi(tamMensajeString);

		arrayMensajes[i] = malloc(tamMensaje + 1);
		recibirMensaje(arrayMensajes[i], socketCliente, tamMensaje);
		arrayMensajes[i][tamMensaje] = '\0';

	}
	return arrayMensajes;
}

