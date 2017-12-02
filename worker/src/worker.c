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

//#define PACKAGESIZE 1024	// Define cual va a ser el size maximo del paquete a enviar

enum keys {
	IP_PROPIA, PUERTO_PROPIO, RUTA_DATABIN
};
char* keysConfigWorker[] = { "IP_PROPIA", "PUERTO_PROPIO", "RUTA_DATABIN", NULL };
char* datosConfigWorker[3];

t_log* logWorker;

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

#define SIZE 1024 //tamaño para comunicaciones entre padre e hijos

struct stat st = {0};
char carpeta_temporal[6] = "../tmp";
//char carpeta_temporal[58] = "/home/utnso/workspace/tp-2017-2c-Mi-Grupo-1234/worker/tmp"

char* guardar_script(char* codigo_script, char* nombre) {
	char* path = malloc(string_length(carpeta_temporal) + string_length(nombre));
	path = string_from_format("%s/script_%s", carpeta_temporal, nombre);
	//path = string_from_format("/home/utnso/workspace/tp-2017-2c-Mi-Grupo-1234/worker/tmp/script_%s", nombre);
	FILE *fp = fopen(path, "w");
	if (fp != NULL) {
		fputs(codigo_script, fp);
		fclose(fp);
	}
	return path;
}

char* leer_bloque(int numeroBloque, int cantBytes) {
	FILE* archivo;
	archivo = fopen(datosConfigWorker[RUTA_DATABIN], "r");
	int tamanioBloque = 1048576;
	char *buffer[cantBytes];
	int posicion = numeroBloque * tamanioBloque;
	fseek(archivo, posicion, SEEK_SET);
	fread(buffer, cantBytes, 1, archivo);
	printf("%i bytes leidos en el bloque %i\n", cantBytes, numeroBloque);
	fclose(archivo);
	return *buffer;
}

int ejecutar_system(char* path_script, char* datos_origen, char* archivo_temporal) {

	char* comando = string_new();
	string_append_with_format(&comando, "chmod +x %s && echo %s | %s | sort > /home/utnso/Escritorio/resultados/%s", path_script, datos_origen, path_script, archivo_temporal);
	log_trace(logWorker, "El comando a ejecutar es %s", comando);
	//int resultado = system(comando);
	int status;
	system(comando);
	wait(&status); //pausa hasta que termina el hijo (system) y guarda el resultado en status
	/*if (WIFEXITED(status)) {
	 log_trace(logWorker, "System termino OK, el exit status del comando fue %d\n", WEXITSTATUS(status));
	 }
	 else {
	 //log_trace(logWorker, "La llamada system no termino normalmente... el codigo de resultado fue: %d\n", resultado);
	 log_trace(logWorker, "System fallo, el codigo de resultado fue: %d\n", resultado);
	 }*/
	free(comando);
	//return resultado;
	return status;
}

int system_transformacion(char* path_script_transformacion, char* datos_origen, char* archivo_temporal) {

	char* comando = string_new();
	string_append_with_format(&comando, "chmod +x %s && echo %s | %s | sort > /home/utnso/Escritorio/resultados/%s", path_script_transformacion, datos_origen, path_script_transformacion, archivo_temporal);
	log_trace(logWorker, "El comando a ejecutar es %s", comando);
	//int resultado = system(comando);
	int status;
	system(comando);
	wait(&status); //pausa hasta que termina el hijo (system) y guarda el resultado en status
	if (WIFEXITED(&status)) {
		int exit_status = WEXITSTATUS(&status);
		log_trace(logWorker, "System termino OK, el exit status del comando fue %d\n", exit_status);
		return 0;
	} else {
		//log_trace(logWorker, "La llamada system no termino normalmente... el codigo de resultado fue: %d\n", resultado);
		//log_trace(logWorker, "System fallo, el codigo de resultado fue: %d\n", resultado);
		log_trace(logWorker, "System fallo\n");
		return -1;
	}
	free(comando);
	//return resultado;
	//return status;
}

