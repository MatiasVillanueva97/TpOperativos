#include "comunicacion.c"

int enviarMensaje(int serverSocket, char *message);

int recibirMensaje(char *mensaje, int socketCliente, int packageSize);

char* serializarMensaje(int32_t idMensaje, char **arrayMensajes, int cantStrings);

int32_t deserializarHeader(int socketCliente);

char** deserializarMensaje(int socketCliente, int cantMensajes);

int enviarHeaderSolo(int serverSocket, int32_t headerId);
