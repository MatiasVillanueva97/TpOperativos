/*
 * Sockets.h
 *
 *  Created on: 8/9/2017
 *      Author: utnso
 */

#ifndef SOCKETS_H_
#define SOCKETS_H_

int crear_servidor(int puerto, void* (*funcion_nuevo_cliente)(void * socket));

int conectar(char *ip, int puerto);

int handshake_cliente(int socket, char id_cliente, char id_servidor);

int handshake_servidor(int socket, char id_servidor, const char *clientes_aceptados);

#endif /* SOCKETS_H_ */
