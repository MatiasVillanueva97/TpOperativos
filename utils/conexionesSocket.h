/*
 * conexionesSocket.h
 *
 *  Created on: 18/9/2017
 *      Author: utnso
 */

#include "conexionesSocket.c"

#ifndef UTILS_CONEXIONESSOCKET_H_
#define UTILS_CONEXIONESSOCKET_H_

/* ******************** funciones para clientes de conexiones ************************ */

/*
 * conecta un cliente a un server
 * recibe la ip y el puerto del server
 * devuelve el número de socket si OK o -1 si ERROR
 */
int conectarA(char *ipServer, char * puertoServer);

/*
 * Cierra la conexión y libera el socket
 * recibe el socket
 */
void cerrarCliente(int socketServer);

/* ******************** funciones para servers de conexiones ************************ */

/*
 * inicializa el server y lo pone a escuchar (función bloqueante por la syscall bloqueante "listen")
 * recibe la ip y el puerto por el que va a quedar escuchando el server
 * devuelve el socket a través del cual escuchará nuevas conexiones si OK o -1 si ERROR
 */
int inicializarServer(char *IP, char *puerto);

/*
 * Acepta la conexión del cliente y abre el *nuevo* socket
 * recibe el socket a través del cual el server está escuchando nuevas conexiones
 * devuelve el nuevo socket a través del cual va a continuar la comunicación con el cliente
 */
int aceptarConexion(int listenningSocket);

/*
 * Cierra la conexión y libera el socket
 * recibe el socket de escucha
 */
void cerrarServer(int listenningSocket);

#endif /* UTILS_CONEXIONESSOCKET_H_ */
