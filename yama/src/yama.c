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

int cantPartesArchivo, cantNodosArchivo;

#define LARGO_IP 16

typedef struct {
	int numero;
	char ip[LARGO_IP];
	int puerto;
} datosConexionNodo;

typedef struct {
	int nroNodo;
	char ipNodo[LARGO_IP];
	int puertoNodo;
	int bloque;
	int bytesOcupados;
	char temporal[40];
} tablaTransformacion;

#define CANT_MENSAJES_POR_BLOQUE_DE_ARCHIVO 5
#define CANT_MENSAJES_POR_NODO 3

t_log* logYAMA;

int getDatosConfiguracion() {
	char *nameArchivoConfig = "configYama.txt";
	if (leerArchivoConfig(nameArchivoConfig, keysConfigYama, datosConfigYama)) { //leerArchivoConfig devuelve 1 si hay error
		printf("Hubo un error al leer el archivo de configuración");
		return 0;
	}
	return 1;
}

int conexionAFileSystem() {
	log_info(logYAMA, "Conexión a FileSystem, IP: %s, Puerto: %s", datosConfigYama[FS_IP], datosConfigYama[FS_PUERTO]);
	int socketFS = conectarA(datosConfigYama[FS_IP], datosConfigYama[FS_PUERTO]);
	if (socketFS < 0) {
		puts("Filesystem not ready\n");
		//preparadoEnviarFs = handshakeClient(&datosConexionFileSystem, NUM_PROCESO_KERNEL);
	}
	return socketFS;
}

int inicializoComoServidor() {
	int listenningSocket = inicializarServer(datosConfigYama[IP_PROPIA], datosConfigYama[PUERTO_PROPIO]);
	if (listenningSocket < 0) {
		log_error(logYAMA, "No pude iniciar como servidor");
		puts("No pude iniciar como servidor");
	}
	return listenningSocket;
}

