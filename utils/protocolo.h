/*
 * protocolo.h
 *
 *  Created on: 25/9/2017
 *      Author: utnso
 */

#include "protocolo.c"
/*
#ifndef UTILS_PROTOCOLO_H_
#define UTILS_PROTOCOLO_H_
/*
//struct headerProtocolo armarHeader(int idMensaje, int tamMensaje);

char* serializarMensaje(uint32_t idMensaje,char **arrayMensajes);

//struct headerProtocolo deserializarHeader(char * headerSerializado);
int deserializarHeader(int socketCliente);

int deserializarTamMensaje(int socketCliente);

void deserializarMensaje(char *mensajeRecibido,int socketCliente,int tamMensaje);


#endif /* UTILS_PROTOCOLO_H_ */
