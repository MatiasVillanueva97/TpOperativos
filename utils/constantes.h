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
//Largos de mensajes preestableciodos

//#define LARGO_STRING_HEADER_TAM_PAYLOAD		8
#define LARGO_STRING_HEADER_ID		4	//es el largo del string que indica cuantos bytes tiene el id del header que se envía por socket
#define LARGO_STRING_TAM_MENSAJE	8	//es el largo del string que indica cuantos bytes tiene el tamaño del mensaje a enviar por socket
