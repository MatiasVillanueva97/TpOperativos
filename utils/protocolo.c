/*
 * constantes.c
 *
 *  Created on: 17/10/2017
 *      Author: utnso
 */

//////////////////////////////////////////////////////////////////////////////
//Tipos de mensajes para el protocolo de comunicación

enum tiposMensajes{
	CODIGO_ERROR,CODIGO_OK,PATH_ARCHIVO,CUATRO_MENSAJES
};

const int protocoloCantidadMensajes[4]={0,0,1,4};

const char* protocoloMensajesPredefinidos[4]={"ERROR","OK","PATH_ARCHIVO",""};
