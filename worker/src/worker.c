/*
 ============================================================================
 Name        : worker.c
 Author      : Grupo 1234
 Description : Proceso Worker
 ============================================================================
 */

#include "../../utils/includes.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>

#define SIZE 1024 //tamaño para comunicaciones entre padre e hijos
//#define PACKAGESIZE 1024	// Define cual va a ser el size maximo del paquete a enviar

enum keys {
	IP_PROPIA, PUERTO_PROPIO, RUTA_DATABIN, FS_IP, FS_PUERTO, NOMBRE_NODO,
};
char* keysConfigWorker[] = { "IP_PROPIA", "PUERTO_PROPIO", "RUTA_DATABIN", "FS_IP", "FS_PUERTO", "NOMBRE_NODO", NULL };
char* datosConfigWorker[6];

t_log* logWorker;

char carpeta_temporal[6] = "../tmp";
//char carpeta_temporal[58] = "/home/utnso/workspace/tp-2017-2c-Mi-Grupo-1234/worker/tmp"
char carpeta_resultados[33] = "/home/utnso/Escritorio/resultados";

/*
 ================================================================
 Worker es el que realiza las operaciones que le pide el Master.
 1) Lee archivo de configuracion del nodo.
 2) Espera conexion de Masters.
 3) Recibe conexion del Master y se forkea
 4) El principal sigue escuchando, el fork ejecuta la orden
 5) Termina la orden, aviso a master que terminó y el fork muere
 Puede haber varios Worker corriendo al mismo tiempo.
 ================================================================
 */

void crear_carpeta_temporal() {
	struct stat st = {0};
	if (stat(carpeta_temporal, &st) == -1) {
		log_trace(logWorker, "Carpeta temporal no existe, creandola: %s", carpeta_temporal);
		//printf("Carpeta temporal no existe, creandola: %s\n", carpeta_temporal);
		mkdir(carpeta_temporal, 0775);
		}
}

char* guardar_script(char* codigo_script, char* nombre) {
	log_info(logWorker, "[guardar_script]: Codigo recibido: %s", codigo_script);
	char* path = malloc(string_length(carpeta_temporal) + string_length(nombre) + 8);
	path = string_from_format("%s/script_%s", carpeta_temporal, nombre);
	//path = string_from_format("/home/utnso/workspace/tp-2017-2c-Mi-Grupo-1234/worker/tmp/script_%s", nombre);
	FILE *fp = fopen(path, "w");
	if (fp != NULL) {
		fputs(codigo_script, fp);
		fclose(fp);
	}
	log_info(logWorker, "[guardar_script]: Path script guardado: %s", path);
	return path;
}

char* guardar_datos_origen(char* datos_origen, char* nombre) {
	log_info(logWorker, "[guardar_datos_origen]: Datos recibidos: %s", datos_origen);
	char* path = malloc(string_length(carpeta_temporal) + string_length(nombre) + 7);
	path = string_from_format("%s/datos_%s", carpeta_temporal, nombre);
	//path = string_from_format("/home/utnso/workspace/tp-2017-2c-Mi-Grupo-1234/worker/tmp/script_%s", nombre);
	FILE *fp = fopen(path, "w");
	if (fp != NULL) {
		fputs(datos_origen, fp);
		fclose(fp);
	}
	log_info(logWorker, "[guardar_datos_origen]: Path datos guardado: %s", path);
	return path;
}

/*char* getBloque(int idBloque){
	int fd;
	int tamanioBloque = 1048576;
	char *buffer = malloc(tamanioBloque);
	int posicion = idBloque * tamanioBloque;
	mapArchivo = mmap(0,tamanioBloque, PROT_READ, MAP_SHARED, fd, posicion);
	if(mapArchivo == MAP_FAILED){
		perror("mmap");
		close(fd);
		exit(1);
	}
	strncpy(buffer, mapArchivo,tamanioBloque);
	printf("bytes leidos en el bloque %i\n",idBloque);
	//printf("%s\n",buffer);
	return buffer;
}*/

