/*
 ============================================================================
 Name        : datanode.c
 Author      : Grupo 1234
 Description : Proceso DataNode
 Resume      : DataNode es donde persisten los datos.
 Escribe sobre el data.bin
 Puede haber varios DataNode corriendo al mismo tiempo.
 ============================================================================
 */

#include "../../utils/includes.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>

#define PACKAGESIZE 1024	// Define cual va a ser el size maximo del paquete a enviar

enum keys {
	IP_PROPIA, PUERTO_PROPIO, FS_IP, FS_PUERTO, RUTA_DATABIN, NOMBRE_NODO
};
char* keysConfigDataNode[] = { "IP_PROPIA", "PUERTO_PROPIO", "FS_IP", "FS_PUERTO", "RUTA_DATABIN", "NOMBRE_NODO",
NULL };

//FILE* archivo;
char* datosConfigDataNode[6];
int tamanoArchivo;
t_log* logDataNode;
void *mapArchivo;
struct stat buff;
int fd;

int conexionAFileSystem() {
	log_info(logDataNode, "Conexión a FileSystem, IP: %s, Puerto: %s", datosConfigDataNode[FS_IP], datosConfigDataNode[FS_PUERTO]);
	int socketFS = conectarA(datosConfigDataNode[FS_IP], datosConfigDataNode[FS_PUERTO]);
	if (socketFS >= 0) {
		puts("Conectado a FileSystem");
	}else{
		puts("Filesystem not ready\n");
		//preparadoEnviarFs = handshakeClient(&datosConexionFileSystem, NUM_PROCESO_KERNEL);
	}
	return socketFS;
}

void setBloque(int idBloque, char* datos) {
	if (idBloque < tamanoArchivo) {
		int bytesEscritos = sizeof(char*) * strlen(datos) + 1;
		int posicion = idBloque * 1048576;
		mapArchivo = mmap(0, buff.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, posicion);
		if (mapArchivo == MAP_FAILED) {
			perror("mmap");
			close(fd);
			exit(1);
		}
		strncpy(mapArchivo, datos, bytesEscritos);
		munmap(mapArchivo, buff.st_size);
	} else {
		printf("El bloque %i no existe en este nodo\n", idBloque);
	}
}

//leer los datos del bloque
char* getBloque(int idBloque) {
	if (idBloque >= tamanoArchivo) {
		printf("El bloque %i no existe en este nodo\n", idBloque);
		return 0;
	}
	char *buffer = malloc(1048576);
	int posicion = idBloque * 1048576;
	mapArchivo = mmap(0, 1048576, PROT_READ, MAP_SHARED, fd, posicion);
	if (mapArchivo == MAP_FAILED) {
		perror("mmap");
		close(fd);
		exit(1);
	}
	strncpy(buffer, mapArchivo, 1048576);
	log_info(logDataNode, "bytes leidos en el bloque %i\n", idBloque);
	printf("bytes leidos en el bloque %i\n", idBloque);
	//printf("%s\n",buffer);
	return buffer;
}

int cantidadBloquesAMandar(char * PATH) {
	FILE* archivo = fopen(PATH, "r+");
	int tamanoArchivo = 1;
	int cantidadBloques = 0;
	fseek(archivo, 0, SEEK_END);
	tamanoArchivo = ftell(archivo);
	cantidadBloques = tamanoArchivo / 1048576;
	return cantidadBloques;
}

