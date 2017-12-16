/*
 ============================================================================
 Name        : worker.c
 Author      : Grupo 1234
 Description : Proceso Worker
 Resume      : Worker es el que realiza las operaciones que le pide el Master.
 1) Lee archivo de configuracion del nodo.
 2) Espera conexion de Masters.
 3) Recibe conexion del Master y se forkea
 4) El principal sigue escuchando, el fork ejecuta la orden
 5) Termina la orden, aviso a master que terminó y el fork muere
 Puede haber varios Worker corriendo al mismo tiempo.
 ============================================================================
 */

#include "../../utils/includes.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>

#define SIZE 1024 //tamaño para comunicaciones entre padre e hijos
#define NULO 0
#define ERROR -1
//#define PACKAGESIZE 1024	// Define cual va a ser el size maximo del paquete a enviar
//tamanioData = stat --format=%s "nombre archivo" //tamaño data.bin en bytes

char *nameArchivoConfig = "configNodo.txt";
enum keys {
	IP_PROPIA, PUERTO_PROPIO, RUTA_DATABIN, FS_IP, FS_PUERTO, NOMBRE_NODO,
};
char* keysConfigWorker[] = { "IP_PROPIA", "PUERTO_PROPIO", "RUTA_DATABIN", "FS_IP", "FS_PUERTO", "NOMBRE_NODO", NULL };
char* datosConfigWorker[6];

pid_t pidPadre;
t_log* logWorker;

//char* carpeta_temporal = "../tmp";
//char* carpeta_resultados = "../resultados";

char* carpeta_log = "../log";

char* carpeta_temporal_transformacion = "../tmpTransformacion";
char* carpeta_resultado_transformacion = "../resultadoTransformacion";

char* carpeta_temporal_reduccion_local = "../tmpReduccionLocal";
char* carpeta_resultado_reduccion_local = "../resultadoReduccionLocal";

char* carpeta_temporal_reduccion_global = "../tmpReduccionGlobal";
char* carpeta_resultado_reduccion_global = "../resultadoReduccionGlobal";

//---------------------- FUNCIONES GENERALES ----------------------

// Recibe el codigo a guardar y un string "nombre". Guarda el codigo en un archivo llamado script_nombre, en la carpeta temporal que corresponda.
char* guardar_script_transformacion(char* codigo_script, char* nombre) {
	//log_info(logWorker, "[guardar_script]: Codigo recibido: %s", codigo_script);
	char* path = string_from_format("%s/script_%s", carpeta_temporal_transformacion, nombre);
	FILE *fp = fopen(path, "w");
	if (fp != NULL) {
		fputs(codigo_script, fp);
		fclose(fp);
	}
	log_info(logWorker, "[guardar_script]: Path script guardado: %s", path);
	return path;
}

char* guardar_script_reduccion_local(char* codigo_script, char* nombre) {
	//log_info(logWorker, "[guardar_script]: Codigo recibido: %s", codigo_script);
	char* path = string_from_format("%s/script_%s", carpeta_temporal_reduccion_local, nombre);
	FILE *fp = fopen(path, "w");
	if (fp != NULL) {
		fputs(codigo_script, fp);
		fclose(fp);
	}
	log_info(logWorker, "[guardar_script]: Path script guardado: %s", path);
	return path;
}

char* guardar_script_reduccion_global(char* codigo_script, char* nombre) {
	//log_info(logWorker, "[guardar_script]: Codigo recibido: %s", codigo_script);
	char* path = string_from_format("%s/script_%s", carpeta_temporal_reduccion_global, nombre);
	FILE *fp = fopen(path, "w");
	if (fp != NULL) {
		fputs(codigo_script, fp);
		fclose(fp);
	}
	log_info(logWorker, "[guardar_script]: Path script guardado: %s", path);
	return path;
}

char* guardar_datos(char* datos, char* carpeta, char* nombre) {
	char* path = string_from_format("%s/%s", carpeta, nombre);
	FILE *archivo = fopen(path, "w");
	if (archivo != NULL) {
		fputs(datos, archivo);
		fclose(archivo);
	} else {
		log_error(logWorker, "[guardar_datos] Hubo un error al abrir el archivo: %s", path);
	}
	log_info(logWorker, "[guardar_datos]: Path archivo guardado: %s", path);
	return path;
}

char * leerArchivo2(char * ubicacionArchivo) {
	log_info(logWorker, "[leerArchivo] Leyendo archivo: %s", ubicacionArchivo);
	FILE *fp;
	// Lee el archivo (transformador o reductor) y lo pasa a string para poder enviarlo
	char *pathArchivo = string_from_format("%s", ubicacionArchivo);
	fp = fopen(pathArchivo, "r"); // read mode
	if (fp == NULL) {
		log_error(logWorker, "El archivo %s no existe", pathArchivo);
	}
	fseek(fp, 0, SEEK_END);
	long lengthArchivo = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	char *archivoString = malloc(sizeof(char) * (lengthArchivo + 1));
	if (archivoString) {
		fread(archivoString, 1, lengthArchivo, fp);
	} else {
		log_error(logWorker, "Malloc al leer archivo");
	}
	fclose(fp);
	log_info(logWorker, "[leerArchivo] Archivo leido: %s, largo string: %d", pathArchivo, string_length(archivoString));
	return archivoString;
}

