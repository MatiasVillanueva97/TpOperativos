#include "filesystem.h"

int buscarPosicionLibre(t_bitarray * bitmap,int cantBloque){
		int i;
		for(i=0;i<cantBloque;i++){
			if(!bitarray_test_bit(bitmap,i)){
			//	log_info(logFs,"Se encontro la posicion %d en el bitmap libre.",i);
				return i;
			}
		}
		return -1;
		log_info(logFs,"No hay posicion libre");
	}

int cantidadDeBloquesLibresEnBitmap(t_bitarray * bitmap,int cantBloques){
int k;
int i=0;
for(k=0;k<cantBloques;k++){
	bool as2 = !bitarray_test_bit(bitmap,k);
	if(as2==true){
		//printf("libre posicion %d\n",k);
		i++;
	}
	else {
		//printf("ocupado posicion %d\n",k);
	}
}
return i;
}

tablaBitmapXNodos * obtenerNodoConMayorPosicionLibre(){

			double mayor=0;
		void obtenerMayor(tablaBitmapXNodos * elemento){
			int cantidadBloquesLibres= cantidadDeBloquesLibresEnBitmap(elemento->bitarray,elemento->cantidadBloques);
			int cantidadBLoques = elemento->cantidadBloques;
			double porcentaje = 100*((double)cantidadBloquesLibres/ (double)cantidadBLoques);
			if(porcentaje>mayor){
				mayor = porcentaje;
			}

		}
		list_iterate(listaDeBitMap,(void*)obtenerMayor);

		bool buscarNodoMasLibre(tablaBitmapXNodos * element){
			int z = cantidadDeBloquesLibresEnBitmap(element->bitarray,element->cantidadBloques);
			int j = element->cantidadBloques;
			double porcentaje = 100*((double)z/(double)j);
			if(porcentaje==mayor){
				return true;
			}else {
				return false;
			}

		}
		tablaBitmapXNodos * nodoMasLibre =list_find(listaDeBitMap,(void*)buscarNodoMasLibre);
		return nodoMasLibre;
		}

tablaBitmapXNodos * buscarNodoPorNombreB(char * Nodo){

bool buscarEnLISTA(tablaBitmapXNodos * elemento){
		return strcmp(elemento->nodo,Nodo)==0;
	}
 tablaBitmapXNodos * nodoEncontrado = list_find(listaDeBitMap,(void*)buscarEnLISTA);
 return nodoEncontrado;
}

ContenidoXNodo * buscarNodoPorNombreS(char * Nodo){

bool buscarEnLISTA(ContenidoXNodo * elemento){
		return strcmp(elemento->nodo,Nodo)==0;
	}
ContenidoXNodo * nodoEncontrado = list_find(tablaNodos,(void*)buscarEnLISTA);
 return nodoEncontrado;
}

tablaArchivo * buscarArchivoPorNombre(char * nombreArchivo){

bool buscarEnLISTA(tablaArchivo * elemento){
		return strcmp(elemento->nombre,nombreArchivo)==0;
	}
tablaArchivo * archivoEncontrado = list_find(tablaArchivos,(void*)buscarEnLISTA);
 return archivoEncontrado;
}

void crearBitmap(char * PATH ,char * nodoConectado,int cantBloques){
	int FileD;
	tablaBitmapXNodos * aux3 =malloc(sizeof(tablaBitmapXNodos));
	FILE* archivoDeBitmap = fopen(PATH,"r+");
	if(archivoDeBitmap == NULL){
		log_info(logFs,"Se tuvo que crear un bitmap nuevo [%s], ya que no habia un bitmap anterior.",nodoConectado);
		archivoDeBitmap =fopen(PATH,"w+");
		int cantidad = ceil(((double)cantBloques)/8.0);
		char* cosa = string_repeat('\0',cantidad);
		fwrite(cosa,1,cantidad,archivoDeBitmap);
		free(cosa);
	}
	else{
		log_info(logFs,"Se cargo el bitmap [%s] al FileSystem ",nodoConectado);
	}
	fclose(archivoDeBitmap);
	FileD = open(PATH,O_RDWR);


	struct stat scriptMap;
	fstat(FileD, &scriptMap);

	char* bitmap2 = mmap(0, scriptMap.st_size, PROT_WRITE |PROT_READ , MAP_SHARED, FileD,  0);
	aux3->bitarray= bitarray_create(bitmap2, ceil(((double)cantBloques)/8.0));
	log_info(logFs,"[Configurar Todo]-Se creo correctamente el bitmap [%s]",nodoConectado);

	aux3->nodo=nodoConectado;
	aux3->cantidadBloques=cantBloques;
	list_add(listaDeBitMap,aux3);


}
	//hacer recv de los nodos del nombre del nodo