int recibirConexion(int listenningSocket) {
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
	char **arrayMensajesSerializar = malloc(sizeof(char*) * cantStrings);
	printf("archivo a trabajar: %s\n", archivo);
	printf("largo del archivo a trabajar: %d\n", string_length(archivo));
	arrayMensajesSerializar[0] = malloc(string_length(archivo) + 1);
	if (!arrayMensajesSerializar[0])
		perror("error de malloc");
	strcpy(arrayMensajesSerializar[0], archivo);
	printf("arrayMensajesSerializar[0]: %s\n", arrayMensajesSerializar[0]);
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
		perror("El FS no mandó los bloques");
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
	cantPartesArchivo = atoi(arrayMensajes[0]);
	printf("cantPartesArchivo: %d\n", cantPartesArchivo);

	arrayMensajes = deserializarMensaje(socketFS, cantPartesArchivo * CANT_MENSAJES_POR_BLOQUE_DE_ARCHIVO);
	bloqueArchivo *bloques = malloc(cantPartesArchivo * sizeof(bloqueArchivo));
	int j = 0;
	for (i = 0; i < cantPartesArchivo; i++) {
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

datosConexionNodo * recibirNodosArchivoFS(int socketFS) {
	int i;
	uint32_t headerId = deserializarHeader(socketFS);
	printf("headerId nodos: %d\n", headerId);
	/*if (headerId != TIPO_MSJ_DATOS_CONEXION_NODOS) {
	 printf("El FS no mandó los nodos\n");
	 datosConexionNodo *nodosError = malloc(sizeof(bloqueArchivo));
	 nodosError[0].numero = 0;
	 strcpy(nodosError[0].ip, "");
	 nodosError[0].puerto = 0;
	 return nodosError;
	 }*/
	int cantidadMensajes = protocoloCantidadMensajes[headerId];
	char **arrayMensajes = deserializarMensaje(socketFS, cantidadMensajes);
	//guardar la data en algún lado
	cantNodosArchivo = atoi(arrayMensajes[0]);
	printf("cantNodos: %d\n", cantNodosArchivo);
	arrayMensajes = deserializarMensaje(socketFS, cantNodosArchivo * CANT_MENSAJES_POR_NODO);
	datosConexionNodo *nodos = malloc(cantNodosArchivo * sizeof(datosConexionNodo));
	int j = 0;
	for (i = 0; i < cantNodosArchivo; i++) {
		nodos[i].numero = atoi(arrayMensajes[j]);
		j++;
		strcpy(nodos[i].ip, arrayMensajes[j]);
		j++;
		nodos[i].puerto = atoi(arrayMensajes[j]);
		j++;
	}
	return nodos;
}

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
	/*for (i = 0; i < cantPartesArchivo; i++) {
	 printf("parte archivo %d: Nodo copia1 %d - Bloque copia1 %d\n", i, bloques[i].nodoCopia1, bloques[i].bloqueCopia1);
	 printf("parte archivo %d: Nodo copia2 %d - Bloque copia2 %d\n", i, bloques[i].nodoCopia2, bloques[i].bloqueCopia2);
	 }
	 printf("\n");*/
	for (i = 0; i < cantPartesArchivo; i++) {
		nodosPorPedazoArchivo[i].nodo1 = bloques[i].nodoCopia1;
		nodosPorPedazoArchivo[i].bloque1 = bloques[i].bloqueCopia1;
		nodosPorPedazoArchivo[i].nodo2 = bloques[i].nodoCopia2;
		nodosPorPedazoArchivo[i].bloque2 = bloques[i].bloqueCopia2;
		nodosPorPedazoArchivo[i].bytes = bloques[i].bytesBloque;
	}
	for (i = 0; i < cantPartesArchivo; i++) {
		printf("nodosPorPedazoArchivo %d: Nodo copia1 %d - Bloque copia1 %d\n", i, nodosPorPedazoArchivo[i].nodo1, nodosPorPedazoArchivo[i].bloque1);
		printf("nodosPorPedazoArchivo %d: Nodo copia2 %d - Bloque copia2 %d\n", i, nodosPorPedazoArchivo[i].nodo2, nodosPorPedazoArchivo[i].bloque2);
	}
	printf("\n");
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
		printf("\nnodo actual: %d, parte de archivo: %d\n", nodoActual.numero, parteArchivo);
		printf("disponibilidad del nodo: %d\n", nodoConDisponibilidad(nodoActual));
		printf("existe la parte del archivo en el nodo: %d\n", existeParteArchivoEnNodo(parteArchivo, nodoActual.numero, nodosPorPedazoArchivo));
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
			printf("\nno disponibilidad partearchivo %d en el nodo %d\n", parteArchivo, nodoActual.numero);
		} else if (!existeParteArchivoEnNodo(parteArchivo, nodoActual.numero, nodosPorPedazoArchivo)) { //no se encuentra el bloque en el nodo
			clockNoExisteParteArchivo = clockMaestro;
			clockMaestro++;
			if (clockMaestro >= cantNodosArchivo)
				clockMaestro = 0;
			printf("\nno existe parte de archivo %d en nodo %d\n", parteArchivo, nodoActual.numero);
			printf("clock maestro: %d\n", clockMaestro);
		}
		if (clockNoExisteParteArchivo == clockMaestro)
			listaCargaNodos[clockNoExisteParteArchivo].disponibilidad += cargaBase;
	}

	return;
}