char* leerArchivo(char* pathArchivo) {
	log_info(logWorker, "[leerArchivo] Leyendo archivo: %s", pathArchivo);
	//printf("hola\n");
	char caracter;
	//printf("cree caracter\n");
	char* contenido = string_new();
	//printf("cree char*\n");
	//printf("antes de abrir\n");
	FILE* archivo = fopen(pathArchivo, "r");
	//printf("entra al while\n");
	while (1) {
		caracter = fgetc(archivo);
		if (caracter == EOF) {
			break;
		}
		string_append_with_format(&contenido, "%c", caracter);
	}
	printf("sale del while\n");
	fclose(archivo);
	log_info(logWorker, "[leerArchivo] Archivo leido: %s", pathArchivo);
	return contenido;
}

int ejecutar_system(char* comando) {
	log_info(logWorker, "[ejecutar_system]: Comando recibido: %s", comando);
	int status;
	system(comando);
	wait(&status); //pausa hasta que termina el hijo (system) y guarda el resultado en status
	int exit_status = WEXITSTATUS(status);
	//if (WIFEXITED(status) == 0) {
	if (WIFEXITED(status) != -1) {
		log_trace(logWorker, "System termino OK, el exit status del comando fue %d\n", exit_status);
		return 0;
	} else {
		log_error(logWorker, "System fallo, el codigo de resultado fue: %d\n", exit_status);
		return -1;
	}
	return 0;
}

//---------------------- FUNCIONES TRANSFORMACION ----------------------

char* guardar_datos_origen(char* datos_origen, char* nombre) {
	char* path = string_from_format("%s/datos_%s", carpeta_temporal_transformacion, nombre);
	FILE *fp = fopen(path, "w");
	if (fp != NULL) {
		fputs(datos_origen, fp);
		fclose(fp);
	}
	log_info(logWorker, "[guardar_datos_origen]: Path datos guardados: %s", path);
	return path;
}

char* leer_bloque(int numeroBloque, int cantBytes) {
	log_info(logWorker, "[leer_bloque]: Numero de bloque: %d - Cantidad de bytes: %d", numeroBloque, cantBytes);
	FILE* archivo;
	archivo = fopen(datosConfigWorker[RUTA_DATABIN], "r");
	int tamanioBloque = 1048576;
	char* buffer = malloc(cantBytes);
	int posicion = numeroBloque * tamanioBloque;
	fseek(archivo, posicion, SEEK_SET);
	fread(buffer, cantBytes, 1, archivo);
	fclose(archivo);
	log_info(logWorker, "%d Bytes leidos en el bloque %d\n", cantBytes, numeroBloque);
	return buffer;
}

char* crear_comando_transformacion(char* path_script_transformacion, char* path_datos_origen, char* archivo_temporal) {
	char* comando = string_from_format("chmod +x %s && cat %s | %s | sort > %s/%s", path_script_transformacion, path_datos_origen, path_script_transformacion, carpeta_resultado_transformacion, archivo_temporal);
	return comando;
}

int transformacion(char* path_script, int origen, int bytesOcupados, char* destino) {
	char* bloque = leer_bloque(origen, bytesOcupados);
	if (bloque == NULL) {
		log_error(logWorker, "No se pudo leer el bloque (numero %d) completo segun el tamaño especificado (%d bytes)", origen, bytesOcupados);
		return -1;
	}

	char* path_datos_origen = guardar_datos_origen(bloque, destino); //revisar

	FILE* temporal;
	temporal = fopen(destino, "w");
	fclose(temporal);

	char* comando = crear_comando_transformacion(path_script, path_datos_origen, destino);
	log_info(logWorker, "[transformacion] El comando a ejecutar es %s", comando);

	int resultado = ejecutar_system(comando);
	log_trace(logWorker, "[transformacion] El resultado fue: %d", resultado);

	if (resultado < 0) {
		log_error(logWorker, "No se pudo transformar y ordenar el bloque solicitado.");
	} else {
		log_trace(logWorker, "Se pudo transformar y ordenar correctamente el bloque solicitado.");
	}
	return resultado;
}

void transformacion_worker(int headerId, int socketCliente) {
	int resultado;

	log_trace(logWorker, "Entrando en transformacion...");
	int cantidadMensajes = protocoloCantidadMensajes[headerId]; //averigua la cantidad de mensajes que le van a llegar
	char **arrayMensajes = deserializarMensaje(socketCliente, cantidadMensajes); //recibe los mensajes en un array de strings
	char *transformadorString = malloc(string_length(arrayMensajes[0]) + 1);
	strcpy(transformadorString, arrayMensajes[0]);
	int bloque = atoi(arrayMensajes[1]);
	int bytesOcupados = atoi(arrayMensajes[2]);
	char* temporalDestino = malloc(string_length(arrayMensajes[3]) + 1);
	strcpy(temporalDestino, arrayMensajes[3]);
	log_info(logWorker, "Datos recibidos: Largo Transformador: %d - Socket: %d - Bloque: %d - Bytes ocupados: %d - Temporal: %s", string_length(transformadorString), socketCliente, bloque, bytesOcupados, temporalDestino);

	liberar_array(arrayMensajes, cantidadMensajes);

	//guardo datos y scripts en /tmpTransformacion
	char* path_script = guardar_script_transformacion(transformadorString, temporalDestino);
	log_trace(logWorker, "Script guardado. Path script: %s", path_script);

	//guardo archivos transformados en /resultadoTransformacion
	resultado = transformacion(path_script, bloque, bytesOcupados, temporalDestino);
	log_info(logWorker, "El resultado de la transformacion fue: %d", resultado);

	if (resultado == 0) {
		log_trace(logWorker, "Enviando header de TRANSFORMACION_OK");
		enviarHeaderSolo(socketCliente, TIPO_MSJ_TRANSFORMACION_OK);
	} else {
		log_error(logWorker, "Enviando header de TRANSFORMACION_ERROR");
		enviarHeaderSolo(socketCliente, TIPO_MSJ_TRANSFORMACION_ERROR);
	}

	free(transformadorString);
	free(temporalDestino);

}

