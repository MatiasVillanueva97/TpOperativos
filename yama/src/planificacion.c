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

typedef struct {
	int carga;
	int numero;
	int disponibilidad;
} cargaNodo;


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
int nodoConDisponibilidad(cargaNodo nodo) {
	if (nodo.disponibilidad > 0)
		return 1;
	return 0;
}

/*
 * calcula la disponibilidad de un nodo en base al algoritmo utilizado
 * recibe el nodo
 * devuelve el valor de disponibilidad (A)
 */
int calcularDisponibilidadNodo(cargaNodo nodo) {
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
cargaNodo nodoConMayorCarga(cargaNodo *listaNodos, int cantNodos) {
	int maxCarga = -1, i;
	cargaNodo nodoMayorCarga;
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


