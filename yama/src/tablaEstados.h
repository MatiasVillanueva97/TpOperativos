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

int buscarMuchosElemTablaEstados(struct filaTablaEstados *arrayFilasEncontradas, struct filaTablaEstados busqueda);

void mostrarTablaEstados();

struct filaTablaEstados* buscarElemTablaEstadosPorJMNBEE(struct filaTablaEstados busqueda);

int modificarElemTablaEstados(struct filaTablaEstados fila, struct filaTablaEstados datosNuevos);

int modificarEstadoFilasTablaEstados(int nroJob, int nroMaster, int nroNodo, int nroBloque, int etapa, int estadoActual, int estadoNuevo);

int getCantFilasByJMNE(int nroJob, int nroMaster, int nroNodo, int etapa);

int getCantFilasByJMEtEs(int nroJob, int nroMaster, int etapa, int estado);

void getAllTemporalesByJMNEtEs(char **temporales, int nroJob, int nroMaster, int nroNodo, int etapa, int estado);

int getNodoReduccGlobal(int nroJob, int nroMaster, int etapa, int estado);

#endif /* YAMA_SRC_TABLAESTADOS_H_ */
