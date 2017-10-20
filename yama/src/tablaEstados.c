/*
 * tablaEstados.c
 *
 *  Created on: 20/10/2017
 *      Author: utnso
 */

struct filaTablaEstados {
	int job;
	int master;
	int nodo;
	int bloque;
	int etapa;
	char temporal[20];
	int estado;
	struct filaTablaEstados *siguiente;
};

struct filaTablaEstados *primeroTablaEstados, *ultimoTablaEstados;

int agregarElemTablaEstados(struct filaTablaEstados fila) {
	struct filaTablaEstados *nuevo;
	/* reservamos memoria para el nuevo elemento */
	nuevo = (struct filaTablaEstados *) malloc(sizeof(fila));
	if (nuevo == NULL) {
		perror("No hay memoria disponible!\n");
		return 0;
	}
	nuevo->bloque = fila.bloque;
	nuevo->estado = fila.estado;
	nuevo->etapa = fila.etapa;
	nuevo->job = fila.job;
	nuevo->master = fila.master;
	nuevo->nodo = fila.nodo;
	strcpy(nuevo->temporal, fila.temporal);
	nuevo->siguiente = NULL;
	/* ahora metemos el nuevo elemento en la lista. lo situamos al final de la lista */
	/* comprobamos si la lista está vacía. si primero==NULL es que no
	 * hay ningún elemento en la lista. también vale ultimo==NULL */
	if (primeroTablaEstados == NULL) {	//primer elemento
		primeroTablaEstados = nuevo;
		ultimoTablaEstados = nuevo;
	} else {	//ya existen elementos
		/* el que hasta ahora era el último tiene que apuntar al nuevo */
		ultimoTablaEstados->siguiente = nuevo;
		/* hacemos que el nuevo sea ahora el último */
		ultimoTablaEstados = nuevo;
	}
	return 1;
	//if()
}

void mostrarListaElementos() {
	struct filaTablaEstados *auxiliar;
	auxiliar = primeroTablaEstados;
	while (auxiliar != NULL) {
		printf("nodo: %d - temporal: %s - etapa: %d\n", auxiliar->nodo, auxiliar->temporal, auxiliar->etapa);
		auxiliar = auxiliar->siguiente;
	}
}

struct filaTablaEstados * buscarElemTablaEstados(struct filaTablaEstados busqueda) {
	struct filaTablaEstados *auxiliar;
	auxiliar = primeroTablaEstados;
	while (auxiliar != NULL) {
		if (busqueda.job && busqueda.job != auxiliar->job) {
			auxiliar = auxiliar->siguiente;
		}else if (busqueda.master && busqueda.master != auxiliar->master) {
			auxiliar = auxiliar->siguiente;
		}else if (busqueda.nodo && busqueda.nodo != auxiliar->nodo) {
			auxiliar = auxiliar->siguiente;
		}else if (busqueda.bloque && busqueda.bloque != auxiliar->bloque) {
			auxiliar = auxiliar->siguiente;
		}else if (busqueda.etapa && busqueda.etapa != auxiliar->etapa) {
			auxiliar = auxiliar->siguiente;
		}else if (!strcmp(busqueda.temporal, "") && !strcmp(busqueda.temporal, auxiliar->temporal)) {
			auxiliar = auxiliar->siguiente;
		}else{	//es el elemento buscado
			return auxiliar;
		}

	}
	return NULL;
}

