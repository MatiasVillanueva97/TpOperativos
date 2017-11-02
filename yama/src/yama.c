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

//para el select
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

// ================================================================ //
// enum y vectores para los datos de configuración levantados del archivo config
// ================================================================ //
enum keys {
	IP_PROPIA, PUERTO_PROPIO, FS_IP, FS_PUERTO
};
char* keysConfigYama[] = { "IP_PROPIA", "PUERTO_PROPIO", "FS_IP", "FS_PUERTO",
NULL };
char* datosConfigYama[4];

// ================================================================ //
// tabla de estados de YAMA
// ================================================================ //
enum etapasTablaEstados {
	TRANSFORMACION, REDUCC_LOCAL, REDUCC_GLOBAL
};
enum estadoTablaEstados {
	EN_PROCESO, ERROR, FIN_OK
};

typedef struct {
	int nodoCopia1;
	int bloqueCopia1;
	int nodoCopia2;
	int bloqueCopia2;
	int bytesBloque;
} bloqueArchivo;

int cantBloquesArchivo, cantNodosArchivo;

typedef struct {
	int numero;
	char ip[12];
	int puerto;
} datosConexionNodo;

#define CANT_MENSAJES_POR_BLOQUE_DE_ARCHIVO 5
#define CANT_MENSAJES_POR_NODO 3

int getDatosConfiguracion() {
	char *nameArchivoConfig = "configYama.txt";
	if (leerArchivoConfig(nameArchivoConfig, keysConfigYama, datosConfigYama)) { //leerArchivoConfig devuelve 1 si hay error
		printf("Hubo un error al leer el archivo de configuración");
		return 0;
	}
	return 1;
}

int conexionAFileSystem(t_log* logYAMA) {
	log_info(logYAMA, "Conexión a FileSystem, IP: %s, Puerto: %s", datosConfigYama[FS_IP], datosConfigYama[FS_PUERTO]);
	int socketFS = conectarA(datosConfigYama[FS_IP], datosConfigYama[FS_PUERTO]);
	if (socketFS < 0) {
		puts("Filesystem not ready\n");
		//preparadoEnviarFs = handshakeClient(&datosConexionFileSystem, NUM_PROCESO_KERNEL);
	}
	return socketFS;
}

int inicializoComoServidor(t_log* logYAMA) {
	int listenningSocket = inicializarServer(datosConfigYama[IP_PROPIA], datosConfigYama[PUERTO_PROPIO]);
	if (listenningSocket < 0) {
		log_error(logYAMA, "No pude iniciar como servidor");
		puts("No pude iniciar como servidor");
	}
	return listenningSocket;
}

int recibirConexion(t_log* logYAMA, int listenningSocket) {
	int socketCliente = aceptarConexion(listenningSocket);
	if (socketCliente < 0) {
		log_error(logYAMA, "Hubo un error al aceptar conexiones");
		puts("Hubo un error al aceptar conexiones\n");
	} else {
		log_info(logYAMA, "Cliente conectado, esperando mensajes");
		puts("Esperando mensajes\n");
	}
	return socketCliente;
}

int pedirMetadataArchivoFS(int socketFS, char *archivo) {
	int cantStrings = 1, i;
	char **arrayMensajesSerializar = malloc(cantStrings);
	arrayMensajesSerializar[0] = malloc(string_length(archivo) + 1);
	strcpy(arrayMensajesSerializar[0], archivo);
	arrayMensajesSerializar[0][string_length(archivo)] = '\0';

	char *mensajeSerializado = serializarMensaje(TIPO_MSJ_PEDIDO_METADATA_ARCHIVO, arrayMensajesSerializar, cantStrings);
	for (i = 0; i < cantStrings; i++) {
		free(arrayMensajesSerializar[i]);
	}
	free(arrayMensajesSerializar);
	return enviarMensaje(socketFS, mensajeSerializado);
}