int transformacion(char* path_script, int origen, int bytesOcupados, char* destino) {
	//char* bloque = leer_bloque(origen, bytesOcupados); //implementar despues con mmap
	char* bloque = "WBAN,Date,Time,StationType,SkyCondition,SkyConditionFlag,Visibility,VisibilityFlag,WeatherType,WeatherTypeFlag,DryBulbFarenheit,DryBulbFarenheitFlag,DryBulbCelsius,DryBulbCelsiusFlag,WetBulbFarenheit,WetBulbFarenheitFlag,WetBulbCelsius,WetBulbCelsiusFlag,DewPointFarenheit,DewPointFarenheitFlag,DewPointCelsius,DewPointCelsiusFlag,RelativeHumidity,RelativeHumidityFlag,WindSpeed,WindSpeedFlag,WindDirection,WindDirectionFlag,ValueForWindCharacter,ValueForWindCharacterFlag,StationPressure,StationPressureFlag,PressureTendency,PressureTendencyFlag,PressureChange,PressureChangeFlag,SeaLevelPressure,SeaLevelPressureFlag,RecordType,RecordTypeFlag,HourlyPrecip,HourlyPrecipFlag,Altimeter,AltimeterFlag\
03011,20130101,0000,0,OVC, , 5.00, , , ,M, ,M, ,M, ,M, ,M, ,M, ,M, , 5, ,120, , , ,M, , , , , ,M, ,AA, , , ,29.93, ";
	if (bloque == NULL) {
		log_error(logWorker, "No se pudo leer el bloque (numero %d) completo segun el tamaño especificado (%d bytes)", origen, bytesOcupados);
		return -1;
	}
	FILE* temporal;
	temporal = fopen(destino, "w");
	fclose(temporal);

	int resultado = system_transformacion(path_script, bloque, destino);
	//script_destroy(archivo_temporal_bloque);
	//free(bloque);
	if (resultado < 0) {
		log_error(logWorker, "No se pudo transformar y ordenar el bloque solicitado.");
	} else {
		log_trace(logWorker, "Se pudo transformar y ordenar correctamente el bloque solicitado.");
	}
	return resultado;
}

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

int reduccion_local(char* path_script, int origen, int bytesOcupados, char* destino) {
	int resultado;
	return resultado;
}