void crearBitmapDeNodosConectados(char * NodoConectado,int cantBloques){
	t_list * nodosConectados=list_create();
	list_add(nodosConectados,string_duplicate(NodoConectado));

	void crearBitmapXNodo(char * nodoConectado){

		char * PATH_bitmap_xNOdo=string_new();
		string_append(&PATH_bitmap_xNOdo,"/home/utnso/workspace/tp-2017-2c-Mi-Grupo-1234/filesystem/bitmaps/");
		string_append(&PATH_bitmap_xNOdo,nodoConectado);
		string_append(&PATH_bitmap_xNOdo,".dat");
		crearBitmap(PATH_bitmap_xNOdo,nodoConectado,cantBloques);

	}
	list_iterate(nodosConectados,(void*)crearBitmapXNodo);

}

void registrarNodo(char * nombre,int bloques,int socket ,char * IP,char * puerto){

	crearBitmapDeNodosConectados(nombre,bloques);
	tablaBitmapXNodos * nodoConBitmap = buscarNodoPorNombreB(nombre);
	ContenidoXNodo * nodo = malloc(sizeof(ContenidoXNodo));
	nodo->nodo=nombre;
	nodo->ip=IP;
	nodo->libre= cantidadDeBloquesLibresEnBitmap(nodoConBitmap->bitarray,bloques);
	nodo->puerto=puerto;
	nodo->socket=socket;
	nodo->total=bloques;

	list_add(tablaNodos,nodo);


}

char * conseguirNombreDePath(char * PATH){
	char** pathDividido = NULL;
	pathDividido = string_split(PATH, "/");
	int q=0;
	while(pathDividido[q]!=NULL){
		q++;
	}
	char * nombre =pathDividido[q-1];
	return nombre;
}

int cantidadBloquesAMandar(char * PATH){
	FILE* archivo=fopen(PATH,"r+");
	int tamanoArchivo =1;
	int cantidadBloques =0;
	fseek(archivo,0,SEEK_END);
	tamanoArchivo=ftell(archivo);
	cantidadBloques = ceil((float)tamanoArchivo/1048576);
	return cantidadBloques;

}

void partirArchivoBinario(char* PATH){
	FILE* archivo = fopen(PATH, "r+");
	int fd = fileno(archivo);
	int tamano;
	struct stat buff;
	fstat(fd, &buff);
	tamano = buff.st_size;

	void * archivoABytes = malloc(buff.st_size);
	void * contenidoAEnviar = malloc(1048576);
int i=50;
	while (tamano >= 0) {
		char * puta=string_new();
					string_append(&puta,"/home/utnso/Escritorio/archivonuevo");
					string_append(&puta,string_itoa(i));
					FILE*archivo2=fopen(puta,"a+");

		if (tamano > 0 && tamano < 1048576) {
			fread(contenidoAEnviar, tamano, 1, archivo);
			fwrite(contenidoAEnviar,tamano,1,archivo2);//aca haria el send?
			break;
		} else {
			fread(contenidoAEnviar, 1048576, 1, archivo);
			fwrite(contenidoAEnviar,1048576,1,archivo2);
			tamano -= 1048576;
		}
i++;
	}

	free(contenidoAEnviar);
	free(archivoABytes);
	fclose(archivo);
}

