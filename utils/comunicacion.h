
#include "comunicacion.c"

//el send no manda siempre todos los bytes que le pongo por el protocolo IP
//leer la cantidad de bytes enviados que es lo que devuelve
int enviarMensaje(int serverSocket,char *message);

//int enviarHeader(int serverSocket,struct headerProtocolo header);

//struct headerProtocolo recibirHeader(int socketCliente);

/*
 * recibe por socket un mensaje
 * parámetros: socket del cliente y el largo del string (string_length)
 * devuelve un mensaje como char*
 */
void recibirMensaje(char *mensaje,int socketCliente,int packageSize);
