/*
 * planificacion.c
 *
 *  Created on: 21/10/2017
 *      Author: utnso
 */

int cantNodosPorArchivo = 3, cantBloquesArchivo = 6;

int nodosPorBloque[cantBloquesArchivo][2];

int cargaBase = 1, cargaMaxima = 0;
int cantNodos = 3;
int cargaNodos[cantNodos];

char algoritmoPlanificacion = 'C';	//C para clock; W para w-clock
/*
 typedef struct {
 int disponibilidad;
 int carga;
 int cargaHistorica;
 } estadoNodo;
 */

/*
 * guarda los nodos en los que está un bloque del archivo
 */
void guardarNodosPorBloque(int bloque, int nodo1, int nodo2) {
	nodosPorBloque[bloque][0] = nodo1;
	nodosPorBloque[bloque][1] = nodo2;
}

/*
 * se fija si existe un bloque determinado en un nodo determinado
 */
int existeBloqueEnNodo(int bloque, int nodo) { //nodos arrancan de 0
	int i;
	for (i = 0; i < 2; i++) {
		if (nodosPorBloque[bloque][i] == nodo) //el nodo existe en la matriz en la fila de ese bloque
			return 1;
	}
	return 0;
}

/*
 *
 */
int nodoConDisponibilidad(int nodo) {

	return 0;
}

/*
 * calcula la disponibilidad de un nodo en base al algoritmo utilizado
 * recibe el nodo
 * devuelve el valor de disponibilidad (A)
 */
int calcularDisponibilidadNodo(int nodo, char algoritmo) {
	if (algoritmoPlanificacion == 'C') {
		return cargaBase;
	} else {
		return (cargaBase + (cargaMaxima - cargaNodos[nodo]));
	}
}

