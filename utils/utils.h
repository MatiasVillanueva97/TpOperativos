#include "utils.c"

#ifndef UTILS_H_
#define UTILS_H_

/*
 * convierte un entero en char* completando con ceros a la izquierda hasta alcanzar el largo deseado
 */
char* intToArrayZerosLeft(int valor, int largoStringDesado);

/*
 * para crear carpeta de log solamente
 */
void crearCarpetaDeLog(char* carpeta);

/*
 * recibe el nombre de la carpeta a crear y el logger para poder loguear la creacion
 */
void crear_carpeta(char* carpeta, t_log* logger);

/*
 * libera cada uno de los elementos de un array de char* y luego libera el array
 */
void liberar_array(char** estructura, int cantidad_elementos);
#endif /* UTILS_H_ */
