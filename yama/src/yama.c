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
#include "../../utils/includes.h"
#include "tablaEstados.h"
#include "planificacion.h"
#include "nroMasterJob.c"
#include "inicializacion.c"

int cantPartesArchivo, cantNodosArchivo;
#include "comunicacionesFS.c"

//para el select
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

typedef struct {
	int nroNodo;
	char ip[LARGO_IP];
	int puerto;
	int bloque;
	int bytesOcupados;
	char temporal[40];
} nodoParaAsignar;

void planificar(bloqueArchivo *bloques, datosConexionNodo *nodos, nodoParaAsignar asignacionesNodos[cantPartesArchivo]) {

	int i, j;

	//sale del archivo config?????????????
	cargaBase = 1;
	printf("cantPartesArchivo: %d\n", cantPartesArchivo);
	printf("cantNodosArchivo: %d\n", cantNodosArchivo);
	//guarda los nodos en los que está cada pedazo de archivo
	nodosPorPedazoArchivo nodosPorPedazoArchivo[cantPartesArchivo];
	//cargo los nodos en los que está cada pedazo de archivo
	//el índice es el número del pedazo de archivo
	for (i = 0; i < cantPartesArchivo; i++) {
		printf("parte archivo %d: Nodo copia1 %d - Bloque copia1 %d\n", i, bloques[i].nodoCopia1, bloques[i].bloqueCopia1);
		printf("parte archivo %d: Nodo copia2 %d - Bloque copia2 %d\n", i, bloques[i].nodoCopia2, bloques[i].bloqueCopia2);
	}
	printf("\n");
	for (i = 0; i < cantPartesArchivo; i++) {
		nodosPorPedazoArchivo[i].nodo1 = bloques[i].nodoCopia1;
		nodosPorPedazoArchivo[i].bloque1 = bloques[i].bloqueCopia1;
		nodosPorPedazoArchivo[i].nodo2 = bloques[i].nodoCopia2;
		nodosPorPedazoArchivo[i].bloque2 = bloques[i].bloqueCopia2;
		nodosPorPedazoArchivo[i].bytes = bloques[i].bytesBloque;
	}
	/*for (i = 0; i < cantPartesArchivo; i++) {
	 printf("nodosPorPedazoArchivo %d: Nodo copia1 %d - Bloque copia1 %d\n", i, nodosPorPedazoArchivo[i].nodo1, nodosPorPedazoArchivo[i].bloque1);
	 printf("nodosPorPedazoArchivo %d: Nodo copia2 %d - Bloque copia2 %d\n", i, nodosPorPedazoArchivo[i].nodo2, nodosPorPedazoArchivo[i].bloque2);
	 }
	 printf("\n");*/
	//tiene la carga de cada nodo
	cargaNodo listaCargaNodos[cantNodosArchivo];
	//pongo la carga inicial de cada nodo
	/*for (i = 0; i < cantNodosArchivo; i++) {
	 listaCargaNodos[i].carga = 0;//de dónde saco estos valores????????????????
	 listaCargaNodos[i].numero = 1;
	 }*/
	listaCargaNodos[0].carga = 0;
	listaCargaNodos[0].numero = 1;
	listaCargaNodos[1].carga = 1;
	listaCargaNodos[1].numero = 2;
	listaCargaNodos[2].carga = 1;
	listaCargaNodos[2].numero = 3;
	cargaNodo nodoMaxCarga;
	nodoMaxCarga = nodoConMayorCarga(listaCargaNodos, cantNodosArchivo);
	cargaMaxima = nodoMaxCarga.carga;
	for (i = 0; i < cantNodosArchivo; i++) {
		listaCargaNodos[i].disponibilidad = calcularDisponibilidadNodo(listaCargaNodos[i]);
	}

	//ordeno los nodos de mayor a menor disponibilidad
	cargaNodo temp;
	for (i = 0; i < cantNodosArchivo; i++) {
		for (j = 0; j < cantNodosArchivo - 1; j++) {
			if (listaCargaNodos[j].disponibilidad < listaCargaNodos[j + 1].disponibilidad) {
				temp = listaCargaNodos[j];
				listaCargaNodos[j] = listaCargaNodos[j + 1];
				listaCargaNodos[j + 1] = temp;
			}
		}
	}

	int parteArchivo = 0;
	int clockMaestro = 0, clockNoExisteParteArchivo = -1,
			clockNodoSinDisponibilidad = -1;

	while (parteArchivo < cantPartesArchivo) {
		cargaNodo nodoActual = listaCargaNodos[clockMaestro];
		//printf("\nnodo actual: %d, parte de archivo: %d\n", nodoActual.numero, parteArchivo);
		//printf("disponibilidad del nodo: %d\n", nodoConDisponibilidad(nodoActual));
		//printf("existe la parte del archivo en el nodo: %d\n", existeParteArchivoEnNodo(parteArchivo, nodoActual.numero, nodosPorPedazoArchivo));
		if (nodoConDisponibilidad(nodoActual) && existeParteArchivoEnNodo(parteArchivo, nodoActual.numero, nodosPorPedazoArchivo)) {
			//asigno parteArchivo al nodo y bloque
			asignacionesNodos[parteArchivo].nroNodo = nodoActual.numero;
			//me fijo si es el nodo de la copia 1 o 2, para cargarle el bloque dentro del nodo
			if (nodoActual.numero == nodosPorPedazoArchivo[parteArchivo].nodo1)
				asignacionesNodos[parteArchivo].bloque = nodosPorPedazoArchivo[parteArchivo].bloque1;
			else
				asignacionesNodos[parteArchivo].bloque = nodosPorPedazoArchivo[parteArchivo].bloque2;
			asignacionesNodos[parteArchivo].bytesOcupados = nodosPorPedazoArchivo[parteArchivo].bytes;
			for (j = 0; j < cantNodosArchivo; j++) {//recorro nodosArchivo para buscar la ip y el puerto
				//nodosArchivo tiene el número, ip y puerto de cada nodo
				if (nodoActual.numero == nodos[j].numero) {
					strcpy(asignacionesNodos[parteArchivo].ip, nodos[j].ip);
					asignacionesNodos[parteArchivo].puerto = nodos[j].puerto;
					break;
				}
			}

			nodoActual.carga++;
			nodoActual.disponibilidad--;

			if (clockNoExisteParteArchivo < 0 || clockMaestro == clockNoExisteParteArchivo) {
				clockMaestro++;
				if (clockMaestro >= cantNodosArchivo)
					clockMaestro = 0;
			} else {
				clockMaestro = clockNoExisteParteArchivo;
			}
			clockNoExisteParteArchivo = -1;
			clockNodoSinDisponibilidad = -1;	//????? A1
			printf("\nasignó partearchivo %d al nodo %d - bloque %d\n", parteArchivo, asignacionesNodos[parteArchivo].nroNodo, asignacionesNodos[parteArchivo].bloque);
			parteArchivo++;

		} else if (!nodoConDisponibilidad(nodoActual)) { //el nodo no tiene disponibilidad
			if (clockNodoSinDisponibilidad < 0)	//????? A1
				clockNodoSinDisponibilidad = clockMaestro;
			nodoActual.disponibilidad += cargaBase;
			clockMaestro++;
			if (clockMaestro >= cantNodosArchivo)
				clockMaestro = 0;
			//	printf("\nno disponibilidad partearchivo %d en el nodo %d\n", parteArchivo, nodoActual.numero);
		} else if (!existeParteArchivoEnNodo(parteArchivo, nodoActual.numero, nodosPorPedazoArchivo)) { //no se encuentra el bloque en el nodo
			clockNoExisteParteArchivo = clockMaestro;
			clockMaestro++;
			if (clockMaestro >= cantNodosArchivo)
				clockMaestro = 0;
			//	printf("\nno existe parte de archivo %d en nodo %d\n", parteArchivo, nodoActual.numero);
			//	printf("clock maestro: %d\n", clockMaestro);
		}
		if (clockNoExisteParteArchivo == clockMaestro)
			listaCargaNodos[clockNoExisteParteArchivo].disponibilidad += cargaBase;
	}
	puts("pasó");
	return;
}