void partirArchivoDeTexto(char * PATH){

	FILE*archivo=fopen(PATH,"r+");
	int fd=fileno(archivo);
	struct stat buff;
	fstat(fd,&buff);
	int tamano = buff.st_size;
	char caracter;
	int a;
	int ultimoBarraN=0;
	int ultimoBarraNAnterior=0;
	int i= 1;
	int ultimoBarraNAntesDeMega=0;
	void * contenidoAEnviar;
	int * posicionBarraN;
	int * posicionArchivoTerminado;
	char * nombre = conseguirNombreDePath(PATH);
	tablaArchivo * nuevoArchivo = malloc(sizeof(tablaArchivo));

t_list * posiciones =list_create();

		while(!feof(archivo)){

			caracter = fgetc(archivo);

			if(caracter=='\n'){

				a=ftell(archivo);
				ultimoBarraN=ftell(archivo);

			}

				if(ftell(archivo)==1048576+ultimoBarraNAntesDeMega){
					ultimoBarraNAntesDeMega=ultimoBarraN;
					posicionBarraN=malloc(sizeof(int));
					*posicionBarraN=ultimoBarraN;
					list_add(posiciones,posicionBarraN);
					ultimoBarraNAnterior=ultimoBarraN;
			}

			else{

			}
		}//while


		fseek(archivo,0,SEEK_END);

		posicionArchivoTerminado=malloc(sizeof(int));

		*posicionArchivoTerminado=ftell(archivo);

		list_add(posiciones,posicionArchivoTerminado);

		fseek(archivo,0,SEEK_SET);

		int posicionActual = 0;

		void partir(int * posicion){

			char * puta=string_new();
			string_append(&puta,"/home/utnso/Escritorio/archivonuevo");
			string_append(&puta,string_itoa(posicionActual));

			FILE*archivo2=fopen(puta,"a+");

						nuevoArchivo->tamanio=tamano;
						nuevoArchivo->nombre=nombre;
						nuevoArchivo->directorioPadre=1;
						nuevoArchivo->tipo=1;
						nuevoArchivo->bloqueCopias = list_create();
						Bloque * bloques = malloc(sizeof(Bloque));
						bloques->bloqueNodo = list_create();
						ContenidoBloque * contenido = malloc(sizeof(ContenidoBloque));


			if(posicionActual==0){
			contenidoAEnviar=malloc(*posicion);
			fread(contenidoAEnviar,*posicion,1,archivo);


			bloques->bytesBloque=*posicion;

			tablaBitmapXNodos * nodo = obtenerNodoConMayorPosicionLibre();
			int posicion0 = buscarPosicionLibre(nodo->bitarray,nodo->cantidadBloques);
			bitarray_set_bit(nodo->bitarray,posicion0);
			printf("Guarde original en %s\n",nodo->nodo);
			printf("\n");

			contenido->bloque=posicion0;
			contenido->nodo=nodo->nodo;
			list_add(bloques->bloqueNodo,contenido);
			list_add(nuevoArchivo->bloqueCopias,bloques);

			fwrite(contenidoAEnviar,*posicion,1,archivo2);//aca haria el send

			tablaBitmapXNodos * nodo1 = obtenerNodoConMayorPosicionLibre();
			int posicion = buscarPosicionLibre(nodo1->bitarray,nodo1->cantidadBloques);
			bitarray_set_bit(nodo1->bitarray,posicion);
			printf("Guarde copia en %s\n",nodo1->nodo);
			printf("\n");

			//segundo send para la copia

			contenido->bloque=posicion;
			contenido->nodo=nodo1->nodo;
			list_add(bloques->bloqueNodo,contenido);
			list_add(nuevoArchivo->bloqueCopias,bloques);


			}else {

				int posicionAnterior=*((int*)list_get(posiciones,posicionActual-1));
				contenidoAEnviar=malloc((*posicion)-posicionAnterior);
				fread(contenidoAEnviar,(*posicion)-posicionAnterior,1,archivo);


				bloques->bytesBloque=(*posicion)-posicionAnterior;

				tablaBitmapXNodos * nodo2 = obtenerNodoConMayorPosicionLibre();
				int posicion1 = buscarPosicionLibre(nodo2->bitarray,nodo2->cantidadBloques);
							bitarray_set_bit(nodo2->bitarray,posicion1);
							printf("Guarde original en %s\n",nodo2->nodo);
							printf("\n");

							contenido->bloque=posicion1;
							contenido->nodo=nodo2->nodo;
							list_add(bloques->bloqueNodo,contenido);
							list_add(nuevoArchivo->bloqueCopias,bloques);

				fwrite(contenidoAEnviar,(*posicion)-posicionAnterior,1,archivo2);

				tablaBitmapXNodos * nodo3 = obtenerNodoConMayorPosicionLibre();
				int posicion2 = buscarPosicionLibre(nodo3->bitarray,nodo3->cantidadBloques);
							bitarray_set_bit(nodo3->bitarray,posicion2);
							printf("Guarde copia en %s\n",nodo3->nodo);
							printf("\n");

							contenido->bloque=posicion2;
							contenido->nodo=nodo3->nodo;
							list_add(bloques->bloqueNodo,contenido);
							list_add(nuevoArchivo->bloqueCopias,bloques);

				//aca el otro send? gg

			}
			posicionActual++;
		fclose(archivo2);
		}
		list_iterate(posiciones,(void*)partir);
		list_add(tablaArchivos,nuevoArchivo);
		fclose(archivo);
		free(contenidoAEnviar);
}

