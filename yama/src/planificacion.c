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

int cargaMaxima, cargaBase, cantBloquesArchivo, cantNodos;

typedef struct {
	int nodo1;
	int nodo2;
} nodosPorBloque;

typedef struct {
	int carga;
	int numero;
	int disponibilidad;
} nodo;


/*
 * se fija si existe un bloque determinado en un nodo determinado
 */
int existeBloqueEnNodo(int bloque, int nodo, nodosPorBloque *nodosPorBloque) { //nodos arrancan de 1
	if (nodosPorBloque[bloque].nodo1 == nodo || nodosPorBloque[bloque].nodo2 == nodo) //el nodo existe en la matriz en la fila de ese bloque
		return 1;
	return 0;
}

/*
 *
 */
int nodoConDisponibilidad(nodo nodo) {
	if (nodo.disponibilidad > 0)
		return 1;
	return 0;
}

/*
 * calcula la disponibilidad de un nodo en base al algoritmo utilizado
 * recibe el nodo
 * devuelve el valor de disponibilidad (A)
 */
int calcularDisponibilidadNodo(nodo nodo) {
	if (algoritmoPlanificacion == 'C') {
		return cargaBase;
	} else {
		return (cargaBase + (cargaMaxima - nodo.carga));
	}
}

/*
 * busca el nodo con mayor disponibilidad
 * recibe un puntero a la lista de nodos
 * devuelve dicho nodo (struct)
 */
nodo nodoConMayorCarga(nodo *listaNodos, int cantNodos) {
	int maxCarga = -1, i;
	nodo nodoMayorCarga;
	for (i = 0; i < cantNodos; i++) {
		if (listaNodos[i].carga > maxCarga) {
			maxCarga = listaNodos[i].carga;
			nodoMayorCarga.numero = listaNodos[i].numero;
			nodoMayorCarga.carga = listaNodos[i].carga;
		}
	}
	return nodoMayorCarga;
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


