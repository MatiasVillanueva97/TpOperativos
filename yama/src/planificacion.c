/*
 * planificacion.c
 *
 *  Created on: 21/10/2017
 *      Author: utnso
 */

typedef struct {
	//int fd;
	int parteArchivo;
	int nodoUsado;
	int bloqueUsado;
	int nodoSuplente;
	int bloqueSuplente;
	int bytes;
} nodosUsadobloqueArchivo;
nodosUsadobloqueArchivo *bloquesArchivoXFD[CANT_MAX_FD];

char algoritmoPlanificacion[10];

int cargaMaxima = 0, disponibBase, nodoReduccGlobal, retardoPlanificacion;

/*
 * se fija si existe un bloque determinado en un nodo determinado
 */
int existeParteArchivoEnNodo(int bloque, int nodo, bloqueArchivo *nodosPorPedazoArchivo) { //nodos arrancan de 1
	if (nodosPorPedazoArchivo[bloque].nodoCopia1 == nodo || nodosPorPedazoArchivo[bloque].nodoCopia2 == nodo) //el nodo existe en la matriz en la fila de ese bloque
		return 1;
	return 0;
}

/*
 *
 */
int nodoConDisponibilidad(datosPropiosNodo nodo) {
	if (nodo.disponibilidad > 0)
		return 1;
	return 0;
}

/*
 * calcula la disponibilidad de un nodo en base al algoritmo utilizado
 * recibe el nodo
 * devuelve el valor de disponibilidad (A)
 */
int calcularDisponibilidadNodo(datosPropiosNodo nodosParaPlanificar) {
	string_to_upper(algoritmoPlanificacion);
	if (strcmp(algoritmoPlanificacion, "CLOCK") == 0) {
		return disponibBase;
	} else {
		return (disponibBase + (cargaMaxima - nodosParaPlanificar.carga));
	}
}

/*
 * busca el nodo con mayor disponibilidad
 * recibe un puntero a la lista de nodos
 * devuelve dicho nodo (struct)
 */
int nodoConMayorCarga(int cantNodos, datosPropiosNodo *nodosParaPlanificar) {
	int maxCarga = -1, i, indexNodoMayorCarga;
	for (i = 0; i < cantNodos; i++) {
		if (nodosParaPlanificar[i].carga > maxCarga) {
			maxCarga = nodosParaPlanificar[i].carga;
			indexNodoMayorCarga = i;
		}
	}
	return indexNodoMayorCarga;
}