//---------------------- FUNCIONES REDUCCION LOCAL ----------------------

/*
 int apareo_archivos(char* path_f1, char* path_f2) { //FALTA ARREGLAR QUE DEJA UNA LINEA EN BLANCO AL PRINCIPIO CUANDO EL ARCHIVO ESTA VACIO
 //log_info(logWorker, "Entre a funcion apareo. Archivo 1: %s. Archivo 2: %s", path_f1, path_f2);
 FILE *fr1, *fr2, *faux;

 char* fst = string_new();
 char* snd = string_new();
 char* thrd = string_new();
 char* frth = string_new();

 int comparacion;
 bool f1 = true, f2 = true;

 fr1 = fopen(path_f1, "r");
 fr2 = fopen(path_f2, "r");
 faux = fopen(path_f1, "r+");

 while (!feof(fr1) && !feof(fr2)) {
 if (f1) {
 fgets(fst, 1000, fr1);
 thrd = string_duplicate(fst);
 string_to_lower(thrd);
 //printf("primer if\n");
 }
 if (f2) {
 fgets(snd, 1000, fr2);
 frth = string_duplicate(snd);
 string_to_lower(frth);
 //printf("segundo if\n");
 }
 comparacion = strcmp(thrd, frth);
 //printf("comparacion: %d\n", comparacion);
 if (comparacion == 0) {
 f1 = true;
 f2 = true;
 fputs(fst, faux);
 fputs(snd, faux);
 } else if (comparacion > 0) {
 f1 = false;
 f2 = true;
 fputs(snd, faux);
 } else {
 f1 = true;
 f2 = false;
 fputs(fst, faux);
 }
 }
 printf("sali del while\n");
 fwrite("\n", 1, 1, faux);
 if (!feof(fr1)) {
 fputs(fst, faux);
 }
 else {
 fputs(snd, faux);
 }
 while (!feof(fr1)) {
 fgets(fst, 1000, fr1);
 fputs(fst, faux);
 }
 while (!feof(fr2)) {
 fgets(snd, 1000, fr2);
 fputs(snd, faux);
 }
 //free(fst);
 //free(snd);
 //free(thrd);
 //free(frth);
 fclose(fr1);
 fclose(fr2);
 fclose(faux);
 //log_info(logWorker, "salgo funcion apareo");
 return 0;
 }
 */

char* crear_comando_apareo(char* archivos_a_mergear, char* nombre_mergeado) {
	char* comando = string_from_format("sort -m %s> %s", archivos_a_mergear, nombre_mergeado);
	return comando;
}

int apareo(char** arrayTemporales, int cantTemporales, char* carpetaOrigen, char* pathDestino) {
	int i;
	char* stringArchivosAAparear = string_new();
	for (i = 0; i < cantTemporales; i++) {
		string_append_with_format(&stringArchivosAAparear, "%s/%s ", carpetaOrigen, arrayTemporales[i]);
	}
	char* comando = crear_comando_apareo(stringArchivosAAparear, pathDestino);
	int resultado_apareo = ejecutar_system(comando);
	return resultado_apareo;
}

char* crear_comando_reduccion(char* path_script_reduccionLoc, char* path_origen, char* archivo_destino) {
	char* comando = string_from_format("chmod +x %s && cat %s | %s > %s", path_script_reduccionLoc, path_origen, path_script_reduccionLoc, archivo_destino);
	return comando;
}

int reduccion(char* path_script, char* path_origen, char* path_destino) {
	char* comando = crear_comando_reduccion(path_script, path_origen, path_destino);
	log_info(logWorker, "[reduccion] El comando a ejecutar es %s", comando);

	int resultado = ejecutar_system(comando);
	log_info(logWorker, "[reduccion] El resultado es: %d", resultado);
	if (resultado < 0) {
		log_error(logWorker, "No se pudo reducir el bloque solicitado.");
	} else {
		log_trace(logWorker, "Se pudo reducir correctamente el bloque solicitado.");
	}
	return resultado;
}

