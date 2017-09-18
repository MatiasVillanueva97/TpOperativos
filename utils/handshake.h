/*
 * handshake.h
 *
 *  Created on: 18/9/2017
 *      Author: utnso
 */

#include "handshake.c"

#ifndef UTILS_HANDSHAKE_H_
#define UTILS_HANDSHAKE_H_

int handshake_cliente(int socket, char id_cliente, char id_servidor);

int handshake_servidor(int socket, char id_servidor, const char *clientes_aceptados);

#endif /* UTILS_HANDSHAKE_H_ */