void planificar(int fileDescriptor, bloqueArchivo *nodosPorPedazoArchivo, nodoParaAsignar *asignacionesNodos, int cantPartesArchivo, int cantNodosArchivo, datosPropiosNodo *nodosParaPlanificar) {
	puts("Entro a planificación");
	printf("retardoPlanificacion: %d\n", retardoPlanificacion);
	printf("algoritmoPlanificacion: %s\n", algoritmoPlanificacion);
	sleep(5);
	usleep(retardoPlanificacion * 100);
//	printf("cantPartesArchivo recibido: %d\n", cantPartesArchivo);
//	printf("cantNodosArchivo recibido: %d\n", cantNodosArchivo);

	int i, j;

//	puts("nodos por bloque reecibidos");
//	for (i = 0; i < cantPartesArchivo; i++) {
//		printf("parte archivo %d: Nodo copia1 %d - Bloque copia1 %d\n", i, nodosPorPedazoArchivo[i].nodoCopia1, nodosPorPedazoArchivo[i].bloqueCopia1);
//		printf("parte archivo %d: Nodo copia2 %d - Bloque copia2 %d\n", i, nodosPorPedazoArchivo[i].nodoCopia2, nodosPorPedazoArchivo[i].bloqueCopia2);
//	}
//	printf("\n");

	//actualiza los nodos que se usan en la planificación con la carga y disponibilidad histórica
	for (i = 0; i < cantNodosArchivo; i++) {
		nodosParaPlanificar[i].carga = getCargaGlobalNodo(nodosParaPlanificar[i].numero);
		nodosParaPlanificar[i].disponibilidad = calcularDisponibilidadNodo(nodosParaPlanificar[i]);
	}
	//obtiene el nro del nodo con mayor carga y actualiza la variable global cargaMaxima
	int indexNodoMaxCarga = nodoConMayorCarga(cantNodosArchivo, nodosParaPlanificar);
	cargaMaxima = nodosParaPlanificar[indexNodoMaxCarga].carga;

	//ordeno los nodos de mayor a menor disponibilidad
	datosPropiosNodo temp;
	for (i = 0; i < cantNodosArchivo; i++) {
		for (j = 0; j < (cantNodosArchivo - 1); j++) {
			if (nodosParaPlanificar[j].disponibilidad < nodosParaPlanificar[j + 1].disponibilidad) {
				temp = nodosParaPlanificar[j];
				nodosParaPlanificar[j] = nodosParaPlanificar[j + 1];
				nodosParaPlanificar[j + 1] = temp;
			}
		}
	}

	int parteArchivo = 0;
	int clockMaestro = 0, clockNoExisteParteArchivo = -1,
			clockNodoSinDisponibilidad = -1;

	nodosUsadobloqueArchivo nodosAsociadosFD[cantPartesArchivo];

	while (parteArchivo < cantPartesArchivo) {
		datosPropiosNodo nodoActual = nodosParaPlanificar[clockMaestro];
//		printf("clockMaestro: %d\n", clockMaestro);
//		printf("parte de archivo: %d\n", parteArchivo);
//		printf("nro nodo actual: %d - carga %d - disponibilidad %d\n", nodoActual.numero, nodoActual.carga, nodoActual.disponibilidad);
//		printf("disponibilidad del nodo: %d\n", nodoConDisponibilidad(nodoActual));
//		printf("existe la parte del archivo en el nodo: %d\n", existeParteArchivoEnNodo(parteArchivo, nodoActual.numero, nodosPorPedazoArchivo));
		if (nodoConDisponibilidad(nodoActual) && existeParteArchivoEnNodo(parteArchivo, nodoActual.numero, nodosPorPedazoArchivo)) {
			//asigno parteArchivo al nodo y bloque
			asignacionesNodos[parteArchivo].nroNodo = nodoActual.numero;

			//nodosAsociadosFD[] se va a usar para agarrar el nodo de la copia en la replanificación
			nodosAsociadosFD[parteArchivo].parteArchivo = parteArchivo;
			nodosAsociadosFD[parteArchivo].nodoUsado = nodoActual.numero;
			//me fijo si es el nodo de la copia 1 o 2, para cargarle el bloque dentro del nodo
			if (nodoActual.numero == nodosPorPedazoArchivo[parteArchivo].nodoCopia1) {
				asignacionesNodos[parteArchivo].bloque = nodosPorPedazoArchivo[parteArchivo].bloqueCopia1;
				nodosAsociadosFD[parteArchivo].nodoSuplente = nodosPorPedazoArchivo[parteArchivo].nodoCopia2;
				nodosAsociadosFD[parteArchivo].bloqueSuplente = nodosPorPedazoArchivo[parteArchivo].bloqueCopia2;
			} else {
				asignacionesNodos[parteArchivo].bloque = nodosPorPedazoArchivo[parteArchivo].bloqueCopia2;
				nodosAsociadosFD[parteArchivo].nodoSuplente = nodosPorPedazoArchivo[parteArchivo].nodoCopia1;
				nodosAsociadosFD[parteArchivo].bloqueSuplente = nodosPorPedazoArchivo[parteArchivo].bloqueCopia1;
			}
			nodosAsociadosFD[parteArchivo].bloqueUsado = asignacionesNodos[parteArchivo].bloque;
			nodosAsociadosFD[parteArchivo].bytes = nodosPorPedazoArchivo[parteArchivo].bytesBloque;
			asignacionesNodos[parteArchivo].bytesOcupados = nodosPorPedazoArchivo[parteArchivo].bytesBloque;

			nodoActual.carga++;
			nodoActual.disponibilidad--;
			nodosParaPlanificar[clockMaestro].carga = nodoActual.carga;
			nodosParaPlanificar[clockMaestro].disponibilidad = nodoActual.disponibilidad;
			if (clockNoExisteParteArchivo < 0 || clockMaestro == clockNoExisteParteArchivo) {
				clockMaestro++;
				if (clockMaestro >= cantNodosArchivo)
					clockMaestro = 0;
			} else {
				clockMaestro = clockNoExisteParteArchivo;
			}
			clockNoExisteParteArchivo = -1;
			clockNodoSinDisponibilidad = -1;	//????? A1
//			printf("asignó partearchivo %d al nodo %d - bloque %d\n", parteArchivo, asignacionesNodos[parteArchivo].nroNodo, asignacionesNodos[parteArchivo].bloque);
			parteArchivo++;

		} else if (!nodoConDisponibilidad(nodoActual)) { //el nodo no tiene disponibilidad
			if (clockNodoSinDisponibilidad < 0) //????? A1
				clockNodoSinDisponibilidad = clockMaestro;
			nodoActual.disponibilidad += disponibBase;
			nodosParaPlanificar[clockMaestro].disponibilidad = nodoActual.disponibilidad;
			clockMaestro++;
			if (clockMaestro >= cantNodosArchivo)
				clockMaestro = 0;
//			printf("\nno disponibilidad partearchivo %d en el nodo %d\n", parteArchivo, nodoActual.numero);
		} else if (!existeParteArchivoEnNodo(parteArchivo, nodoActual.numero, nodosPorPedazoArchivo)) { //no se encuentra el bloque en el nodo
			clockNoExisteParteArchivo = clockMaestro;
			clockMaestro++;
			if (clockMaestro >= cantNodosArchivo)
				clockMaestro = 0;
//			printf("\nno existe parte de archivo %d en nodo %d\n", parteArchivo, nodoActual.numero);
		}
//		puts("\nestado al final de la vuelta\n----------------");
//		printf("parteArchivo: %d\n", parteArchivo);
//		printf("clock maestro: %d\n", clockMaestro);
//		printf("clockNodoSinDisponibilidad: %d\n", clockNodoSinDisponibilidad);
//		printf("clockNoExisteParteArchivo: %d\n", clockNoExisteParteArchivo);
//		printf("nodoActual.numero: %d\n", nodoActual.numero);
//		printf("nodoActual.carga: %d\n", nodoActual.carga);
//		printf("nodoActual.disponibilidad: %d\n", nodoActual.disponibilidad);
		if (clockNoExisteParteArchivo == clockMaestro)
			nodosParaPlanificar[clockMaestro].disponibilidad += disponibBase;

//		getchar();
//		printf("\n-------- termina la vuelta -----------\n");
	}

	//guarda los datos del nodo usado y el suplente asociados a un FD para usar como índice
	//esto está pensado para la replanificación, para que use el nodo suplente
	bloquesArchivoXFD[fileDescriptor] = nodosAsociadosFD;
	for (i = 0; i < cantPartesArchivo; i++) {
		printf("bytes %d, nodoUsado  %d, bloqueUsado  %d, nodoSuplente %d, bloqueSuplente  %d\n", bloquesArchivoXFD[fileDescriptor][i].bytes, bloquesArchivoXFD[fileDescriptor][i].nodoUsado, bloquesArchivoXFD[fileDescriptor][i].bloqueUsado, bloquesArchivoXFD[fileDescriptor][i].nodoSuplente, bloquesArchivoXFD[fileDescriptor][i].bloqueSuplente);
	}

	//actualiza la carga en cada nodo usado de la listaGlobalNodos[]
	for (i = 0; i < cantNodosArchivo; i++) {
		actualizarCargaGlobalNodo(nodosParaPlanificar[i].numero, nodosParaPlanificar[i].carga);
	}

	//ordeno los nodos de menor a mayor por carga
	for (i = 0; i < cantNodosArchivo; i++) {
		for (j = 0; j < (cantNodosArchivo - 1); j++) {
			if (nodosParaPlanificar[j].carga > nodosParaPlanificar[j + 1].carga) {
				temp = nodosParaPlanificar[j];
				nodosParaPlanificar[j] = nodosParaPlanificar[j + 1];
				nodosParaPlanificar[j + 1] = temp;
			}
		}
	}
	return;
}

/*	in_array para c
 * typedef int (*cmpfunc)(void *, void *);

 int in_array(void *array[], int size, void *lookfor, cmpfunc cmp)
 {
 int i;

 for (i = 0; i < size; i++)
 if (cmp(lookfor, array[i]) == 0)
 return 1;
 return 0;
 }

 int main()
 {
 char *str[] = {"this is test", "a", "b", "c", "d"};

 if (in_array(str, 5, "c", strcmp))
 printf("yes\n");
 else
 printf("no\n");

 return 0;
 }
 */
