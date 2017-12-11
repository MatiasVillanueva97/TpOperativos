/*
 ============================================================================
 Name        : yama.c
 Author      : Grupo 1234
 Description : Proceso YAMA
 ============================================================================
 */

// ================================================================ //
// YAMA coordina con Master donde correr los jobs.
// Se conecta a FileSystem. Única instancia.
// Solo hay un YAMA corriendo al mismo tiempo.
// ================================================================ //
#define CANT_MAX_FD	50

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/log.h>
#include "../../utils/constantes.h"
#include "../../utils/utils.h"
#include "../../utils/conexionesSocket.h"
#include "../../utils/archivoConfig.h"
#include "../../utils/comunicacion.h"

typedef struct {
	int nroNodo;
	int bloque;
	int bytesOcupados;
	char temporal[LARGO_TEMPORAL];
} nodoParaAsignar;

typedef struct {
	int nodoCopia1;
	int bloqueCopia1;
	int nodoCopia2;
	int bloqueCopia2;
	int bytesBloque;
} bloqueArchivo;

typedef struct {
	int numero;
	char ip[LARGO_IP];
	int puerto;
	int carga;
	int disponibilidad;
	char nombre[LARGO_NOMBRE_NODO];
} datosPropiosNodo;
datosPropiosNodo listaGlobalNodos[50];

#include "../../utils/protocolo.c"
#include "tablaEstados.h"
#include "planificacion.h"
#include "nroMasterJob.c"
#include "inicializacion.c"
#include "comunicacionesFS.c"

//para el select
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

char* serializarMensajeTransformacion(nodoParaAsignar *datosParaTransformacion, int cantPartesArchivo) {
	int i, j, k, cantStringsASerializar, largoStringDestinoCopia;

	cantStringsASerializar = (cantPartesArchivo * 6) + 1;
	char **arrayMensajes = malloc(sizeof(char*) * cantStringsASerializar);

	largoStringDestinoCopia = 4 + 1;
	arrayMensajes[0] = malloc(largoStringDestinoCopia);
	strcpy(arrayMensajes[0], intToArrayZerosLeft(cantPartesArchivo, 4));
	j = 1;
	printf("\n ---------- Tabla de transformación a enviar a master ---------- \n");
	printf("\tNodo\tIP\t\tPuerto\tBloque\tBytes\t\tTemporal\n");
	printf("---------------------------------------------------------------------------------------------\n");
	for (i = 0; i < cantPartesArchivo; i++) {
		printf("\t%d\t%s\t%d\t%d\t%d\t\t%s\n", datosParaTransformacion[i].nroNodo, listaGlobalNodos[datosParaTransformacion[i].nroNodo].ip, listaGlobalNodos[datosParaTransformacion[i].nroNodo].puerto, datosParaTransformacion[i].bloque, datosParaTransformacion[i].bytesOcupados, datosParaTransformacion[i].temporal);

		//número de nodo
		largoStringDestinoCopia = 4 + 1;
		arrayMensajes[j] = malloc(largoStringDestinoCopia);
		strcpy(arrayMensajes[j], intToArrayZerosLeft(datosParaTransformacion[i].nroNodo, 4));
		j++;

		//IP
		largoStringDestinoCopia = string_length(listaGlobalNodos[datosParaTransformacion[i].nroNodo].ip) + 1;
		arrayMensajes[j] = malloc(largoStringDestinoCopia);
		strcpy(arrayMensajes[j], listaGlobalNodos[datosParaTransformacion[i].nroNodo].ip);
		j++;

		//puerto
		largoStringDestinoCopia = LARGO_PUERTO + 1;
		arrayMensajes[j] = malloc(largoStringDestinoCopia);
		strcpy(arrayMensajes[j], intToArrayZerosLeft(listaGlobalNodos[datosParaTransformacion[i].nroNodo].puerto, LARGO_PUERTO));
		j++;

		//bloque
		largoStringDestinoCopia = 4 + 1;
		arrayMensajes[j] = malloc(largoStringDestinoCopia);
		strcpy(arrayMensajes[j], intToArrayZerosLeft(datosParaTransformacion[i].bloque, 4));
		j++;

		//bytes ocupados
		largoStringDestinoCopia = 8 + 1;
		arrayMensajes[j] = malloc(largoStringDestinoCopia);
		strcpy(arrayMensajes[j], intToArrayZerosLeft(datosParaTransformacion[i].bytesOcupados, 8));
		j++;

		//temporal
		largoStringDestinoCopia = string_length(datosParaTransformacion[i].temporal) + 1;
		arrayMensajes[j] = malloc(largoStringDestinoCopia);
		strcpy(arrayMensajes[j], datosParaTransformacion[i].temporal);
		j++;
	}
	printf("\n");
	char *mensajeSerializado = serializarMensaje(TIPO_MSJ_TABLA_TRANSFORMACION, arrayMensajes, cantStringsASerializar);
	for (j = 0; j < cantStringsASerializar; j++) {
		free(arrayMensajes[j]);
	}
	free(arrayMensajes);
	return mensajeSerializado;

}

