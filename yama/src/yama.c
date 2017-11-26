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
#define LARGO_IP 16
#define LARGO_PUERTO 4
typedef struct {
	int numero;
	char ip[LARGO_IP];
	int puerto;
	int carga;
	int disponibilidad;
} datosPropiosNodo;
datosPropiosNodo listaGlobalNodos[50];

#define LARGO_TEMPORAL 40
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

int cantPartesArchivo, cantNodosArchivo;

#include "../../utils/includes.h"
#include "tablaEstados.h"
#include "planificacion.h"
#include "nroMasterJob.c"
#include "inicializacion.c"
#include "comunicacionesFS.c"

//para el select
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

char* serializarNodosTransformacion(nodoParaAsignar datosParaTransformacion[cantPartesArchivo]) {
	int i, j, k, cantStringsASerializar, largoStringDestinoCopia;

	cantStringsASerializar = (cantPartesArchivo * 6) + 1;
	char **arrayMensajes = malloc(sizeof(char*) * cantStringsASerializar);

	largoStringDestinoCopia = 4 + 1;
	arrayMensajes[0] = malloc(largoStringDestinoCopia);
	strcpy(arrayMensajes[0], intToArrayZerosLeft(cantPartesArchivo, 4));
	j = 1;
	for (i = 0; i < cantPartesArchivo; i++) {
		printf("\nnodo %d - ip %s - puerto %d - bloque %d - bytes %d - temporal %s\n", datosParaTransformacion[i].nroNodo, listaGlobalNodos[datosParaTransformacion[i].nroNodo].ip, listaGlobalNodos[datosParaTransformacion[i].nroNodo].puerto, datosParaTransformacion[i].bloque, datosParaTransformacion[i].bytesOcupados, datosParaTransformacion[i].temporal);

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

	char *mensajeSerializado = serializarMensaje(TIPO_MSJ_TABLA_TRANSFORMACION, arrayMensajes, cantStringsASerializar);
	for (j = 0; j < cantStringsASerializar; j++) {
		free(arrayMensajes[j]);
	}
	free(arrayMensajes);
	return mensajeSerializado;

}

char* serializarMensajeReduccLocal(int nroNodoRecibido, nroMasterJob masterJobActual, char *temporalRedLocal) {
	int j, k, h;
	int cantidadTemporales = getCantFilasByJMNEtEs(masterJobActual.nroJob, masterJobActual.nroMaster, nroNodoRecibido, TRANSFORMACION, FIN_OK);
	char **temporales = malloc(sizeof(char*) * cantidadTemporales);
	getAllTemporalesByJMNEtEs(temporales, masterJobActual.nroJob, masterJobActual.nroMaster, nroNodoRecibido, TRANSFORMACION, FIN_OK);

	//armar el string serializado
	int cantNodosReduccLocal = 1;
	int cantStrings = 1 + cantNodosReduccLocal * (4 + cantidadTemporales) + 1;
	char **arrayMensajesSerializarRedLocal = malloc(sizeof(char*) * cantStrings);

	//cantidad de nodos
	j = 0;
	arrayMensajesSerializarRedLocal[j] = malloc(4 + 1);
	if (!arrayMensajesSerializarRedLocal[j])
		perror("error de malloc");
	strcpy(arrayMensajesSerializarRedLocal[j], intToArrayZerosLeft(cantNodosReduccLocal, 4));
	j++;

	//para cada nodo
	for (k = 0; k < cantNodosReduccLocal; k++) {
		//nro de nodo
		arrayMensajesSerializarRedLocal[j] = malloc(4 + 1);
		if (!arrayMensajesSerializarRedLocal[j])
			perror("error de malloc");
		strcpy(arrayMensajesSerializarRedLocal[j], intToArrayZerosLeft(nroNodoRecibido, 4));
		j++;
		//IP del nodo
		arrayMensajesSerializarRedLocal[j] = malloc(string_length(listaGlobalNodos[nroNodoRecibido].ip) + 1);
		if (!arrayMensajesSerializarRedLocal[j])
			perror("error de malloc");
		strcpy(arrayMensajesSerializarRedLocal[j], listaGlobalNodos[nroNodoRecibido].ip);
		j++;
		//puerto del nodo
		arrayMensajesSerializarRedLocal[j] = malloc(LARGO_PUERTO + 1);
		if (!arrayMensajesSerializarRedLocal[j])
			perror("error de malloc");
		strcpy(arrayMensajesSerializarRedLocal[j], intToArrayZerosLeft(listaGlobalNodos[nroNodoRecibido].puerto, LARGO_PUERTO));
		j++;
		//cantidad de temporales
		arrayMensajesSerializarRedLocal[j] = malloc(4 + 1);
		if (!arrayMensajesSerializarRedLocal[j])
			perror("error de malloc");
		strcpy(arrayMensajesSerializarRedLocal[j], intToArrayZerosLeft(cantidadTemporales, 4));
		j++;
		//todos los temporales uno a continuación del otro
		for (h = 0; h < cantidadTemporales; h++) {
			//temporal h
			printf("temporal %s\n", temporales[h]);
			arrayMensajesSerializarRedLocal[j] = malloc(string_length(temporales[h]) + 1);
			if (!arrayMensajesSerializarRedLocal[j])
				perror("error de malloc");
			strcpy(arrayMensajesSerializarRedLocal[j], temporales[h]);
			j++;
		}
	}
	arrayMensajesSerializarRedLocal[j] = malloc(string_length(temporalRedLocal) + 1);
	if (!arrayMensajesSerializarRedLocal[j])
		perror("error de malloc");
	strcpy(arrayMensajesSerializarRedLocal[j], temporalRedLocal);
	j++;

	///////////////mensaje serializado
	char *mensajeSerializadoRedLocal = serializarMensaje(TIPO_MSJ_TABLA_REDUCCION_LOCAL, arrayMensajesSerializarRedLocal, cantStrings);
	for (j = 0; j < cantStrings; j++) {
		free(arrayMensajesSerializarRedLocal[j]);
	}
	free(arrayMensajesSerializarRedLocal);

	for (h = 0; h < cantidadTemporales; h++) {
		free(temporales[j]); //TODO: revisar que no rompa
	}
	free(temporales);
	return mensajeSerializadoRedLocal;
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
	int preparadoEnviarFs = 1, h, i, j, k;
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
			nroSocket, nroNodoReduccGlobal, nroNodoRecibido, nroBloqueRecibido;
	nroMasterJob masterJobActual;
	for (;;) {
		socketsLecturaTemp = socketsLecturaMaster;
		getchar();
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
							cerrarCliente(socketConectado); // bye!
							FD_CLR(socketConectado, &socketsLecturaMaster); // remove from master set
						}
						int cantidadMensajes = protocoloCantidadMensajes[headerId];
						char **arrayMensajes = deserializarMensaje(socketConectado, cantidadMensajes);
						switch (headerId) {

						case TIPO_MSJ_TRANSFORMACION_OK:
							;
							nroNodoRecibido = atoi(arrayMensajes[0]);
							nroBloqueRecibido = atoi(arrayMensajes[1]);
							printf("\nnodo recibido: %d\n", nroNodoRecibido);
							printf("bloque recibido: %d\n", nroBloqueRecibido);
							free(arrayMensajes);

							//pongo la fila en estado FIN_OK
							if (modificarEstadoFilasTablaEstados(masterJobActual.nroJob, masterJobActual.nroMaster, nroNodoRecibido, nroBloqueRecibido, TRANSFORMACION, EN_PROCESO, FIN_OK) == 0) {
								puts("No se pudo modificar ninguna fila de la tabla de estados");
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
								if (!agregarElemTablaEstados(fila))
									perror("Error al agregar elementos a la tabla de estados");

								/* *********** enviar data para reducción local ********** */
								//obtener los registros de la tabla de estados, cantidad y temporales
								char *mensajeSerializadoRedLocal = serializarMensajeReduccLocal(nroNodoRecibido, masterJobActual, temporalRedLocal);
								printf("\nmensaje serializado para reducción local: %s\n", mensajeSerializadoRedLocal);
								enviarMensaje(socketConectado, mensajeSerializadoRedLocal);
							}

							//puts("\nlista de elementos asignados a transformación");
							//mostrarTablaEstados();

							break;
						case TIPO_MSJ_TRANSFORMACION_ERROR:
							;
							nroNodoRecibido = atoi(arrayMensajes[0]);
							free(arrayMensajes);
							if (modificarEstadoFilasTablaEstados(masterJobActual.nroJob, masterJobActual.nroMaster, nroNodoRecibido, nroBloqueRecibido, TRANSFORMACION, EN_PROCESO, ERROR) == 0) {
								puts("No se pudo modificar ninguna fila de la tabla de estados");
							}
							break;
						case TIPO_MSJ_REDUCC_LOCAL_OK:
							;
							nroNodoRecibido = atoi(arrayMensajes[0]);
							free(arrayMensajes);
							//modificar el estado en la tabla de estados
							if (modificarEstadoFilasTablaEstados(masterJobActual.nroJob, masterJobActual.nroMaster, nroNodoRecibido, 0, REDUCC_LOCAL, EN_PROCESO, FIN_OK) == 0) {
								puts("No se pudo modificar ninguna fila de la tabla de estados");
							}

							//verificar que todos los nodos del job y master hayan terminado la reducción local
							if (getCantFilasByJMEtEs(masterJobActual.nroJob, masterJobActual.nroMaster, REDUCC_LOCAL, EN_PROCESO) == 0) {
								int nodoReduccGlobal = 2;//TODO: definir este nodo

								/* ************** agregado de la fila de reducción global en tabla de estados *************** */
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
								if (!agregarElemTablaEstados(fila))
									perror("Error al agregar elementos a la tabla de estados");

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
								}

								/* ******* envío de la tabla para reducción global ****** */
								char *mensajeSerializadoRedGlobal = serializarMensajeReduccGlobal(cantNodosReduccGlobal, filasReduccGlobal, temporalRedGlobal);
								printf("\nmensaje serializado para reducción local: %s\n", mensajeSerializadoRedGlobal);
								enviarMensaje(socketConectado, mensajeSerializadoRedGlobal);
							}
							break;
						case TIPO_MSJ_REDUCC_LOCAL_ERROR:
							;
							nroNodoRecibido = atoi(arrayMensajes[0]);
							free(arrayMensajes);
							if (modificarEstadoFilasTablaEstados(masterJobActual.nroJob, masterJobActual.nroMaster, nroNodoRecibido, 0, REDUCC_LOCAL, EN_PROCESO, ERROR) == 0) {
								puts("No se pudo modificar ninguna fila de la tabla de estados");
							}
							break;
						case TIPO_MSJ_REDUCC_GLOBAL_OK:
							;
							free(arrayMensajes);
							//obtengo el nodo donde se hizo la reducción global
							int nroNodoReduccGlobal = getNodoReduccGlobal(masterJobActual.nroJob, masterJobActual.nroMaster, REDUCC_GLOBAL, EN_PROCESO);
							//modifica el estado de la fila
							if (modificarEstadoFilasTablaEstados(masterJobActual.nroJob, masterJobActual.nroMaster, nroNodoReduccGlobal, 0, REDUCC_GLOBAL, EN_PROCESO, FIN_OK) == 0) {
								puts("No se pudo modificar ninguna fila de la tabla de estados");
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
							if (!agregarElemTablaEstados(fila))
								perror("Error al agregar elementos a la tabla de estados");

							/* ******* envío de la tabla para reducción global ****** */
							char *mensajeSerializadoAlmFinal = serializarMensajeAlmFinal(nroNodoReduccGlobal, temporalAlmFinal);
							printf("\nmensaje serializado para reducción local: %s\n", mensajeSerializadoAlmFinal);
							enviarMensaje(socketConectado, mensajeSerializadoAlmFinal);

							break;
						case TIPO_MSJ_REDUCC_GLOBAL_ERROR:
							;
							free(arrayMensajes);
							nroNodoReduccGlobal = 1;
							if (modificarEstadoFilasTablaEstados(masterJobActual.nroJob, masterJobActual.nroMaster, nroNodoReduccGlobal, 0, REDUCC_GLOBAL, EN_PROCESO, ERROR) == 0) {
								puts("No se pudo modificar ninguna fila de la tabla de estados");
							}
							break;
						case TIPO_MSJ_ALM_FINAL_OK:
							;
							free(arrayMensajes);
							int nroNodoAlmacFinal = getNodoReduccGlobal(masterJobActual.nroJob, masterJobActual.nroMaster, ALMAC_FINAL, EN_PROCESO);
							//modifica el estado de la fila
							if (modificarEstadoFilasTablaEstados(masterJobActual.nroJob, masterJobActual.nroMaster, nroNodoReduccGlobal, 0, ALMAC_FINAL, EN_PROCESO, FIN_OK) == 0) {
								puts("No se pudo modificar ninguna fila de la tabla de estados");
							}
							cerrarCliente(socketConectado); // bye!
							FD_CLR(socketConectado, &socketsLecturaMaster); // remove from master set
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
							preparadoEnviarFs = 1;
							if (preparadoEnviarFs) {
								if (pedirMetadataArchivoFS(socketFS, archivo) > 0) {
									/* ************* solicitud de info del archivo al FS *************** */
									bloqueArchivo *bloques = recibirMetadataArchivoFS(socketFS);
									//enviarHeaderSolo(socketFS, TIPO_MSJ_OK);
									recibirNodosArchivoFS(socketFS);
									//enviarHeaderSolo(socketFS, TIPO_MSJ_OK);
									/* ***************************************************************** */

									/* ************* inicio planificación *************** */
									//le paso el vector donde debe ir guardando las asignaciones de nodos planificados
									//indexado por partes del archivo
									nodoParaAsignar asignacionesNodos[cantPartesArchivo];
									planificar(bloques, asignacionesNodos);
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
									enviarMensaje(socketConectado, mensajeSerializado);
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
					}
					// END handle data from client
				} //if (FD_ISSET(i, &socketsLecturaTemp)) END got new incoming connection
			} //for (nroSocket = 0; nroSocket <= maxFD; nroSocket++) END looping through file descriptors
		} else {
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