void reduccion_local_worker(int headerId, int socketCliente) {
	//aparear archivos
	//ejecutar reductor
	//guardar resultado en el temporal que me pasa master (arrayMensajes[2])

	log_trace(logWorker, "Entrando en Reduccion Local...");
	int resultado;
	int i, j;

	int cantidadMensajes = protocoloCantidadMensajes[headerId]; //averigua la cantidad de mensajes que le van a llegar
	char **arrayMensajes = deserializarMensaje(socketCliente, cantidadMensajes); //recibe los mensajes en un array de strings

	char *reductorString = malloc(string_length(arrayMensajes[0]) + 1);
	strcpy(reductorString, arrayMensajes[0]);
	log_info(logWorker, "[Reduccion Local] Recibi reductor: %s", reductorString);

	int cantTemporales;
	cantTemporales = atoi(arrayMensajes[1]);
	log_info(logWorker, "[Reduccion Local] Recibi cantidad de temporales: %d", cantTemporales);

	liberar_array(arrayMensajes, cantidadMensajes);

	char **arrayTemporales = deserializarMensaje(socketCliente, cantTemporales);
	char **arrayTempDestino = deserializarMensaje(socketCliente, 1);
	char *temporalDestino = malloc(string_length(arrayTempDestino[0]));
	strcpy(temporalDestino, arrayTempDestino[0]);
	log_info(logWorker, "[Reduccion Local] Recibi temporal destino: %s", temporalDestino);

	liberar_array(arrayTempDestino, 1);

	//guardo scripts en /tmpReduccionLocal
	char* path_script = guardar_script_reduccion_local(reductorString, temporalDestino);

	//guardo archivos apareados en /tmpReduccionLocal
	char* path_apareado = string_from_format("%s/origen_%s", carpeta_temporal_reduccion_local, temporalDestino);
	log_info(logWorker, "[Reduccion local] Empezando apareo");
	int resultado_apareo = apareo(arrayTemporales, cantTemporales, carpeta_resultado_transformacion, path_apareado);
	log_info(logWorker, "[Reduccion local] Termine apareo, resultado = %d", resultado_apareo);

	liberar_array(arrayTemporales, cantTemporales);

	//guardo archivos resultado en /resultadoReduccionLocal
	char* path_temporal_destino = string_from_format("%s/%s", carpeta_resultado_reduccion_local, temporalDestino);
	resultado = reduccion(path_script, path_apareado, path_temporal_destino);

	free(reductorString);
	free(temporalDestino);

	if (resultado >= 0) {
		log_trace(logWorker, "[Reduccion local] Enviando mensaje de MSJ_REDUCC_LOCAL_OK");
		enviarHeaderSolo(socketCliente, TIPO_MSJ_REDUCC_LOCAL_OK);
	} else {
		log_error(logWorker, "[Reduccion local] Enviando mensaje de MSJ_REDUCC_LOCAL_ERROR");
		enviarHeaderSolo(socketCliente, TIPO_MSJ_REDUCC_LOCAL_ERROR);
	}
}

//---------------------- FUNCIONES REDUCCION GLOBAL ----------------------

typedef struct filaReduccionGlobal {
	int nodo;
	char ip[LARGO_IP];
	char puerto[LARGO_PUERTO + 1];
	char temporalReduccionLocal[LARGO_TEMPORAL];
} filaReduccionGlobal;

void liberar_estructura(filaReduccionGlobal estructura[], int cantidadElementos) {
	int i;
	for (i = 0; i < cantidadElementos; i++) {
		//free(estructura[i].nodo);
		free(estructura[i].ip);
		free(estructura[i].puerto);
		free(estructura[i].temporalReduccionLocal);
	}
	free(estructura);
}

int conectarAWorker(char* ip, char* puerto) {
	log_trace(logWorker, "Conexión a Worker, IP: %s, Puerto: %s", ip, puerto);
	int socketWorker = conectarA(ip, puerto);
	if (socketWorker < 0) {
		log_error(logWorker, "Fallo la conexión a Worker, IP: %s, Puerto: %s", ip, puerto);
	}
	return socketWorker;
}

int32_t handshakeWorker(int socketWorker) {
	int cantStringsHandshake = protocoloCantidadMensajes[TIPO_MSJ_HANDSHAKE];
	char **arrayMensajesHandshake = malloc(sizeof(char*) * cantStringsHandshake);
	char *mensaje = intToArrayZerosLeft(NUM_PROCESO_WORKER, 4);
	arrayMensajesHandshake[0] = malloc(string_length(mensaje) + 1);
	strcpy(arrayMensajesHandshake[0], mensaje);
	char *mensajeSerializadoHS = serializarMensaje(TIPO_MSJ_HANDSHAKE, arrayMensajesHandshake, cantStringsHandshake);
	enviarMensaje(socketWorker, mensajeSerializadoHS);

	liberar_array(arrayMensajesHandshake, cantStringsHandshake);
	free(mensajeSerializadoHS);

	//recibe y devuelve la respuesta del otro worker
	return deserializarHeader(socketWorker);

}

