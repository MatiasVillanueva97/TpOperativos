/*
 * tablaEstados.c
 *
 *  Created on: 20/10/2017
 *      Author: utnso
 */

// ================================================================ //
// tabla de estados de YAMA
// ================================================================ //
enum etapasTablaEstados {
	TRANSFORMACION, REDUCC_LOCAL, REDUCC_GLOBAL, ALMAC_FINAL
};
enum estadoTablaEstados {
	EN_PROCESO, ERROR, FIN_OK
};

struct filaTablaEstados {
	int job;
	int master;
	int nodo;
	int bloque;
	int etapa;
	char temporal[LARGO_TEMPORAL];
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

void mostrarTablaEstados() {
	struct filaTablaEstados *auxiliar;
	auxiliar = primeroTablaEstados;
	while (auxiliar != NULL) {
		printf("job %d - master %d - nodo %d - bloque: %d - etapa: %d - temporal %s - estado %d\n", auxiliar->job, auxiliar->master, auxiliar->nodo, auxiliar->bloque, auxiliar->etapa, auxiliar->temporal, auxiliar->estado);
		auxiliar = auxiliar->siguiente;
	}
}

/*
 * busca N filas de la tabla de estados
 * recibe los datos de búsqueda
 los que no sean parámetro de búsqueda se los pone en 0 o ""
 * recibe el array de structs que va completando con los datos encontrados,
 ese array debe tener un tamaño determinado antes de llamar a esta función
 */
int buscarMuchosElemTablaEstados(struct filaTablaEstados *arrayFilasEncontradas, struct filaTablaEstados busqueda) {
	struct filaTablaEstados *auxiliar;
	auxiliar = primeroTablaEstados;
	int i = 0;
	while (auxiliar != NULL) {
//		printf("job %d - %d\n", busqueda.job, auxiliar->job);
//		printf("master %d - %d\n", busqueda.master, auxiliar->master);
//		printf("nodo %d - %d\n", busqueda.nodo, auxiliar->nodo);
//		printf("bloque %d - %d\n", busqueda.bloque, auxiliar->bloque);
//		printf("etapa %d - %d\n", busqueda.etapa, auxiliar->etapa);
//		printf("temporal %s - %s\n", busqueda.temporal, auxiliar->temporal);
//		printf("estado %d - %d\n", busqueda.estado, auxiliar->estado);
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
		} else if (strcmp(busqueda.temporal, "") != 0 && !strcmp(busqueda.temporal, auxiliar->temporal)) {
			auxiliar = auxiliar->siguiente;
		} else if (busqueda.estado && busqueda.estado != auxiliar->estado) {
			auxiliar = auxiliar->siguiente;
		} else {
			arrayFilasEncontradas[i].job = busqueda.job;
			arrayFilasEncontradas[i].master = busqueda.master;
			arrayFilasEncontradas[i].nodo = busqueda.nodo;
			arrayFilasEncontradas[i].bloque = busqueda.bloque;
			arrayFilasEncontradas[i].etapa = busqueda.etapa;
			strcpy(arrayFilasEncontradas[i].temporal, busqueda.temporal);
			arrayFilasEncontradas[i].estado = busqueda.estado;
			arrayFilasEncontradas[i].siguiente = NULL;
			i++;
			auxiliar = auxiliar->siguiente;
		}
//		printf("i: %d\n", i);
//		printf("auxiliar: %d\n", auxiliar);
	}
	return i;
}

/*
 * busca una fila de la tabla de estados
 * recibe los datos de la fila de búsqueda
 * datos obligatorios: job, master, nodo, bloque, etapa, estado
 * devuelve un puntero apuntando a la fila encontrada (nodo de la lista enlazada)
 * o NULL si no encuentra los datos
 */