char* serializarNodosTransformacion(nodoParaAsignar datosParaTransformacion[cantPartesArchivo]) {
	int i, j, k, cantStringsASerializar, largoStringDestinoCopia;

	cantStringsASerializar = (cantPartesArchivo * 6) + 1;
	char **arrayMensajes = malloc(sizeof(char*) * cantStringsASerializar);

	largoStringDestinoCopia = 4 + 1;
	arrayMensajes[0] = malloc(largoStringDestinoCopia);
	strcpy(arrayMensajes[0], intToArrayZerosLeft(cantPartesArchivo, 4));
	j = 1;
	for (i = 0; i < cantPartesArchivo; i++) {
		printf("\nnodo %d - ip %s - puerto %d - bloque %d - bytes %d - temporal %s\n", datosParaTransformacion[i].nroNodo, datosParaTransformacion[i].ip, datosParaTransformacion[i].puerto, datosParaTransformacion[i].bloque, datosParaTransformacion[i].bytesOcupados, datosParaTransformacion[i].temporal);

		//número de nodo
		largoStringDestinoCopia = 4 + 1;
		arrayMensajes[j] = malloc(largoStringDestinoCopia);
		strcpy(arrayMensajes[j], intToArrayZerosLeft(datosParaTransformacion[i].nroNodo, 4));
		j++;

		//IP
		largoStringDestinoCopia = string_length(datosParaTransformacion[i].ip) + 1;
		arrayMensajes[j] = malloc(largoStringDestinoCopia);
		strcpy(arrayMensajes[j], datosParaTransformacion[i].ip);
		j++;

		//puerto
		largoStringDestinoCopia = 4 + 1;
		arrayMensajes[j] = malloc(largoStringDestinoCopia);
		strcpy(arrayMensajes[j], intToArrayZerosLeft(datosParaTransformacion[i].puerto, 4));
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

	char *mensajeSerializado = serializarMensaje(TIPO_MSJ_TABLA_TRANSFORMACION, arrayMensajes, cantStringsASerializar);
	return mensajeSerializado;

}

int main(int argc, char *argv[]) {
	logYAMA = log_create("logYAMA.log", "YAMA", false, LOG_LEVEL_TRACE); //creo el logger, sin mostrar por pantalla
	int preparadoEnviarFs = 1, i, j, k;
	char mensajeHeaderSolo[4];
	log_info(logYAMA, "Iniciando proceso YAMA");
	printf("\n*** Proceso Yama ***\n");
	//para el select
	fd_set socketsLecturaMaster;    // master file descriptor list
	fd_set socketsLecturaTemp;  // temp file descriptor list for select()
	int maxFD;        // maximum file descriptor number

// 1º) leer archivo de config.
	if (!getDatosConfiguracion()) {
		return EXIT_FAILURE;
	}
	/* ************** conexión como cliente al FS *************** */
	int socketFS;
	if ((socketFS = conexionAFileSystem()) < 0) {
		preparadoEnviarFs = 0;
	}
	/* ************** inicialización como server ************ */
	int listenningSocket;
	if ((listenningSocket = inicializoComoServidor()) < 0) {
		return EXIT_FAILURE;
	}
	// clear the set ahead of time
	FD_ZERO(&socketsLecturaMaster);
	FD_ZERO(&socketsLecturaTemp);
	// add the listener to the master set
	FD_SET(listenningSocket, &socketsLecturaMaster);
	// keep track of the biggest file descriptor
	maxFD = listenningSocket; // so far, it's this one
	/* *********** crea la lista para la tabla de estados ********************* */
	//t_list * listaTablaEstados = list_create();
	int socketCliente, socketConectado, cantStrings, bytesRecibidos = 0,
			nroSocket, nroNodoReduccGlobal, nroNodoRecibido;
	nroMasterJob masterJobActual;
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
									strcpy(mensajeHeaderSolo, intToArrayZerosLeft(TIPO_MSJ_HANDSHAKE_RESPUESTA_OK, 4));
								} else {
									strcpy(mensajeHeaderSolo, intToArrayZerosLeft(TIPO_MSJ_HANDSHAKE_RESPUESTA_DENEGADO, 4));
								}
								enviarMensaje(socketCliente, mensajeHeaderSolo);
							}
						}
					} else {	//conexión preexistente
						/* *************************** recepción de un mensaje ****************************/
						socketConectado = nroSocket;
						masterJobActual = getNroMasterJobByFD(socketConectado);
						printf("socketConectado: %d\n", socketConectado);
						printf("master actual: %d\n", masterJobActual.nroMaster);
						printf("job actual: %d\n", masterJobActual.nroJob);
						int32_t headerId = deserializarHeader(socketConectado);
						if (headerId == -1) {//error o desconexión de un cliente
							close(socketConectado); // bye!
							FD_CLR(socketConectado, &socketsLecturaMaster); // remove from master set
						}
						int cantidadMensajes = protocoloCantidadMensajes[headerId];
						char **arrayMensajes = deserializarMensaje(socketConectado, cantidadMensajes);
						switch (headerId) {

						case TIPO_MSJ_TRANSFORMACION_OK:
							;

							nroNodoRecibido = atoi(arrayMensajes[0]);
							free(arrayMensajes);
							if (modificarEstadoFilasTablaEstados(masterJobActual.nroJob, masterJobActual.nroMaster, nroNodoRecibido, TRANSFORMACION, EN_PROCESO, FIN_OK) == 0) {
								puts("No se pudo modificar ninguna fila de la tabla de estados");
							}

							break;
						case TIPO_MSJ_TRANSFORMACION_ERROR:
							;
							nroNodoRecibido = atoi(arrayMensajes[0]);
							free(arrayMensajes);
							if (modificarEstadoFilasTablaEstados(masterJobActual.nroJob, masterJobActual.nroMaster, nroNodoRecibido, TRANSFORMACION, EN_PROCESO, ERROR) == 0) {
								puts("No se pudo modificar ninguna fila de la tabla de estados");
							}
							break;
						case TIPO_MSJ_REDUCC_LOCAL_OK:
							;
							nroNodoRecibido = atoi(arrayMensajes[0]);
							free(arrayMensajes);
							if (modificarEstadoFilasTablaEstados(masterJobActual.nroJob, masterJobActual.nroMaster, nroNodoRecibido, REDUCC_LOCAL, EN_PROCESO, FIN_OK) == 0) {
								puts("No se pudo modificar ninguna fila de la tabla de estados");
							}
							break;
						case TIPO_MSJ_REDUCC_LOCAL_ERROR:
							;
							nroNodoRecibido = atoi(arrayMensajes[0]);
							free(arrayMensajes);
							if (modificarEstadoFilasTablaEstados(masterJobActual.nroJob, masterJobActual.nroMaster, nroNodoRecibido, REDUCC_LOCAL, EN_PROCESO, ERROR) == 0) {
								puts("No se pudo modificar ninguna fila de la tabla de estados");
							}
							break;
						case TIPO_MSJ_REDUCC_GLOBAL_OK:
							;
							free(arrayMensajes);
							nroNodoReduccGlobal = 1;
							if (modificarEstadoFilasTablaEstados(masterJobActual.nroJob, masterJobActual.nroMaster, nroNodoReduccGlobal, REDUCC_GLOBAL, EN_PROCESO, FIN_OK) == 0) {
								puts("No se pudo modificar ninguna fila de la tabla de estados");
							}
							break;
						case TIPO_MSJ_REDUCC_GLOBAL_ERROR:
							;
							free(arrayMensajes);
							nroNodoReduccGlobal = 1;
							if (modificarEstadoFilasTablaEstados(masterJobActual.nroJob, masterJobActual.nroMaster, nroNodoReduccGlobal, REDUCC_GLOBAL, EN_PROCESO, ERROR) == 0) {
								puts("No se pudo modificar ninguna fila de la tabla de estados");
							}
							break;
						case TIPO_MSJ_ALM_FINAL_OK:
							;
							free(arrayMensajes);
							break;
						case TIPO_MSJ_ALM_FINAL_ERROR:
							;
							free(arrayMensajes);
							break;
						case TIPO_MSJ_PATH_ARCHIVO_TRANSFORMAR:
							;
							char *archivo = malloc(string_length(arrayMensajes[0]) + 1);
							strcpy(archivo, arrayMensajes[0]);
							free(arrayMensajes);
							//pide la metadata del archivo al FS
							if (preparadoEnviarFs) {
								if (pedirMetadataArchivoFS(socketFS, archivo)) {

									/* ************* solicitud de info del archivo al FS *************** */
									bloqueArchivo *bloques = recibirMetadataArchivoFS(socketFS);
									//enviarHeaderSolo(socketFS, TIPO_MSJ_OK);
									datosConexionNodo *nodosArchivo = recibirNodosArchivoFS(socketFS);
									//enviarHeaderSolo(socketFS, TIPO_MSJ_OK);
									/* ***************************************************************** */

									/* ************* inicio planificación *************** */
									//le paso el vector donde debe ir guardando las asignaciones de nodos planificados
									//indexado por partes del archivo
									nodoParaAsignar asignacionesNodos[cantPartesArchivo];
									planificar(bloques, nodosArchivo, asignacionesNodos);
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
										if (!agregarElemTablaEstados(fila))
											perror("Error al agregar elementos a la tabla de estados");

										//guarda el archivo temporal en el vector que se va a usar
										//en la tabla de transformación para el master
										strcpy(asignacionesNodos[i].temporal, temporal);
									}
									//puts("\nlista de elementos asignados a transformación");
									//mostrarTablaEstados();
									/* ************** fin agregado en tabla de estados *************** */

									/* ****** envío de nodos para la transformación ******************* */
									//envía al master la lista de nodos donde trabajar cada bloque
									char *mensajeSerializado = serializarNodosTransformacion(asignacionesNodos);
									printf("\nmensaje serializado: \n%s\n", mensajeSerializado);
									if (!enviarMensaje(socketConectado, mensajeSerializado)) {
										return 0;
									}
									/* **************************************************************** */

								} else {
									perror("No se pudo pedir el archivo al FS");
								}
							}
							break;
						default:
							;
							free(arrayMensajes);
							break;
						}
					} // END handle data from client
				} //if (FD_ISSET(i, &socketsLecturaTemp)) END got new incoming connection
			} //for (nroSocket = 0; nroSocket <= maxFD; nroSocket++) END looping through file descriptors
		} else {
			perror("Error en select()");
		}
	} // END for(;;)

//cerrarServer(listenningSocket);
//cerrarServer(socketCliente);
	log_info(logYAMA, "Server cerrado");

	log_destroy(logYAMA);
	return EXIT_SUCCESS;
}

