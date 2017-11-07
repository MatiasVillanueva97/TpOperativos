#include "filesystem.h"

enum keys {
	IP_PROPIA, PUERTO_PROPIO
};

char* keysConfigFilesystem[] = { "IP_PROPIA", "PUERTO_PROPIO", NULL };

char* datosConfigFilesystem[2];


void crearBitmap(char * PATH, char * nodoConectado, int cantBloques) {
	int FileD;
	FILE* archivoDeBitmap = fopen(PATH, "r+");
	if (archivoDeBitmap == NULL) {
		log_info(logFs, "Se tuvo que crear un bitmap nuevo [%s], ya que no habia un bitmap anterior.", nodoConectado);
		archivoDeBitmap = fopen(PATH, "w+");
		int cantidad = ceil(((double) cantBloques) / 8.0);
		char* cosa = string_repeat('\0', cantidad);
		fwrite(cosa, 1, cantidad, archivoDeBitmap);
		free(cosa);
	} else {
		log_info(logFs, "Se cargo el bitmap [%s] al FileSystem ", nodoConectado);
	}
	fclose(archivoDeBitmap);
	FileD = open(PATH, O_RDWR);

	struct stat scriptMap;
	fstat(FileD, &scriptMap);

	char* bitmap2 = mmap(0, scriptMap.st_size, PROT_WRITE | PROT_READ, MAP_SHARED, FileD, 0);
	bitMap = bitarray_create(bitmap2, ceil(((double) cantBloques) / 8.0));
	log_info(logFs, "[Configurar Todo]-Se creo correctamente el bitmap [%s]", nodoConectado);

	int k;
	for (k = 0; k < 34; k++) {
		bool as2 = !bitarray_test_bit(bitMap, k);
		if (as2 == true) {
			printf("libre posicion %d\n", k);
		} else {
			printf("ocupado posicion %d\n", k);
		}
	}

}

//hacer recv de los nodos del nombre del nodo
void crearBitmapDeNodosConectados(char * NodoConectado, int cantBloques) {
	t_list * nodosConectados = list_create();

	list_add(nodosConectados, NodoConectado);
	void crearBitmapXNodo(char * nodoConectado) {

		char * PATH_bitmap_xNOdo = string_new();
		string_append(&PATH_bitmap_xNOdo, "../bitmaps/");
		string_append(&PATH_bitmap_xNOdo, nodoConectado);
		string_append(&PATH_bitmap_xNOdo, ".dat");
		crearBitmap(PATH_bitmap_xNOdo, nodoConectado, cantBloques);

	}
	list_iterate(nodosConectados, crearBitmapXNodo);
}

int cantidadBloques(char * PATH) {
	FILE* archivo = fopen(PATH, "r+");
	int tamanoArchivo = 1;
	int cantidadBloques = 0;
	fseek(archivo, 0, SEEK_END);
	tamanoArchivo = ftell(archivo);
	cantidadBloques = ceil((float) tamanoArchivo / 1048576);
	return cantidadBloques;
}

void partirArchivoBinario(char* PATH) {
	FILE* archivo = fopen(PATH, "r+");
	int fd = fileno(archivo);
	int tamano;
	struct stat buff;
	fstat(fd, &buff);
	tamano = buff.st_size;

	void * archivoABytes = malloc(buff.st_size);
	void * contenidoAEnviar = malloc(1048576);
	while (tamano >= 0) {

		if (tamano > 0 && tamano < 1048576) {
			fread(contenidoAEnviar, tamano, 1, archivo);
			//send
			break;
		} else {
			fread(contenidoAEnviar, 1048576, 1, archivo);
			//send
			tamano -= 1048576;
		}

	}

	free(contenidoAEnviar);
	free(archivoABytes);
	fclose(archivo);
}