struct filaTablaEstados * buscarElemTablaEstadosPorJMNBEE(struct filaTablaEstados busqueda) {
	struct filaTablaEstados *auxiliar;
	auxiliar = primeroTablaEstados;
	while (auxiliar != NULL) {
		if (busqueda.job == auxiliar->job && busqueda.master == auxiliar->master && busqueda.nodo == auxiliar->nodo && busqueda.bloque == auxiliar->bloque && busqueda.etapa == auxiliar->etapa && busqueda.estado == auxiliar->estado) {
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
	struct filaTablaEstados *filaAModificar = buscarElemTablaEstadosPorJMNBEE(fila);
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

/*
 * modifica el estado de N filas de la tabla de estados
 * recibe el nro de Job, el nro de Master, el nro de Nodo y la etapa que se usan para buscar las filas
 * recibe el estado nuevo al que hay que actualizar las filas correspondientes
 * devuelve la cantidad de filas que pudo modificar
 */
int modificarEstadoFilasTablaEstados(int nroJob, int nroMaster, int nroNodo, int nroBloque, int etapa, int estadoActual, int estadoNuevo) {
	int cantFilasModificadas = 0;
	struct filaTablaEstados *auxiliar;
	auxiliar = primeroTablaEstados;
	while (auxiliar != NULL) {
		if (auxiliar->job == nroJob && auxiliar->master == nroMaster && auxiliar->nodo == nroNodo && auxiliar->bloque == nroBloque && auxiliar->etapa == etapa && auxiliar->estado == estadoActual) {
			auxiliar->estado = estadoNuevo;
			cantFilasModificadas++;
		}
		auxiliar = auxiliar->siguiente;
	}
	return cantFilasModificadas;
}

/*
 * devuelve la cantidad de filas que respondan a los parámetros de búsqueda
 */
int getCantFilasByJMNEtEs(int nroJob, int nroMaster, int nroNodo, int etapa, int estado) {
	int cantidad = 0;
	struct filaTablaEstados *auxiliar;
	auxiliar = primeroTablaEstados;
	while (auxiliar != NULL) {
		if (auxiliar->job == nroJob && auxiliar->master == nroMaster && auxiliar->nodo == nroNodo && auxiliar->etapa == etapa && auxiliar->estado == estado) {
			cantidad++;
		}
		auxiliar = auxiliar->siguiente;
	}
	return cantidad;
}

/*
 * devuelve la cantidad de filas que respondan a los parámetros de búsqueda
 * idem anterior sin el número de nodo
 */
int getCantFilasByJMEtEs(int nroJob, int nroMaster, int etapa, int estado) {
	int cantidad = 0;
	struct filaTablaEstados *auxiliar;
	auxiliar = primeroTablaEstados;
	while (auxiliar != NULL) {
		if (auxiliar->job == nroJob && auxiliar->master == nroMaster && auxiliar->etapa == etapa && auxiliar->estado == estado) {
			cantidad++;
		}
		auxiliar = auxiliar->siguiente;
	}
	return cantidad;
}

void getAllTemporalesByJMNEtEs(char **temporales, int nroJob, int nroMaster, int nroNodo, int etapa, int estado) {
	struct filaTablaEstados *auxiliar;
	auxiliar = primeroTablaEstados;
	int j, i = 0;
	while (auxiliar != NULL) {
		if (auxiliar->job == nroJob && auxiliar->master == nroMaster && auxiliar->nodo == nroNodo && auxiliar->etapa == etapa && auxiliar->estado == estado) {
			temporales[i] = malloc(string_length(auxiliar->temporal) + 1);
			if (!temporales[i])
				perror("error de malloc");
			strcpy(temporales[i], auxiliar->temporal);
			i++;
		}
		auxiliar = auxiliar->siguiente;
	}
}

int getNodoReduccGlobal(int nroJob, int nroMaster, int etapa, int estado) {
	struct filaTablaEstados *auxiliar;
	auxiliar = primeroTablaEstados;
	while (auxiliar != NULL) {
		if (auxiliar->job == nroJob && auxiliar->master == nroMaster && auxiliar->etapa == etapa && auxiliar->estado == estado) {
			return auxiliar->nodo;
		}
		auxiliar = auxiliar->siguiente;
	}
	return 0;
}