char* serializarMensajeReduccLocal(int nroNodoRecibido, nroMasterJob masterJobActual, char *temporalReduccLocal) {
	int j, k, h;
	int cantidadTemporalesTransformacion = getCantFilasByJMNEtEs(masterJobActual.nroJob, masterJobActual.nroMaster, nroNodoRecibido, TRANSFORMACION, FIN_OK);
	char **temporales = malloc(sizeof(char*) * cantidadTemporalesTransformacion);
	getAllTemporalesByJMNEtEs(temporales, masterJobActual.nroJob, masterJobActual.nroMaster, nroNodoRecibido, TRANSFORMACION, FIN_OK);

	//armar el string serializado
	int cantNodosReduccLocal = 1;
	int cantStrings = 1 + cantNodosReduccLocal * (4 + cantidadTemporalesTransformacion + 1);
	char **arrayMensajesSerializarReduccLocal = malloc(sizeof(char*) * cantStrings);

	//cantidad de nodos
	j = 0;
	arrayMensajesSerializarReduccLocal[j] = malloc(4 + 1);
	if (!arrayMensajesSerializarReduccLocal[j])
		perror("error de malloc");
	strcpy(arrayMensajesSerializarReduccLocal[j], intToArrayZerosLeft(cantNodosReduccLocal, 4));
	j++;

	//para cada nodo
	for (k = 0; k < cantNodosReduccLocal; k++) {
		//nro de nodo
		arrayMensajesSerializarReduccLocal[j] = malloc(4 + 1);
		if (!arrayMensajesSerializarReduccLocal[j])
			perror("error de malloc");
		strcpy(arrayMensajesSerializarReduccLocal[j], intToArrayZerosLeft(nroNodoRecibido, 4));
		j++;
		//IP del nodo
		arrayMensajesSerializarReduccLocal[j] = malloc(string_length(listaGlobalNodos[nroNodoRecibido].ip) + 1);
		if (!arrayMensajesSerializarReduccLocal[j])
			perror("error de malloc");
		strcpy(arrayMensajesSerializarReduccLocal[j], listaGlobalNodos[nroNodoRecibido].ip);
		j++;
		//puerto del nodo
		arrayMensajesSerializarReduccLocal[j] = malloc(LARGO_PUERTO + 1);
		if (!arrayMensajesSerializarReduccLocal[j])
			perror("error de malloc");
		strcpy(arrayMensajesSerializarReduccLocal[j], intToArrayZerosLeft(listaGlobalNodos[nroNodoRecibido].puerto, LARGO_PUERTO));
		j++;
		//cantidad de temporales
		arrayMensajesSerializarReduccLocal[j] = malloc(4 + 1);
		if (!arrayMensajesSerializarReduccLocal[j])
			perror("error de malloc");
		strcpy(arrayMensajesSerializarReduccLocal[j], intToArrayZerosLeft(cantidadTemporalesTransformacion, 4));
		j++;
		//todos los temporales uno a continuación del otro
		for (h = 0; h < cantidadTemporalesTransformacion; h++) {
			//temporal h
			printf("temporal %s\n", temporales[h]);
			arrayMensajesSerializarReduccLocal[j] = malloc(string_length(temporales[h]) + 1);
			if (!arrayMensajesSerializarReduccLocal[j])
				perror("error de malloc");
			strcpy(arrayMensajesSerializarReduccLocal[j], temporales[h]);
			j++;
		}
		arrayMensajesSerializarReduccLocal[j] = malloc(string_length(temporalReduccLocal) + 1);
		if (!arrayMensajesSerializarReduccLocal[j])
			perror("error de malloc");
		strcpy(arrayMensajesSerializarReduccLocal[j], temporalReduccLocal);
		j++;
	}

	///////////////mensaje serializado
	char *mensajeSerializadoReduccLocal = serializarMensaje(TIPO_MSJ_TABLA_REDUCCION_LOCAL, arrayMensajesSerializarReduccLocal, cantStrings);
	for (j = 0; j < cantStrings; j++) {
		free(arrayMensajesSerializarReduccLocal[j]);
	}
	free(arrayMensajesSerializarReduccLocal);

//	for (h = 0; h < cantidadTemporales; h++) {
//		free(temporales[j]); //TODO: revisar que no rompa
//	}
	free(temporales);
	return mensajeSerializadoReduccLocal;
}

char* serializarMensajeReduccGlobal(int cantNodosReduccGlobal, struct filaTablaEstados *filasReduccGlobal, char* temporalRedGlobal) {
	int i, j;
	int cantStrings = 1 + cantNodosReduccGlobal * 4 + 1;
	char **arrayMensajesSerializarRedGlobal = malloc(sizeof(char*) * cantStrings);

	//cantidad de nodos
	j = 0;
	arrayMensajesSerializarRedGlobal[j] = malloc(4 + 1);
	if (!arrayMensajesSerializarRedGlobal[j])
		perror("error de malloc");
	strcpy(arrayMensajesSerializarRedGlobal[j], intToArrayZerosLeft(cantNodosReduccGlobal, 4));
	j++;
	int nroNodo;
	for (i = 0; i < cantNodosReduccGlobal; i++) {
		nroNodo = filasReduccGlobal[i].nodo;
		//nro de nodo
		arrayMensajesSerializarRedGlobal[j] = malloc(4 + 1);
		if (!arrayMensajesSerializarRedGlobal[j])
			perror("error de malloc");
		strcpy(arrayMensajesSerializarRedGlobal[j], intToArrayZerosLeft(nroNodo, 4));
		j++;
		//IP del nodo
		arrayMensajesSerializarRedGlobal[j] = malloc(string_length(listaGlobalNodos[nroNodo].ip) + 1);
		if (!arrayMensajesSerializarRedGlobal[j])
			perror("error de malloc");
		strcpy(arrayMensajesSerializarRedGlobal[j], listaGlobalNodos[nroNodo].ip);
		j++;
		//puerto del nodo
		arrayMensajesSerializarRedGlobal[j] = malloc(LARGO_PUERTO + 1);
		if (!arrayMensajesSerializarRedGlobal[j])
			perror("error de malloc");
		strcpy(arrayMensajesSerializarRedGlobal[j], intToArrayZerosLeft(listaGlobalNodos[nroNodo].puerto, LARGO_PUERTO));
		j++;
		//temporal de la fila
		arrayMensajesSerializarRedGlobal[j] = malloc(string_length(filasReduccGlobal[i].temporal) + 1);
		if (!arrayMensajesSerializarRedGlobal[j])
			perror("error de malloc");
		strcpy(arrayMensajesSerializarRedGlobal[j], filasReduccGlobal[i].temporal);
		j++;
	}
	//temporal global
	arrayMensajesSerializarRedGlobal[j] = malloc(string_length(temporalRedGlobal) + 1);
	if (!arrayMensajesSerializarRedGlobal[j])
		perror("error de malloc");
	strcpy(arrayMensajesSerializarRedGlobal[j], temporalRedGlobal);
	j++;

	char *mensajeSerializadoRedGlobal = serializarMensaje(TIPO_MSJ_TABLA_REDUCCION_GLOBAL, arrayMensajesSerializarRedGlobal, cantStrings);
	for (j = 0; j < cantStrings; j++) {
		free(arrayMensajesSerializarRedGlobal[j]);
	}
	free(arrayMensajesSerializarRedGlobal);
	return mensajeSerializadoRedGlobal;
}

