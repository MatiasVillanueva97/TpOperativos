/*
 * constantes.c
 *
 *  Created on: 17/10/2017
 *      Author: utnso
 */

//////////////////////////////////////////////////////////////////////////////
//Tipos de mensajes para el protocolo de comunicación

enum tiposMensajes{
	TIPO_MSJ_ERROR,TIPO_MSJ_OK,TIPO_MSJ_HANDSHAKE,TIPO_MSJ_PATH_ARCHIVO,TIPO_MSJ_CUATRO_MENSAJES
};

const int protocoloCantidadMensajes[5]={0,0,1,1,4};

const char* protocoloMensajesPredefinidos[5]={"ERROR","OK","HANDSHAKE","PATH_ARCHIVO",""};
