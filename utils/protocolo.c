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
	TIPO_MSJ_HANDSHAKE_RESPUESTA_OK,
	TIPO_MSJ_PATH_ARCHIVO,
	TIPO_MSJ_PATH_ARCHIVO_TRANSFORMAR,
	TIPO_MSJ_CUATRO_MENSAJES,
	TIPO_MSJ_PEDIDO_METADATA_ARCHIVO,
	TIPO_MSJ_METADATA_ARCHIVO,
	TIPO_MSJ_DATOS_CONEXION_NODOS,
	TIPO_MSJ_TRANSFORMACION_OK,
	TIPO_MSJ_TRANSFORMACION_ERROR,
	TIPO_MSJ_REDUCC_LOCAL_OK,
	TIPO_MSJ_REDUCC_LOCAL_ERROR,
	TIPO_MSJ_REDUCC_GLOBAL_OK,
	TIPO_MSJ_REDUCC_GLOBAL_ERROR,
	TIPO_MSJ_ALM_FINAL_OK,
	TIPO_MSJ_ALM_FINAL_ERROR
};

//tener en cuenta!!!!!!!!!!!!!!!!!
//dentro del corchete de los array va el último elemento del enum
//si se agrega algo al final del enum modificar este parámetros en los arrays
//de esta forma al sumarle 1 se genera el array del tamaño necesario
const int protocoloCantidadMensajes[TIPO_MSJ_ALM_FINAL_ERROR + 1] = { 0, 0, 1,
		0, 1, 1, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0 };
const char* protocoloMensajesPredefinidos[TIPO_MSJ_ALM_FINAL_ERROR + 1] = {
		"ERROR", "OK", "HANDSHAKE", "HANDSHAKE_RESPUESTA_OK", "PATH_ARCHIVO",
		"PATH_ARCHIVO_TRANSFORMAR", "CUATRO_MENSAJES",
		"PEDIDO_METADATA_ARCHIVO", "METADATA_ARCHIVO", "DATOS_CONEXION_NODOS",
		"TIPO_MSJ_TRANSFORMACION_OK", "TIPO_MSJ_TRANSFORMACION_ERROR",
		"TIPO_MSJ_REDUCC_LOCAL_OK", "TIPO_MSJ_REDUCC_LOCAL_ERROR",
		"TIPO_MSJ_REDUCC_GLOBAL_OK", "TIPO_MSJ_REDUCC_GLOBAL_ERROR",
		"TIPO_MSJ_ALM_FINAL_OK", "TIPO_MSJ_ALM_FINAL_ERROR" };