void recibirTablaReduccionGlobal(filaReduccionGlobal* datosReduccionGlobal, int socketMaster, int cantNodos) {
	int bytesEnviados, i;
	int cantMensajesXFila = 4;
	int cantStrings = cantMensajesXFila * cantNodos;
	char **arrayTablaReduccionGlobal = deserializarMensaje(socketMaster, cantStrings);

	//recibir la tabla de reduccion global
	log_info(logWorker, "\n ---------- Tabla de reduccion global ---------- \n");
	log_info(logWorker, "\tNodo\tIP\t\tPuerto\t\tTemporal\n");
	log_info(logWorker, "---------------------------------------------------------------------------------------------\n");
	int j = 0;
	for (i = 0; i < cantNodos; i++) {
		// cada msje es una fila de la tabla reduccion global
		datosReduccionGlobal[i].nodo = atoi(arrayTablaReduccionGlobal[j]);
		j++;
		strcpy(datosReduccionGlobal[i].ip, arrayTablaReduccionGlobal[j]);
		j++;
		strcpy(datosReduccionGlobal[i].puerto, arrayTablaReduccionGlobal[j]);
		j++;
		strcpy(datosReduccionGlobal[i].temporalReduccionLocal, arrayTablaReduccionGlobal[j]);
		j++;
		log_info(logWorker, "\t%d\t%s\t%s\t%s\n", datosReduccionGlobal[i].nodo, datosReduccionGlobal[i].ip, datosReduccionGlobal[i].puerto, datosReduccionGlobal[i].temporalReduccionLocal);
	}

	liberar_array(arrayTablaReduccionGlobal, cantStrings);
}

void enviarPathTemporal(int socketWorker, char* nombreArchivo) {
	//arma el array de strings para serializar
	char **arrayMensajes = malloc(sizeof(char*));
	arrayMensajes[0] = malloc(string_length(nombreArchivo) + 1);
	strcpy(arrayMensajes[0], nombreArchivo);
	//serializa los mensajes y los envía
	int cantidadMensajes = protocoloCantidadMensajes[TIPO_MSJ_PATH_ARCHIVO];
	char *mensajeSerializado = serializarMensaje(TIPO_MSJ_PATH_ARCHIVO, arrayMensajes, cantidadMensajes);
	liberar_array(arrayMensajes, cantidadMensajes);

	int bytesEnviados = enviarMensaje(socketWorker, mensajeSerializado); //envio el mensaje serializado al worker
	//log_info(logWorker, "Mensaje almacenamiento final serializado: %s",mensajeSerializado);
	if (bytesEnviados > 0) {
		log_trace(logWorker, "[reduccion_global]: Envie path del archivo de reduccion local");
	} else {
		log_error(logWorker, "[reduccion_global]: Error al enviar path del archivo de reduccion local");
	}

	free(mensajeSerializado);

	//return bytesEnviados;
}

void traer_temporal_worker(int socketWorker, char* nombreArchivo) {
	enviarPathTemporal(socketWorker, nombreArchivo);
	int32_t headerId = deserializarHeader(socketWorker);
	int cantidadMensajes = protocoloCantidadMensajes[headerId];
	char **arrayArchivo = deserializarMensaje(socketWorker, cantidadMensajes);
	printf("array recibido: %s\n", arrayArchivo[0]);
	char* path_datos = guardar_datos(arrayArchivo[0], carpeta_resultado_reduccion_local, nombreArchivo);
	liberar_array(arrayArchivo, cantidadMensajes);
	puts("ya libero array");
}

int enviar_contenido_archivo(int socketCliente, char* archivo) {
	log_info(logWorker, "[enviar_contenido_archivo] Archivo: %s", archivo);
	char* pathArchivo = string_new();
	string_append_with_format(&pathArchivo, "%s/%s", carpeta_resultado_reduccion_local, archivo);
	log_info(logWorker, "[enviar_contenido_archivo] Path archivo: %s", pathArchivo);
	char* contenidoArchivo = leerArchivo2(pathArchivo);
	//char* contenidoArchivo = "materia de mierda\n";

	char **arrayArchivo = malloc(sizeof(char*));
	arrayArchivo[0] = malloc(string_length(contenidoArchivo) + 1);
	strcpy(arrayArchivo[0], contenidoArchivo);

	//serializa los mensajes y los envía
	char *mensajeSerializado = serializarMensaje(TIPO_MSJ_CONTENIDO_ARCHIVO_REDUC_GLOBAL, arrayArchivo, 1);
	liberar_array(arrayArchivo, 1);

	int bytesEnviados = enviarMensaje(socketCliente, mensajeSerializado);//envio el mensaje serializado al worker
	log_trace(logWorker, "[reduccion_global]: Envie contenido del archivo al worker encargado");

	free(mensajeSerializado);

	return bytesEnviados;
}