bloqueArchivo* recibirMetadataArchivoFS(int socketFS) {
	int i;
	uint32_t headerId = deserializarHeader(socketFS);
	if (headerId != TIPO_MSJ_METADATA_ARCHIVO) {
		perror("El FS no mandó lo solicitado");
		bloqueArchivo *bloquesError = malloc(sizeof(bloqueArchivo));
		bloquesError[0].nodoCopia1 = 0;
		bloquesError[0].bloqueCopia1 = 0;
		bloquesError[0].nodoCopia2 = 0;
		bloquesError[0].bloqueCopia2 = 0;
		bloquesError[0].bytesBloque = 0;
		return bloquesError;
	}
	int cantidadMensajes = protocoloCantidadMensajes[headerId];
	char **arrayMensajes = deserializarMensaje(socketFS, cantidadMensajes);
	//guardar la data en algún lado
	cantBloquesArchivo = atoi(arrayMensajes[0]);
	arrayMensajes = deserializarMensaje(socketFS, cantBloquesArchivo * CANT_MENSAJES_POR_BLOQUE_DE_ARCHIVO);
	bloqueArchivo *bloques = malloc(cantBloquesArchivo * sizeof(bloqueArchivo));
	int j = 0;
	for (i = 0; i < cantBloquesArchivo; i++) {
		bloques[i].nodoCopia1 = atoi(arrayMensajes[j]);
		j++;
		bloques[i].bloqueCopia1 = atoi(arrayMensajes[j]);
		j++;
		bloques[i].nodoCopia2 = atoi(arrayMensajes[j]);
		j++;
		bloques[i].bloqueCopia2 = atoi(arrayMensajes[j]);
		j++;
		bloques[i].bytesBloque = atoi(arrayMensajes[j]);

		j++;
	}
	return bloques;
}

datosConexionNodo * recibirNodosArchivoFS( socketFS) {
	int i;
	uint32_t headerId = deserializarHeader(socketFS);
	if (headerId != TIPO_MSJ_DATOS_CONEXION_NODOS) {
		perror("El FS no mandó lo solicitado");
		datosConexionNodo *nodosError = malloc(sizeof(bloqueArchivo));
		nodosError[0].numero = 0;
		strcpy(nodosError[0].ip, "");
		nodosError[0].puerto = 0;
		return nodosError;
	}
	int cantidadMensajes = protocoloCantidadMensajes[headerId];
	char **arrayMensajes = deserializarMensaje(socketFS, cantidadMensajes);
	//guardar la data en algún lado
	cantNodosArchivo = atoi(arrayMensajes[0]);
	arrayMensajes = deserializarMensaje(socketFS, cantNodosArchivo * CANT_MENSAJES_POR_NODO);
	datosConexionNodo *nodos = malloc(cantNodosArchivo * sizeof(datosConexionNodo));
	int j = 0;
	for (i = 0; i < cantBloquesArchivo; i++) {
		nodos[i].numero = atoi(arrayMensajes[j]);
		j++;
		strcpy(nodos[i].ip, arrayMensajes[j]);
		j++;
		nodos[i].puerto = atoi(arrayMensajes[j]);
		j++;
	}
	return nodos;
}

