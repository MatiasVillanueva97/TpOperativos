/*
 * tablaEstados.h
 *
 *  Created on: 20/10/2017
 *      Author: utnso
 */

#include "tablaEstados.c"

#ifndef YAMA_SRC_TABLAESTADOS_H_
#define YAMA_SRC_TABLAESTADOS_H_

int agregarElemTablaEstados(struct filaTablaEstados fila);

void mostrarListaElementos();

struct filaTablaEstados* buscarElemTablaEstados(struct filaTablaEstados busqueda);

int modificarElemTablaEstados(struct filaTablaEstados fila, struct filaTablaEstados datosNuevos);

#endif /* YAMA_SRC_TABLAESTADOS_H_ */
