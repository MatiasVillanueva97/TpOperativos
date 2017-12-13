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

char* carpeta_log = "../log";
char* carpeta_temporal = "../tmp";
char* carpeta_resultados = "../resultados";
char* carpeta_temporales_reduccion = "../reduccionLocal";
char* carpeta_temporales_reduccionGlob = "../reduccionGlobal";


//---------------------- FUNCIONES GENERALES ----------------------
/*
 * Recibe el codigo a guardar y un string "nombre".
 * Guarda el codigo en un archivo llamado script_nombre, en la carpeta temporal
 */
char* guardar_script(char* codigo_script, char* nombre) {
	//log_info(logWorker, "[guardar_script]: Codigo recibido: %s", codigo_script);
	char* path = string_from_format("%s/script_%s", carpeta_temporal, nombre);
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
	}
	else {
		log_error(logWorker, "[guardar_datos] Hubo un error al abrir el archivo: %s", path);
	}
	log_info(logWorker, "[guardar_datos]: Path archivo guardado: %s", path);
	return path;
}

char* leerArchivo(char* pathArchivo) {
	char caracter;
	char* contenido = string_new();
	FILE* archivo = fopen(pathArchivo, "r");
	while(1) {

		caracter = fgetc(archivo);
		if( feof(archivo) ) {
			break ;
		}
		string_append_with_format(&contenido, "%c", caracter);
	}
	fclose(archivo);
	return contenido;
}


int ejecutar_system(char* comando) {
	log_info(logWorker, "[ejecutar_system]: Comando recibido: %s", comando);
	int status;
	system(comando);
	wait(&status); //pausa hasta que termina el hijo (system) y guarda el resultado en status
	if (WIFEXITED(status) == 0) {
		int exit_status = WEXITSTATUS(status);
		log_trace(logWorker, "System termino OK, el exit status del comando fue %d\n", exit_status);
		return 0;
	} else {
		//log_error(logWorker, "System fallo, el codigo de resultado fue: %d\n", resultado);
		log_error(logWorker, "System fallo\n");
		return -1;
	}
	return 0;
}


//---------------------- FUNCIONES TRANSFORMACION ----------------------

char* guardar_datos_origen(char* datos_origen, char* nombre) {
	//log_info(logWorker, "[guardar_datos_origen]: Datos recibidos: %s", datos_origen);
	char* path = string_from_format("%s/datos_%s", carpeta_temporal, nombre);
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
	log_info(logWorker, "%d bytes leidos en el bloque %d\n", cantBytes, numeroBloque);
	//log_info(logWorker, "[leer_bloque]: Datos leidos: %s", buffer);
	//printf("[leer_bloque]: Datos leidos: %s\n", buffer);
	return buffer;
}

char* crear_comando_transformacion(char* path_script_transformacion, char* path_datos_origen, char* archivo_temporal) {
	char* comando = string_from_format("chmod +x %s && cat %s | %s | sort > %s/%s", path_script_transformacion, path_datos_origen, path_script_transformacion, carpeta_resultados, archivo_temporal);
	return comando;
}

/*
char* crear_comando_transformacion(char* path_script_transformacion, char* datos_origen, char* archivo_temporal) {
	char* comando = string_from_format("chmod +x %s && echo \"%s\" | %s | sort > %s/%s", path_script_transformacion, datos_origen, path_script_transformacion, carpeta_resultados, archivo_temporal);
	return comando;
}
 */


