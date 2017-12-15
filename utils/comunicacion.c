#include "constantes.h"
#include <stdint.h>

/* ****************************** funciones para enviar mensajes ******************************/
//el send no manda siempre todos los bytes que le pongo por el protocolo IP
//leer la cantidad de bytes enviados que es lo que devuelve
int enviarMensaje(int serverSocket, char *message) {
	int cantBytesEnviados = send(serverSocket, message, string_length(message), MSG_WAITALL);

	if (cantBytesEnviados != string_length(message)) {

		puts("Error. No se enviaron todos los bytes del mensaje.\n");
		printf("Largo mensaje: %d, bytes enviados: %d\n", string_length(message), cantBytesEnviados);
		return 0;

	}
	return cantBytesEnviados;
}

int enviarHeaderSolo(int serverSocket, int32_t headerId) {
	int32_t largoStringId = LARGO_STRING_HEADER_ID;
	char idString[largoStringId];
	strcpy(idString, intToArrayZerosLeft(headerId, 4));
	//printf("mensajeHeaderSolo: %s\n", idString);
	return enviarMensaje(serverSocket, idString);
}

/* ******************************** funciones para recibir mensajes ********************************/
/*
 * recibe por socket un mensaje
 * parámetros: el puntero donde guardar el mensaje, socket del cliente y el largo del string (string_length)
 */
int recibirMensaje(char *message, int socketCliente, int packageSize) {
	//char *message=malloc(packageSize);
	int cantBytesRecibidos = recv(socketCliente, message, packageSize, MSG_WAITALL);
	if (cantBytesRecibidos == -1) {
		perror("Error en recepción mensaje");
		strcpy(message, "-1");
	}
	//printf ("cant bytes recibidos: %d", cantBytesRecibidos);
	return cantBytesRecibidos;
	}

/* **************** funciones para serializar y deserializar mensajes ************ */
char* serializarMensaje(int32_t idMensaje, char **arrayMensajes, int cantStrings) {
	int i;
	int32_t offsetPuntero = 0;
	int32_t largoStringId = LARGO_STRING_HEADER_ID;
	int32_t largoStringTamMensaje = LARGO_STRING_TAM_MENSAJE;

//pido memoria para el mensaje serializado calculando el tamaño a poner en el malloc
	int largoMensajeSerializado = largoStringId;
	for (i = 0; i < cantStrings; i++) {
		largoMensajeSerializado += largoStringTamMensaje;
		largoMensajeSerializado += string_length(arrayMensajes[i]);
	}
	char *mensajeSerializado = malloc(largoMensajeSerializado + 1);

//agrego el id del mensaje a la serialización
	char *idString = intToArrayZerosLeft(idMensaje, largoStringId);
	memcpy(mensajeSerializado + offsetPuntero, idString, largoStringId);
	offsetPuntero += largoStringId;
//cada vuelta del for agrega el tamaño del mensaje y el mensaje
	for (i = 0; i < cantStrings; i++) {
		int32_t largoMensaje = string_length(arrayMensajes[i]);

		char *largoMensajeString = intToArrayZerosLeft(largoMensaje, largoStringTamMensaje);
		memcpy(mensajeSerializado + offsetPuntero, largoMensajeString, largoStringTamMensaje);
		offsetPuntero += largoStringTamMensaje;
		memcpy(mensajeSerializado + offsetPuntero, arrayMensajes[i], largoMensaje);
		offsetPuntero += largoMensaje;
	}
	mensajeSerializado[largoMensajeSerializado] = '\0';
	return mensajeSerializado;
}

/*
 * hace un recv del tamaño del header para extraer el mismo del mensaje serializado
 * devuelve el id del header como int
 */
int32_t deserializarHeader(int socketCliente) {
	char idString[LARGO_STRING_HEADER_ID + 1];
//	recibirMensaje(idString, socketCliente, LARGO_STRING_HEADER_ID);
	int bytes = recibirMensaje(idString, socketCliente, LARGO_STRING_HEADER_ID);
	if (bytes <= 0) {
		return -1;
	}
	idString[LARGO_STRING_HEADER_ID] = '\0';
	int32_t headerId = atoi(idString);
	return headerId;
}

/*
 * hace un recv con el tamaño del tamMensaje para recibir el tamaño del mensaje posta
 * hace un recv con el tamaño de mensaje leído para recibir el mensaje posta
 * recibe el socket del cliente
 * devuelve el mensaje
 */
char** deserializarMensaje(int socketCliente, int cantMensajes) {
	int i, cantBytesRecibidos = 0;

	char **arrayMensajes = malloc(sizeof(char*) * cantMensajes);
	if (!arrayMensajes)
		perror("error de malloc");
	for (i = 0; i < cantMensajes; i++) {
		char tamMensajeString[LARGO_STRING_TAM_MENSAJE + 1];
		cantBytesRecibidos += recibirMensaje(tamMensajeString, socketCliente, LARGO_STRING_TAM_MENSAJE);
		tamMensajeString[LARGO_STRING_TAM_MENSAJE] = '\0';
		int tamMensaje = atoi(tamMensajeString);

		arrayMensajes[i] = malloc(tamMensaje + 1);
		if (!arrayMensajes[i])
			perror("Error de malloc");
		cantBytesRecibidos += recibirMensaje(arrayMensajes[i], socketCliente, tamMensaje);
		arrayMensajes[i][tamMensaje] = '\0';
	}
	//printf("cantBytesRecibidos: %d\n", cantBytesRecibidos);
	return arrayMensajes;
}