char* serializarMensajeAlmFinal(int nroNodoReduccGlobal, char *temporalAlmFinal) {
	int i, j;
	int cantStrings = 4;
	char **arrayMensajesSerializar = malloc(sizeof(char*) * cantStrings);

	j = 0;
	//nro de nodo
	arrayMensajesSerializar[j] = malloc(4 + 1);
	if (!arrayMensajesSerializar[j])
		perror("error de malloc");
	strcpy(arrayMensajesSerializar[j], intToArrayZerosLeft(nroNodoReduccGlobal, 4));
	j++;
	//IP del nodo
	arrayMensajesSerializar[j] = malloc(string_length(listaGlobalNodos[nroNodoReduccGlobal].ip) + 1);
	if (!arrayMensajesSerializar[j])
		perror("error de malloc");
	strcpy(arrayMensajesSerializar[j], listaGlobalNodos[nroNodoReduccGlobal].ip);
	j++;
	//puerto del nodo
	arrayMensajesSerializar[j] = malloc(LARGO_PUERTO + 1);
	if (!arrayMensajesSerializar[j])
		perror("error de malloc");
	strcpy(arrayMensajesSerializar[j], intToArrayZerosLeft(listaGlobalNodos[nroNodoReduccGlobal].puerto, LARGO_PUERTO));
	j++;
	//temporal de la fila
	arrayMensajesSerializar[j] = malloc(string_length(temporalAlmFinal) + 1);
	if (!arrayMensajesSerializar[j])
		perror("error de malloc");
	strcpy(arrayMensajesSerializar[j], temporalAlmFinal);
	j++;

	char *mensajeSerializado = serializarMensaje(TIPO_MSJ_TABLA_ALMACENAMIENTO_FINAL, arrayMensajesSerializar, cantStrings);
	for (j = 0; j < cantStrings; j++) {
		free(arrayMensajesSerializar[j]);
	}
	free(arrayMensajesSerializar);
	return mensajeSerializado;
}

