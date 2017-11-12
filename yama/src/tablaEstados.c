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
	char temporal[40];
	int estado;
	struct filaTablaEstados *siguiente;
};

struct filaTablaEstados *primeroTablaEstados, *ultimoTablaEstados;

/*
 * agrega una fila a la tabla de estados agregando un nodo al final de la lista enlazada
 * devuelve 1 en caso de éxito o 0 en caso de error
 */
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
		printf("nodo: %d - bloque: %d - etapa: %d\n", auxiliar->nodo, auxiliar->bloque, auxiliar->estado);
		auxiliar = auxiliar->siguiente;
	}
}

/*
 * busca N filas de la tabla de estados
 * recibe los datos de búsqueda
 * devuelve un array de punteros apuntando a la filas
 * EN CONSTRUCCIÓN
 */
struct filaTablaEstados * buscarMuchosElemTablaEstados(struct filaTablaEstados busqueda) {
	/*struct filaTablaEstados *auxiliar;
	 auxiliar = primeroTablaEstados;
	 while (auxiliar != NULL) {
	 if (busqueda.job && busqueda.job != auxiliar->job) {
	 auxiliar = auxiliar->siguiente;
	 } else if (busqueda.master && busqueda.master != auxiliar->master) {
	 auxiliar = auxiliar->siguiente;
	 } else if (busqueda.nodo && busqueda.nodo != auxiliar->nodo) {
	 auxiliar = auxiliar->siguiente;
	 } else if (busqueda.bloque && busqueda.bloque != auxiliar->bloque) {
	 auxiliar = auxiliar->siguiente;
	 } else if (busqueda.etapa && busqueda.etapa != auxiliar->etapa) {
	 auxiliar = auxiliar->siguiente;
	 } else if (!strcmp(busqueda.temporal, "") && !strcmp(busqueda.temporal, auxiliar->temporal)) {
	 auxiliar = auxiliar->siguiente;
	 } else {	//es el elemento buscado
	 return auxiliar;
	 }

	 }*/
	return NULL;
}

/*
 * busca una fila de la tabla de estados
 * recibe los datos de la fila de búsqueda
 * datos obligatorios: job, master, nodo, bloque, etapa
 * devuelve un puntero apuntando a la fila encontrada (nodo de la lista enlazada)
 * o NULL si no encuentra los datos
 */
struct filaTablaEstados * buscarElemTablaEstadosPorJMNBE(struct filaTablaEstados busqueda) {
	struct filaTablaEstados *auxiliar;
	auxiliar = primeroTablaEstados;
	while (auxiliar != NULL) {
		if (busqueda.job == auxiliar->job && busqueda.master == auxiliar->master && busqueda.nodo == auxiliar->nodo && busqueda.bloque == auxiliar->bloque && busqueda.etapa == auxiliar->etapa) {
			return auxiliar;
		}
		auxiliar = auxiliar->siguiente;
	}
	return NULL;
}

/*
 * modifica una fila de la lista
 * recibe la fila a modificar y los datos que quiero modificar en forma de struct con todos los datos
 * lo que no quier modificar pongo en 0 o ""
 * devuelve 1 si pudo modificar o 0 si no encontró la fila que se buscaba
 */
int modificarElemTablaEstados(struct filaTablaEstados fila, struct filaTablaEstados datosNuevos) {
	struct filaTablaEstados *filaAModificar = buscarElemTablaEstadosPorJMNBE(fila);
	if (filaAModificar == NULL) {	//no se encontró la fila a modificar
		return 0;
	}
	if (datosNuevos.bloque)
		filaAModificar->bloque = datosNuevos.bloque;
	if (datosNuevos.estado)
		filaAModificar->estado = datosNuevos.estado;
	if (datosNuevos.etapa)
		filaAModificar->etapa = datosNuevos.etapa;
	if (datosNuevos.job)
		filaAModificar->job = datosNuevos.job;
	if (datosNuevos.master)
		filaAModificar->master = datosNuevos.master;
	if (datosNuevos.nodo)
		filaAModificar->nodo = datosNuevos.nodo;
	if (strcmp(datosNuevos.temporal, ""))
		strcpy(filaAModificar->temporal, datosNuevos.temporal);
	return 1;
}