void planificar(bloqueArchivo *bloques, datosConexionNodo *nodos) {
	/*printf("Bloque 1 - Copia 1: Nodo %d - Bloque %d\n", bloques[0].nodoCopia1, bloques[0].bloqueCopia1);
	 printf("Bloque 1 - Copia 2: Nodo %d - Bloque %d\n", bloques[0].nodoCopia2, bloques[0].bloqueCopia2);
	 printf("Bloque 2 - Copia 1: Nodo %d - Bloque %d\n", bloques[1].nodoCopia1, bloques[1].bloqueCopia1);
	 printf("Bloque 2 - Copia 2: Nodo %d - Bloque %d\n", bloques[1].nodoCopia2, bloques[1].bloqueCopia2);*/

	int i, j;

	//sale del archivo config?????????????
	cargaBase = 1;

	//guarda los nodos en los que está cada bloque
	nodosPorBloque nodosPorBloque[cantBloquesArchivo];
	//cargo los nodos en los que está cada bloque
	//el índice es el número del bloque
	for (i = 0; i < cantBloquesArchivo; i++) {
		nodosPorBloque[i].nodo1 = bloques[i].nodoCopia1;
		nodosPorBloque[i].bloque1 = bloques[i].bloqueCopia1;
		nodosPorBloque[i].nodo2 = bloques[i].nodoCopia2;
		nodosPorBloque[i].bloque2 = bloques[i].bloqueCopia2;
	}

	//tiene la carga de cada nodo
	cargaNodo listaNodos[cantNodosArchivo];
	//pongo la carga inicial de cada nodo
	for (i = 0; i < cantNodosArchivo; i++) {
		listaNodos[i].carga = 0;//de dónde saco estos valores????????????????
		listaNodos[i].numero = 1;
	}
	/*listaNodos[0].carga = 0;
	 listaNodos[0].numero = 1;
	 listaNodos[1].carga = 1;
	 listaNodos[1].numero = 2;
	 listaNodos[2].carga = 1;
	 listaNodos[2].numero = 3;*/
	cargaNodo nodoMaxCarga;
	nodoMaxCarga = nodoConMayorCarga(listaNodos, cantNodosArchivo);
	cargaMaxima = nodoMaxCarga.carga;
	for (i = 0; i < cantNodosArchivo; i++) {
		listaNodos[i].disponibilidad = calcularDisponibilidadNodo(listaNodos[i]);
	}

	//ordeno los nodos de mayor a menor disponibilidad
	cargaNodo temp;
	for (i = 0; i < cantNodosArchivo; i++) {
		for (j = 0; j < cantNodosArchivo - 1; j++) {
			if (listaNodos[j].disponibilidad < listaNodos[j + 1].disponibilidad) {
				temp = listaNodos[j];
				listaNodos[j] = listaNodos[j + 1];
				listaNodos[j + 1] = temp;
			}
		}
	}

	int bloque = 0;
	int clockMaestro = 0, clockNoExisteBloque = -1,
			clockNodoDisponibilidad = -1;
	//indexado por bloques, contiene el nodo al cual fue asignado el bloque
	int asignacionsBloquesNodos[cantBloquesArchivo][2];
	while (bloque < cantBloquesArchivo) {
		cargaNodo nodoActual = listaNodos[clockMaestro];
		if (nodoConDisponibilidad(nodoActual) && existeBloqueEnNodo(bloque, nodoActual.numero, nodosPorBloque)) {
			//asigno bloque al nodo
			asignacionsBloquesNodos[bloque][0] = nodoActual.numero;
			nodoActual.carga++;
			nodoActual.disponibilidad--;

			if (clockNoExisteBloque < 0 || clockMaestro == clockNoExisteBloque) {
				clockMaestro++;
				if (clockMaestro >= cantNodosArchivo)
					clockMaestro = 0;
			} else {
				clockMaestro = clockNoExisteBloque;
			}
			clockNoExisteBloque = -1;
			clockNodoDisponibilidad = -1;	//????? A1
			bloque++;
		} else if (!nodoConDisponibilidad(nodoActual)) { //el nodo no tiene disponibilidad
			if (clockNodoDisponibilidad < 0)	//????? A1
				clockNodoDisponibilidad = clockMaestro;
			nodoActual.disponibilidad += cargaBase;
			clockMaestro++;
			if (clockMaestro >= cantNodosArchivo)
				clockMaestro = 0;

		} else if (!existeBloqueEnNodo(bloque, nodoActual.numero, nodosPorBloque)) { //no se encuentra el bloque en el nodo
			clockNoExisteBloque = clockMaestro;
			clockMaestro++;
			if (clockMaestro >= cantNodosArchivo)
				clockMaestro = 0;

		}
		if (clockNoExisteBloque == clockMaestro)
			listaNodos[clockNoExisteBloque].disponibilidad += cargaBase;
	}

}