char* leer_bloque(int numeroBloque, int cantBytes) {
	log_info(logWorker, "[leer_bloque]: Numero de bloque: %d - Cantidad de bytes: %d", numeroBloque, cantBytes);
	FILE* archivo;
	archivo = fopen(datosConfigWorker[RUTA_DATABIN], "r");
	int tamanioBloque = 1048576;
	char* buffer = malloc(cantBytes);
	int posicion = numeroBloque * tamanioBloque;
	fseek(archivo, posicion, SEEK_SET);
	fread(buffer, cantBytes, 1, archivo);
	//log_info(logWorker, "[leer_bloque]: Datos leidos: %s", buffer);
	log_info(logWorker, "%d bytes leidos en el bloque %d\n", cantBytes, numeroBloque);
	//printf("%d bytes leidos en el bloque %d\n", cantBytes, numeroBloque);
	//printf("[leer_bloque]: Datos leidos: %s\n", buffer);
	fclose(archivo);
	return buffer;
}

char* crear_comando_transformacion(char* path_script_transformacion, char* path_datos_origen, char* archivo_temporal) {
	char* comando = string_new();
	string_append_with_format(&comando, "chmod +x %s && cat %s | %s | sort > %s/%s", path_script_transformacion, path_datos_origen, path_script_transformacion, carpeta_resultados, archivo_temporal);
	return comando;
}

/*
char* crear_comando_transformacion(char* path_script_transformacion, char* datos_origen, char* archivo_temporal) {
	char* comando = string_new();
	string_append_with_format(&comando, "chmod +x %s && echo \"%s\" | %s | sort > %s/%s", path_script_transformacion, datos_origen, path_script_transformacion, carpeta_resultados, archivo_temporal);
	return comando;
}
*/

char* crear_comando_reduccionLoc(char* path_script_reduccionLoc, char* path_origen, char* archivo_destino) {
	char* comando = string_new();
	string_append_with_format(&comando, "chmod +x %s && cat %s | %s > %s/%s", path_script_reduccionLoc, path_origen, path_script_reduccionLoc, carpeta_resultados, archivo_destino);
	return comando;
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
		//log_error(logWorker, "La llamada system no termino normalmente... el codigo de resultado fue: %d\n", resultado);
		//log_error(logWorker, "System fallo, el codigo de resultado fue: %d\n", resultado);
		log_error(logWorker, "System fallo\n");
		return -1;
	}
	free(comando);
	return 0;
}

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

