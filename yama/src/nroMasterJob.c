/*
 * datosMasterJob.c
 *
 *  Created on: 24/11/2017
 *      Author: utnso
 */

int maxNroMaster = 0, maxNroJob = 0;

struct nodosUsadosPlanificacion {
	int numero;
	int cantidadVecesUsados;
};

typedef struct datosMasterJob {
	int fileDescriptor;
	int nroMaster;
	int nroJob;
	int nodoReduccGlobal;
	int cantBloquesArchivo;
	int cantNodosUsados;
	struct nodosUsadosPlanificacion *nodosUsados;
	struct datosMasterJob *siguiente;
} datosMasterJob;	//datos JOB

datosMasterJob *primeroDatosMasterJob, *ultimoDatosMasterJob;

//vector con los datos de un job y master conectados
//está indexada por FileDescriptor
//se actualiza cuando el select recibe una conexión nueva y crea el FD
//o cuando se desconecta el master y el select da de baja el FD
//datosMasterJob listaNrosMasterJob[CANT_MAX_FD];

/*
 * agrega un elemento en la lista
 */
int agregarElemDatosMasterJob(datosMasterJob fila) {
	datosMasterJob *nuevo;
	/* reservamos memoria para el nuevo elemento */
	nuevo = (datosMasterJob *) malloc(sizeof(fila));
	if (nuevo == NULL) {
		perror("No hay memoria disponible!\n");
		return 0;
	}
	nuevo->fileDescriptor = fila.fileDescriptor;
	nuevo->nroMaster = fila.nroMaster;
	nuevo->nroJob = fila.nroJob;
	nuevo->nodoReduccGlobal = fila.nodoReduccGlobal;
	nuevo->cantBloquesArchivo = fila.cantBloquesArchivo;
	nuevo->cantNodosUsados = fila.cantNodosUsados;
	nuevo->nodosUsados = fila.nodosUsados;
	nuevo->siguiente = NULL;
	/* ahora metemos el nuevo elemento en la lista. lo situamos al final de la lista */
	/* comprobamos si la lista está vacía. si primero==NULL es que no
	 * hay ningún elemento en la lista. también vale ultimo==NULL */
	if (primeroDatosMasterJob == NULL) {	//primer elemento
		primeroDatosMasterJob = nuevo;
		ultimoDatosMasterJob = nuevo;
	} else {	//ya existen elementos
		/* el que hasta ahora era el último tiene que apuntar al nuevo */
		ultimoDatosMasterJob->siguiente = nuevo;
		/* hacemos que el nuevo sea ahora el último */
		ultimoDatosMasterJob = nuevo;
	}
	return 1;
}

datosMasterJob* getDatosMasterJobByFD(int fileDescriptorBuscado) {
	datosMasterJob *auxiliar;
	auxiliar = primeroDatosMasterJob;
	while (auxiliar != NULL) {
		if (auxiliar->fileDescriptor == fileDescriptorBuscado) {
			return auxiliar;
		}
		auxiliar = auxiliar->siguiente;
	}
	return NULL;
//	return &listaNrosMasterJob[fileDescriptor];
}

eliminarElemDatosMasterJobByFD(int fileDescriptorBuscado) {
	datosMasterJob *auxiliar, *anterior;
	auxiliar = primeroDatosMasterJob;
	anterior = primeroDatosMasterJob;
	while (auxiliar != NULL) {
		if (auxiliar->fileDescriptor == fileDescriptorBuscado) {
			if (auxiliar != primeroDatosMasterJob) {
				anterior->siguiente = auxiliar->siguiente;
			} else {//si son iguales el elemento buscado es el primero de la lista
				primeroDatosMasterJob = auxiliar->siguiente;
			}
			free(auxiliar);
			break;
		}
		anterior = auxiliar;
		auxiliar = auxiliar->siguiente;
	}
}

//crea un nuevo elemento en la lista con el número de job, master y FD recibido
void asignarDatosMasterJob(int nroMaster, int nroJob, int fileDescriptor) {
	printf("nroMaster %d - nroJob %d\n", nroMaster, nroJob);
	datosMasterJob fila;
	fila.nroMaster = nroMaster;
	fila.nroJob = nroJob;
	fila.fileDescriptor = fileDescriptor;
	fila.nodoReduccGlobal = 0;
	fila.cantBloquesArchivo = 0;
	fila.cantNodosUsados = 0;
	fila.nodosUsados = NULL;
	agregarElemDatosMasterJob(fila);

//	listaNrosMasterJob[fileDescriptor].nroJob = nroJob;
//	listaNrosMasterJob[fileDescriptor].nroMaster = nroMaster;
}



void asignarNodoReduccGlobal(int nodoReduccGlobal, int fileDescriptorBuscado) {
	datosMasterJob *masterJobBuscado;
	masterJobBuscado = getDatosMasterJobByFD(fileDescriptorBuscado);
	masterJobBuscado->nodoReduccGlobal = nodoReduccGlobal;
//	listaNrosMasterJob[fileDescriptor].nodoReduccGlobal = nodoReduccGlobal;

	//aumenta la carga del nodo encargado de la reducción global
	int cargaNodoReduccGlobal = masterJobBuscado->cantBloquesArchivo;
	if (cargaNodoReduccGlobal % 2 != 0)
		cargaNodoReduccGlobal++;
	int cantidadSumar = cargaNodoReduccGlobal / 2;
	aumentarCargaGlobalNodo(nodoReduccGlobal, cantidadSumar);
}

int getNuevoNroMaster() {
	return (++maxNroMaster);
}

int getNuevoNroJob() {
	return (++maxNroJob);
}
