/*
 * planificacion.h
 *
 *  Created on: 24/10/2017
 *      Author: utnso
 */

#include "planificacion.c"

#ifndef YAMA_SRC_PLANIFICACION_H_
#define YAMA_SRC_PLANIFICACION_H_

//void guardarNodosPorBloque(int bloque, int nodo1, int nodo2);

int existeBloqueEnNodo(int bloque, int nodo, nodosPorBloque*);

int nodoConDisponibilidad(cargaNodo nodo);

int calcularDisponibilidadNodo(cargaNodo nodo);

cargaNodo nodoConMayorCarga(cargaNodo *listaNodos,int cantNodos);

//void ordenarMayorDisponibilidad(int *listaNodos, int cantNodos);

#endif /* YAMA_SRC_PLANIFICACION_H_ */
