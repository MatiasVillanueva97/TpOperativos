
#include "comunicacion.c"

struct headerProtocolo armarHeader(int idMensaje, int tamMensaje);

char* serializarHeader(struct headerProtocolo);


//el send no manda siempre todos los bytes que le pongo por el protocolo IP
//leer la cantidad de bytes enviados que es lo que devuelve
int enviarMensaje(int serverSocket,char *message);

char* recibirHeader(int socketCliente,char *message);

char* recibirMensaje(int socketCliente,char *message,int packageSize);
