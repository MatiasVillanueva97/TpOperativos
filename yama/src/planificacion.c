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

int cargaMaxima, cargaBase;

typedef struct {
	int nodo1;
	int bloque1;
	int nodo2;
	int bloque2;
	int bytes;
} nodosPorPedazoArchivo;

/*
 * se fija si existe un bloque determinado en un nodo determinado
 */
int existeParteArchivoEnNodo(int bloque, int nodo, nodosPorPedazoArchivo *nodosPorPedazoArchivo) { //nodos arrancan de 1
	if (nodosPorPedazoArchivo[bloque].nodo1 == nodo || nodosPorPedazoArchivo[bloque].nodo2 == nodo) //el nodo existe en la matriz en la fila de ese bloque
		return 1;
	return 0;
}

/*
 *
 */
int nodoConDisponibilidad(int nroNodo) {
	if (listaGlobalNodos[nroNodo].disponibilidad > 0)
		return 1;
	return 0;
}

/*
 * calcula la disponibilidad de un nodo en base al algoritmo utilizado
 * recibe el nodo
 * devuelve el valor de disponibilidad (A)
 */
int calcularDisponibilidadNodo(int nroNodo) {
	if (algoritmoPlanificacion == 'C') {
		return cargaBase;
	} else {
		return (cargaBase + (cargaMaxima - listaGlobalNodos[nroNodo].carga));
	}
}

/*
 * busca el nodo con mayor disponibilidad
 * recibe un puntero a la lista de nodos
 * devuelve dicho nodo (struct)
 */
int nodoConMayorCarga(int cantNodos) {
	int maxCarga = -1, i, nroNodoMayorCarga;

	for (i = 1; i <= cantNodos; i++) {
		if (listaGlobalNodos[i].carga > maxCarga) {
			maxCarga = listaGlobalNodos[i].carga;
			nroNodoMayorCarga = listaGlobalNodos[i].numero;
		}
	}
	return nroNodoMayorCarga;
}

