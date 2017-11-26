/*
 * planificacion.c
 *
 *  Created on: 21/10/2017
 *      Author: utnso
 */

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

//sale del archivo config?????????????
char algoritmoPlanificacion = 'W';	//C para clock; W para w-clock

int cargaMaxima, disponibBase;

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
	if (algoritmoPlanificacion == 'C') {
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

void planificar(bloqueArchivo *nodosPorPedazoArchivo, nodoParaAsignar *asignacionesNodos, int cantPartesArchivo, int cantNodosArchivo, datosPropiosNodo *nodosParaPlanificar) {
//	printf("cantPartesArchivo recibido: %d\n", cantPartesArchivo);
//	printf("cantNodosArchivo recibido: %d\n", cantNodosArchivo);

	int i, j;

//	puts("nodos por bloque reecibidos");
//	for (i = 0; i < cantPartesArchivo; i++) {
//		printf("parte archivo %d: Nodo copia1 %d - Bloque copia1 %d\n", i, nodosPorPedazoArchivo[i].nodoCopia1, nodosPorPedazoArchivo[i].bloqueCopia1);
//		printf("parte archivo %d: Nodo copia2 %d - Bloque copia2 %d\n", i, nodosPorPedazoArchivo[i].nodoCopia2, nodosPorPedazoArchivo[i].bloqueCopia2);
//	}
//	printf("\n");

	disponibBase = 1;	//sale del archivo config?????????????

	//pongo la carga inicial de cada nodo
	nodosParaPlanificar[0].carga = 0;
	nodosParaPlanificar[1].carga = 1;
	nodosParaPlanificar[2].carga = 1;
	int indexNodoMaxCarga = nodoConMayorCarga(cantNodosArchivo, nodosParaPlanificar);
	cargaMaxima = nodosParaPlanificar[indexNodoMaxCarga].carga;
	for (i = 0; i < cantNodosArchivo; i++) {
		nodosParaPlanificar[i].disponibilidad = calcularDisponibilidadNodo(nodosParaPlanificar[i]);
	}

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
			//me fijo si es el nodo de la copia 1 o 2, para cargarle el bloque dentro del nodo
			if (nodoActual.numero == nodosPorPedazoArchivo[parteArchivo].nodoCopia1)
				asignacionesNodos[parteArchivo].bloque = nodosPorPedazoArchivo[parteArchivo].bloqueCopia1;
			else
				asignacionesNodos[parteArchivo].bloque = nodosPorPedazoArchivo[parteArchivo].bloqueCopia2;
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
	return;
}