int main(int argc, char *argv[]) {
	t_log* logYAMA;
	logYAMA = log_create("logYAMA.log", "YAMA", false, LOG_LEVEL_TRACE); //creo el logger, sin mostrar por pantalla
	int preparadoEnviarFs = 1, i;
	int cantElementosTablaEstados = 0, maxMasterTablaEstados = 0;
	int maxJobTablaEstados = 0;
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
	if ((socketFS = conexionAFileSystem(logYAMA)) < 0) {
		preparadoEnviarFs = 0;
	}
	/* ************** inicialización como server ************ */
	int listenningSocket;
	if ((listenningSocket = inicializoComoServidor(logYAMA)) < 0) {
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
	int socketCliente, numMaster, socketConectado;
	for (;;) {
		socketsLecturaTemp = socketsLecturaMaster;

		if (select(maxFD + 1, &socketsLecturaTemp, NULL, NULL, NULL) == -1) {
			perror("Error en select()");
			//exit(4);
		}

		for (i = 0; i <= maxFD; i++) {
			if (FD_ISSET(i, &socketsLecturaTemp)) {
				if (i == listenningSocket) {
					if ((socketCliente = recibirConexion(logYAMA, listenningSocket)) >= 0) {
						numMaster = socketCliente;
					}
					uint32_t headerId = deserializarHeader(socketCliente);
					if (headerId == TIPO_MSJ_HANDSHAKE) { // && handshake() == ok)
						FD_SET(socketCliente, &socketsLecturaMaster); // add to master set
						if (socketCliente > maxFD) {    // keep track of the max
							maxFD = socketCliente;
						}
						/* *********** solo de prueba *************** */
						int cantStrings = 1;
						int cantidadMensajes = protocoloCantidadMensajes[headerId];
						char **arrayMensajesR = deserializarMensaje(socketConectado, cantidadMensajes);
						/*if (!strcmp(arrayMensajesR[0], "master")) {
							char *mensaje = "handhake ok";
						}else{
							char *mensaje = "handhake error";
						}*/
						char *mensaje = "handhake ok";
						char **arrayMensajesE = malloc(cantStrings);
						arrayMensajesE[0] = malloc(string_length(mensaje) + 1);
						strcpy(arrayMensajesE[0], mensaje);
						arrayMensajesE[0][string_length(mensaje)] = '\0';
						char *mensajeSerializado1 = serializarMensaje(TIPO_MSJ_HANDSHAKE_RESPUESTA_OK, arrayMensajesE, 0);
						enviarMensaje(socketCliente, mensajeSerializado1);
					} else {
						int cantStrings = 1;
						char *mensaje = "volá de acá vos";
						char **arrayMensajes = malloc(cantStrings);
						arrayMensajes[0] = malloc(string_length(mensaje) + 1);
						strcpy(arrayMensajes[0], mensaje);
						arrayMensajes[0][string_length(mensaje)] = '\0';
						char *mensajeSerializado1 = serializarMensaje(TIPO_MSJ_HANDSHAKE_RESPUESTA_OK, arrayMensajes, cantStrings);
						enviarMensaje(socketCliente, mensajeSerializado1);
					}
				} else {	//conexión preexistente
					/* *************************** recepción de un mensaje ****************************/
					socketConectado = i;
					uint32_t headerId = deserializarHeader(socketConectado);
					if (headerId < 0) {	//error
						//close(socketConectado); // bye!
						//FD_CLR(socketConectado, &socketsLecturaMaster); // remove from master set
					}
					int cantidadMensajes = protocoloCantidadMensajes[headerId];
					char **arrayMensajes = deserializarMensaje(socketConectado, cantidadMensajes);
					switch (headerId) {

					case TIPO_MSJ_TRANSFORMACION_OK:
						;
						free(arrayMensajes);
						break;
					case TIPO_MSJ_TRANSFORMACION_ERROR:
						;
						free(arrayMensajes);
						break;
					case TIPO_MSJ_REDUCC_LOCAL_OK:
						;
						free(arrayMensajes);
						break;
					case TIPO_MSJ_REDUCC_LOCAL_ERROR:
						;
						free(arrayMensajes);
						break;
					case TIPO_MSJ_REDUCC_GLOBAL_OK:
						;
						free(arrayMensajes);
						break;
					case TIPO_MSJ_REDUCC_GLOBAL_ERROR:
						;
						free(arrayMensajes);
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
								bloqueArchivo *bloques = recibirMetadataArchivoFS(socketFS);
								datosConexionNodo *nodosArchivo = recibirNodosArchivoFS(socketFS);
								//planificación
								planificar(bloques, nodosArchivo);
								//guarda la info de los bloques del archivo en la tabla de estados
								struct filaTablaEstados fila;
								for (i = 0; i < cantBloquesArchivo; i++) {
									fila.job = 1;		//modificar
									fila.master = 1;		//modificar
									fila.nodo = bloques[i].nodoCopia1;//planificar
									fila.bloque = bloques[i].bloqueCopia1;//planificar
									fila.etapa = TRANSFORMACION;
									//genera el nombre del archivo temporal
									char* temporal = string_from_format("m%dj%dn%db%de%d", fila.master, fila.job, fila.nodo, fila.bloque, fila.etapa);
									strcpy(fila.temporal, temporal);//modificar
									fila.estado = EN_PROCESO;
									fila.siguiente = NULL;
									if (!agregarElemTablaEstados(fila))
										perror("Error al agregar elementos a la tabla de estados");
								}
							} else {
								perror("No se pudo enviar el archivo al FS");
							}

							puts("lista de elementos 1");
							mostrarListaElementos();
						}

						//envía al master la lista de nodos donde trabajar cada bloque

						break;
					case TIPO_MSJ_CUATRO_MENSAJES:
						;
						for (i = 0; i < cantidadMensajes; i++) {
							printf("mensajeRecibido %d: %s\n", i, arrayMensajes[i]);
							free(arrayMensajes[i]);
						}

						free(arrayMensajes);
						break;
					default:
						;
						free(arrayMensajes);
						break;
					}
				}
			}
		}
	}

	//cerrarServer(listenningSocket);
	//cerrarServer(socketCliente);
	log_info(logYAMA, "Server cerrado");

	log_destroy(logYAMA);
	return EXIT_SUCCESS;
}

