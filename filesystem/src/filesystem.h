/*
 * filesystem.h
 *
 *  Created on: 21/9/2017
 *      Author: utnso
 */

#ifndef SRC_FILESYSTEM_H_
#define SRC_FILESYSTEM_H_

//bibliotecas
#include "../../utils/includes.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/bitarray.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>
#include <commons/collections/list.h>


//File System

//Tabla de Directorios
typedef struct{
  int index;
  char * nombre;
  int padre;
}tablaDeDirectorios;

//Tabla de Nodos
typedef struct {
	char* nodo;
	char * puerto;
	char * ip;
	uint32_t total;
	uint32_t libre;
	uint32_t socket;
} ContenidoXNodo;
typedef struct{
	uint32_t a ;
	uint32_t b ;
}bloqueVector;

typedef struct {
	uint32_t tamanio;
	uint32_t libres;
	t_list * nodo;
	t_list * contenidoXNodo;
}tablaDeNodos;

//Tabla de Archivos

typedef struct {
	char* nodo;
	uint32_t bloque;
	uint32_t bytes;
}BloqueCopia;

typedef struct{
	char * nombre;
	int directorioPadre;
	int tamanio;
	int tipo;
	int cantidadDeBLoquesaMandar;
	t_list* bloqueCopias;
}tablaArchivo;

//info para yama---------------------
typedef struct {
	char * nodo;
	int bloque;
	int bytes;
}ContenidoBloque;

typedef struct {
	t_list * bloqueNodo;
	int bytesBloque;
}Bloque;

typedef struct{
	//char * nombre;
	uint32_t tamaniobytes;
	uint32_t tipo;
	uint32_t directorioPadre;
	uint32_t estado;
	Bloque bloque;
}archivoEnFileSystem;
//----------
//Bitmap por Nodo
typedef struct  {
	char* nodo;
	t_bitarray 	* bitarray;
	uint32_t cantidadBloques;
} tablaBitmapXNodos;

//Variables

t_list * tablaArchivos;
t_list * tablaNodos;
t_list * tabaBitmapXNodo;
t_list * listaDeBitMap;
tablaDeNodos * tablaNodosGlobal;

t_log * logFs;
t_config * configFs;
t_list * listaDeNodosDeEstadoAnterior;
t_list * listaDeNodosDeFormateo;
t_list * listaDirectorios;
t_list * registroArchivos;
t_config * persistirNodos;
t_config * persistirArchivo;
t_config *directorios;
t_config * registroArchivo;
int formateado;
int estadoAnterior;
int	estadoEstable;
bool estadoEstableFuncion();
#endif /* SRC_FILESYSTEM_H_ */
