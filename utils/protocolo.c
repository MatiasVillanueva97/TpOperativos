/*
 * constantes.c
 *
 *  Created on: 17/10/2017
 *      Author: utnso
 */

//////////////////////////////////////////////////////////////////////////////
//Tipos de mensajes para el protocolo de comunicación
enum tiposMensajes {
	TIPO_MSJ_ERROR,
	TIPO_MSJ_OK,
	TIPO_MSJ_HANDSHAKE,
	TIPO_MSJ_PATH_ARCHIVO,
	TIPO_MSJ_PATH_ARCHIVO_TRANSFORMAR,
	TIPO_MSJ_CUATRO_MENSAJES,
	TIPO_MSJ_PEDIDO_METADATA_ARCHIVO,
	TIPO_MSJ_METADATA_ARCHIVO
};

//tener en cuenta!!!!!!!!!!!!!!!!!
//dentro del corchete de los array va el último elemento del enum
//si se agrega algo al final del enum modificar este parámetros en los arrays
//de esta forma al sumarle 1 se genera el array del tamaño necesario
const int protocoloCantidadMensajes[TIPO_MSJ_METADATA_ARCHIVO + 1] = { 0, 0, 1,
		1, 1, 4, 1, 13 };
const char* protocoloMensajesPredefinidos[TIPO_MSJ_METADATA_ARCHIVO + 1] = {
		"ERROR", "OK", "HANDSHAKE", "PATH_ARCHIVO","PATH_ARCHIVO_TRANSFORMAR", "CUATRO_MENSAJES",
		"PEDIDO_METADATA_ARCHIVO", "METADATA_ARCHIVO" };