void reduccion_global_worker(int headerId, int socketCliente) {
	//recibir archivos temporales de los demas workers
	//aparear archivos
	//ejecutar reductor
	//guardar resultado en el temporal que me pasa master

	log_trace(logWorker, "Entrando en Reduccion Global");
	crear_carpeta(carpeta_temporal_reduccion_global, logWorker);
	crear_carpeta(carpeta_resultado_reduccion_global, logWorker);

	int resultado;
	int i, j;

	int cantidadMensajes = protocoloCantidadMensajes[headerId]; //averigua la cantidad de mensajes que le van a llegar
	char **arrayMensajes = deserializarMensaje(socketCliente, cantidadMensajes); //recibe los mensajes en un array de strings

	char *reductorString = malloc(string_length(arrayMensajes[0]) + 1);
	strcpy(reductorString, arrayMensajes[0]);
	log_info(logWorker, "Largo reductor string: %d", string_length(reductorString));

	int cantWorkers;
	cantWorkers = atoi(arrayMensajes[1]);
	log_info(logWorker, "Cantidad de workers a conectarse: %d", cantWorkers);

	liberar_array(arrayMensajes, cantidadMensajes);

	filaReduccionGlobal datosReduccionGlobal[cantWorkers];
	recibirTablaReduccionGlobal(datosReduccionGlobal, socketCliente, cantWorkers);

	char **arrayArchDestino = deserializarMensaje(socketCliente, 1);
	char *archivoDestino = malloc(string_length(arrayArchDestino[0]));
	strcpy(archivoDestino, arrayArchDestino[0]);

	liberar_array(arrayArchDestino, 1);

	//guardo scripts en /tmpReduccionGlobal
	char* path_script = guardar_script_reduccion_global(reductorString, archivoDestino);
	free(reductorString);

	char** arrayTemporales = malloc(sizeof(char*) * cantWorkers);

	//guardo archivos de reduccion local de todos los nodos en /resultadoReduccionLocal
	for (i = 1; i < cantWorkers; i++) {
		int socketWorker = conectarAWorker(datosReduccionGlobal[i].ip, datosReduccionGlobal[i].puerto);
		int32_t headerIdWorker = handshakeWorker(socketWorker);
		if (headerIdWorker != TIPO_MSJ_HANDSHAKE_RESPUESTA_OK) {
			log_error(logWorker, "Error de handshake con el worker con IP: %s y Puerto: %s", datosReduccionGlobal[i].ip, datosReduccionGlobal[i].puerto);
		} else {
			log_trace(logWorker, "Conectado al worker con IP: %s y Puerto: %s", datosReduccionGlobal[i].ip, datosReduccionGlobal[i].puerto);
			traer_temporal_worker(socketWorker, datosReduccionGlobal[i].temporalReduccionLocal);
		}
	}

	//copio los nombres de los temporales a aparear para usarlos en la funcion apareo
	for (j = 0; j < cantWorkers; j++) {
		arrayTemporales[j] = malloc(string_length(datosReduccionGlobal[j].temporalReduccionLocal) + 1);
		strcpy(arrayTemporales[j], datosReduccionGlobal[j].temporalReduccionLocal);
		printf("nombre temporal: %s\n", arrayTemporales[j]);
	}
	//liberar_estructura(datosReduccionGlobal, cantWorkers);

	//guardo archivos apareados en /tmpReduccionGlobal
	char* path_apareado = string_from_format("%s/origen_%s", carpeta_temporal_reduccion_global, archivoDestino);
	log_info(logWorker, "[Reduccion global] Empezando apareo");
	int resultado_apareo = apareo(arrayTemporales, cantWorkers, carpeta_resultado_reduccion_local, path_apareado);
	liberar_array(arrayTemporales, cantWorkers);
	log_info(logWorker, "[Reduccion global] Termine apareo, resultado = %d", resultado_apareo);

	//guardo resultado destino en /resultadoReduccionGlobal
	char* path_destino = string_from_format("%s/%s", carpeta_resultado_reduccion_global, archivoDestino);
	free(archivoDestino);
	resultado = reduccion(path_script, path_apareado, path_destino);

	if (resultado >= 0) {
		log_trace(logWorker, "Enviando header de MSJ_REDUCC_GLOBAL_OK");
		enviarHeaderSolo(socketCliente, TIPO_MSJ_REDUCC_GLOBAL_OK);
	} else {
		log_error(logWorker, "Enviando header de MSJ_REDUCC_GLOBAL_ERROR");
		enviarHeaderSolo(socketCliente, TIPO_MSJ_REDUCC_GLOBAL_ERROR);
	}
}

//---------------------- FUNCIONES ALMACENAMIENTO FINAL ----------------------

int conexionAFileSystem() {
	log_info(logWorker, "Conexión a FileSystem, IP: %s, Puerto: %s", datosConfigWorker[FS_IP], datosConfigWorker[FS_PUERTO]);
	int socketFS = conectarA(datosConfigWorker[FS_IP], datosConfigWorker[FS_PUERTO]);
	if (socketFS < 0) {
		log_error(logWorker, "Filesystem not ready");
		//preparadoEnviarFs = handshakeClient(&datosConexionFileSystem, NUM_PROCESO_KERNEL);
	}
	return socketFS;
}

char* partirPath(char* path) {
	int i, cantidadStrings = 0;
	char* pathPartido = string_new();
	char* elemento = string_new();
	char** arrayDeStrings = string_split(path, "/");
	while(1) {
		elemento = arrayDeStrings[cantidadStrings];
		if (elemento == NULL) {
			break;
		}
		cantidadStrings += 1;
	}
	printf("%d\n", cantidadStrings);
	for (i = 0; i < cantidadStrings-1; i++) {
		printf("%s\n", arrayDeStrings[i]);
		string_append(&pathPartido, arrayDeStrings[i]);
		string_append(&pathPartido, "/");
	}
	return pathPartido;
}