/* ******************* buscar elemento de la tabla ************************ */
/*fila.master = 1;
 fila.job = 1;
 fila.nodo = bloques[1].nodoCopia1;
 fila.bloque = bloques[1].bloqueCopia1;
 fila.etapa = TRANSFORMACION;

 struct filaTablaEstados *elementoBuscado = buscarElemTablaEstadosPorJMNBE(fila);

 if (elementoBuscado == NULL) {
 puts("no existe el registro buscado");
 } else {
 puts("búsqueda del elemento job 1 master 1");
 printf("nodo: %d - temporal: %s - etapa: %d\n", elementoBuscado->nodo, elementoBuscado->temporal, elementoBuscado->etapa);
 }*/
/* ************************************************************************ */

/* ******************* modificar un elemento de la tabla ****************** */
/*fila.master = 1;
 fila.job = 1;
 fila.nodo = bloques[1].nodoCopia1;
 fila.bloque = bloques[1].bloqueCopia1;
 fila.etapa = TRANSFORMACION;
 mostrarListaElementos();
 struct filaTablaEstados datosNuevos;
 datosNuevos.master = 1;
 datosNuevos.job = 1;
 datosNuevos.nodo = bloques[1].nodoCopia1;
 datosNuevos.bloque = bloques[1].bloqueCopia1;
 datosNuevos.etapa = TRANSFORMACION;
 datosNuevos.estado = FIN_OK;
 modificarElemTablaEstados(fila, datosNuevos);
 puts("elementos después de ser modificados");
 mostrarListaElementos();*/
/* ************************************************************************ */
