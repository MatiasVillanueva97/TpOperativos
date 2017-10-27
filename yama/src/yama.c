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
	int tamArchivo;
	char tipoArchivo;
	int cantBloquesArchivo;
	int nodoCopia1;
	int bloqueCopia1;
	int nodoCopia2;
	int bloqueCopia2;
	int bytesBloque;
} bloqueArchivo;

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

bloqueArchivo* pedirMetadataArchivoFS(int socketFS, char *archivo) {
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
	enviarMensaje(socketFS, mensajeSerializado);

	uint32_t headerId = deserializarHeader(socketFS);
	int cantidadMensajes = protocoloCantidadMensajes[headerId];
	char **arrayMensajes = deserializarMensaje(socketFS, cantidadMensajes);
	if (headerId != TIPO_MSJ_METADATA_ARCHIVO) {
		perror("El FS no mandó lo solicitado");
		bloqueArchivo *bloquesError = malloc(sizeof(bloqueArchivo));
		bloquesError[0].tamArchivo = 0;
		bloquesError[0].tipoArchivo = 0;
		bloquesError[0].cantBloquesArchivo = 0;
		bloquesError[0].nodoCopia1 = 0;
		bloquesError[0].bloqueCopia1 = 0;
		bloquesError[0].nodoCopia2 = 0;
		bloquesError[0].bloqueCopia2 = 0;
		bloquesError[0].bytesBloque = 0;
		return bloquesError;
	} else {
		//guardar la data en algún lado
		int tamArchivo = atoi(arrayMensajes[0]);
		char tipoArchivo = arrayMensajes[1][0];
		int cantBloquesArchivo = atoi(arrayMensajes[2]);
		bloqueArchivo *bloques = malloc(cantBloquesArchivo * sizeof(bloqueArchivo));
		int j = 3;
		for (i = 0; i < cantBloquesArchivo; i++) {
			bloques[i].tamArchivo = tamArchivo;
			bloques[i].tipoArchivo = tipoArchivo;
			bloques[i].cantBloquesArchivo = cantBloquesArchivo;
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

}

void planificar() {
	//sale del archivo config?????????????
	cargaBase = 1;

	//estos valores salen de la info del FS
	cantBloquesArchivo = 6;
	//guarda los nodos en los que está cada bloque
	nodosPorBloque nodosPorBloque[cantBloquesArchivo];
	//cargo los nodos en los que está cada bloque
	nodosPorBloque[0].nodo1 = 1;
	nodosPorBloque[0].nodo2 = 3;
	nodosPorBloque[1].nodo1 = 2;
	nodosPorBloque[1].nodo2 = 3;
	nodosPorBloque[2].nodo1 = 2;
	nodosPorBloque[2].nodo2 = 3;
	nodosPorBloque[3].nodo1 = 2;
	nodosPorBloque[3].nodo2 = 3;
	nodosPorBloque[4].nodo1 = 1;
	nodosPorBloque[4].nodo2 = 3;
	nodosPorBloque[5].nodo1 = 1;
	nodosPorBloque[5].nodo2 = 2;

	//estos valores salen de la info del FS
	cantNodos = 3;
	//tiene la carga de cada nodo
	nodo listaNodos[cantNodos];
	//pongo la carga inicial de cada nodo
	listaNodos[0].carga = 0;
	listaNodos[0].numero = 1;
	listaNodos[1].carga = 1;
	listaNodos[1].numero = 2;
	listaNodos[2].carga = 1;
	listaNodos[2].numero = 3;
	nodo nodoMaxCarga;
	nodoMaxCarga = nodoConMayorCarga(listaNodos, cantNodos);
	cargaMaxima = nodoMaxCarga.carga;
	listaNodos[0].disponibilidad = calcularDisponibilidadNodo(listaNodos[0]);
	listaNodos[1].disponibilidad = calcularDisponibilidadNodo(listaNodos[1]);
	listaNodos[2].disponibilidad = calcularDisponibilidadNodo(listaNodos[2]);

	//ordeno los nodos de mayor a menor disponibilidad
	int i, j;
	nodo temp;
	for (i = 0; i < cantNodos; i++) {
		for (j = 0; j < cantNodos - 1; j++) {
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
	int asignacionsBloquesNodos[cantBloquesArchivo];
	while (bloque < cantBloquesArchivo) {
		nodo nodoActual = listaNodos[clockMaestro];
		if (nodoConDisponibilidad(nodoActual) && existeBloqueEnNodo(bloque, nodoActual.numero, nodosPorBloque)) {
			//asigno bloque al nodo
			asignacionsBloquesNodos[bloque] = nodoActual.numero;
			nodoActual.carga++;
			nodoActual.disponibilidad--;
			//mover el clock auxiliar

			if (clockNoExisteBloque < 0 || clockMaestro == clockNoExisteBloque) {
				clockMaestro++;
				if (clockMaestro >= cantNodos)
					clockMaestro = 0;
			} else {
				clockMaestro = clockNoExisteBloque;
			}
			clockNoExisteBloque = -1;
			bloque++;
		} else if (!nodoConDisponibilidad(nodoActual)) { //el nodo no tiene disponibilidad
			clockNodoDisponibilidad = clockMaestro;	//?????
			nodoActual.disponibilidad += cargaBase;
			clockMaestro++;
			if (clockMaestro >= cantNodos)
				clockMaestro = 0;

		} else if (!existeBloqueEnNodo(bloque, nodoActual.numero, nodosPorBloque)) { //no se encuentra el bloque en el nodo
			clockNoExisteBloque = clockMaestro;
			clockMaestro++;
			if (clockMaestro >= cantNodos)
				clockMaestro = 0;

		}
		if (clockNoExisteBloque == clockMaestro)
			listaNodos[clockNoExisteBloque].disponibilidad += cargaBase;
	}

}

int main(int argc, char *argv[]) {
	planificar();
	return 0;
	t_log* logYAMA;
	logYAMA = log_create("logYAMA.log", "YAMA", false, LOG_LEVEL_TRACE); //creo el logger, sin mostrar por pantalla
	int preparadoEnviarFs = 1, i;
	int cantElementosTablaEstados = 0, maxMasterTablaEstados = 0;
	int maxJobTablaEstados = 0;
	log_info(logYAMA, "Iniciando proceso YAMA");
	printf("\n*** Proceso Yama ***\n");

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

	int socketCliente;
	if ((socketCliente = recibirConexion(logYAMA, listenningSocket)) < 0) {
		return EXIT_FAILURE;
	}
	int numMaster = socketCliente;

	/* *************************** espera recepción de un mensaje ****************************/
	uint32_t headerId = deserializarHeader(socketCliente);
//printf("\nmensaje predefinido: %s\n", protocoloMensajesPredefinidos[headerId]);
	int cantidadMensajes = protocoloCantidadMensajes[headerId];
	char **arrayMensajes = deserializarMensaje(socketCliente, cantidadMensajes);
	switch (headerId) {
	case TIPO_MSJ_HANDSHAKE:
		;
		free(arrayMensajes);
		break;
	case TIPO_MSJ_PATH_ARCHIVO:
		;
		char *archivo = malloc(string_length(arrayMensajes[0]) + 1);
		strcpy(archivo, arrayMensajes[0]);
		free(arrayMensajes);

		//pide la metadata del archivo al FS
		if (preparadoEnviarFs) {
			bloqueArchivo *bloques = pedirMetadataArchivoFS(socketFS, archivo);
			/*printf("Tamaño del archivo: %d\nTipo del archivo: %c\n", bloques[0].tamArchivo, bloques[0].tipoArchivo);
			 printf("Bloque 1 - Copia 1: Nodo %d - Bloque %d\n", bloques[0].nodoCopia1, bloques[0].bloqueCopia1);
			 printf("Bloque 1 - Copia 2: Nodo %d - Bloque %d\n", bloques[0].nodoCopia2, bloques[0].bloqueCopia2);
			 printf("Bloque 2 - Copia 1: Nodo %d - Bloque %d\n", bloques[1].nodoCopia1, bloques[1].bloqueCopia1);
			 printf("Bloque 2 - Copia 2: Nodo %d - Bloque %d\n", bloques[1].nodoCopia2, bloques[1].bloqueCopia2);*/

			//planificación
			//guarda la info de los bloques del archivo en la tabla de estados
			int cantBloquesArchivo = bloques[0].cantBloquesArchivo;
			struct filaTablaEstados fila;
			for (i = 0; i < cantBloquesArchivo; i++) {
				fila.master = 1;		//modificar
				fila.job = 1;		//modificar
				fila.nodo = bloques[i].nodoCopia1;	//planificar
				fila.bloque = bloques[i].bloqueCopia1;	//planificar
				fila.etapa = TRANSFORMACION;
				fila.estado = EN_PROCESO;
				//fila.
				//genera el nombre del archivo temporal
				char* temporal = string_from_format("m%dj%dn%db%d", fila.master, fila.job, fila.nodo, fila.bloque);
				strcpy(fila.temporal, temporal);	//modificar
				fila.siguiente = NULL;
				if (!agregarElemTablaEstados(fila)) {
					perror("Error al agregar elementos a la tabla de estados");
				}

			}
			puts("lista de elementos 1");
			mostrarListaElementos();

			/* ******************* buscar elemento de la tabla ************************ */
			fila.master = 1;
			fila.job = 1;
			fila.nodo = bloques[1].nodoCopia1;
			fila.bloque = 0;
			fila.etapa = 0;
			fila.estado = 0;
			char* temporal = string_from_format("m%dj%dn%db%d", fila.master, fila.job, fila.nodo, fila.bloque);
			strcpy(fila.temporal, "");
			fila.siguiente = NULL;

			struct filaTablaEstados *elementoBuscado = buscarElemTablaEstados(fila);

			if (elementoBuscado == NULL) {
				puts("no existe el registro buscado");
			} else {
				puts("búsqueda del elemento job 1 master 1");
				printf("nodo: %d - temporal: %s - etapa: %d\n", elementoBuscado->nodo, elementoBuscado->temporal, elementoBuscado->etapa);
			}
			/* ************************************************************************ */

			/* ******************* modificar un elemento de la tabla ****************** */
			fila.master = 1;
			fila.job = 1;
			fila.nodo = bloques[1].nodoCopia1;
			fila.bloque = 0;
			fila.etapa = 0;
			fila.estado = 0;
			strcpy(fila.temporal, "");
			fila.siguiente = NULL;

			struct filaTablaEstados datosNuevos;
			datosNuevos.master = 1;
			datosNuevos.job = 1;
			datosNuevos.nodo = bloques[1].nodoCopia1;
			datosNuevos.bloque = 0;
			datosNuevos.etapa = 0;
			datosNuevos.estado = FIN_OK;
			strcpy(datosNuevos.temporal, "");
			fila.siguiente = NULL;
			modificarElemTablaEstados(fila, datosNuevos);
			puts("elementos después de ser modificados");
			mostrarListaElementos();
			/* ************************************************************************ */
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

	cerrarServer(listenningSocket);
//cerrarServer(socketCliente);
	log_info(logYAMA, "Server cerrado");

	log_destroy(logYAMA);
	return EXIT_SUCCESS;
}
