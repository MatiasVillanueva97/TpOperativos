/*
 * globalNodos.c
 *
 *  Created on: 12/12/2017
 *      Author: utnso
 */

typedef struct {
	int numero;
	char ip[LARGO_IP];
	int puerto;
	int carga;
	int disponibilidad;
	char nombre[LARGO_NOMBRE_NODO];
} datosPropiosNodo;

datosPropiosNodo *listaGlobalNodos;
int largoListaGlobalNodos = 0;

setSizeListaGlobalNodos(int cantNodos) {
	if (cantNodos > getLargoListaGlobalNodos()) { //se redimensiona si se recibieron más nodos que antes
	/* Redimensionamos el puntero y lo asignamos a un puntero temporal */
		void *tmp_ptr = realloc(listaGlobalNodos, sizeof(datosPropiosNodo) * cantNodos);

		if (tmp_ptr == NULL) {
			puts("Error!!!! No se pudo reasignar memoria para la Lista Global de Nodos");
			getchar();
			/* Error: tomar medidas necesarias */
		} else {
			/* Reasignación exitosa. Asignar memoria a ptr */
			listaGlobalNodos = (datosPropiosNodo*) tmp_ptr;
			largoListaGlobalNodos = cantNodos;
		}
	}
}

datosPropiosNodo* getDatosGlobalesNodo(int nodoBuscado) {
	return &listaGlobalNodos[nodoBuscado];
}

int getLargoListaGlobalNodos() {
	//return (sizeof(listaGlobalNodos) / sizeof(*listaGlobalNodos));
	return largoListaGlobalNodos;
}

int getCargaGlobalNodo(int nroNodo) {
	return listaGlobalNodos[nroNodo].carga;
}

void actualizarCargaGlobalNodo(int nroNodo, int carga) {
	listaGlobalNodos[nroNodo].carga = carga;
}

void disminuirCargaGlobalNodo(int nroNodo, int cantidadRestar) {
	listaGlobalNodos[nroNodo].carga -= cantidadRestar;
}

void aumentarCargaGlobalNodo(int nroNodo, int cantidadSumar) {
	listaGlobalNodos[nroNodo].carga += cantidadSumar;
}