void partirArchivoDeTexto(char * PATH) {
	FILE *archivo = fopen(PATH, "r+");
	int fd = fileno(archivo);
	struct stat buff;
	fstat(fd, &buff);
	char caracter;
	int a;
	int ultimoBarraN = 0;
	int ultimoBarraNAnterior = 0;
	int ultimoBarraNAntesDeMega = 0;
	void * contenidoAEnviar;
	int * posicionBarraN;
	int * posicionArchivoTerminado;

	t_list * posiciones = list_create();

	while (!feof(archivo)) {

		caracter = fgetc(archivo);

		if (caracter == '\n') {

			a = ftell(archivo);
			ultimoBarraN = ftell(archivo);

		}

		if (ftell(archivo) == 1048576 + ultimoBarraNAntesDeMega) {
			ultimoBarraNAntesDeMega = ultimoBarraN;
			posicionBarraN = malloc(sizeof(int));
			*posicionBarraN = ultimoBarraN;
			list_add(posiciones, posicionBarraN);
			ultimoBarraNAnterior = ultimoBarraN;
		}

		else {

		}
	} //while

	fseek(archivo, 0, SEEK_END);
	posicionArchivoTerminado = malloc(sizeof(int));
	*posicionArchivoTerminado = ftell(archivo);
	list_add(posiciones, posicionArchivoTerminado);
	fseek(archivo, 0, SEEK_SET);
	int posicionActual = 0;

	void partir(int * posicion) {

		char * puta = string_new();
		string_append(&puta, "/home/utnso/Escritorio/archivonuevo");
		string_append(&puta, string_itoa(posicionActual));

		FILE*archivo2 = fopen(puta, "a+");
		if (posicionActual == 0) {
			contenidoAEnviar = malloc(*posicion);
			fread(contenidoAEnviar, *posicion, 1, archivo);
			//aca haria el send

		} else {
			int posicionAnterior = *((int*) list_get(posiciones, posicionActual - 1));
			contenidoAEnviar = malloc((*posicion) - posicionAnterior);
			fread(contenidoAEnviar, (*posicion) - posicionAnterior, 1, archivo);
			//aca el otro send

		}
		posicionActual++;
		fclose(archivo2);
	}
	list_iterate(posiciones, partir);
	fclose(archivo);
	free(contenidoAEnviar);
}

void almacenarArchivo(char * PATH, int TipoArchivo) {

	if (TipoArchivo == 1) {
		partirArchivoBinario(PATH);
	} else {
		partirArchivoDeTexto(PATH);
	}
}

void leerArchivo(char * PATH) {
	puts("leo archivo yamafs");
}

void soyServidor(char * puerto) {
	void *get_in_addr(struct sockaddr *sa) {
		if (sa->sa_family == AF_INET) {
			return &(((struct sockaddr_in*) sa)->sin_addr);
		}

		return &(((struct sockaddr_in6*) sa)->sin6_addr);
	}
	int i;
	int nuevoSocket;
	int SocketYama;
	int SocketWorker;
	int quiensoy;
	int fdmax;
	struct sockaddr_storage remoteaddr;
	int listener = crearSocketYBindeo(puerto);
	fdmax = listener;
	socklen_t addrlen;
	fd_set master;
	fd_set SocketsDataNodes;
	FD_ZERO(&master);
	FD_ZERO(&SocketsDataNodes);
	FD_SET(listener, &master);
	escuchar(listener);
	bool estable;
	estable = true;

	while (1) {
		SocketsDataNodes = master;
		if (select(fdmax + 1, &SocketsDataNodes, NULL, NULL, NULL) == -1) {
			perror("select");
			exit(4);
		}

		for (i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &SocketsDataNodes)) {
				if (i == listener) {
					addrlen = sizeof remoteaddr;
					nuevoSocket = accept(listener, (struct sockaddr *) &remoteaddr, &addrlen);

					if (nuevoSocket == -1) {
						perror("accept");
					} else {
						FD_SET(nuevoSocket, &master);
						recv(nuevoSocket, &quiensoy, sizeof(int), MSG_WAITALL); //Devuelve -1 en error
						switch (quiensoy) {

						case yama: {
							if (estable) {
								SocketYama = nuevoSocket;
								enviarInfoBloques(SocketYama);
								puts("conectado");
							} else {
								puts("todavia no es estado estable");
							}
						}
							break;
						case worker: {
							SocketWorker = nuevoSocket;
							//atender a worker,supongo que almacenar el archivo
						}
							break;
						case datanode: {
							//necesito recibir informacion del datanode,id de datanode y bitmap para la proxima entrega
							crearBitmapDeNodosConectados("Nodo123", 33); //donde dice Nodo329 iria el id del datanode
							bitarray_set_bit(bitMap, 33);
							estable = true;
						}
							break;
						}
						if (nuevoSocket > fdmax) {
							fdmax = nuevoSocket;
						}
					}
				} else {/* esto sirve para cuando ya estan conectados ,
				 si te envian mensajes la funcion
				 recibirmensaje era parte de mi serializacion
				 void * stream;
				 Mensaje mensaje = recibirMensaje(i, &stream);
				 if (mensaje.accion <= 0) {
				 if (mensaje.accion == 0) {
				 printf("Se murio socket %d hung up\n", i); ///hacer log
				 } else {
				 perror("Hubo un error que no deberia pasar");
				 }
				 close(i);
				 FD_CLR(i, &master);
				 } else {
				 if (i == SocketYama) {
				 //cosas que haga yama
				 switch (mensaje.accion) {
				 case INT:{
				 printf("%d",*(int*)stream);
				 }
				 break;
				 case ARCHIVO: {

				 }
				 break;
				 case CADENA: {
				 }
				 break;
				 }
				 } else {
				 //soy datanode
				 }

				 }*/
				}
			}
		}
	}
}