void almacenarArchivo(char * PATH, int TipoArchivo){

	if (TipoArchivo == 1) {
		partirArchivoBinario(PATH);
	} else {
		partirArchivoDeTexto(PATH);
	}
}

void leerArchivo(char * PATH) {
	puts("leo archivo yamafs");
}

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

	int cantStrings = 11, i=0;
	char **arrayMensajesSerializar = malloc(cantStrings);


	arrayMensajesSerializar[i] = malloc(string_length(cantBloquesFile) + 1);
	strcpy(arrayMensajesSerializar[i], cantBloquesFile);
	arrayMensajesSerializar[i][string_length(cantBloquesFile)] = '\0';

	arrayMensajesSerializar[i+1] = malloc(string_length(bloque0NodoCopia0) + 1);
	strcpy(arrayMensajesSerializar[i+1], bloque0NodoCopia0);
	arrayMensajesSerializar[i+1][string_length(bloque0NodoCopia0)] = '\0';

	arrayMensajesSerializar[i+2] = malloc(string_length(bloque0BloqueCopia0) + 1);
	strcpy(arrayMensajesSerializar[i+2], bloque0BloqueCopia0);
	arrayMensajesSerializar[i+2][string_length(bloque0BloqueCopia0)] = '\0';

	arrayMensajesSerializar[i+3] = malloc(string_length(bloque0NodoCopia1) + 1);
	strcpy(arrayMensajesSerializar[i+3], bloque0NodoCopia1);
	arrayMensajesSerializar[i+3][string_length(bloque0NodoCopia1)] = '\0';

	arrayMensajesSerializar[i+4] = malloc(string_length(bloque0BloqueCopia1) + 1);
	strcpy(arrayMensajesSerializar[i+4], bloque0BloqueCopia1);
	arrayMensajesSerializar[i+4][string_length(bloque0BloqueCopia1)] = '\0';

	arrayMensajesSerializar[i+5] = malloc(string_length(bytesBloque0) + 1);
	strcpy(arrayMensajesSerializar[i+5], bytesBloque0);
	arrayMensajesSerializar[i+5][string_length(bytesBloque0)] = '\0';

	arrayMensajesSerializar[i+6] = malloc(string_length(bloque1NodoCopia0) + 1);
	strcpy(arrayMensajesSerializar[i+6], bloque1NodoCopia0);
	arrayMensajesSerializar[i+6][string_length(bloque1NodoCopia0)] = '\0';

	arrayMensajesSerializar[i+7] = malloc(string_length(bloque1BloqueCopia0) + 1);
	strcpy(arrayMensajesSerializar[i+7], bloque1BloqueCopia0);
	arrayMensajesSerializar[i+7][string_length(bloque1BloqueCopia0)] = '\0';

	arrayMensajesSerializar[i+8] = malloc(string_length(bloque1NodoCopia1) + 1);
	strcpy(arrayMensajesSerializar[i+8], bloque1NodoCopia1);
	arrayMensajesSerializar[i+8][string_length(bloque1NodoCopia1)] = '\0';

	arrayMensajesSerializar[i+9] = malloc(string_length(bloque1BloqueCopia1) + 1);
	strcpy(arrayMensajesSerializar[i+9], bloque1BloqueCopia1);
	arrayMensajesSerializar[i+9][string_length(bloque1BloqueCopia1)] = '\0';

	arrayMensajesSerializar[i+10] = malloc(string_length(bytesBloque1) + 1);
	strcpy(arrayMensajesSerializar[i+10], bytesBloque1);
	arrayMensajesSerializar[i+10][string_length(bytesBloque1)] = '\0';

	char *mensajeSerializado = serializarMensaje(TIPO_MSJ_METADATA_ARCHIVO, arrayMensajesSerializar, cantStrings);
	printf("mensaje serializado: %s\n", mensajeSerializado);
	enviarMensaje(socketCliente, mensajeSerializado);
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
	//int SocketWorker;
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
					nuevoSocket = accept(listener,
							(struct sockaddr *) &remoteaddr, &addrlen);

					if (nuevoSocket == -1) {
						perror("accept");
					} else {
						FD_SET(nuevoSocket, &master);
						recv(nuevoSocket, &quiensoy, sizeof(int), MSG_WAITALL); //Devuelve -1 en error
						switch (quiensoy) {

						case yama: {
							if(estable)
							{
								SocketYama = nuevoSocket;
								enviarInfoBloques(SocketYama);
							puts("conectado");
							}
							else{
								puts("todavia no es estado estable");
							}
						}
							break;
						case worker: {
							//SocketWorker = nuevoSocket;
							//atender a worker,supongo que almacenar el archivo
						}
							break;
						case datanode: {
							registrarNodo("Nodo1",60,nuevoSocket,"192.168.1.1","5000");
							registrarNodo("Nodo2",90,nuevoSocket,"192.168.1.1","5000");
							registrarNodo("Nodo3",150,nuevoSocket,"192.168.1.1","5000");
							registrarNodo("Nodo4",150,nuevoSocket,"192.168.1.1","5000");

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
						if (mensaje.accion == 0) {*/
							printf("Se murio socket %d \n", i); ///hacer log
						/*} else {
							perror("Hubo un error que no deberia pasar");
						}*/
							close(i);//se cierra el socket conectado,si se comenta esto
						//el socket no cierra de este lado por ende podria seguir recibiendo mensajes
						FD_CLR(i, &master);
					/*} else {
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


int sumatoriaDeBloquesLibres(){
	int suma = 0;
	void sumar(tablaBitmapXNodos * elemento){
		int a = cantidadDeBloquesLibresEnBitmap(elemento->bitarray,elemento->cantidadBloques);
		suma = suma +a ;
	}
	list_iterate(listaDeBitMap,(void*)sumar);
return suma;
}
int sumatoriaDeBloquesTotal(){
	int suma = 0;
		void sumar(tablaBitmapXNodos * elemento){
			int a = elemento->cantidadBloques;
			suma = suma +a ;
		}
		list_iterate(listaDeBitMap,(void*)sumar);
		return suma;
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



int main() {

	char * PUERTO;
	logFs = log_create("/home/utnso/workspace/tp-2017-2c-Mi-Grupo-1234/filesystem/FileSystem.log", "FileSystem", 0, 0);
	FILE * directorios =fopen("/home/utnso/workspace/tp-2017-2c-Mi-Grupo-1234/filesystem/directorios.dat","w+");
	FILE * nodos =fopen("/home/utnso/workspace/tp-2017-2c-Mi-Grupo-1234/filesystem/nodo.bin","wb");
	tablaNodos = list_create();
	tablaArchivos = list_create();
	/*tablaNodosGlobal=malloc(sizeof(tablaDeNodos));
	tablaNodosGlobal->nodo=list_create();
	tablaNodosGlobal->contenidoXNodo=list_create();*/
	listaDeBitMap =list_create();

	//pthread_t hiloConsola;
	configFs =config_create("/home/utnso/workspace/tp-2017-2c-Mi-Grupo-1234/config/configFilesystem.txt");
	PUERTO = config_get_string_value(configFs, "PUERTO_PROPIO");
	//pthread_create(&hiloConsola,NULL,(void*)IniciarConsola,NULL);
	//soyServidor(PUERTO);
	//pthread_join(hiloConsola, NULL);
	registrarNodo("Nodo1",60,2,"192.168.1.1","5000");
	registrarNodo("Nodo2",90,3,"192.168.1.1","5000");
	registrarNodo("Nodo3",150,4,"192.168.1.1","5000");
	registrarNodo("Nodo4",150,5,"192.168.1.1","5000");
	//partirArchivoDeTexto("/home/utnso/Escritorio/Nuevo.txt");
	//partirArchivoDeTexto("/home/utnso/Escritorio/FileSystem.h");
/*
	tablaArchivo * archivo = buscarArchivoPorNombre("Nuevo.txt");
	printf("%s\n",archivo->nombre);
	printf("bytes : %d\n",archivo->tamanio);
	tablaArchivo * archivo2 = buscarArchivoPorNombre("FileSystem.h");
	printf("%s\n",archivo2->nombre);
	printf("bytes : %d\n",archivo2->tamanio);

*/

	fclose(directorios);
	fclose(nodos);
	log_destroy(logFs);
	return 0;
}
