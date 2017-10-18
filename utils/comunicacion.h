
#include "comunicacion.c"

int enviarMensaje(int serverSocket,char *message);

void recibirMensaje(char *mensaje,int socketCliente,int packageSize);

char* serializarMensaje(uint32_t idMensaje,char **arrayMensajes);

uint32_t deserializarHeader(int socketCliente);

char** deserializarMensaje(int socketCliente,int cantMensajes);
