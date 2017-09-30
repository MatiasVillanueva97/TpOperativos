/*
 * protocolo.h
 *
 *  Created on: 25/9/2017
 *      Author: utnso
 */

#include "protocolo.c"

#ifndef UTILS_PROTOCOLO_H_
#define UTILS_PROTOCOLO_H_

struct headerProtocolo armarHeader(int idMensaje, int tamMensaje);

char* serializarHeader(struct headerProtocolo);

struct headerProtocolo deserializarHeader(char * headerSerializado);

#endif /* UTILS_PROTOCOLO_H_ */