/*
 void enviarListaNodos(int socket){
 void* mensaje=malloc(sizeof(tablaGlobalNodos.nodo));
 int posicionActual=0;
 char* contenido=string_new();
 int i=0;
 for(;i<list_size(tablaGlobalNodos.nodo),i++){
 int tamanioNodo=string_length(list_get(tablaGlobalNodos.nodo,i));
 memcpy(mensaje,&tamanioNodo,sizeof(int));
 posicionActual+=sizeof(int);
 memcpy(mensaje+posicionActual,list_get(tablaGlobalNodos.nodo,i),tamanioNodo);
 posicionActual+=tamanioNodo;
 }
 int tamanioMsj=sacarTamanioMEnsaje();
 sendRemasterizado(socket, ES_FS, tamanioMsj, mensaje);
 }*/

void enviarInfoBloques(int socketCliente) {
	uint32_t headerId = deserializarHeader(socketCliente);
	printf("id: %d\n", headerId);
	printf("mensaje predefinido: %s\n", protocoloMensajesPredefinidos[headerId]);
	int cantidadMensajes = protocoloCantidadMensajes[headerId];
	char **arrayMensajes = deserializarMensaje(socketCliente, cantidadMensajes);
	char *archivo = malloc(string_length(arrayMensajes[0]) + 1);
	strcpy(archivo, arrayMensajes[0]);
	free(arrayMensajes);
	printf("archivo: %s\n", archivo);

	char *cantBloquesFile = "0002";
	char *bloque0NodoCopia0 = "0003";
	char *bloque0BloqueCopia0 = "0033";
	char *bloque0NodoCopia1 = "0001";
	char *bloque0BloqueCopia1 = "0013";
	char *bytesBloque0 = "00012564";
	char *bloque1NodoCopia0 = "0005";
	char *bloque1BloqueCopia0 = "0020";
	char *bloque1NodoCopia1 = "0004";
	char *bloque1BloqueCopia1 = "0025";
	char *bytesBloque1 = "00003264";

	int cantStrings = 11, i = 0;
	char **arrayMensajesSerializar = malloc(cantStrings);

	arrayMensajesSerializar[i] = malloc(string_length(cantBloquesFile) + 1);
	strcpy(arrayMensajesSerializar[i], cantBloquesFile);
	arrayMensajesSerializar[i][string_length(cantBloquesFile)] = '\0';

	arrayMensajesSerializar[i + 1] = malloc(string_length(bloque0NodoCopia0) + 1);
	strcpy(arrayMensajesSerializar[i + 1], bloque0NodoCopia0);
	arrayMensajesSerializar[i + 1][string_length(bloque0NodoCopia0)] = '\0';

	arrayMensajesSerializar[i + 2] = malloc(string_length(bloque0BloqueCopia0) + 1);
	strcpy(arrayMensajesSerializar[i + 2], bloque0BloqueCopia0);
	arrayMensajesSerializar[i + 2][string_length(bloque0BloqueCopia0)] = '\0';

	arrayMensajesSerializar[i + 3] = malloc(string_length(bloque0NodoCopia1) + 1);
	strcpy(arrayMensajesSerializar[i + 3], bloque0NodoCopia1);
	arrayMensajesSerializar[i + 3][string_length(bloque0NodoCopia1)] = '\0';

	arrayMensajesSerializar[i + 4] = malloc(string_length(bloque0BloqueCopia1) + 1);
	strcpy(arrayMensajesSerializar[i + 4], bloque0BloqueCopia1);
	arrayMensajesSerializar[i + 4][string_length(bloque0BloqueCopia1)] = '\0';

	arrayMensajesSerializar[i + 5] = malloc(string_length(bytesBloque0) + 1);
	strcpy(arrayMensajesSerializar[i + 5], bytesBloque0);
	arrayMensajesSerializar[i + 5][string_length(bytesBloque0)] = '\0';

	arrayMensajesSerializar[i + 6] = malloc(string_length(bloque1NodoCopia0) + 1);
	strcpy(arrayMensajesSerializar[i + 6], bloque1NodoCopia0);
	arrayMensajesSerializar[i + 6][string_length(bloque1NodoCopia0)] = '\0';

	arrayMensajesSerializar[i + 7] = malloc(string_length(bloque1BloqueCopia0) + 1);
	strcpy(arrayMensajesSerializar[i + 7], bloque1BloqueCopia0);
	arrayMensajesSerializar[i + 7][string_length(bloque1BloqueCopia0)] = '\0';

	arrayMensajesSerializar[i + 8] = malloc(string_length(bloque1NodoCopia1) + 1);
	strcpy(arrayMensajesSerializar[i + 8], bloque1NodoCopia1);
	arrayMensajesSerializar[i + 8][string_length(bloque1NodoCopia1)] = '\0';

	arrayMensajesSerializar[i + 9] = malloc(string_length(bloque1BloqueCopia1) + 1);
	strcpy(arrayMensajesSerializar[i + 9], bloque1BloqueCopia1);
	arrayMensajesSerializar[i + 9][string_length(bloque1BloqueCopia1)] = '\0';

	arrayMensajesSerializar[i + 10] = malloc(string_length(bytesBloque1) + 1);
	strcpy(arrayMensajesSerializar[i + 10], bytesBloque1);
	arrayMensajesSerializar[i + 10][string_length(bytesBloque1)] = '\0';

	char *mensajeSerializado = serializarMensaje(TIPO_MSJ_METADATA_ARCHIVO, arrayMensajesSerializar, cantStrings);
	printf("mensaje serializado: %s\n", mensajeSerializado);
	enviarMensaje(socketCliente, mensajeSerializado);
}

