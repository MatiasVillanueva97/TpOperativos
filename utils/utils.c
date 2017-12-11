/*
 * Funciones varias
 */

#include <fcntl.h>

/*
 * convierte un entero en char* completando con ceros a la izquierda hasta alcanzar el largo deseado
 */
char* intToArrayZerosLeft(int valor, int largoStringDesado) {
	char *a = string_itoa(valor);
	char *sz = string_repeat('0', largoStringDesado);
	strcpy(sz + string_length(sz) - string_length(a), a);
	return sz;
}

/*
 * recibe el nombre de la carpeta a crear y el logger para poder loguear la creacion
 */
void crear_carpeta(char* carpeta, t_log* logger) {
	struct stat st = {0};
	if (stat(carpeta, &st) == -1) {
		log_info(logger, "Carpeta no existe, creando: %s", carpeta);
		mkdir(carpeta, 0775);
		}
	else {
		log_info(logger, "Carpeta ya existe: %s", carpeta);
	}
}

/*
 * libera cada uno de los elementos de un array de char* y luego libera el array
 */
void liberar_array(char** estructura, int cantidad_elementos) {
	int i;
	for (i = 0; i < cantidad_elementos; i++) {
		free(estructura[i]);
	}
	free(estructura);
}
