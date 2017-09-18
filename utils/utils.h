#ifndef UTILS_H_
#define UTILS_H_

/* ******************** funciones para clientes de conexiones ************************ */

/*
 * lee el archivo de configuración y guarda los datos en un array
 * recibe el nombre del archivo de configuración, el puntero al array con las keys a leer y el puntero al array donde guardar los datos
 * devuelve 0 si OK o 1 si ERROR
 */
int leerArchivoConfig(char *nameArchivoConfig, char **keysConfig, char **datosConfig);

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
void cerrarClient(int socket);

/* ******************** funciones para servers de conexiones ************************ */

/*
 * inicializa el server y lo pone a escuchar (función bloqueante por la syscall bloqueante "listen")
 * recibe la ip y el puerto por el que va a quedar escuchando el server
 * devuelve el socket a través del cual escuchará nuevas conexiones si OK o -1 si ERROR
 */
int inicializarServer(char *IP, char * puerto);

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

#endif /* UTILS_H_ */