int almacenamientoFinal(char* rutaArchivo, char* rutaFinal) {
	char* buffer = leerArchivo2(rutaArchivo);
	//printf("%s\n",buffer);
	int socketFS = conexionAFileSystem();
	int preparadoEnviarFs = 1;
	if (socketFS < 0) {
		preparadoEnviarFs = 0;
	}
	int modulo = worker;
	//se identifica con el FS
	send(socketFS, &modulo, sizeof(int), MSG_WAITALL);
	//arma el array de strings para serializar
	int cantidadMensajes = protocoloCantidadMensajes[TIPO_MSJ_WORKER_ALMACENAMIENTO_FINAL];
	char **arrayMensajes = malloc(sizeof(char*) * cantidadMensajes);
	arrayMensajes[0] = malloc(string_length(rutaFinal) + 1);
	strcpy(arrayMensajes[0], rutaFinal);
	arrayMensajes[1] = malloc(string_length(buffer) + 1);
	strcpy(arrayMensajes[1], buffer);
	char* pathIncompleto = partirPath(rutaFinal);
	arrayMensajes[2] = malloc(string_length(pathIncompleto) + 1);
	strcpy(arrayMensajes[2], pathIncompleto);
	//serializa los mensajes y los envía
	char *mensajeSerializado = serializarMensaje(TIPO_MSJ_WORKER_ALMACENAMIENTO_FINAL, arrayMensajes, cantidadMensajes);
	liberar_array(arrayMensajes, cantidadMensajes);

	int bytesEnviados = enviarMensaje(socketFS, mensajeSerializado); //envio el mensaje serializado a FS
	//log_info(logWorker, "Mensaje almacenamiento final serializado: %s",mensajeSerializado);
	log_trace(logWorker, "[almacenamiento_final]: Envie contenido del archivo a FS");

	int resultado;
	if (bytesEnviados == string_length(mensajeSerializado)) {
		resultado = 0;
	}
	else {
		resultado = -1;
	}
	free(mensajeSerializado);

	return resultado;
}

void almacenamiento_final_worker(int headerId, int socketCliente) {
	int resultado;

	log_trace(logWorker, "Entrando en almacenamiento final...");
	int cantidadMensajes = protocoloCantidadMensajes[headerId]; //averigua la cantidad de mensajes que le van a llegar
	char **arrayMensajes = deserializarMensaje(socketCliente, cantidadMensajes); //recibe los mensajes en un array de strings
	//char* archivoReducGlobal = malloc(string_length(arrayMensajes[0]) + 1);
	char* archivoReducGlobal = string_new();
	//strcpy(archivoReducGlobal, arrayMensajes[0]);
	string_append_with_format(&archivoReducGlobal, "%s/%s", carpeta_resultado_reduccion_global, arrayMensajes[0]);

	char* archivoFinal = malloc(string_length(arrayMensajes[1]) + 1);
	strcpy(archivoFinal, arrayMensajes[1]);

	log_info(logWorker, "Datos recibidos: Archivo Reduccion Global: %s - Archivo final: %s", archivoReducGlobal, archivoFinal);
	//printf("Datos recibidos\n");

	liberar_array(arrayMensajes, cantidadMensajes);

	resultado = almacenamientoFinal(archivoReducGlobal, archivoFinal);

	//free(archivoReducGlobal);
	free(archivoFinal);

	if (resultado >= 0) {
		log_trace(logWorker, "Enviando header de ALM_FINAL_OK");
		enviarHeaderSolo(socketCliente, TIPO_MSJ_ALM_FINAL_OK);
	} else {
		log_error(logWorker, "Enviando header de ALM_FINAL_ERROR");
		enviarHeaderSolo(socketCliente, TIPO_MSJ_ALM_FINAL_ERROR);
	}
}

//---------------------- FUNCIONES ZOMBIES ----------------------

void limpiarZombie(int sig) {
	int saved_errno = errno;
	while (waitpid((pid_t) (WAIT_ANY), NULO, WNOHANG) > NULO)
		;
	errno = saved_errno;
}

void detectarZombie() {

	struct sigaction sa;
	sa.sa_handler = &limpiarZombie;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
	if (sigaction(SIGCHLD, &sa, NULO) == ERROR) {
		perror(NULO);
		exit(EXIT_FAILURE);
	}
}

//---------------------- MAIN ----------------------