int transformacion(char* path_script, int origen, int bytesOcupados, char* destino) {
	char* bloque = leer_bloque(origen, bytesOcupados);
	//char* bloque = "WBAN,Date,Time,StationType,SkyCondition,SkyConditionFlag,Visibility,VisibilityFlag,WeatherType,WeatherTypeFlag,DryBulbFarenheit,DryBulbFarenheitFlag,DryBulbCelsius,DryBulbCelsiusFlag,WetBulbFarenheit,WetBulbFarenheitFlag,WetBulbCelsius,WetBulbCelsiusFlag,DewPointFarenheit,DewPointFarenheitFlag,DewPointCelsius,DewPointCelsiusFlag,RelativeHumidity,RelativeHumidityFlag,WindSpeed,WindSpeedFlag,WindDirection,WindDirectionFlag,ValueForWindCharacter,ValueForWindCharacterFlag,StationPressure,StationPressureFlag,PressureTendency,PressureTendencyFlag,PressureChange,PressureChangeFlag,SeaLevelPressure,SeaLevelPressureFlag,RecordType,RecordTypeFlag,HourlyPrecip,HourlyPrecipFlag,Altimeter,AltimeterFlag\
	03011,20130101,0000,0,OVC, , 5.00, , , ,M, ,M, ,M, ,M, ,M, ,M, ,M, , 5, ,120, , , ,M, , , , , ,M, ,AA, , , ,29.93, ";
	if (bloque == NULL) {
		log_error(logWorker, "No se pudo leer el bloque (numero %d) completo segun el tamaño especificado (%d bytes)", origen, bytesOcupados);
		return -1;
	}

	char* path_datos_origen = guardar_datos_origen(bloque, destino); //revisar

	FILE* temporal;
	temporal = fopen(destino, "w");
	fclose(temporal);

	//char* comando = crear_comando_transformacion(path_script, bloque, destino);
	char* comando = crear_comando_transformacion(path_script, path_datos_origen, destino);
	log_info(logWorker, "[transformacion] El comando a ejecutar es %s", comando);

	int resultado = ejecutar_system(comando);
	log_trace(logWorker, "[transformacion] El resultado fue: %d", resultado);
	//free(bloque);
	if (resultado < 0) {
		log_error(logWorker, "No se pudo transformar y ordenar el bloque solicitado.");
	} else {
		log_trace(logWorker, "Se pudo transformar y ordenar correctamente el bloque solicitado.");
	}
	return resultado;
}


void transformacion_worker(int headerId, int socketCliente) {
	int resultado;

	log_trace(logWorker, "Entrando en transformacion");
	int cantidadMensajes = protocoloCantidadMensajes[headerId]; //averigua la cantidad de mensajes que le van a llegar
	char **arrayMensajes = deserializarMensaje(socketCliente, cantidadMensajes); //recibe los mensajes en un array de strings
	char *transformadorString = malloc(string_length(arrayMensajes[0]) + 1);
	strcpy(transformadorString, arrayMensajes[0]);
	int bloque = atoi(arrayMensajes[1]);
	int bytesOcupados = atoi(arrayMensajes[2]);
	char* temporalDestino = malloc(string_length(arrayMensajes[3]) + 1);
	strcpy(temporalDestino, arrayMensajes[3]);
	//log_info(logWorker,"Datos recibidos: Transformador %s\nSocket %d - Bloque %d - Bytes %d - Temporal %s", transformadorString, socketCliente, bloque, bytesOcupados, temporalDestino);
	log_info(logWorker,"Datos recibidos: Largo Transformador: %d - Socket: %d - Bloque: %d - Bytes ocupados: %d - Temporal: %s", string_length(transformadorString), socketCliente, bloque, bytesOcupados, temporalDestino);
	//printf("Datos recibidos\n");

	liberar_array(arrayMensajes, cantidadMensajes);

	char* path_script = guardar_script(transformadorString, temporalDestino);
	log_trace(logWorker,"Script guardado. Path script: %s", path_script);

	log_info(logWorker,"Antes de entrar a la funcion transformacion");
	resultado = transformacion(path_script, bloque, bytesOcupados, temporalDestino);
	log_info(logWorker, "El resultado de la transformacion fue: %d", resultado);

	free(transformadorString);
	free(temporalDestino);
	if (resultado == 0) {
		log_trace(logWorker, "Enviando header de TRANSFORMACION_OK");
		enviarHeaderSolo(socketCliente, TIPO_MSJ_TRANSFORMACION_OK);
	}
	else {
		log_error(logWorker, "Enviando header de TRANSFORMACION_ERROR");
		enviarHeaderSolo(socketCliente, TIPO_MSJ_TRANSFORMACION_ERROR);
	}
}


//---------------------- FUNCIONES REDUCCION LOCAL ----------------------