int main(int argc, char *argv[]) {
	logYAMA = log_create("logYAMA.log", "YAMA", false, LOG_LEVEL_TRACE); //creo el logger, sin mostrar por pantalla
	int h, i, j, k;
	char mensajeHeaderSolo[4];
	int32_t headerId;
	log_info(logYAMA, "Iniciando proceso YAMA");
	printf("\n*** Proceso Yama ***\n");
	//para el select
	fd_set socketsLecturaMaster;    // master file descriptor list
	fd_set socketsLecturaTemp;  // temp file descriptor list for select()
	int maxFD;        // maximum file descriptor number
	// clear the set ahead of time
	FD_ZERO(&socketsLecturaMaster);
	FD_ZERO(&socketsLecturaTemp);

	if (!getDatosConfiguracion()) {
		log_error(logYAMA, "No se pudieron obtener los datos del archivo de configuración. Se aborta la ejecución");
		log_info(logYAMA, "Server cerrado");
		log_destroy(logYAMA);
		return EXIT_FAILURE;
	}
	//para la planificación
	disponibBase = atoi(datosConfigYama[DISPONIBILIDAD_BASE]);
	strcpy(algoritmoPlanificacion, datosConfigYama[ALGORITMO_BALANCEO]);
	/* ************** conexión como cliente al FS *************** */
	int socketFS, preparadoFs = 0;
	if ((socketFS = conexionAFileSystem()) < 0) {
		//preparadoEnviarFs = 0;
		log_error(logYAMA, "No se pudo conectar al FileSystem. Se aborta la ejecución");
		log_info(logYAMA, "Server cerrado");
		log_destroy(logYAMA);
		puts("Abortar ejecución");
		return EXIT_FAILURE;
	}
	int modulo = yama;
	while (preparadoFs == 0) {
		send(socketFS, &modulo, sizeof(int), MSG_WAITALL);
		headerId = deserializarHeader(socketFS);
		if (headerId == TIPO_MSJ_HANDSHAKE_RESPUESTA_OK) {
			log_info(logYAMA, "El Filesystem está estable. Se puede continuar la ejecución");
			preparadoFs = 1;
		} else {
			log_info(logYAMA, "Se está esperando que el Filesystem esté estable");
			puts("Sigo esperando que el Filesystem esté estable");
		}
	}
	// add the socketFs to the master set
	//FD_SET(socketFS, &socketsLecturaMaster);
	// keep track of the biggest file descriptor
	//maxFD = socketFS; // so far, it's this one

	/* ************** inicialización como server ************ */
	int listenningSocket;
	if ((listenningSocket = inicializoComoServidor()) < 0) {
		log_error(logYAMA, "No se puede iniciar como servidor. Se aborta la ejecución");
		log_info(logYAMA, "Server cerrado");
		log_destroy(logYAMA);
		return EXIT_FAILURE;
	}
	// add the listener to the master set
	FD_SET(listenningSocket, &socketsLecturaMaster);
	// keep track of the biggest file descriptor
	maxFD = listenningSocket; // so far, it's this one

	int socketCliente, socketConectado, cantStrings, bytesRecibidos = 0,
			nroSocket, nroNodoReduccGlobal, nroNodoRecibido, nroBloqueRecibido,
			nroNodoAlmacFinal, cantPartesArchivo, cantNodosArchivo;
	nroMasterJob masterJobActual;
	//pongo la carga de cada nodo en 0 al iniciar
	int largoListaGlobalNodos = sizeof(listaGlobalNodos) / sizeof(datosPropiosNodo);
	for (i = 0; i < largoListaGlobalNodos; i++) {
		listaGlobalNodos[i].carga = 0;
	}
	for (;;) {
		socketsLecturaTemp = socketsLecturaMaster;
		if (select(maxFD + 1, &socketsLecturaTemp, NULL, NULL, NULL) != -1) {
			for (nroSocket = 0; nroSocket <= maxFD; nroSocket++) {
				if (FD_ISSET(nroSocket, &socketsLecturaTemp)) {
					if (nroSocket == listenningSocket) {	//conexión nueva
						if ((socketCliente = recibirConexion(listenningSocket)) >= 0) {
							int32_t headerId = deserializarHeader(socketCliente);

							if (headerId == TIPO_MSJ_HANDSHAKE) {
								int cantidadMensajes = protocoloCantidadMensajes[headerId];
								char **arrayMensajesRHS = deserializarMensaje(socketCliente, cantidadMensajes);
								int idEmisorMensaje = atoi(arrayMensajesRHS[0]);
								free(arrayMensajesRHS);
								if (idEmisorMensaje == NUM_PROCESO_MASTER) {
									FD_SET(socketCliente, &socketsLecturaMaster); // add to master set
									if (socketCliente > maxFD) { // keep track of the max
										maxFD = socketCliente;
									}
									asignarNroMasterJob(getNuevoNroMaster(), getNuevoNroJob(), socketCliente);
									enviarHeaderSolo(socketCliente, TIPO_MSJ_HANDSHAKE_RESPUESTA_OK);
									log_info(logYAMA, "Handshake verificado. Se acepta una nueva conexión de un Master y se la comienza a escuchar");

								} else {
									log_info(logYAMA, "Handshake denegado. No se acepta la nueva conexión");
									enviarHeaderSolo(socketCliente, TIPO_MSJ_HANDSHAKE_RESPUESTA_DENEGADO);
								}
							}
						}
					} else {	//conexión preexistente
						/* *************************** recepción de un mensaje ****************************/
						socketConectado = nroSocket;
						log_info(logYAMA, "Se recibió un mensaje de proceso conectado por FD %d", socketConectado);
						masterJobActual = getNroMasterJobByFD(socketConectado);
						printf("\nSocket conectado: %d\n", socketConectado);
						printf("Master actual: %d\n", masterJobActual.nroMaster);
						printf("Job actual: %d\n", masterJobActual.nroJob);
						int32_t headerId = deserializarHeader(socketConectado);
						printf("\nHeader Id: %d\n", headerId);
						printf("Header mensaje: %s\n", protocoloMensajesPredefinidos[headerId]);
						if (headerId <= 0) {//error o desconexión de un cliente
							log_error(logYAMA, "Se desconectó el proceso conectado por FD %d. Se lo deja de escuchar", socketConectado);
							cerrarCliente(socketConectado); // bye!
							FD_CLR(socketConectado, &socketsLecturaMaster); // remove from master set
						}
						int cantidadMensajes = protocoloCantidadMensajes[headerId];
						char **arrayMensajes = deserializarMensaje(socketConectado, cantidadMensajes);
						switch (headerId) {
						case TIPO_MSJ_PATH_ARCHIVO_TRANSFORMAR:
							;
							char *archivo = malloc(string_length(arrayMensajes[0]) + 1);
							strcpy(archivo, arrayMensajes[0]);
							free(arrayMensajes);
							//pide la metadata del archivo al FS
							if (pedirMetadataArchivoFS(socketFS, archivo) > 0) {

								/* ************* solicitud de info del archivo al FS *************** */
								//recibir las partes del archivo
								int32_t headerId = deserializarHeader(socketFS);
								if (headerId != TIPO_MSJ_METADATA_ARCHIVO) {
									perror("El FS no mandó los bloques");
								}
								printf("Header Id: %d\n", headerId);
								printf("Header mensaje: %s\n", protocoloMensajesPredefinidos[headerId]);
								cantPartesArchivo = getCantidadPartesArchivoFS(socketFS, protocoloCantidadMensajes[headerId]);

								bloqueArchivo *bloques = recibirMetadataArchivoFS(socketFS, cantPartesArchivo);
								printf("\n ---------- Lista de bloques del archivo devuelto por FS ---------- \n");
								for (i = 0; i < cantPartesArchivo; i++) {
									printf("nodoCopia1 %d - bloqueCopia1 %d - nodoCopia2 %d - bloqueCopia2 %d - bytes %d\n", bloques[i].nodoCopia1, bloques[i].bloqueCopia1, bloques[i].nodoCopia2, bloques[i].bloqueCopia2, bloques[i].bytesBloque);
								}

								/* ********************* */
								//recibir la info de los nodos donde están esos archivos
								headerId = deserializarHeader(socketFS);
								printf("Header Id: %d\n", headerId);
								printf("Header mensaje: %s\n", protocoloMensajesPredefinidos[headerId]);
								if (headerId != TIPO_MSJ_DATOS_CONEXION_NODOS) {
									printf("El FS no mandó los nodos\n");
								}
								cantNodosArchivo = getCantidadNodosFS(socketFS, protocoloCantidadMensajes[headerId]);
								//guardar los nodos en la listaGlobal
								datosPropiosNodo nodosParaPlanificar[cantNodosArchivo];
								recibirNodosArchivoFS(socketFS, cantNodosArchivo, nodosParaPlanificar);
								printf("\n ---------- Lista global de nodos ---------- \n");
								for (k = 0; k < cantNodosArchivo; k++) {
									printf("Nodo %d: nro %d - ip %s - puerto %d\n", k + 1, listaGlobalNodos[k + 1].numero, listaGlobalNodos[k + 1].ip, listaGlobalNodos[k + 1].puerto);
									printf("Planificar %d: nro %d - ip %s - puerto %d\n", k, nodosParaPlanificar[k].numero, nodosParaPlanificar[k].ip, nodosParaPlanificar[k].puerto);
								}
								/* ***************************************************************** */

								/* ************* inicio planificación *************** */
								//le paso el vector donde debe ir guardando las asignaciones de nodos planificados
								//indexado por partes del archivo
								nodoParaAsignar asignacionesNodos[cantPartesArchivo];
								planificar(socketConectado, bloques, asignacionesNodos, cantPartesArchivo, cantNodosArchivo, nodosParaPlanificar);
								printf("\n ---------- Lista de nodos para planificación ---------- \n");
								for (i = 0; i < cantNodosArchivo; i++) {
									printf("Nro nodo %d - Carga %d\n", nodosParaPlanificar[i].numero, nodosParaPlanificar[i].carga);
								}
								//guardo el nodo donde se va a hacer la reducción global de ese master y job
								asignarNodoReduccGlobal((uint16_t) nodosParaPlanificar[0].numero, socketConectado);
								/* ************* fin planificación *************** */

								/* ************** agregado en tabla de estados *************** */
								//guarda la info de los bloques del archivo en la tabla de estados
								struct filaTablaEstados fila;
								for (i = 0; i < cantPartesArchivo; i++) {
									//printf("parte de archivo %d asignado a: nodo %d - bloque %d\n", i, asignacionesNodos[i][0], asignacionesNodos[i][1]);
									//genera una fila en la tabla de estados
									fila.job = masterJobActual.nroJob;
									fila.master = masterJobActual.nroMaster;
									fila.nodo = asignacionesNodos[i].nroNodo;
									fila.bloque = asignacionesNodos[i].bloque;
									fila.etapa = TRANSFORMACION;
									char* temporal = string_from_format("m%dj%dn%db%de%d", fila.master, fila.job, fila.nodo, fila.bloque, fila.etapa);
									strcpy(fila.temporal, temporal);
									fila.estado = EN_PROCESO;
									fila.siguiente = NULL;
									if (!agregarElemTablaEstados(fila)) {
										log_error(logYAMA, "Ocurrió un error al agregar un elemento la tabla de estados en la etapa de pedido de bloques de archivo");
										perror("Error al agregar elementos a la tabla de estados");
									}

									//guarda el archivo temporal en el vector que se va a usar
									//en la tabla de transformación para el master
									strcpy(asignacionesNodos[i].temporal, temporal);
								}
								//puts("\nlista de elementos asignados a transformación");
								//mostrarTablaEstados();
								/* ************** fin agregado en tabla de estados *************** */

								/* ****** envío de nodos para la transformación ******************* */
								//envía al master la lista de nodos donde trabajar cada bloque
								char *mensajeSerializado = serializarMensajeTransformacion(asignacionesNodos, cantPartesArchivo);
								//printf("\n ---------- Mensaje serializado ---------- \n%s\n", mensajeSerializado);
								enviarMensaje(socketConectado, mensajeSerializado);
								/* **************************************************************** */
								//puts("\nlista de elementos luego de enviar la tabla de transformación");
								mostrarTablaEstados();
							} else {
								perror("No se pudo pedir el archivo al FS");
							}

							break;
						case TIPO_MSJ_TRANSFORMACION_OK:
							;
							nroNodoRecibido = atoi(arrayMensajes[0]);
							nroBloqueRecibido = atoi(arrayMensajes[1]);
							log_info(logYAMA, "Se recibió mensaje de fin de transformación OK, nodo %d, bloque %d", nroNodoRecibido, nroBloqueRecibido);
							printf("Nodo recibido: %d\n", nroNodoRecibido);
							printf("Bloque recibido: %d\n", nroBloqueRecibido);
							free(arrayMensajes);
							disminuirCargaGlobalNodo(listaGlobalNodos[nroNodoRecibido], 1);

							//pongo la fila en estado FIN_OK
							if (modificarEstadoFilasTablaEstados(masterJobActual.nroJob, masterJobActual.nroMaster, nroNodoRecibido, nroBloqueRecibido, TRANSFORMACION, EN_PROCESO, FIN_OK) == 0) {
								log_error(logYAMA, "Ocurrió un error al modificar la tabla de estados en el fin de la transformación OK");
								puts("No se pudo modificar ninguna fila de la tabla de estados");
							} else {
								log_info(logYAMA, "Se modificó la tabla de estados en el fin de la transformación OK");
							}
							//si no queda ninguna fila de ese nodo en proceso inicia la reducción local
							if (getCantFilasByJMNEtEs(masterJobActual.nroJob, masterJobActual.nroMaster, nroNodoRecibido, TRANSFORMACION, EN_PROCESO) == 0) {
								//inicia la reducción local del nodo recibido

								/* ************** agregado de la fila de reducción local en tabla de estados *************** */
								struct filaTablaEstados fila;
								fila.job = masterJobActual.nroJob;
								fila.master = masterJobActual.nroMaster;
								fila.nodo = nroNodoRecibido;
								fila.bloque = 0;
								fila.etapa = REDUCC_LOCAL;
								char* temporalRedLocal = string_from_format("m%dj%dn%de%d", fila.master, fila.job, fila.nodo, fila.etapa);
								strcpy(fila.temporal, temporalRedLocal);
								fila.estado = EN_PROCESO;
								fila.siguiente = NULL;

								if (!agregarElemTablaEstados(fila)) {
									log_error(logYAMA, "Ocurrió un error al agregar un elemento la tabla de estados en fin de transformación OK");
									perror("Error al agregar elementos a la tabla de estados");
								}
								/* *********** enviar data para reducción local ********** */
								//obtener los registros de la tabla de estados, cantidad y temporales
								char *mensajeSerializadoRedLocal = serializarMensajeReduccLocal(nroNodoRecibido, masterJobActual, temporalRedLocal);
								printf("\nmensaje serializado para reducción local: %s\n", mensajeSerializadoRedLocal);
								enviarMensaje(socketConectado, mensajeSerializadoRedLocal);
								log_trace(logYAMA, "Se da inicio a la Reducción Local en el nodo %d", nroNodoRecibido);
								puts("\nlista de elementos luego de enviar la tabla de reducción local");
								mostrarTablaEstados();
							}

							break;
						case TIPO_MSJ_TRANSFORMACION_ERROR:
							;
							nroNodoRecibido = atoi(arrayMensajes[0]);
							nroBloqueRecibido = atoi(arrayMensajes[1]);
							log_info(logYAMA, "Se recibió mensaje de fin de transformación ERROR, nodo %d, bloque %d. Se intenta replanificar.", nroNodoRecibido, nroBloqueRecibido);
							printf("Nodo recibido: %d\n", nroNodoRecibido);
							printf("Bloque recibido: %d\n", nroBloqueRecibido);
							free(arrayMensajes);
							disminuirCargaGlobalNodo(listaGlobalNodos[nroNodoRecibido], 1);

							if (modificarEstadoFilasTablaEstados(masterJobActual.nroJob, masterJobActual.nroMaster, nroNodoRecibido, nroBloqueRecibido, TRANSFORMACION, EN_PROCESO, ERROR) == 0) {
								log_error(logYAMA, "Ocurrió un error al modificar la tabla de estados");
								puts("No se pudo modificar ninguna fila de la tabla de estados");
							} else {
								log_info(logYAMA, "Se modificó la tabla de estados en el fin de la transformación ERROR");
							}
							//replanificar: envía el nodo y bloque de la copia del bloque
							//de archivo que no se pudo transformar
							//TODO: hacer de nuevo la replanificación !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
							/*
							 * tengo que pedir de nuevo los bloques del archivo
							 * replanificar el bloque que falló
							 */

							int cantBloquesArchivo = listaNrosMasterJob[socketConectado].cantBloquesArchivo;
							nodosUsadobloqueArchivo nodoSuplente;
							int nodoSuplenteEncontrado = 0;
							for (i = 0; i < cantBloquesArchivo; i++) {
								if (nroNodoRecibido == bloquesArchivoXFD[socketConectado][i].nodoUsado && nroBloqueRecibido == bloquesArchivoXFD[socketConectado][i].bloqueUsado) {
									//tomar el nodo suplente para enviarlo
									nodoSuplente = bloquesArchivoXFD[socketConectado][i];
									nodoSuplenteEncontrado = 1;
								}
							}
							if (!nodoSuplenteEncontrado) {
								//no se pudo encontrar un bloque alternativo, se aborta el job
								enviarHeaderSolo(socketConectado, TIPO_MSJ_ABORTAR_JOB);
								cerrarCliente(socketConectado);
								FD_CLR(socketConectado, &socketsLecturaMaster); // remove from master set
							} else {
								//se genera la nueva línea en la tabla de estados
								struct filaTablaEstados fila;
								fila.job = masterJobActual.nroJob;
								fila.master = masterJobActual.nroMaster;
								fila.nodo = nodoSuplente.nodoSuplente;
								fila.bloque = nodoSuplente.bloqueSuplente;
								fila.etapa = TRANSFORMACION;
								char* temporal = string_from_format("m%dj%dn%db%de%d", fila.master, fila.job, fila.nodo, fila.bloque, fila.etapa);
								strcpy(fila.temporal, temporal);
								fila.estado = EN_PROCESO;
								fila.siguiente = NULL;

								if (!agregarElemTablaEstados(fila)) {
									log_error(logYAMA, "Ocurrió un error al agregar un elemento la tabla de estados en fin de transformación ERROR");
									perror("Error al agregar elementos a la tabla de estados");
								}
								//se envía al master la orden de transformación en el nodo donde está la copia
								nodoParaAsignar dataReplanificacion[1];
								dataReplanificacion[0].nroNodo = nodoSuplente.nodoSuplente;
								dataReplanificacion[0].bloque = nodoSuplente.bloqueSuplente;
								dataReplanificacion[0].bytesOcupados = nodoSuplente.bytes;
								strcpy(dataReplanificacion[0].temporal, temporal);
								char *mensajeSerializado = serializarMensajeTransformacion(dataReplanificacion, 1);
								printf("\nmensaje serializado: \n%s\n", mensajeSerializado);
								enviarMensaje(socketConectado, mensajeSerializado);
							}
							break;
						case TIPO_MSJ_REDUCC_LOCAL_OK:
							;
							nroNodoRecibido = atoi(arrayMensajes[0]);
							log_info(logYAMA, "Se recibió mensaje de fin de Reducción Local OK, nodo %d.", nroNodoRecibido);
							printf("nroNodoRecibido: %d\n", nroNodoRecibido);
							free(arrayMensajes);
							disminuirCargaGlobalNodo(listaGlobalNodos[nroNodoRecibido], 1);

							//modificar el estado en la tabla de estados
							if (modificarEstadoFilasTablaEstados(masterJobActual.nroJob, masterJobActual.nroMaster, nroNodoRecibido, 0, REDUCC_LOCAL, EN_PROCESO, FIN_OK) == 0) {
								log_error(logYAMA, "Ocurrió un error al modificar la tabla de estados en el fin de la Reducción Local OK");
								puts("No se pudo modificar ninguna fila de la tabla de estados");
							} else {
								log_info(logYAMA, "Se modificó la tabla de estados en el fin de la Reducción Local OK");
							}

							//verificar que todos los nodos del job y master hayan terminado la reducción local
							if (getCantFilasByJMEtEs(masterJobActual.nroJob, masterJobActual.nroMaster, REDUCC_LOCAL, EN_PROCESO) == 0) {
								/* ************** agregado de la fila de reducción global en tabla de estados *************** */
								int nodoReduccGlobal = (int) getNroMasterJobByFD(socketConectado).nodoReduccGlobal;

								struct filaTablaEstados fila;
								fila.job = masterJobActual.nroJob;
								fila.master = masterJobActual.nroMaster;
								fila.nodo = nodoReduccGlobal;
								fila.bloque = 0;
								fila.etapa = REDUCC_GLOBAL;
								char* temporalRedGlobal = string_from_format("m%dj%de%d", fila.master, fila.job, fila.etapa);
								strcpy(fila.temporal, temporalRedGlobal);
								fila.estado = EN_PROCESO;
								fila.siguiente = NULL;
								if (!agregarElemTablaEstados(fila)) {
									log_error(logYAMA, "Ocurrió un error al agregar un elemento la tabla de estados en fin de Reducción Local OK");
									perror("Error al agregar elementos a la tabla de estados");
								}
								//iniciar la reducción global
								int cantNodosReduccGlobal = getCantFilasByJMEtEs(masterJobActual.nroJob, masterJobActual.nroMaster, REDUCC_LOCAL, FIN_OK);
								struct filaTablaEstados filasReduccGlobal[cantNodosReduccGlobal];

								struct filaTablaEstados filaBusqueda;
								filaBusqueda.job = masterJobActual.nroJob;
								filaBusqueda.master = masterJobActual.nroMaster;
								filaBusqueda.nodo = 0;
								filaBusqueda.bloque = 0;
								filaBusqueda.etapa = REDUCC_LOCAL;
								strcpy(filaBusqueda.temporal, "");
								filaBusqueda.estado = FIN_OK;
								filaBusqueda.siguiente = NULL;
								int cantFilasEncontradas = buscarMuchosElemTablaEstados(filasReduccGlobal, filaBusqueda);
								if (cantFilasEncontradas == 0) {
									puts("no se encontró ninguna fila de la tabla de estados para hacer la reducción global");
								}
								if (cantFilasEncontradas != cantNodosReduccGlobal) {
									puts("hubo un error en la cantidad de filas encontradas");
								} else {
									//hacer que el nodo donde se hace la reducción global vaya primero
									//buscar el nodo asignado en el vector filasReduccGlobal e intercambiarlo por el primero
									//buscar la fila por nodo, cuando la encuentra la guarda en un temporal
									//pone la fila 0  en la fila i donde estaba la del nodo buscado
									//pone la del temporal en la fila 0
									struct filaTablaEstados temporal;
									for (k = 0; k < cantNodosReduccGlobal;
											k++) {
										printf("k %d - nodo %d - temporal %s\n", k, filasReduccGlobal[k].nodo, filasReduccGlobal[k].temporal);
										if (filasReduccGlobal[k].nodo == nodoReduccGlobal) {
											temporal = filasReduccGlobal[k];
											filasReduccGlobal[k] = filasReduccGlobal[0];
											filasReduccGlobal[0] = temporal;
											break;
										}
									}
									for (k = 0; k < cantNodosReduccGlobal;
											k++) {
										printf("k %d - nodo %d - temporal %s\n", k, filasReduccGlobal[k].nodo, filasReduccGlobal[k].temporal);
									}

									/* ******* envío de la tabla para reducción global ****** */
									char *mensajeSerializadoRedGlobal = serializarMensajeReduccGlobal(cantNodosReduccGlobal, filasReduccGlobal, temporalRedGlobal);
									printf("\nmensaje serializado para reducción global: %s\n", mensajeSerializadoRedGlobal);
									enviarMensaje(socketConectado, mensajeSerializadoRedGlobal);
									log_info(logYAMA, "Se da inicio a la Reducción Global en el nodo %d", nodoReduccGlobal);
									puts("\nlista de elementos luego de enviar la tabla de reducción global");
									mostrarTablaEstados();
								}
							}
							break;
						case TIPO_MSJ_REDUCC_LOCAL_ERROR:
							;
							nroNodoRecibido = atoi(arrayMensajes[0]);
							log_info(logYAMA, "Se recibió mensaje de fin de Reducción Local ERROR, nodo %d.", nroNodoRecibido);
							free(arrayMensajes);
							disminuirCargaGlobalNodo(listaGlobalNodos[nroNodoRecibido], 1);

							if (modificarEstadoFilasTablaEstados(masterJobActual.nroJob, masterJobActual.nroMaster, nroNodoRecibido, 0, REDUCC_LOCAL, EN_PROCESO, ERROR) == 0) {
								log_error(logYAMA, "Ocurrió un error al modificar la tabla de estados en el fin de la Reducción Local ERROR");
								puts("No se pudo modificar ninguna fila de la tabla de estados");
							} else {
								log_info(logYAMA, "Se modificó la tabla de estados en el fin de la Reducción Local ERROR");
							}
							//abortar el job
							enviarHeaderSolo(socketConectado, TIPO_MSJ_ABORTAR_JOB);
							cerrarCliente(socketConectado);
							FD_CLR(socketConectado, &socketsLecturaMaster); // remove from master set
							log_info(logYAMA, "Se aborta el Job %d del master %d conectado por FD %d", masterJobActual.nroJob, masterJobActual.nroMaster, socketConectado);
							break;
						case TIPO_MSJ_REDUCC_GLOBAL_OK:
							;
							free(arrayMensajes);
							//obtengo el nodo donde se hizo la reducción global
							int nroNodoReduccGlobal = getNodoReduccGlobal(masterJobActual.nroJob, masterJobActual.nroMaster, REDUCC_GLOBAL, EN_PROCESO);
							log_info(logYAMA, "Se recibió mensaje de fin de Reducción Global OK, nodo %d.", nroNodoReduccGlobal);
							disminuirCargaGlobalNodo(listaGlobalNodos[nroNodoReduccGlobal], 1);

							//modifica el estado de la fila
							if (modificarEstadoFilasTablaEstados(masterJobActual.nroJob, masterJobActual.nroMaster, nroNodoReduccGlobal, 0, REDUCC_GLOBAL, EN_PROCESO, FIN_OK) == 0) {
								log_error(logYAMA, "Ocurrió un error al modificar la tabla de estados en el fin de la Reducción Global OK");
								puts("No se pudo modificar ninguna fila de la tabla de estados");
							} else {
								log_info(logYAMA, "Se modificó la tabla de estados en el fin de la Reducción Global OK");
							}

							/* ************** agregado de la fila de almacenamiento final en tabla de estados *************** */
							struct filaTablaEstados fila;
							fila.job = masterJobActual.nroJob;
							fila.master = masterJobActual.nroMaster;
							fila.nodo = nroNodoReduccGlobal;
							fila.bloque = 0;
							fila.etapa = ALMAC_FINAL;
							char* temporalAlmFinal = string_from_format("m%dj%de%d", fila.master, fila.job, fila.etapa);
							strcpy(fila.temporal, temporalAlmFinal);
							fila.estado = EN_PROCESO;
							fila.siguiente = NULL;

							if (!agregarElemTablaEstados(fila)) {
								log_error(logYAMA, "Ocurrió un error al agregar un elemento la tabla de estados en fin de Reducción Global OK");
								perror("Error al agregar elementos a la tabla de estados");
							}
							/* ******* envío de la tabla para reducción global ****** */
							char *mensajeSerializadoAlmFinal = serializarMensajeAlmFinal(nroNodoReduccGlobal, temporalAlmFinal);
							printf("\nmensaje serializado para reducción local: %s\n", mensajeSerializadoAlmFinal);
							enviarMensaje(socketConectado, mensajeSerializadoAlmFinal);
							log_info(logYAMA, "Se da inicio al Almacenamiento Final en el nodo %d", nroNodoReduccGlobal);

							break;
						case TIPO_MSJ_REDUCC_GLOBAL_ERROR:
							;
							free(arrayMensajes);
							nroNodoReduccGlobal = getNodoReduccGlobal(masterJobActual.nroJob, masterJobActual.nroMaster, REDUCC_GLOBAL, EN_PROCESO);
							log_info(logYAMA, "Se recibió mensaje de fin de Reducción Global ERROR, nodo %d.", nroNodoReduccGlobal);
							disminuirCargaGlobalNodo(listaGlobalNodos[nroNodoReduccGlobal], 1);

							if (modificarEstadoFilasTablaEstados(masterJobActual.nroJob, masterJobActual.nroMaster, nroNodoReduccGlobal, 0, REDUCC_GLOBAL, EN_PROCESO, ERROR) == 0) {
								log_error(logYAMA, "Ocurrió un error al modificar la tabla de estados en el fin de la Reducción Global ERROR");
								puts("No se pudo modificar ninguna fila de la tabla de estados");
							} else {
								log_info(logYAMA, "Se modificó la tabla de estados en el fin de la Reducción Global ERROR");
							}
							//abortar el job
							enviarHeaderSolo(socketConectado, TIPO_MSJ_ABORTAR_JOB);
							cerrarCliente(socketConectado);
							FD_CLR(socketConectado, &socketsLecturaMaster); // remove from master set
							log_trace(logYAMA, "Se aborta el Job %d del master %d conectado por FD %d", masterJobActual.nroJob, masterJobActual.nroMaster, socketConectado);
							break;
						case TIPO_MSJ_ALM_FINAL_OK:
							;
							free(arrayMensajes);
							nroNodoAlmacFinal = getNodoReduccGlobal(masterJobActual.nroJob, masterJobActual.nroMaster, ALMAC_FINAL, EN_PROCESO);
							log_info(logYAMA, "Se recibió mensaje de fin de Almacenamiento Final OK, nodo %d.", nroNodoAlmacFinal);
							disminuirCargaGlobalNodo(listaGlobalNodos[nroNodoAlmacFinal], 1);

							//modifica el estado de la fila
							if (modificarEstadoFilasTablaEstados(masterJobActual.nroJob, masterJobActual.nroMaster, nroNodoAlmacFinal, 0, ALMAC_FINAL, EN_PROCESO, FIN_OK) == 0) {
								log_error(logYAMA, "Ocurrió un error al modificar la tabla de estados en el fin del Almacenamiento Final OK");
								puts("No se pudo modificar ninguna fila de la tabla de estados");
							} else {
								log_info(logYAMA, "Se modificó la tabla de estados en el fin del Almacenamiento Final OK");
							}
							enviarHeaderSolo(socketConectado, TIPO_MSJ_FINALIZAR_JOB);
							cerrarCliente(socketConectado); // bye!
							FD_CLR(socketConectado, &socketsLecturaMaster); // remove from master set
							log_trace(logYAMA, "Se da por finalizado correctamente el Job %d del master %d conectado por FD %d", masterJobActual.nroJob, masterJobActual.nroMaster, socketConectado);
							break;
						case TIPO_MSJ_ALM_FINAL_ERROR:
							;
							free(arrayMensajes);
							nroNodoAlmacFinal = getNodoReduccGlobal(masterJobActual.nroJob, masterJobActual.nroMaster, ALMAC_FINAL, EN_PROCESO);
							log_info(logYAMA, "Se recibió mensaje de fin de Almacenamiento Final ERROR, nodo %d.", nroNodoAlmacFinal);
							disminuirCargaGlobalNodo(listaGlobalNodos[nroNodoAlmacFinal], 1);

							//modifica el estado de la fila
							if (modificarEstadoFilasTablaEstados(masterJobActual.nroJob, masterJobActual.nroMaster, nroNodoAlmacFinal, 0, ALMAC_FINAL, EN_PROCESO, ERROR) == 0) {
								log_error(logYAMA, "Ocurrió un error al modificar la tabla de estados en el fin del Almacenamiento Final ERROR");
								puts("No se pudo modificar ninguna fila de la tabla de estados");
							} else {
								log_info(logYAMA, "Se modificó la tabla de estados en el fin del Almacenamiento Final ERROR");
							}

							//abortar el job
							enviarHeaderSolo(socketConectado, TIPO_MSJ_ABORTAR_JOB);
							cerrarCliente(socketConectado);
							FD_CLR(socketConectado, &socketsLecturaMaster); // remove from master set
							log_trace(logYAMA, "Se aborta el Job %d del master %d conectado por FD %d", masterJobActual.nroJob, masterJobActual.nroMaster, socketConectado);
							break;

						default:
							;
							free(arrayMensajes);
							break;
						}
					}
					// END handle data from client
				} //if (FD_ISSET(i, &socketsLecturaTemp)) END got new incoming connection
			} //for (nroSocket = 0; nroSocket <= maxFD; nroSocket++) END looping through file descriptors
		} else {
			log_error(logYAMA, "Ocurrió un error en el select() principal");
			perror("Error en select()");
		}
	}
// END for(;;)

//cerrarServer(listenningSocket);
//cerrarServer(socketCliente);
	log_info(logYAMA, "Server cerrado");

	log_destroy(logYAMA);
	return EXIT_SUCCESS;
}

