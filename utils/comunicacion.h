#include "comunicacion.c"

int enviarMensaje(int serverSocket, char *message);

void recibirMensaje(char *mensaje, int socketCliente, int packageSize);

char* serializarMensaje(uint32_t idMensaje, char **arrayMensajes, int cantStrings);

uint32_t deserializarHeader(int socketCliente);

char** deserializarMensaje(int socketCliente, int cantMensajes);

int enviarHeaderSolo(int serverSocket, uint32_t headerId);