int apareo_archivos(char* path_f1, char* path_f2) { //FALTA ARREGLAR QUE DEJA UNA LINEA EN BLANCO AL PRINCIPIO CUANDO EL ARCHIVO ESTA VACIO
	FILE *fr1, *fr2, *faux;

	char* fst = string_new();
	char* snd = string_new();
	char* thrd = string_new();
	char* frth = string_new();

	int comparacion;
	bool f1 = true, f2 = true;

	faux = fopen(path_f1, "r+");
	fr1 = fopen(path_f1, "r");
	fr2 = fopen(path_f2, "r");

	while (!feof(fr1) && !feof(fr2)) {
		if (f1) {
			fgets(fst, 1000, fr1);
			thrd = string_duplicate(fst);
			string_to_lower(thrd);
		}
		if (f2) {
			fgets(snd, 1000, fr2);
			frth = string_duplicate(snd);
			string_to_lower(frth);
		}
		comparacion = strcmp(thrd, frth);
		if (comparacion == 0) {
			f1 = true;
			f2 = true;
			fwrite(fst,1,string_length(fst),faux);
			fwrite(snd,1,string_length(snd),faux);
		} else if (comparacion > 0) {
			f1 = false;
			f2 = true;
			fwrite (snd,1,string_length(snd),faux);
		} else {
			f2 = false;
			f1 = true;
			fwrite(fst, 1, string_length(fst), faux);
		}
	}
	fwrite("\n", 1, 1, faux);
	if (!feof(fr1)) {
		fwrite(fst, 1, string_length(fst), faux);
	}
	else {
		fwrite(snd, 1, string_length(snd), faux);
	}
	while (!feof(fr1)) {
		fgets(fst, 1000, fr1);
		fwrite(fst, 1, string_length(fst), faux);
	}
	while (!feof(fr2)) {
		fgets(snd, 1000, fr2);
		fwrite(snd, 1, string_length(snd), faux);
	}
	free(fst);
	free(snd);
	free(thrd);
	free(frth);
	fclose(fr1);
	fclose(fr2);
	fclose(faux);
	return 0;
}

char* crear_comando_reduccion(char* path_script_reduccionLoc, char* path_origen, char* archivo_destino) {
	char* comando = string_from_format("chmod +x %s && cat %s | %s > %s/%s", path_script_reduccionLoc, path_origen, path_script_reduccionLoc, carpeta_resultados, archivo_destino);
	return comando;
}

int reduccion(char* path_script, char* path_origen, char* path_destino) {
	char* comando = crear_comando_reduccion(path_script, path_origen, path_destino);
	log_info(logWorker, "[reduccion] El comando a ejecutar es %s", comando);

	int resultado = ejecutar_system(comando);
	log_info(logWorker, "[reduccion] El resultado es: %d", resultado);
	if (resultado < 0) {
		log_error(logWorker, "No se pudo transformar y ordenar el bloque solicitado.");
	} else {
		log_trace(logWorker, "Se pudo transformar y ordenar correctamente el bloque solicitado.");
	}
	return resultado;
}

void reduccion_local_worker(int headerId, int socketCliente) {
	//aparear archivos
	//ejecutar reductor
	//guardar resultado en el temporal que me pasa master (arrayMensajes[2])

	int resultado;
	int i;

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
	strcpy(temporalDestino,arrayTempDestino[0]);
	log_info(logWorker, "[Reduccion Local] Recibi temporal destino: %s", temporalDestino);

	liberar_array(arrayTempDestino, 1);

	char* path_script = guardar_script(reductorString, temporalDestino);

	char* path_apareado = string_from_format("%s/origen_%s", carpeta_temporal, temporalDestino);

	char* path_temporal_destino = string_from_format("%s/%s", carpeta_temporales_reduccion, temporalDestino);

	FILE *apareado = fopen(path_apareado, "w");
	fclose (apareado);
	log_info(logWorker, "[Reduccion Local] Cree archivo: %s", path_apareado);

	log_info(logWorker, "[Reduccion local] Empezando apareo");
	for (i = 0; i < cantTemporales; i++) {
		apareo_archivos(path_apareado,arrayTemporales[i]);
	}
	log_info(logWorker, "[Reduccion local] Termine apareo");

	resultado = reduccion(path_script, path_apareado, path_temporal_destino);

	free(reductorString);
	free(temporalDestino);

	if (resultado >= 0) {
		log_trace(logWorker, "[Reduccion local] Enviando mensaje de MSJ_REDUCC_LOCAL_OK");
		enviarHeaderSolo(socketCliente, TIPO_MSJ_REDUCC_LOCAL_OK);
	}
	else {
		log_error(logWorker, "[Reduccion local] Enviando mensaje de MSJ_REDUCC_LOCAL_ERROR");
		enviarHeaderSolo(socketCliente, TIPO_MSJ_REDUCC_LOCAL_ERROR);
	}
}