int main(int argc, char *argv[]) {
	//tamanioData = stat --format=%s "nombre archivo" //tamaño data.bin en bytes
	int i, j, k, h;
	logWorker = log_create("logFile.log", "WORKER", false, LOG_LEVEL_TRACE); //creo el logger, sin mostrar por pantalla

	log_info(logWorker, "Iniciando Worker");
	printf("\n*** Proceso worker ***\n");

	// 1º) leer archivo de config.
	char *nameArchivoConfig = "configNodo.txt";
	if (leerArchivoConfig(nameArchivoConfig, keysConfigWorker, datosConfigWorker)) { //leerArchivoConfig devuelve 1 si hay error
		printf("Hubo un error al leer el archivo de configuración");
		return 0;
	}

	// 2º) inicializar server y aguardar conexiones (de master)
	//HAY QUE VER COMO SE CONECTA CON OTROS WORKERS
	int listenningSocket = inicializarServer(datosConfigWorker[IP_PROPIA], datosConfigWorker[PUERTO_PROPIO]);

	if (listenningSocket < 0) {
		log_error(logWorker, "No se pudo iniciar worker como servidor");
		puts("No pude iniciar como servidor");
		return EXIT_FAILURE;
	}



	if (stat(carpeta_temporal, &st) == -1) {
		mkdir(carpeta_temporal, 0775);
	}

	while (1) {	//inicio bucle para recibir conexiones y forkear
		puts("Ya estoy preparado para recibir conexiones\n");

		int socketCliente = aceptarConexion(listenningSocket);
		if (socketCliente < 0) {
			log_error(logWorker, "Hubo un error al aceptar conexiones");
			puts("Hubo un error al aceptar conexiones\n");
			return EXIT_FAILURE;
		}

		log_info(logWorker, "Master conectado al worker, esperando mensajes");
		printf("master conectado con socket %d\n", socketCliente);
		/*
		 // ***** PIPES: 0 lectura, 1 escritura *****
		 int pipe_padreAHijo[2];
		 int pipe_hijoAPadre[2];

		 pipe(pipe_padreAHijo);
		 pipe(pipe_hijoAPadre);
		 */

		pid_t pid;
		int status;
		//char* buffer=malloc(SIZE);
		pid = fork();
		if (pid < 0) {
			puts("Error al forkear");
		} else if (pid == 0) { //aca ya se hizo el proceso hijo
			puts("Estoy dentro del hijo");
			close(listenningSocket);

			/*
			 dup2(pipe_padreAHijo[0],STDIN_FILENO);
			 dup2(pipe_hijoAPadre[1],STDOUT_FILENO);
			 // ***** Cerramos pipes duplicados *****
			 close(pipe_padreAHijo[1]);
			 close(pipe_hijoAPadre[0]);
			 close(pipe_hijoAPadre[1]);
			 close(pipe_padreAHijo[0]);
			 */
			char *argv[] = { NULL };
			char *envp[] = { NULL };
			/*
			 // ***** prueba de recepcion de mensaje sencillo de master *****
			 char lalala[6];
			 recibirMensaje(lalala, socketCliente, 6);
			 printf("%s\n", lalala);
			 */

			int32_t headerId = deserializarHeader(socketCliente); //recibe el id del header para saber qué esperar

			/* *****Cantidad de mensajes segun etapa*****
			 Transformacion (4): script, bloque (origen), bytesOcupados, temporal (destino)
			 Reduccion local (3): script, lista de temporales (origen), temporal(destino)
			 Reduccion global (4): script, lista de procesos Worker con sus IPs y Puertos, temporales de Reducción Local (origen), temporal (destino)
			 Almacenam final (2): archivo reduc global (origen), nombre y ruta archivo final (destino)
			 */

			//char* comando = "echo hola pepe | ./script_transformacion.py > /tmp/resultado";
			int resultado;

			if (headerId == TIPO_MSJ_DATA_TRANSFORMACION_WORKER) {

				int cantidadMensajes = protocoloCantidadMensajes[headerId]; //averigua la cantidad de mensajes que le van a llegar
				char **arrayMensajes = deserializarMensaje(socketCliente, cantidadMensajes); //recibe los mensajes en un array de strings
				char *transformadorString = malloc(string_length(arrayMensajes[0]) + 1);
				strcpy(transformadorString, arrayMensajes[0]);
				int bloque = atoi(arrayMensajes[1]);
				int bytesOcupados = atoi(arrayMensajes[2]);
				char* temporalDestino = malloc(string_length(arrayMensajes[3]) + 1);
				strcpy(temporalDestino, arrayMensajes[3]);
				printf("Datos recibidos: Transformador %s\nSocket %d - Bloque %d - Bytes %d - Temporal %s\n", transformadorString, socketCliente, bloque, bytesOcupados, temporalDestino);

				for (i = 0; i < cantidadMensajes; i++) {
					free(arrayMensajes[i]);
				}
				free(arrayMensajes);

				char* path_script = guardar_script(transformadorString, temporalDestino);
				resultado = transformacion(path_script, bloque, bytesOcupados, temporalDestino);

				free(path_script);
				free(transformadorString);
				free(temporalDestino);
				if (resultado == 0) {
					enviarHeaderSolo(socketCliente, TIPO_MSJ_TRANSFORMACION_OK);
				}
				else {
					enviarHeaderSolo(socketCliente, TIPO_MSJ_TRANSFORMACION_ERROR);
				}
			}

			if (headerId == TIPO_MSJ_DATA_REDUCCION_LOCAL_WORKER) {
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



				char* path_temporales_reduccion = string_new();
				path_temporales_reduccion = "/home/utnso/Escritorio/reduccionLocal";
				char* path_temporal_destino = string_new();
				string_append_with_format(&path_temporal_destino, "%s/%s", path_temporales_reduccion, temporalDestino);

				FILE *temporalDestinoLocal = fopen(path_temporal_destino, "w");
				fclose (temporalDestinoLocal);

				for (i = 0; i < cantTemporales; i++) {
					apareo_archivos(path_temporal_destino,arrayTemporales[i]);
				}


				/*
				char* temporalDestino = malloc(string_length(arrayMensajes[3]) + 1);
				strcpy(temporalDestino, arrayMensajes[3]);
				printf("Datos recibidos: Transformador %s\nSocket %d - Bloque %d - Bytes %d - Temporal %s\n", transformadorString, socketCliente, bloque, bytesOcupados, temporalDestino);
				*/


				//aparear archivos
				//ejecutar reductor
				//guardar resultado en el temporal que me pasa master (arrayMensajes[2])


			}

			if (headerId == (int32_t) "REDUCCION_GLOBAL") {

			}

			if (headerId == (int32_t) "ALMACENAMIENTO_FINAL") {

			}

			if (resultado == 0) {
				enviarHeaderSolo(socketCliente, resultado);
			}

			exit(0);
			//aca termina el hijo
		} else {
			puts("Pasó por el padre");
			close(socketCliente);
			/*
			 close(pipe_padreAHijo[0]); //Lado de lectura de lo que el padre le pasa al hijo.
			 close(pipe_hijoAPadre[1]); //Lado de escritura de lo que hijo le pasa al padre.

			 //REVISAR
			 //write(pipe_padreAHijo[1],argv[1],strlen(argv[1])); //codigo
			 //write(pipe_padreAHijo[1],argv[2],strlen(argv[2])); //origen
			 //write(pipe_padreAHijo[1],argv[3],strlen(argv[3])); //destino
			 //-------------------------
			 close(pipe_padreAHijo[1]);
			 */
			waitpid(pid, &status, 0);
			/*
			 read(pipe_hijoAPadre[0], buffer, SIZE);
			 close(pipe_hijoAPadre[0]);
			 exit(0);
			 */
		}

		/*
		 FILE* fd = fopen("/tmp/resultado","w"); //ESCRIBIR RESULTADO
		 fputs(buffer,fd);
		 fclose(fd);

		 free(buffer);
		 */

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