/*
int transformacion(char* path_script, int origen, int bytesOcupados, char* destino) {
	char* bloque = leer_bloque(origen, bytesOcupados);
	//char* bloque = "WBAN,Date,Time,StationType,SkyCondition,SkyConditionFlag,Visibility,VisibilityFlag,WeatherType,WeatherTypeFlag,DryBulbFarenheit,DryBulbFarenheitFlag,DryBulbCelsius,DryBulbCelsiusFlag,WetBulbFarenheit,WetBulbFarenheitFlag,WetBulbCelsius,WetBulbCelsiusFlag,DewPointFarenheit,DewPointFarenheitFlag,DewPointCelsius,DewPointCelsiusFlag,RelativeHumidity,RelativeHumidityFlag,WindSpeed,WindSpeedFlag,WindDirection,WindDirectionFlag,ValueForWindCharacter,ValueForWindCharacterFlag,StationPressure,StationPressureFlag,PressureTendency,PressureTendencyFlag,PressureChange,PressureChangeFlag,SeaLevelPressure,SeaLevelPressureFlag,RecordType,RecordTypeFlag,HourlyPrecip,HourlyPrecipFlag,Altimeter,AltimeterFlag\
03011,20130101,0000,0,OVC, , 5.00, , , ,M, ,M, ,M, ,M, ,M, ,M, ,M, , 5, ,120, , , ,M, , , , , ,M, ,AA, , , ,29.93, ";
	if (bloque == NULL) {
		log_error(logWorker, "No se pudo leer el bloque (numero %d) completo segun el tamaño especificado (%d bytes)", origen, bytesOcupados);
		return -1;
	}

	FILE* temporal;
	temporal = fopen(destino, "w");
	fclose(temporal);

	char* comando = crear_comando_transformacion(path_script, bloque, destino);
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
*/

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
		comparacion = strcmp(thrd,frth);
		if (comparacion == 0) {
			f1 = true;
			f2 = true;
			fwrite (fst,1,string_length(fst),faux);
			fwrite (snd,1,string_length(snd),faux);
		} else if (comparacion > 0) {
			f1 = false;
			f2 = true;
			fwrite (snd,1,string_length(snd),faux);
		} else {
			f2 = false;
			f1 = true;
			fwrite (fst,1,string_length(fst),faux);
		}
	}
	fwrite ("\n",1,1,faux);
	if (!feof(fr1)) {
		fwrite (fst,1,string_length(fst),faux);
	}
	else {
		fwrite (snd,1,string_length(snd),faux);
	}
	while (!feof(fr1)) {
		fgets(fst, 1000, fr1);
		fwrite (fst,1,string_length(fst),faux);
	}
	while (!feof(fr2)) {
		fgets(snd, 1000, fr2);
		fwrite (snd,1,string_length(snd),faux);
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

int reduccion_local(char* path_script, char* path_origen, char* path_destino) {

	char* comando = crear_comando_reduccionLoc(path_script, path_origen, path_destino);
	log_info(logWorker, "[reduccion_local] El comando a ejecutar es %s", comando);

	int resultado = ejecutar_system(comando);
	log_info(logWorker, "[reduccion_local] El resultado es: %d", resultado);
	if (resultado < 0) {
		log_error(logWorker, "No se pudo transformar y ordenar el bloque solicitado.");
		//printf("No se pudo transformar y ordenar el bloque solicitado.\n");
	} else {
		log_trace(logWorker, "Se pudo transformar y ordenar correctamente el bloque solicitado.");
		//printf("Se pudo transformar y ordenar correctamente el bloque solicitado.\n");
	}
	return resultado;
}

int conexionAFileSystem() {
	log_info(logWorker, "Conexión a FileSystem, IP: %s, Puerto: %s", datosConfigWorker[FS_IP], datosConfigWorker[FS_PUERTO]);
	int socketFS = conectarA(datosConfigWorker[FS_IP], datosConfigWorker[FS_PUERTO]);
	if (socketFS < 0) {
		puts("Filesystem not ready\n");
		//preparadoEnviarFs = handshakeClient(&datosConexionFileSystem, NUM_PROCESO_KERNEL);
	}
	return socketFS;
}

void almacenamientoFinal(char* rutaArchivo, char* rutaFinal){
	FILE* archivo = fopen(rutaArchivo, "r");
	int fd = fileno(archivo);
	int tamano;
	struct stat buff;
	fstat(fd, &buff);
	tamano = buff.st_size;
	char* buffer = malloc(tamano);
	fseek(archivo, 0, SEEK_SET);
	fread(buffer,tamano,1,archivo);
//	printf("%s\n",buffer);
	int socketFS; //= conectarA("127.0.0.1","5000");
	int preparadoEnviarFs = 1, i;
	if ((socketFS = conexionAFileSystem()) < 0) {
		preparadoEnviarFs = 0;
	}
	int modulo = worker;
	//se identifica con el FS
	send(socketFS, &modulo, sizeof(int), MSG_WAITALL);
	//arma el array de strings para serializar
	int bytesEnviados;
	char **arrayMensajes = malloc(sizeof(char*) * 2);
	arrayMensajes[0] = malloc(string_length(rutaFinal) + 1);
	strcpy(arrayMensajes[0], rutaFinal);
	arrayMensajes[1] = malloc(string_length(buffer) + 1);
	strcpy(arrayMensajes[1], buffer);
		//serializa los mensajes y los envía
	char *mensajeSerializado = serializarMensaje(TIPO_MSJ_WORKER_ALMACENAMIENTO_FINAL, arrayMensajes, 2);
	bytesEnviados = enviarMensaje(socketFS, mensajeSerializado);//envio el mensaje serializado a FS
	printf("%s\n",mensajeSerializado);
	//libera todos los pedidos de malloc
	for (i = 0; i < 2; i++) {
		free(arrayMensajes[i]);
	}
	free(arrayMensajes);
	free(buffer);
}
int main(int argc, char *argv[]) {
	//tamanioData = stat --format=%s "nombre archivo" //tamaño data.bin en bytes
	int i, j, k, h;
	logWorker = log_create("logFile.log", "WORKER", true, LOG_LEVEL_TRACE); //creo el logger, mostrando por pantalla

	log_info(logWorker, "Iniciando Worker");
	printf("\n*** Proceso worker ***\n");

	// 1º) leer archivo de config.
	char *nameArchivoConfig = "configNodo.txt";
	if (leerArchivoConfig(nameArchivoConfig, keysConfigWorker, datosConfigWorker)) { //leerArchivoConfig devuelve 1 si hay error
		log_error(logWorker, "Hubo un error al leer el archivo de configuración.");
		//printf("Hubo un error al leer el archivo de configuración.\n");
		return 0;
	}

	//almacenamientoFinal("/home/utnso/test.txt","/ruta/donde/almacena/fs/archivoFinal.loQueSea");

	// 2º) inicializar server y aguardar conexiones (de master)
	//HAY QUE VER COMO SE CONECTA CON OTROS WORKERS
	int listenningSocket = inicializarServer(datosConfigWorker[IP_PROPIA], datosConfigWorker[PUERTO_PROPIO]);
	if (listenningSocket < 0) {
		log_error(logWorker, "No se pudo iniciar worker como servidor");
		//puts("No pude iniciar como servidor\n");
		return EXIT_FAILURE;
	}
	log_trace(logWorker, "Se inicio worker como server. IP: %s, Puerto: %s", datosConfigWorker[IP_PROPIA], datosConfigWorker[PUERTO_PROPIO]);

	crear_carpeta_temporal();

	while (1) {	//inicio bucle para recibir conexiones y forkear
		puts("\nYa estoy preparado para recibir conexiones\n-----------------------------------------\n");

		int socketCliente = aceptarConexion(listenningSocket);
		if (socketCliente < 0) {
			log_error(logWorker, "Hubo un error al aceptar conexiones");
			//puts("Hubo un error al aceptar conexiones\n");
			return EXIT_FAILURE;
		}

		log_trace(logWorker, "Master conectado con socket %d", socketCliente);
		//printf("Master conectado con socket %d\n", socketCliente);

		pid_t pid;
		int status;
		//char* buffer=malloc(SIZE);
		pid = fork();
		if (pid < 0) {
			log_error(logWorker, "Error al forkear");
			//puts("Error al forkear");
		} else if (pid == 0) { //aca ya se hizo el proceso hijo
			log_trace(logWorker, "Hijo creado");
			//puts("Estoy dentro del hijo");
			close(listenningSocket);

			char *argv[] = { NULL };
			char *envp[] = { NULL };
			/*
			 // ***** prueba de recepcion de mensaje sencillo de master *****
			 char lalala[6];
			 recibirMensaje(lalala, socketCliente, 6);
			 printf("%s\n", lalala);
			 */

			int32_t headerId = deserializarHeader(socketCliente); //recibe el id del header para saber qué esperar
			log_trace(logWorker,"Header: %d", headerId);
			//printf ("Header: %d\n", headerId);

			/* *****Cantidad de mensajes segun etapa*****
			 Transformacion (4): script, bloque (origen), bytesOcupados, temporal (destino)
			 Reduccion local (3): script, lista de temporales (origen), temporal(destino)
			 Reduccion global (4): script, lista de procesos Worker con sus IPs y Puertos, temporales de Reducción Local (origen), temporal (destino)
			 Almacenam final (2): archivo reduc global (origen), nombre y ruta archivo final (destino)
			 */

			//char* comando = "echo hola pepe | ./script_transformacion.py > /tmp/resultado";
			int resultado;

			if (headerId == TIPO_MSJ_DATA_TRANSFORMACION_WORKER) {

				log_trace(logWorker, "Entrando en transformacion");
				//printf("Entrando en transformacion\n");
				int cantidadMensajes = protocoloCantidadMensajes[headerId]; //averigua la cantidad de mensajes que le van a llegar
				char **arrayMensajes = deserializarMensaje(socketCliente, cantidadMensajes); //recibe los mensajes en un array de strings
				char *transformadorString = malloc(string_length(arrayMensajes[0]) + 1);
				strcpy(transformadorString, arrayMensajes[0]);
				int bloque = atoi(arrayMensajes[1]);
				int bytesOcupados = atoi(arrayMensajes[2]);
				char* temporalDestino = malloc(string_length(arrayMensajes[3]) + 1);
				strcpy(temporalDestino, arrayMensajes[3]);
				log_info(logWorker,"Datos recibidos: Transformador %s\nSocket %d - Bloque %d - Bytes %d - Temporal %s", transformadorString, socketCliente, bloque, bytesOcupados, temporalDestino);
				//printf("Datos recibidos\n");

				for (i = 0; i < cantidadMensajes; i++) {
					free(arrayMensajes[i]);
				}
				free(arrayMensajes);

				char* path_script = guardar_script(transformadorString, temporalDestino);
				log_trace(logWorker,"Script guardado. Path script: %s", path_script);
				//printf("Script guardado. Path script: %s\n", path_script);
				log_info(logWorker,"Antes de entrar a la funcion transformacion");
				resultado = transformacion(path_script, bloque, bytesOcupados, temporalDestino);
				log_info(logWorker, "El resultado de la transformacion fue: %d", resultado);
				//printf("El resultado de la transformacion fue: %d\n", resultado);

				free(path_script);
				free(transformadorString);
				free(temporalDestino);
				if (resultado == 0) {
					log_trace(logWorker, "Enviando header de TRANSFORMACION_OK");
					//printf("Enviando header de OK\n");
					enviarHeaderSolo(socketCliente, TIPO_MSJ_TRANSFORMACION_OK);
				}
				else {
					//printf("Enviando header de ERROR\n");
					log_error(logWorker, "Enviando header de TRANSFORMACION_ERROR");
					enviarHeaderSolo(socketCliente, TIPO_MSJ_TRANSFORMACION_ERROR);
				}
			}

			if (headerId == TIPO_MSJ_DATA_REDUCCION_LOCAL_WORKER) {

				//aparear archivos
				//ejecutar reductor
				//guardar resultado en el temporal que me pasa master (arrayMensajes[2])

				int cantidadMensajes = protocoloCantidadMensajes[headerId]; //averigua la cantidad de mensajes que le van a llegar
				char **arrayMensajes = deserializarMensaje(socketCliente, cantidadMensajes); //recibe los mensajes en un array de strings

				char *reductorString = malloc(string_length(arrayMensajes[0]) + 1);
				strcpy(reductorString, arrayMensajes[0]);

				int cantTemporales;
				cantTemporales = atoi(arrayMensajes[1]);

				for (i = 0; i < cantidadMensajes; i++) {
					free(arrayMensajes[i]);
				}
				free(arrayMensajes);

				char **arrayTemporales = deserializarMensaje(socketCliente, cantTemporales);

				char **arrayTempDestino = deserializarMensaje(socketCliente, 1);
				char *temporalDestino = malloc(string_length(arrayTempDestino[0]));
				strcpy(temporalDestino,arrayTempDestino[0]);

				char* path_script = guardar_script(reductorString, temporalDestino);

				char* path_temporal_origen = string_new();
				string_append_with_format(&path_temporal_origen, "%s/origen_%s", carpeta_temporal, temporalDestino);


				char* path_temporales_reduccion = string_new();
				path_temporales_reduccion = "/home/utnso/Escritorio/reduccionLocal";
				char* path_temporal_destino = string_new();
				string_append_with_format(&path_temporal_destino, "%s/%s", path_temporales_reduccion, temporalDestino);

				FILE *temporalOrigenDestino = fopen(path_temporal_origen, "w");
				fclose (temporalOrigenDestino);

				for (i = 0; i < cantTemporales; i++) {
					apareo_archivos(path_temporal_origen,arrayTemporales[i]);
				}

				resultado = reduccion_local(path_script,path_temporal_origen,path_temporal_destino);

				free(path_script);
				free(reductorString);
				free(temporalDestino);
				if (resultado == 0) {
					enviarHeaderSolo(socketCliente, TIPO_MSJ_REDUCC_LOCAL_OK);
				}
				else {
					enviarHeaderSolo(socketCliente, TIPO_MSJ_REDUCC_LOCAL_ERROR);
				}
			}

			if (headerId == (int32_t) "REDUCCION_GLOBAL") {

			}

			if (headerId == (int32_t) "ALMACENAMIENTO_FINAL") {

			}

			//TODO: ??????????????????????? esto está bien???
			//porque lo único que hace es mandar un header=0............. raro....
			if (resultado == 0) {
				enviarHeaderSolo(socketCliente, resultado);
			}

			exit(0);
			//aca termina el hijo
		} else {
			puts("Pasó por el padre");
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
	return EXIT_SUCCESS;
}