//---------------------- FUNCIONES REDUCCION GLOBAL ----------------------

typedef struct filaReduccionGlobal {
	int nodo;
	char ip[LARGO_IP];
	char puerto[LARGO_PUERTO];
	char temporalReduccionLocal[LARGO_TEMPORAL];
} filaReduccionGlobal;

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
	/*
	//recibir la tabla de reduccion global
	printf("\n ---------- Tabla de reduccion global ---------- \n");
	printf("\tNodo\tIP\t\tPuerto\t\tTemporal\n");
	printf("---------------------------------------------------------------------------------------------\n");
	*/
	for (i = 0; i < cantNodos; i++) {
		// cada msje es una fila de la tabla reduccion global
		datosReduccionGlobal[i].nodo = atoi(arrayTablaReduccionGlobal[0]);
		strcpy(datosReduccionGlobal[i].ip, arrayTablaReduccionGlobal[1]);
		strcpy(datosReduccionGlobal[i].puerto, arrayTablaReduccionGlobal[2]);
		strcpy(datosReduccionGlobal[i].temporalReduccionLocal, arrayTablaReduccionGlobal[3]);
		//printf("\t%d\t%s\t%d\t%s\n", datosReduccionGlobal[i].nodo, datosReduccionGlobal[i].ip, datosReduccionGlobal[i].puerto, datosReduccionGlobal[i].temporalReduccionGlobal);
	}
	//printf("\n");

	liberar_array(arrayTablaReduccionGlobal, cantStrings);
}

void enviarPathTemporal(int socketWorker, char* nombreArchivo){
	//arma el array de strings para serializar
	char **arrayMensajes = malloc(sizeof(char*));
	arrayMensajes[0] = malloc(string_length(nombreArchivo) + 1);
	strcpy(arrayMensajes[0], nombreArchivo);
	//serializa los mensajes y los envía
	int cantidadMensajes = protocoloCantidadMensajes[TIPO_MSJ_PATH_ARCHIVO];
	char *mensajeSerializado = serializarMensaje(TIPO_MSJ_PATH_ARCHIVO, arrayMensajes, cantidadMensajes);
	liberar_array(arrayMensajes, cantidadMensajes);

	int bytesEnviados = enviarMensaje(socketWorker, mensajeSerializado);//envio el mensaje serializado al worker
	//log_info(logWorker, "Mensaje almacenamiento final serializado: %s",mensajeSerializado);
	if (bytesEnviados > 0) {
		log_trace(logWorker, "[reduccion_global]: Envie path del archivo de reduccion local");
	}
	else {
		log_error(logWorker, "[reduccion_global]: Error al enviar path del archivo de reduccion local");
	}

	free(mensajeSerializado);

	//return bytesEnviados;
}

void traer_temporal_worker(int socketWorker, char* nombreArchivo) {
	enviarPathTemporal(socketWorker, nombreArchivo);
	char **arrayArchivo = deserializarMensaje(socketWorker, 1);
	char* path_datos = guardar_datos(arrayArchivo[0], carpeta_temporales_reduccionGlob, nombreArchivo);

	liberar_array(arrayArchivo, 1);
}

