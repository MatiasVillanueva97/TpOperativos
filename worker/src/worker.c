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
	IP_PROPIA, PUERTO_PROPIO
};
char* keysConfigWorker[] = { "IP_PROPIA", "PUERTO_PROPIO", NULL };
char* datosConfigWorker[2];

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

char* guardar_script(char* codigo_script, char* nombre){
	char* path = string_new();
	string_append_with_format(&path, "../tmp/script_%s", nombre);
	//path = "../tmp/script_" + nombre;
	FILE *fp = fopen(path, "w");
	    if (fp != NULL)
	    {
	        fputs(codigo_script, fp);
	        fclose(fp);
	    }
	    return path;
}

char* leer_bloque(int bloque, int cantBytes){

}

int system_transformacion(char* path_script_transformacion, char* datos_origen, char* archivo_temporal){

	char* comando = string_new();

	string_append_with_format(&comando, "echo %s | ./%s > ../tmp/%s",
			datos_origen, path_script_transformacion, archivo_temporal);

	log_trace(logWorker, "El comando a ejecutar es %s", comando);

	int resultado = system(comando);

	int status;

	wait(&status); //pausa hasta que termina el hijo (system) y guarda el resultado en status

	if(WIFEXITED(status)){
		log_trace(logWorker, "El exit status del comando fue %d\n", WEXITSTATUS(status));
	}else{
		log_trace(logWorker, "La llamada system no termino normalmente... el codigo de resultado fue: %d\n", resultado);
	}

	free(comando);

	return resultado;

}

int transformacion(char* path_script, int origen, int bytesOcupados, char* destino){

	char* bloque = leer_bloque(origen, bytesOcupados);

	if(bloque == NULL){
		log_error(logWorker, "No se pudo leer el bloque (numero %d) completo segun el tamanio especificado (%d bytes)",
							  origen, bytesOcupados);
		return -1;
	}

	FILE* temporal;
	temporal = fopen (destino,"w");
	fclose (temporal);

	/*
	t_script archivo_temporal_bloque = reconstruir_archivo("../tmp/bloques_sin_transformar/", bloque, datos_transformacion.ocupado_del_bloque +1);

	if(archivo_temporal_bloque.ruta == NULL){
		log_error(logWorker, "No se pudo construir el archivo temporal para almacenar el contenido del bloque leido");

		free(bloque);

		script_destroy(archivo_temporal_bloque);

		return -1;
	}
	*/

	int resultado = system_transformacion(path_script, bloque, destino);

	//script_destroy(archivo_temporal_bloque);

	free(bloque);

	if(resultado < 0){ // segun man system retorna -1 si salio mal la ejecucion, y retorna cero si el comando es nulo (cosa que creo que no pasa).
		log_error(logWorker, "No se pudo transformar y ordenar el bloque solicitado. System devolvio %d", resultado);
		return -1;
	}

	log_trace(logWorker, "Se pudo transformar y ordenar correctamente el bloque solicitado. System devolvio %d", resultado);

	return 0;

}



int main(int argc, char *argv[]) {

#define SIZE 1024 //tamaño para comunicaciones entre padre e hijos
	//tamanioData = stat --format=%s "nombre archivo" //tamaño data.bin en bytes

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
		log_error(logWorker, "No pude iniciar como servidor");
		puts("No pude iniciar como servidor");
		return EXIT_FAILURE;
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
		puts("Ya me conecté, ahora estoy esperando mensajes\n");

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

			int32_t headerId = deserializarHeader(socketCliente);  //recibe el id del header para saber qué esperar
			int cantidadMensajes = protocoloCantidadMensajes[headerId];  //averigua la cantidad de mensajes que le van a llegar
			char **arrayMensajes = deserializarMensaje(socketCliente, cantidadMensajes);  //recibe los mensajes en un array de strings
			//transformacion (4): script, bloque (origen), bytesOcupados, temporal (destino)
			//reduc local (3): script, lista de temporales (origen), temporal(destino)
			//reduc global (4): script, lista de procesos Worker con sus IPs y Puertos, temporales de Reducción Local (origen), temporal (destino)
			//almac final (2): archivo reduc global (origen), nombre y ruta archivo final (destino)

			//char* comando = "echo hola pepe | ./script_transformacion.py > /tmp/resultado";

			int resultado;

			if (headerId == (int32_t)"TRANSFORMACION") {
				char* path_script = guardar_script(arrayMensajes[0],arrayMensajes[3]);
				int bloque = atoi(arrayMensajes[1]);
				int bytesOcupados = atoi(arrayMensajes[2]);
				resultado = transformacion(path_script,bloque,bytesOcupados,arrayMensajes[3]);
			}

			if (headerId == (int32_t)"REDUCCION_LOCAL") {

			}

			if (headerId == (int32_t)"REDUCCION_GLOBAL") {

			}

			if (headerId == (int32_t)"ALMACENAMIENTO_FINAL") {

			}


			if (resultado == 0) {
				enviarHeaderSolo(socketCliente,resultado);
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
			waitpid(pid,&status,0);
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