void planificar(bloqueArchivo *bloques, nodoParaAsignar asignacionesNodos[cantPartesArchivo]) {

	int i, j;

	//sale del archivo config?????????????
	cargaBase = 1;
	printf("cantPartesArchivo: %d\n", cantPartesArchivo);
	printf("cantNodosArchivo: %d\n", cantNodosArchivo);
	//guarda los nodos en los que está cada pedazo de archivo
	nodosPorPedazoArchivo nodosPorPedazoArchivo[cantPartesArchivo];
	//cargo los nodos en los que está cada pedazo de archivo
	//el índice es el número del pedazo de archivo
	for (i = 0; i < cantPartesArchivo; i++) {
		printf("parte archivo %d: Nodo copia1 %d - Bloque copia1 %d\n", i, bloques[i].nodoCopia1, bloques[i].bloqueCopia1);
		printf("parte archivo %d: Nodo copia2 %d - Bloque copia2 %d\n", i, bloques[i].nodoCopia2, bloques[i].bloqueCopia2);
	}
	printf("\n");
	for (i = 0; i < cantPartesArchivo; i++) {
		nodosPorPedazoArchivo[i].nodo1 = bloques[i].nodoCopia1;
		nodosPorPedazoArchivo[i].bloque1 = bloques[i].bloqueCopia1;
		nodosPorPedazoArchivo[i].nodo2 = bloques[i].nodoCopia2;
		nodosPorPedazoArchivo[i].bloque2 = bloques[i].bloqueCopia2;
		nodosPorPedazoArchivo[i].bytes = bloques[i].bytesBloque;
	}
	/*for (i = 0; i < cantPartesArchivo; i++) {
	 printf("nodosPorPedazoArchivo %d: Nodo copia1 %d - Bloque copia1 %d\n", i, nodosPorPedazoArchivo[i].nodo1, nodosPorPedazoArchivo[i].bloque1);
	 printf("nodosPorPedazoArchivo %d: Nodo copia2 %d - Bloque copia2 %d\n", i, nodosPorPedazoArchivo[i].nodo2, nodosPorPedazoArchivo[i].bloque2);
	 }
	 printf("\n");*/
	//tiene la carga de cada nodo
	//cargaNodo listaCargaNodos[cantNodosArchivo];
	//pongo la carga inicial de cada nodo
	/*for (i = 0; i < cantNodosArchivo; i++) {
	 listaCargaNodos[i].carga = 0;//de dónde saco estos valores????????????????
	 listaCargaNodos[i].numero = 1;
	 }*/
	listaGlobalNodos[1].carga = 0;
	listaGlobalNodos[1].numero = 1;
	listaGlobalNodos[2].carga = 1;
	listaGlobalNodos[2].numero = 2;
	listaGlobalNodos[3].carga = 1;
	listaGlobalNodos[3].numero = 3;
	int nroNodoMaxCarga = nodoConMayorCarga(cantNodosArchivo);
	cargaMaxima = listaGlobalNodos[nroNodoMaxCarga].carga;
	for (i = 1; i <= cantNodosArchivo; i++) {
		listaGlobalNodos[i].disponibilidad = calcularDisponibilidadNodo(listaGlobalNodos[i].numero);
	}

	//ordeno los nodos de mayor a menor disponibilidad
	datosPropiosNodo temp;
	for (i = 1; i <= cantNodosArchivo; i++) {
		for (j = 1; j <= cantNodosArchivo - 1; j++) {
			if (listaGlobalNodos[j].disponibilidad < listaGlobalNodos[j + 1].disponibilidad) {
				temp = listaGlobalNodos[j];
				listaGlobalNodos[j] = listaGlobalNodos[j + 1];
				listaGlobalNodos[j + 1] = temp;
			}
		}
	}

	int parteArchivo = 0;
	int clockMaestro = 1, clockNoExisteParteArchivo = -1,
			clockNodoSinDisponibilidad = -1;

	while (parteArchivo < cantPartesArchivo) {
		datosPropiosNodo nodoActual = listaGlobalNodos[clockMaestro];
		//printf("\nnodo actual: %d, parte de archivo: %d\n", nodoActual.numero, parteArchivo);
		//printf("disponibilidad del nodo: %d\n", nodoConDisponibilidad(nodoActual.numero));
		//printf("existe la parte del archivo en el nodo: %d\n", existeParteArchivoEnNodo(parteArchivo, nodoActual.numero, nodosPorPedazoArchivo));
		if (nodoConDisponibilidad(nodoActual.numero) && existeParteArchivoEnNodo(parteArchivo, nodoActual.numero, nodosPorPedazoArchivo)) {
			//asigno parteArchivo al nodo y bloque
			asignacionesNodos[parteArchivo].nroNodo = nodoActual.numero;
			//me fijo si es el nodo de la copia 1 o 2, para cargarle el bloque dentro del nodo
			if (nodoActual.numero == nodosPorPedazoArchivo[parteArchivo].nodo1)
				asignacionesNodos[parteArchivo].bloque = nodosPorPedazoArchivo[parteArchivo].bloque1;
			else
				asignacionesNodos[parteArchivo].bloque = nodosPorPedazoArchivo[parteArchivo].bloque2;
			asignacionesNodos[parteArchivo].bytesOcupados = nodosPorPedazoArchivo[parteArchivo].bytes;

			nodoActual.carga++;
			nodoActual.disponibilidad--;

			if (clockNoExisteParteArchivo < 0 || clockMaestro == clockNoExisteParteArchivo) {
				clockMaestro++;
				if (clockMaestro > cantNodosArchivo)
					clockMaestro = 1;
			} else {
				clockMaestro = clockNoExisteParteArchivo;
			}
			clockNoExisteParteArchivo = -1;
			clockNodoSinDisponibilidad = -1;	//????? A1
			printf("\nasignó partearchivo %d al nodo %d - bloque %d\n", parteArchivo, asignacionesNodos[parteArchivo].nroNodo, asignacionesNodos[parteArchivo].bloque);
			parteArchivo++;

		} else if (!nodoConDisponibilidad(nodoActual.numero)) { //el nodo no tiene disponibilidad
			if (clockNodoSinDisponibilidad < 0)	//????? A1
				clockNodoSinDisponibilidad = clockMaestro;
			nodoActual.disponibilidad += cargaBase;
			clockMaestro++;
			if (clockMaestro > cantNodosArchivo)
				clockMaestro = 1;
			//	printf("\nno disponibilidad partearchivo %d en el nodo %d\n", parteArchivo, nodoActual.numero);
		} else if (!existeParteArchivoEnNodo(parteArchivo, nodoActual.numero, nodosPorPedazoArchivo)) { //no se encuentra el bloque en el nodo
			clockNoExisteParteArchivo = clockMaestro;
			clockMaestro++;
			if (clockMaestro > cantNodosArchivo)
				clockMaestro = 1;
			//	printf("\nno existe parte de archivo %d en nodo %d\n", parteArchivo, nodoActual.numero);
			//	printf("clock maestro: %d\n", clockMaestro);
		}
		if (clockNoExisteParteArchivo == clockMaestro)
			listaGlobalNodos[clockNoExisteParteArchivo].disponibilidad += cargaBase;
	}
	puts("pasóoo");
	return;
}

/*
 * ordena la lista de nodos de mayor a menor disponibilidad
 */
/*void ordenarMayorDisponibilidad(int *listaNodos, int cantNodos) {
 int i, j, temp;
 for (i = 0; i < cantNodos; i++) {
 for (j = 0; j < cantNodos - 1; j++) {
 if (calcularDisponibilidadNodo(listaNodos[j]) < calcularDisponibilidadNodo(listaNodos[j + 1])) {
 temp = listaNodos[j];
 listaNodos[j] = listaNodos[j + 1];
 listaNodos[j + 1] = temp;
 }
 }
 }
 }*/

