#ifndef UTILS_H_
#define UTILS_H_

int leerArchivoConfig(char *pathArchivoConfig, char **keysConfig, char **datosConfig);

int conectarA(char *IP, char * puerto);

int inicializarServer(char *IP, char * puerto);

void cerrarClient(int socket);

void cerrarServer(int socket);

#endif /* UTILS_H_ */