void pruebas2(int socket, char **datosConfigDataNode) {
	char* nombre = datosConfigDataNode[5];
	char * ip = datosConfigDataNode[0];
	char * puerto = datosConfigDataNode[1];
	int cantidadBloq = cantidadBloquesAMandar(datosConfigDataNode[4]);

	int cantStrings = 4;
	char **arrayMensajesSerializar = malloc(sizeof(char*) * cantStrings);
	if (!arrayMensajesSerializar)
		perror("error de malloc 1");

	int i = 0;

	arrayMensajesSerializar[i] = malloc(string_length(nombre) + 1);
	if (!arrayMensajesSerializar[i])
		perror("error de malloc 1");
	strcpy(arrayMensajesSerializar[i], nombre);
	i++;

	char *cantidadBloqString = intToArrayZerosLeft(cantidadBloq, 4);
	arrayMensajesSerializar[i] = malloc(string_length(cantidadBloqString) + 1);
	if (!arrayMensajesSerializar[i])
		perror("error de malloc 1");
	strcpy(arrayMensajesSerializar[i], cantidadBloqString);
	i++;

	arrayMensajesSerializar[i] = malloc(string_length(ip) + 1);
	if (!arrayMensajesSerializar[i])
		perror("error de malloc 1");
	strcpy(arrayMensajesSerializar[i], ip);
	i++;

	arrayMensajesSerializar[i] = malloc(string_length(puerto) + 1);
	if (!arrayMensajesSerializar[i])
		perror("error de malloc 1");
	strcpy(arrayMensajesSerializar[i], puerto);
	i++;

	char *mensajeSerializado = serializarMensaje(TIPO_MSJ_DATANODE, arrayMensajesSerializar, cantStrings);
	int bytesEnviados = enviarMensaje(socket, mensajeSerializado);

}
int main(int argc, char *argv[]) {

	logDataNode = log_create("logDataNode.log", "DataNode", false, LOG_LEVEL_TRACE); //creo el logger, sin mostrar por pantalla
	log_info(logDataNode, "Iniciando proceso DataNode");
	printf("\n*** Proceso DataNode ***\n");
	int preparadoEnviarFs = 1, i;

	// 1º) leo archivo de config.
	char *nameArchivoConfig = "configNodo.txt";
	if (leerArchivoConfig(nameArchivoConfig, keysConfigDataNode, datosConfigDataNode)) { //leerArchivoConfig devuelve 1 si hay error
		printf("Hubo un error al leer el archivo de configuración");
		return 0;
	}

	//TODO revisar esto!
	//2°) abro el archivo data.bin
	//archivo = fopen(datosConfigDataNode[4], "rb+");
	/*
	 //tamaño de archivo data.bin
	 int fd = fileno(archivo);
	 struct stat buff;
	 fstat(fd, &buff);
	 tamanoArchivo = buff.st_size / 1048576;
	 */
	fd = open(datosConfigDataNode[4], O_RDWR);
	if (fd == -1) {
		perror("open");
		exit(1);
	}
	if (fstat(fd, &buff) < 0) {
		perror("fstat");
		close(fd);
		exit(1);
	}
	//tamaño de archivo data.bin
	tamanoArchivo = buff.st_size / 1048576;

	//3°) me conecto al FS y espero solicitudes
	int socketFS; //= conectarA("127.0.0.1","5000");
	if ((socketFS = conexionAFileSystem()) < 0) {
		preparadoEnviarFs = 0;
	}

	int modulo = datanode;
	send(socketFS, &modulo, sizeof(int), MSG_WAITALL);
	pruebas2(socketFS, datosConfigDataNode);
	int j = 1;

	while (1) {

		int32_t header = deserializarHeader(socketFS);
		switch (header) {

		case TIPO_MSJ_ARCHIVO: {
			int cantMensajes = protocoloCantidadMensajes[header];
			char ** arrayMensajesRecibidos = deserializarMensaje(socketFS, cantMensajes);
			setBloque(atoi(arrayMensajesRecibidos[1]), arrayMensajesRecibidos[0]);
			printf("setie bloque \n");
			printf("numero de bloque %s\n", arrayMensajesRecibidos[1]);
			printf("tipo archivo %s\n", arrayMensajesRecibidos[2]);
			printf("recibi mensaje %d \n", j);
			j++;
		}
			break;

		case TIPO_MSJ_PEDIR_BLOQUES: {
			//piden bloques y los mando
			int cantMensajes = protocoloCantidadMensajes[header];
			char ** arrayMensajesRecibidos = deserializarMensaje(socketFS, cantMensajes);
			printf("bloque %d\n", atoi(arrayMensajesRecibidos[0]));
			char * buffer = getBloque(atoi(arrayMensajesRecibidos[0]));
			munmap(mapArchivo, 1048576);

			printf("paso get bloque\n");

			//SEND
			int cantStrings1 = 1;
			char **arrayMensajesSerializarEnviar = malloc(sizeof(char*) * cantStrings1);
			if (!arrayMensajesSerializarEnviar)
				perror("error de malloc 1");

			int i = 0;

			arrayMensajesSerializarEnviar[i] = malloc(string_length(buffer) + 1);
			if (!arrayMensajesSerializarEnviar[i])
				perror("error de malloc 1");
			strcpy(arrayMensajesSerializarEnviar[i], buffer);
			i++;

			char *mensajeSerializado = serializarMensaje(TIPO_MSJ_BLOQUE_DESDE_DATANODE, arrayMensajesSerializarEnviar, cantStrings1);
			//printf("mensajeSerializado : %s\n", mensajeSerializado);
			int bytesEnviados = enviarMensaje(socketFS, mensajeSerializado);
			printf("bytes enviados: %d\n", bytesEnviados);
		}
			break;

		default: {
			printf("Se cayo FileSystem\n");
			exit(0);
			break;

		}

			break;
		}
	}

	close(fd);
	log_destroy(logDataNode);
	return EXIT_SUCCESS;
}
