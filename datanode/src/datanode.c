/*
 ============================================================================
 Name        : datanode.c
 Author      : Grupo 1234
 Description : Proceso DataNode
 ============================================================================
 */

#include "../../utils/includes.h"

#define PACKAGESIZE 1024	// Define cual va a ser el size maximo del paquete a enviar

// ================================================================ //
// enum y vectores para los datos de configuración levantados del archivo config
// ================================================================ //

enum keys {
	IP_PROPIA, PUERTO_PROPIO, FS_IP, FS_PUERTO
};
char* keysConfigDataNode[] = { "IP_PROPIA", "PUERTO_PROPIO", "FS_IP", "FS_PUERTO","RUTA_DATABIN",NULL };

char* datosConfigDataNode[5];

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
	FILE* archivo = fopen("data.bin","r+");
	fclose(archivo);//esto solo para que no quede abierto por el momento
	//3°)Me conecto al FS y espero solicitudes
	int socketFS;
	if ((socketFS = conexionAFileSystem()) < 0) {
		preparadoEnviarFs = 0;
	}

	return 0;
}