int main(int argc, char *argv[]) {
	//signal(SIGINT, configuracionSenial);
	//pidPadre = getpid();
	crearCarpetaDeLog(carpeta_log);
	logWorker = log_create("../log/logWorker.log", "WORKER", true, LOG_LEVEL_TRACE); //creo el logger, mostrando por pantalla

	printf("\n*** Proceso worker ***\n");
	log_trace(logWorker, "Iniciando Worker");

	// 1º) Leer archivo de config.
	if (leerArchivoConfig(nameArchivoConfig, keysConfigWorker, datosConfigWorker)) { //leerArchivoConfig devuelve 1 si hay error
		log_error(logWorker, "Hubo un error al leer el archivo de configuración.");
		return EXIT_FAILURE;
	}

	//Creo las carpetas de Transformación
	crear_carpeta(carpeta_temporal_transformacion, logWorker);
	crear_carpeta(carpeta_resultado_transformacion, logWorker);

	//Creo las carpetas de Reduccion Local
	crear_carpeta(carpeta_temporal_reduccion_local, logWorker);
	crear_carpeta(carpeta_resultado_reduccion_local, logWorker);

	// 2º) Inicializar server y aguardar conexiones (de master)
	int listenningSocket = inicializarServer(datosConfigWorker[IP_PROPIA], datosConfigWorker[PUERTO_PROPIO]);
	if (listenningSocket < 0) {
		log_error(logWorker, "No se pudo iniciar worker como servidor");
		return EXIT_FAILURE;
	}

	log_trace(logWorker, "Se inicio worker como server. IP: %s, Puerto: %s", datosConfigWorker[IP_PROPIA], datosConfigWorker[PUERTO_PROPIO]);

	while (1) {	//inicio bucle para recibir conexiones y forkear
		puts("\nYa estoy preparado para recibir conexiones\n-----------------------------------------\n");

		int socketCliente = aceptarConexion(listenningSocket);
		if (socketCliente < 0) {
			log_error(logWorker, "Hubo un error al aceptar conexiones");
			return EXIT_FAILURE;
		}

		log_trace(logWorker, "Proceso conectado con socket %d", socketCliente);

		pid_t pid;
		int status;
		pid = fork();
		if (pid < 0) {
			log_error(logWorker, "Error al forkear");
		} else if (pid == 0) { //aca ya se hizo el proceso hijo
			log_trace(logWorker, "Hijo creado");
			close(listenningSocket);

			char *argv[] = { NULL };
			char *envp[] = { NULL };

			/*
			 // ***** prueba de recepcion de mensaje sencillo de master *****
			 char lalala[6];
			 recibirMensaje(lalala, socketCliente, 6);
			 printf("%s\n", lalala);
			 */

			int32_t headerId = deserializarHeader(socketCliente);
			if (headerId == TIPO_MSJ_HANDSHAKE) {

				int cantidadMensajes = protocoloCantidadMensajes[headerId];
				char **arrayMensajesRHS = deserializarMensaje(socketCliente, cantidadMensajes);
				int idEmisorMensaje = atoi(arrayMensajesRHS[0]);
				liberar_array(arrayMensajesRHS, cantidadMensajes);

				if (idEmisorMensaje == NUM_PROCESO_MASTER) { //se me conecto un Master
					enviarHeaderSolo(socketCliente, TIPO_MSJ_HANDSHAKE_RESPUESTA_OK); //respondo al handshake
					log_trace(logWorker, "Master conectado, socket: %d", socketCliente);

					headerId = deserializarHeader(socketCliente); //recibe el id del header para saber qué esperar
					log_trace(logWorker, "HeaderId recibido: %d - HeaderId msje recibido: %s", headerId, protocoloMensajesPredefinidos[headerId]);

					/* *****Cantidad de mensajes segun etapa*****
					 * Transformacion (4): script, bloque (origen), bytesOcupados, temporal (destino)
					 * Reduccion local (3): script, lista de temporales (origen), temporal(destino)
					 * Reduccion global (4): script, lista de procesos Worker con sus IPs y Puertos, temporales de Reducción Local (origen), temporal (destino)
					 * Almacenam final (2): archivo reduc global (origen), nombre y ruta archivo final (destino)
					 */

					if (headerId == TIPO_MSJ_DATA_TRANSFORMACION_WORKER) {
						transformacion_worker(headerId, socketCliente);
					}

					if (headerId == TIPO_MSJ_DATA_REDUCCION_LOCAL_WORKER) {
						reduccion_local_worker(headerId, socketCliente);
					}

					if (headerId == TIPO_MSJ_DATA_REDUCCION_GLOBAL_WORKER) {
						reduccion_global_worker(headerId, socketCliente);
					}

					if (headerId == TIPO_MSJ_DATA_ALMACENAMIENTO_FINAL_WORKER) {
						almacenamiento_final_worker(headerId, socketCliente);
					}
				}

				if (idEmisorMensaje == NUM_PROCESO_WORKER) { //se me conectó un Worker
					enviarHeaderSolo(socketCliente, TIPO_MSJ_HANDSHAKE_RESPUESTA_OK); //respondo al handshake
					log_trace(logWorker, "Worker conectado, socket: %d", socketCliente);

					headerId = deserializarHeader(socketCliente); //recibe el id del header para saber qué esperar
					log_trace(logWorker, "HeaderId recibido de: %d - HeaderId msje recibido: %s", headerId, protocoloMensajesPredefinidos[headerId]);

					int cantidadMensajes = protocoloCantidadMensajes[headerId]; //averigua la cantidad de mensajes que le van a llegar
					char **arrayMensajes = deserializarMensaje(socketCliente, cantidadMensajes); //recibe los mensajes en un array de strings
					char *archivo = malloc(string_length(arrayMensajes[0]) + 1);
					strcpy(archivo, arrayMensajes[0]);
					log_info(logWorker, "[Reduccion global (server)] Nombre archivo a enviar: %s", archivo);

					liberar_array(arrayMensajes, cantidadMensajes);

					int resultado_envio = enviar_contenido_archivo(socketCliente, archivo);
					free(archivo);
					log_trace(logWorker, "[Reduccion global (server)] Bytes enviados = %d", resultado_envio);
				}
			}

			//close(socketCliente);

			exit(0);
			//aca termina el hijo
		} else {
			log_info(logWorker, "Pasó por el padre");
			close(socketCliente);
		}

	}

	log_info(logWorker, "Worker finalizado.");
	log_destroy(logWorker);
	return EXIT_SUCCESS;

}
