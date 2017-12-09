/*
 * conexiones.c
 *
 *  Created on: 24/11/2017
 *      Author: utnso
 */

// ================================================================ //
// enum y vectores para los datos de configuración levantados del archivo config
// ================================================================ //
enum keys {
	IP_PROPIA, PUERTO_PROPIO, FS_IP, FS_PUERTO, DISPONIBILIDAD_BASE,ALGORITMO_BALANCEO
};
char* keysConfigYama[] = {
		"IP_PROPIA",
		"PUERTO_PROPIO",
		"FS_IP",
		"FS_PUERTO",
		"DISPONIBILIDAD_BASE",
		"ALGORITMO_BALANCEO",
		NULL };
char* datosConfigYama[6];

t_log* logYAMA;

int getDatosConfiguracion() {
	char *nameArchivoConfig = "configYama.txt";
	if (leerArchivoConfig(nameArchivoConfig, keysConfigYama, datosConfigYama)) { //leerArchivoConfig devuelve 1 si hay error
		printf("Hubo un error al leer el archivo de configuración");
		return 0;
	}
	return 1;
}

int conexionAFileSystem() {
	log_info(logYAMA, "Conexión a FileSystem, IP: %s, Puerto: %s", datosConfigYama[FS_IP], datosConfigYama[FS_PUERTO]);
	int socketFS = conectarA(datosConfigYama[FS_IP], datosConfigYama[FS_PUERTO]);
	if (socketFS < 0) {
		puts("El FileSystem no está funcionando");
		//preparadoEnviarFs = handshakeClient(&datosConexionFileSystem, NUM_PROCESO_KERNEL);
	}
	return socketFS;
}

int inicializoComoServidor() {
	int listenningSocket = inicializarServer(datosConfigYama[IP_PROPIA], datosConfigYama[PUERTO_PROPIO]);
	if (listenningSocket < 0) {
		puts("No pude iniciar como servidor");
	}
	return listenningSocket;
}

int recibirConexion(int listenningSocket) {
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
