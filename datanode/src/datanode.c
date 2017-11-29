/*
 ============================================================================
 Name        : datanode.c
 Author      : Grupo 1234
 Description : Proceso DataNode
 ============================================================================
 */

#include "../../utils/includes.h"
#include <sys/stat.h>

#define PACKAGESIZE 1024	// Define cual va a ser el size maximo del paquete a enviar

// ================================================================ //
// enum y vectores para los datos de configuración levantados del archivo config
// ================================================================ //

enum keys {
	IP_PROPIA, PUERTO_PROPIO, FS_IP, FS_PUERTO, RUTA_DATABIN, NOMBRE_NODO
};
char* keysConfigDataNode[] = {
		"IP_PROPIA",
		"PUERTO_PROPIO",
		"FS_IP",
		"FS_PUERTO",
		"RUTA_DATABIN",
		"NOMBRE_NODO",
		NULL };

char* datosConfigDataNode[6];
FILE* archivo;
int tamanoArchivo;
t_log* logDataNode;

int conexionAFileSystem() {
	log_info(logDataNode, "Conexión a FileSystem, IP: %s, Puerto: %s", datosConfigDataNode[FS_IP], datosConfigDataNode[FS_PUERTO]);
	int socketFS = conectarA(datosConfigDataNode[FS_IP], datosConfigDataNode[FS_PUERTO]);
	if (socketFS < 0) {
		puts("Filesystem not ready\n");
		//preparadoEnviarFs = handshakeClient(&datosConexionFileSystem, NUM_PROCESO_KERNEL);
	}
	return socketFS;
}
// ================================================================ //
// DataNode es donde persisten los datos.
// Escribe sobre el data.bin
// Puede haber varios DataNode corriendo al mismo tiempo.
// ================================================================ //
void setBloque(int idBloque,char* datos){
	if (idBloque < tamanoArchivo){
		int posicion = idBloque * 1048576;
		fseek(archivo, posicion, SEEK_SET);
		fwrite(datos, 1048576, 1,archivo);
		log_info(logDataNode, "bytes guardados en el bloque %i\n",idBloque);
	}else{
		printf("El bloque %i no existe en este nodo\n",idBloque);
	}
}
//leer los datos del bloque
char* getBloque(int idBloque){
	if (idBloque >= tamanoArchivo){
		printf("El bloque %i no existe en este nodo\n",idBloque);
		return 0;
	}
	char *buffer = malloc(1048576);
	int posicion = idBloque * 1048576;
	fseek(archivo, posicion, SEEK_SET);
	fread(buffer,1048576,1,archivo);
	log_info(logDataNode, "bytes leidos en el bloque %i\n",idBloque);
	printf("bytes leidos en el bloque %i\n",idBloque);
	printf("%s\n",buffer);
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
	printf("bytes enviados: %d\n", bytesEnviados);

}
int main(int argc, char *argv[]) {

	// 1º leer archivo de config.
	//int archivoConfigOK = leerArchivoConfig(pathArchivoConfig, &datosConfigDatanode);

	//if (!archivoConfigOK) {
	//	printf("Hubo un error al leer el archivo de configuración");
	//	return 0;
	//}

	// 2º) conectarse a FileSystem
	// int socketFilesystem = inicializarClient(datosConfigDatanode.YAMA_IP, datosConfigDatanode->YAMA_PUERTO);
	//conectarseA(socketFilesystem);

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

	//2°)Abro el archivo data.bin
	archivo = fopen(datosConfigDataNode[4],"rb+");

	//tamaño de archivo data.bin
	int fd=fileno(archivo);
	struct stat buff;
	fstat(fd,&buff);
	tamanoArchivo = buff.st_size/1048576;

	//************TEST setBloque getBoque*******************
	char c[1048576]="hola como estas";
	char c2[1048576]="bien gracias";
	char* datosLeidos;

	setBloque(0,"");
	setBloque(1,"");
	setBloque(2,"");
	setBloque(3,"");
	setBloque(4,"");
	setBloque(3,"caca");
	setBloque(9,"rata");
	setBloque(1,c);
	setBloque(0,c2);
	datosLeidos=getBloque(10);
	datosLeidos=getBloque(0);
	datosLeidos=getBloque(1);
	datosLeidos=getBloque(2);
	datosLeidos=getBloque(3);
	datosLeidos=getBloque(4);
	//free(c3);

	//******************************************************

	//3°)Me conecto al FS y espero solicitudes
	int socketFS;
	if ((socketFS = conexionAFileSystem()) < 0) {
		preparadoEnviarFs = 0;
	}
	int modulo = datanode;
	send(socketFS, &modulo, sizeof(int), MSG_WAITALL);

	pruebas2(socketFS, datosConfigDataNode);
	//getchar();

	//*********Test espera bloque de FS devuelve string de 1048576 bytes******//
	char* buffer = malloc(sizeof(int));
	recv(socketFS,buffer,sizeof(int),MSG_WAITALL);
	int bloque=atoi(buffer);
	datosLeidos=getBloque(bloque);
	send(socketFS,&datosLeidos,1048576,MSG_WAITALL);
	//************************************************************************//

	fclose(archivo);//esto solo para que no quede abierto por el momento
	return 0;
}
