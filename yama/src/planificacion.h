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

int existeParteArchivoEnNodo(int bloque, int nodo, bloqueArchivo*);

int nodoConDisponibilidad(datosPropiosNodo nodo);

int calcularDisponibilidadNodo(datosPropiosNodo nodosParaPlanificar);

int nodoConMayorCarga(int cantNodos, datosPropiosNodo *nodosParaPlanificar);

//void ordenarMayorDisponibilidad(int *listaNodos, int cantNodos);

#endif /* YAMA_SRC_PLANIFICACION_H_ */