int enviar_contenido_archivo(int socketCliente, char* pathArchivo) {
	char* contenidoArchivo = leerArchivo(pathArchivo);

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

	crear_carpeta(carpeta_temporales_reduccionGlob, logWorker);

	int resultado;
	int i, j;

	int cantidadMensajes = protocoloCantidadMensajes[headerId]; //averigua la cantidad de mensajes que le van a llegar
	char **arrayMensajes = deserializarMensaje(socketCliente, cantidadMensajes); //recibe los mensajes en un array de strings

	char *reductorString = malloc(string_length(arrayMensajes[0]) + 1);
	strcpy(reductorString, arrayMensajes[0]);

	int cantWorkers;
	cantWorkers = atoi(arrayMensajes[1]);

	liberar_array(arrayMensajes, cantidadMensajes);

	filaReduccionGlobal datosReduccionGlobal[cantWorkers];
	recibirTablaReduccionGlobal(datosReduccionGlobal, socketCliente, cantWorkers);

	char **arrayArchDestino = deserializarMensaje(socketCliente, 1);
	char *archivoDestino = malloc(string_length(arrayArchDestino[0]));
	strcpy(archivoDestino,arrayArchDestino[0]);

	liberar_array(arrayArchDestino, 1);

	char* path_script = guardar_script(reductorString, archivoDestino);

	char* path_apareado = string_from_format("%s/origen_%s", carpeta_temporales_reduccionGlob, archivoDestino);

	FILE *apareado = fopen(path_apareado, "w");
	fclose (apareado);

	char* path_destino = string_from_format("%s/%s", carpeta_temporales_reduccionGlob, archivoDestino);

	for (i = 0; i < cantWorkers; i++) {
		int socketWorker = conectarAWorker(datosReduccionGlobal[i].ip, datosReduccionGlobal[i].puerto);
		int32_t headerIdWorker = handshakeWorker(socketWorker);
		if (headerIdWorker != TIPO_MSJ_HANDSHAKE_RESPUESTA_OK) {
			log_error(logWorker, "Error de handshake con el worker con IP: %s y Puerto: %s", datosReduccionGlobal[i].ip, datosReduccionGlobal[i].puerto);
		} else {
			log_trace(logWorker, "Conectado al worker con IP: %s y Puerto: %s", datosReduccionGlobal[i].ip, datosReduccionGlobal[i].puerto);
			traer_temporal_worker(socketWorker, datosReduccionGlobal[i].temporalReduccionLocal);
			apareo_archivos(path_apareado,datosReduccionGlobal[i].temporalReduccionLocal);
		}
	}

	resultado = reduccion(path_script, path_apareado, path_destino);

	free(reductorString);
	free(archivoDestino);

	if (resultado >= 0) {
		enviarHeaderSolo(socketCliente, TIPO_MSJ_REDUCC_GLOBAL_OK);
	}
	else {
		enviarHeaderSolo(socketCliente, TIPO_MSJ_REDUCC_GLOBAL_OK);
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

int almacenamientoFinal(char* rutaArchivo, char* rutaFinal){
	char* buffer = leerArchivo(rutaArchivo);
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
	char **arrayMensajes = malloc(sizeof(char*) * 2);
	arrayMensajes[0] = malloc(string_length(rutaFinal) + 1);
	strcpy(arrayMensajes[0], rutaFinal);
	arrayMensajes[1] = malloc(string_length(buffer) + 1);
	strcpy(arrayMensajes[1], buffer);
	//serializa los mensajes y los envía
	char *mensajeSerializado = serializarMensaje(TIPO_MSJ_WORKER_ALMACENAMIENTO_FINAL, arrayMensajes, 2);
	liberar_array(arrayMensajes, 2);

	int bytesEnviados = enviarMensaje(socketFS, mensajeSerializado);//envio el mensaje serializado a FS
	//log_info(logWorker, "Mensaje almacenamiento final serializado: %s",mensajeSerializado);
	log_trace(logWorker, "[almacenamiento_final]: Envie contenido del archivo a FS");

	free(mensajeSerializado);

	return bytesEnviados;
}

void almacenamiento_final_worker(int headerId, int socketCliente) {
	int resultado;

	log_trace(logWorker, "Entrando en almacenamiento final");
	int cantidadMensajes = protocoloCantidadMensajes[headerId]; //averigua la cantidad de mensajes que le van a llegar
	char **arrayMensajes = deserializarMensaje(socketCliente, cantidadMensajes); //recibe los mensajes en un array de strings
	char* archivoReducGlobal = malloc(string_length(arrayMensajes[0]) + 1);
	strcpy(archivoReducGlobal, arrayMensajes[0]);
	char* archivoFinal = malloc(string_length(arrayMensajes[1]) + 1);
	strcpy(archivoFinal, arrayMensajes[1]);

	log_info(logWorker,"Datos recibidos: Archivo Reduccion Global: %s - Archivo final: %s", archivoReducGlobal, archivoFinal);
	//printf("Datos recibidos\n");

	liberar_array(arrayMensajes, cantidadMensajes);

	resultado = almacenamientoFinal(archivoReducGlobal, archivoFinal);

	free(archivoReducGlobal);
	free(archivoFinal);

	if (resultado > 0) {
		log_trace(logWorker, "Enviando header de ALM_FINAL_OK");
		enviarHeaderSolo(socketCliente, TIPO_MSJ_ALM_FINAL_OK);
	}
	else {
		log_error(logWorker, "Enviando header de ALM_FINAL_ERROR");
		enviarHeaderSolo(socketCliente, TIPO_MSJ_ALM_FINAL_ERROR);
	}
}


//---------------------- FUNCIONES ZOMBIES ----------------------
void limpiarZombie(int sig) {
  int saved_errno = errno;
  while(waitpid((pid_t)(WAIT_ANY), NULO, WNOHANG) > NULO);
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

/*
 * ====================================MAIN====================================
 */
int main(int argc, char *argv[]) {
	//signal(SIGINT, configuracionSenial);
	//pidPadre = getpid();
	crearCarpetaDeLog(carpeta_log);
	logWorker = log_create("../log/logWorker.log", "WORKER", true, LOG_LEVEL_TRACE); //creo el logger, mostrando por pantalla

	log_trace(logWorker, "Iniciando Worker");
	printf("\n*** Proceso worker ***\n");

	// 1º) Leer archivo de config.
	if (leerArchivoConfig(nameArchivoConfig, keysConfigWorker, datosConfigWorker)) { //leerArchivoConfig devuelve 1 si hay error
		log_error(logWorker, "Hubo un error al leer el archivo de configuración.");
		return EXIT_FAILURE;
	}

	//Creo las carpetas
	crear_carpeta(carpeta_temporal, logWorker);
	crear_carpeta(carpeta_resultados, logWorker);
	crear_carpeta(carpeta_temporales_reduccion, logWorker);

	// 2º) Inicializar server y aguardar conexiones (de master)
	//HAY QUE VER COMO SE CONECTA CON OTROS WORKERS
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

				if (idEmisorMensaje == NUM_PROCESO_MASTER) {
					enviarHeaderSolo(socketCliente, TIPO_MSJ_HANDSHAKE_RESPUESTA_OK);
					log_trace(logWorker, "Master conectado, socket: %d", socketCliente);

					headerId = deserializarHeader(socketCliente); //recibe el id del header para saber qué esperar
					log_trace(logWorker,"Header: %d", headerId);

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

				if (idEmisorMensaje == NUM_PROCESO_WORKER) {
					enviarHeaderSolo(socketCliente, TIPO_MSJ_HANDSHAKE_RESPUESTA_OK); //respondo al handshake
					log_trace(logWorker, "Worker conectado, socket: %d", socketCliente);

					int32_t headerId = deserializarHeader(socketCliente); //recibe el id del header para saber qué esperar
					int cantidadMensajes = protocoloCantidadMensajes[headerId]; //averigua la cantidad de mensajes que le van a llegar
					char **arrayMensajes = deserializarMensaje(socketCliente, cantidadMensajes); //recibe los mensajes en un array de strings
					char *pathArchivo = malloc(string_length(arrayMensajes[0]) + 1);
					strcpy(pathArchivo, arrayMensajes[0]);

					liberar_array(arrayMensajes, cantidadMensajes);

					enviar_contenido_archivo(socketCliente, pathArchivo);

					free(pathArchivo);

				}
			}



			//close(socketCliente);

			exit(0);
			//aca termina el hijo
		} else {
			log_info(logWorker, "Pasó por el padre");
			close(socketCliente);
			waitpid(pid, &status, 0);
		}

		/*
		 3º) creo forks para ejecutar instrucciones de master
		 int pid = fork();
		 if (pid < 0) {
		 log_error(logWorker, "Error al forkear");
		 return EXIT_FAILURE;
		 } else if (pid == 0) {
		 log_info(logWorker, "Hijo creado");
		 printf("Soy el hijo y mi PID es %d\n", getpid());
		 sleep(1000);
		 // Ejecuto lo q me pidió el master con execv() o system()
		 execv("/bin/ls",NULL);
		 return EXIT_SUCCESS;
		 } else {
		 log_info(logWorker, "Hijo creado");
		 printf("Soy el padre y mi PID sigue siendo %d\n", getpid());
		 // Acá debería seguir escuchando conexiones

		 close(socketCliente);//cierro el socket en el padre para poder conectarlo con otro master
		 }
		 */
	}
	/*
	 wait();
	 */
	log_destroy(logWorker);
	return EXIT_SUCCESS;
}
