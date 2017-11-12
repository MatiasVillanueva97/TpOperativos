/*
 * Funciones varias
 */

/*
 * convierte un entero en char* completando con ceros a la izquierda hasta alcanzar el largo deseado
 */
char* intToArrayZerosLeft(int valor, int largoStringDesado) {
	char *a = string_itoa(valor);
	char *sz = string_repeat('0', largoStringDesado);
	strcpy(sz + string_length(sz) - string_length(a), a);
	return sz;
}

char* copiarString(char *origen, int sizeDestino) {
	char *destino = malloc(sizeDestino);
	memset(destino, '\0', sizeof(sizeDestino));
	strcpy(destino, origen);
	return destino;
}