char* serializarNodosTransformacion(tablaTransformacion respuestaTransformacion[cantPartesArchivo]) {
	int i, j, k, cantStringsASerializar, largoStringDestinoCopia;

	cantStringsASerializar = (cantPartesArchivo * 6) + 1;
	char **arrayMensajes = malloc(sizeof(char*) * cantStringsASerializar);

	largoStringDestinoCopia = 4 + 1;
	arrayMensajes[0] = malloc(largoStringDestinoCopia);
	strcpy(arrayMensajes[0], intToArrayZerosLeft(cantPartesArchivo, 4));
	j = 1;
	for (i = 0; i < cantPartesArchivo; i++) {
		printf("\nnodo %d - ip %s - puerto %d - bloque %d - bytes %d - temporal %s\n", respuestaTransformacion[i].nroNodo, respuestaTransformacion[i].ipNodo, respuestaTransformacion[i].puertoNodo, respuestaTransformacion[i].bloque, respuestaTransformacion[i].bytesOcupados, respuestaTransformacion[i].temporal);

		//número de nodo
		largoStringDestinoCopia = 4 + 1;
		arrayMensajes[j] = malloc(largoStringDestinoCopia);
		strcpy(arrayMensajes[j], intToArrayZerosLeft(respuestaTransformacion[i].nroNodo, 4));
		//arrayMensajes[j] = copiarString(intToArrayZerosLeft(respuestaTransformacion[i].nroNodo, 4), largoStringDestinoCopia);
		printf("nro nodo guardado: %s\n",arrayMensajes[j]);
		j++;

		//IP
		largoStringDestinoCopia = string_length(respuestaTransformacion[i].ipNodo) + 1;
		arrayMensajes[j] = malloc(largoStringDestinoCopia);
		strcpy(arrayMensajes[j], respuestaTransformacion[i].ipNodo);
		//arrayMensajes[j] = copiarString(respuestaTransformacion[i].ipNodo, largoStringDestinoCopia);
		j++;

		//puerto
		largoStringDestinoCopia = 4 + 1;
		arrayMensajes[j] = malloc(largoStringDestinoCopia);
		strcpy(arrayMensajes[j], intToArrayZerosLeft(respuestaTransformacion[i].puertoNodo, 4));
		//arrayMensajes[j] = copiarString(intToArrayZerosLeft(respuestaTransformacion[i].puertoNodo, 4), largoStringDestinoCopia);
		printf("puerto guardado: %s\n",arrayMensajes[j]);
		j++;

		//bloque
		largoStringDestinoCopia = 4 + 1;
		arrayMensajes[j] = malloc(largoStringDestinoCopia);
		strcpy(arrayMensajes[j], intToArrayZerosLeft(respuestaTransformacion[i].bloque, 4));
		//arrayMensajes[j] = copiarString(intToArrayZerosLeft(respuestaTransformacion[i].bloque, 4), largoStringDestinoCopia);
		j++;

		//bytes ocupados
		largoStringDestinoCopia = 8 + 1;
		arrayMensajes[j] = malloc(largoStringDestinoCopia);
		strcpy(arrayMensajes[j], intToArrayZerosLeft(respuestaTransformacion[i].bytesOcupados, 8));
		//arrayMensajes[j] = copiarString(intToArrayZerosLeft(respuestaTransformacion[i].bytesOcupados, 8), largoStringDestinoCopia);
		j++;

		//temporal
		largoStringDestinoCopia = string_length(respuestaTransformacion[i].temporal) + 1;
		arrayMensajes[j] = malloc(largoStringDestinoCopia);
		strcpy(arrayMensajes[j], respuestaTransformacion[i].temporal);
		//arrayMensajes[j] = copiarString(respuestaTransformacion[i].temporal, largoStringDestinoCopia);
		j++;
	}

	char *mensajeSerializado = serializarMensaje(TIPO_MSJ_TABLA_TRANSFORMACION, arrayMensajes, cantStringsASerializar);
	return mensajeSerializado;

}