int buscarPosicionLibre(int cantBloque) {
	int i;
	for (i = 0; i < cantBloque + 1; i++) {
		if (!bitarray_test_bit(bitMap, i)) {
			log_info(logFs, "Se encontro la posicion %d en el bitmap libre.", i);
			return i;
		}
	}
	log_info(logFs, "No hay posicion libre");
}

int main() {

	char * PUERTO;
	logFs = log_create("FileSystem.log", "FileSystem", 0, 0);
	FILE * directorios = fopen("/home/utnso/Escritorio/anda/directorios.dat", "w+");
	FILE * nodos = fopen("/home/utnso/Escritorio/anda/nodo.bin", "wb");
	tablaNodos = list_create();
	tablaArchivos = list_create();
	tablaNodosGlobal = malloc(sizeof(tablaDeNodos));
	tablaNodosGlobal->nodo = list_create();
	tablaNodosGlobal->contenidoXNodo = list_create();

	pthread_t hiloConsola;

	char *nameArchivoConfig = "configFilesystem.txt";
	if (leerArchivoConfig(nameArchivoConfig, keysConfigFilesystem, datosConfigFilesystem)) { //leerArchivoConfig devuelve 1 si hay error
		printf("Hubo un error al leer el archivo de configuración");
		return 0;
	}

	pthread_create(&hiloConsola, NULL, IniciarConsola, NULL);
	soyServidor(datosConfigFilesystem[PUERTO_PROPIO]);
	pthread_join(hiloConsola, NULL);
	fclose(directorios);
	fclose(nodos);
	log_destroy(logFs);
	return 0;
}
