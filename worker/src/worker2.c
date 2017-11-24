/*
 ============================================================================
 Name        : worker.c
 Author      : Grupo 1234
 Description : Proceso Worker
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/log.h>
#include "../../utils/conexionesSocket.h"
#include "../../utils/archivoConfig.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

//#define PACKAGESIZE 1024	// Define cual va a ser el size maximo del paquete a enviar

enum keys {
	IP_PROPIA, PUERTO_PROPIO
};
char* keysConfigWorker[] = { "IP_PROPIA", "PUERTO_PROPIO", NULL };
char* datosConfigWorker[2];

// ================================================================ //
// Worker es el que realiza las operaciones que le pide el Master.
// 1) Lee archivo de configuracion del nodo.
// 2) Espera conexion de Masters.
// 3) Recibe conexion del Master y se forkea
// 4) El principal sigue escuchando, el fork ejecuta la orden
// 5) Termina la orden, aviso a master que terminó y el fork muere
// Puede haber varios Worker corriendo al mismo tiempo.
// ================================================================ //

int main(int argc, char *argv[]) {

    #define SIZE 1024 //tamaño para comunicaciones entre padre e hijos
    //tamanioData = stat --format=%s "nombre archivo" //tamaño data.bin en bytes

	t_log* logWorker;
	logWorker = log_create("logFile.log", "WORKER", false, LOG_LEVEL_TRACE); //creo el logger, sin mostrar por pantalla

	log_info(logWorker, "Iniciando Worker");
	printf("\n*** Proceso worker ***\n");

	// 1º) leer archivo de config.
	char *nameArchivoConfig = "configNodo.txt";
	if (leerArchivoConfig(nameArchivoConfig, keysConfigWorker, datosConfigWorker)) {//leerArchivoConfig devuelve 1 si hay error
		printf("Hubo un error al leer el archivo de configuración");
		return 0;
	}

	// 2º) inicializar server y aguardar conexiones (de master)
	//HAY QUE VER COMO SE CONECTA CON OTROS WORKERS
	int listenningSocket = inicializarServer(datosConfigWorker[IP_PROPIA], "5302");

	if (listenningSocket < 0) {
		log_error(logWorker, "No pude iniciar como servidor");
		puts("No pude iniciar como servidor");
		return EXIT_FAILURE;
	}
	while (1) {	//inicio bucle para forkear
		puts("Ya estoy preparado para recibir conexiones\n");

		int socketCliente = aceptarConexion(listenningSocket);
		if (socketCliente < 0) {
			log_error(logWorker, "Hubo un error al aceptar conexiones");
			puts("Hubo un error al aceptar conexiones\n");
			return EXIT_FAILURE;
		}

		log_info(logWorker, "Worker conectado, esperando mensajes");
		puts("Ya me conecté, ahora estoy esperando mensajes\n");

		int pipe_padreAHijo[2]; //PIPES: 0 lectura, 1 escritura
		int pipe_hijoAPadre[2];

		pipe(pipe_padreAHijo);
		pipe(pipe_hijoAPadre);

		pid_t pid;
		int status;
		char* buffer=malloc(SIZE);

		if ((pid=fork()) == 0 ) //aca ya se hizo el proceso hijo
		{
		    dup2(pipe_padreAHijo[0],STDIN_FILENO);
		    dup2(pipe_hijoAPadre[1],STDOUT_FILENO);

		    // Cerramos pipes duplicados
		    close(pipe_padreAHijo[1]);
		    close(pipe_hijoAPadre[0]);
		    close(pipe_hijoAPadre[1]);
		    close(pipe_padreAHijo[0]);


		    char *argv[] = {NULL};
		    char *envp[] = {NULL};
		    //ACA CORRER CON SYSTEM --> ej:   system("echo hola pepe | ./script_transformacion.py > /tmp/resultado");
		    //execve("./script_transformacion.py", argv, envp);
		    exit(1);

		} //aca termina el hijo

		else {
		    close(pipe_padreAHijo[0]); //Lado de lectura de lo que el padre le pasa al hijo.
		    close(pipe_hijoAPadre[1]); //Lado de escritura de lo que hijo le pasa al padre.

		    /*REVISAR
		    write(pipe_padreAHijo[1],argv[1],strlen(argv[1])); //codigo
		    write(pipe_padreAHijo[1],argv[2],strlen(argv[2])); //origen
		    write(pipe_padreAHijo[1],argv[3],strlen(argv[3])); //destino
		    */

		    close(pipe_padreAHijo[1]);
		    waitpid(pid,&status,0);

		    read(pipe_hijoAPadre[0], buffer, SIZE);
		    close(pipe_hijoAPadre[0]);

		}

		//FILE* fd = fopen("/tmp/resultado","w"); //ESCRIBIR RESULTADO
		//fputs(buffer,fd);
		//fclose(fd);


		free(buffer);
		return 0;






		// 3º) creo forks para ejecutar instrucciones de master
//		int pid = fork();
//		if (pid < 0) {
//			log_error(logWorker, "Error al forkear");
//			return EXIT_FAILURE;
//		} else if (pid == 0) {
//			log_info(logWorker, "Hijo creado");
//			printf("Soy el hijo y mi PID es %d\n", getpid());
//			sleep(1000);
//			// Ejecuto lo q me pidió el master con execv() o system()
//			execv("/bin/ls",NULL);
//			return EXIT_SUCCESS;
//		} else {
//			log_info(logWorker, "Hijo creado");
//			printf("Soy el padre y mi PID sigue siendo %d\n", getpid());
//			// Acá debería seguir escuchando conexiones
//			close(socketCliente);//cierro el socket en el padre para poder conectarlo con otro master
//		}
	}
//	wait();
	return EXIT_SUCCESS;
}