int main(int argc, char *argv[]) {
	logYAMA = log_create("logYAMA.log", "YAMA", false, LOG_LEVEL_TRACE); //creo el logger, sin mostrar por pantalla
	int preparadoEnviarFs = 1, i, j, k, master, job;
	int cantElementosTablaEstados = 0, maxMasterTablaEstados = 0;
	int maxJobTablaEstados = 0;
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
	int socketCliente, numMaster, socketConectado, cantStrings, h = 0,
			bytesRecibidos = 0, nroSocket;
	printf("valor de listenningSocket: %d\n", listenningSocket);
	for (;;) {
		socketsLecturaTemp = socketsLecturaMaster;
		printf("pasóoo %d\n", h);
		h++;
		if (select(maxFD + 1, &socketsLecturaTemp, NULL, NULL, NULL) != -1) {

			for (nroSocket = 0; nroSocket <= maxFD; nroSocket++) {
				printf("valor de nroSocket %d\n", nroSocket);
				if (FD_ISSET(nroSocket, &socketsLecturaTemp)) {
					if (nroSocket == listenningSocket) {	//conexión nueva
						if ((socketCliente = recibirConexion(listenningSocket)) >= 0) {
							numMaster = socketCliente;
						}
						uint32_t headerId = deserializarHeader(socketCliente);

						if (headerId == TIPO_MSJ_HANDSHAKE) {
							int cantidadMensajes = protocoloCantidadMensajes[headerId];
							char **arrayMensajesRHS = deserializarMensaje(socketCliente, cantidadMensajes);
							int idEmisorMensaje = atoi(arrayMensajesRHS[0]);
							free(arrayMensajesRHS);
							if (idEmisorMensaje == NUM_PROCESO_MASTER) {
								FD_SET(socketCliente, &socketsLecturaMaster); // add to master set
								//FD_SET(socketCliente, &socketsLecturaTemp); // add to master set
								if (socketCliente > maxFD) { // keep track of the max
									maxFD = socketCliente;
								}
								strcpy(mensajeHeaderSolo, intToArrayZerosLeft(TIPO_MSJ_HANDSHAKE_RESPUESTA_OK, 4));
							} else {
								strcpy(mensajeHeaderSolo, intToArrayZerosLeft(TIPO_MSJ_HANDSHAKE_RESPUESTA_DENEGADO, 4));
							}
							enviarMensaje(socketCliente, mensajeHeaderSolo);
						}
					} else {	//conexión preexistente
						/* *************************** recepción de un mensaje ****************************/
						socketConectado = nroSocket;
						printf("valor de socketConectado: %d\n", socketConectado);
						uint32_t headerId = deserializarHeader(socketConectado);
						printf("valor de headerId: %d\n", headerId);
						if (headerId < 0) {	//error
							//close(socketConectado); // bye!
							//FD_CLR(socketConectado, &socketsLecturaMaster); // remove from master set
						}
						int cantidadMensajes = protocoloCantidadMensajes[headerId];
						printf("valor de cantidadMensajes: %d\n", cantidadMensajes);
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
							//archivo = copiarString(arrayMensajes[0], string_length(archivo));
							free(arrayMensajes);
							//pide la metadata del archivo al FS
							if (preparadoEnviarFs) {
								if (pedirMetadataArchivoFS(socketFS, archivo)) {

									/*bloqueArchivo *bloques = recibirMetadataArchivoFS(socketFS);
									 enviarHeaderSolo(socketFS, TIPO_MSJ_OK);
									 puts("\npasame los nodos");
									 datosConexionNodo *nodosArchivo = recibirNodosArchivoFS(socketFS);
									 enviarHeaderSolo(socketFS, TIPO_MSJ_OK);*/

									/* ***************** datos de bloques y nodos inventados para probar **************** */
									bloqueArchivo bloques[6];
									datosConexionNodo nodosArchivo[3];
									bloques[0].nodoCopia1 = 1;
									bloques[0].bloqueCopia1 = 33;
									bloques[0].nodoCopia2 = 3;
									bloques[0].bloqueCopia2 = 13;
									bloques[0].bytesBloque = 12564;

									bloques[1].nodoCopia1 = 2;
									bloques[1].bloqueCopia1 = 36;
									bloques[1].nodoCopia2 = 3;
									bloques[1].bloqueCopia2 = 21;
									bloques[1].bytesBloque = 3264;

									bloques[2].nodoCopia1 = 2;
									bloques[2].bloqueCopia1 = 12;
									bloques[2].nodoCopia2 = 3;
									bloques[2].bloqueCopia2 = 55;
									bloques[2].bytesBloque = 3264;

									bloques[3].nodoCopia1 = 2;
									bloques[3].bloqueCopia1 = 65;
									bloques[3].nodoCopia2 = 3;
									bloques[3].bloqueCopia2 = 5;
									bloques[3].bytesBloque = 10264;

									bloques[4].nodoCopia1 = 1;
									bloques[4].bloqueCopia1 = 88;
									bloques[4].nodoCopia2 = 3;
									bloques[4].bloqueCopia2 = 101;
									bloques[4].bytesBloque = 4264;

									bloques[5].nodoCopia1 = 1;
									bloques[5].bloqueCopia1 = 74;
									bloques[5].nodoCopia2 = 2;
									bloques[5].bloqueCopia2 = 120;
									bloques[5].bytesBloque = 32334;

									cantPartesArchivo = 6;

									nodosArchivo[0].numero = 1;
									strcpy(nodosArchivo[0].ip, "127.000.000.001");
									nodosArchivo[0].puerto = 5300;

									nodosArchivo[1].numero = 2;
									strcpy(nodosArchivo[1].ip, "192.168.001.095");
									nodosArchivo[1].puerto = 5250;

									nodosArchivo[2].numero = 3;
									strcpy(nodosArchivo[2].ip, "192.168.001.201");
									nodosArchivo[2].puerto = 4095;

									cantNodosArchivo = 3;
									/* ********************************************************************************** */
									master = 1;	//modificar
									job = 1;	//modificar
									/* ************* inicio planificación *************** */
									//indexado por partes del archivo, contiene el nodo, bloque y bytes
									//al cual fue asignado el pedazo de archivo
									nodoParaAsignar asignacionesNodos[cantPartesArchivo];
									planificar(bloques, nodosArchivo, asignacionesNodos);
									/* ************* fin planificación *************** */

									//guarda la info de los bloques del archivo en la tabla de estados
									struct filaTablaEstados fila;
									tablaTransformacion respuestaTransformacion[cantPartesArchivo];
									for (i = 0; i < cantPartesArchivo; i++) {
										//printf("parte de archivo %d asignado a: nodo %d - bloque %d\n", i, asignacionesNodos[i][0], asignacionesNodos[i][1]);
										//genera una fila en la tabla de estados
										fila.job = master;
										fila.master = job;
										fila.nodo = asignacionesNodos[i].nroNodo;
										fila.bloque = asignacionesNodos[i].bloque;
										fila.etapa = TRANSFORMACION;
										char* temporal = string_from_format("m%dj%dn%db%de%d", fila.master, fila.job, fila.nodo, fila.bloque, fila.etapa);
										strcpy(fila.temporal, temporal);
										fila.estado = EN_PROCESO;
										fila.siguiente = NULL;
										if (!agregarElemTablaEstados(fila))
											perror("Error al agregar elementos a la tabla de estados");

										//genera una fila en la tabla de transformación para el master
										respuestaTransformacion[i].nroNodo = asignacionesNodos[i].nroNodo;
										strcpy(respuestaTransformacion[i].ipNodo, asignacionesNodos[i].ip);
										respuestaTransformacion[i].puertoNodo = asignacionesNodos[i].puerto;
										respuestaTransformacion[i].bloque = asignacionesNodos[i].bloque;
										respuestaTransformacion[i].bytesOcupados = asignacionesNodos[i].bytesOcupados;
										strcpy(respuestaTransformacion[i].temporal, temporal);
									}

									//puts("\nlista de elementos asignados a transformación");
									//mostrarListaElementos();

									//envía al master la lista de nodos donde trabajar cada bloque
									char *mensajeSerializado = serializarNodosTransformacion(respuestaTransformacion);
									printf("mensaje serializado: %s\n", mensajeSerializado);
									if (!enviarMensaje(socketConectado, mensajeSerializado)) {
										return 0;
									}
								} else {
									perror("No se pudo pedir el archivo al FS");
								}
							}
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
							puts("pasó por default");
							break;
						}
					} // END handle data from client
				} //if (FD_ISSET(i, &socketsLecturaTemp)) END got new incoming connection
				printf("valor de nroSocket al final del bucle: %d\n", nroSocket);
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
