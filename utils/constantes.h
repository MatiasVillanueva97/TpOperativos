/*
 * constantes.h
 *
 *  Created on: 16/4/2017
 *      Author: utnso
 */

//////////////////////////////////////////////////////////////////////////////
//Identificadores de procesos

#define NUM_PROCESO_MASTER 1
#define NUM_PROCESO_YAMA 2
#define NUM_PROCESO_FS 3
#define NUM_PROCESO_WORKER 4
#define NUM_PROCESO_DATANODE 5

//////////////////////////////////////////////////////////////////////////////
//Tipos de mensajes para el protocolo de comunicación

#define MENSAJE_TAM_VARIABLE_ID		11		//archivo sobre el cual el master quiere trabajar

//////////////////////////////////////////////////////////////////////////////
//Largos de mensajes preestableciodos

#define LARGO_STRING_HEADER_TAM_PAYLOAD		8
#define LARGO_STRING_HEADER_ID				4
