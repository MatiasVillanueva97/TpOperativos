#include "filesystem.h"

bool buscarEnListaAnterior(char * nombre) {
	bool buscarEnLISTA(char * nombreDentroDeLista) {
		return strcmp(nombreDentroDeLista, nombre) == 0;
	}
	return list_any_satisfy(listaDeNodosAnteriores, (void*) buscarEnLISTA);
}
int cantidadBloquesAMandar(char * PATH) {
	FILE* archivo = fopen(PATH, "r+");
	int tamanoArchivo = 1;
	int cantidadBloques = 0;
	fseek(archivo, 0, SEEK_END);
	tamanoArchivo = ftell(archivo);
	cantidadBloques = ceil((float) tamanoArchivo / 1048576);
	return cantidadBloques;

}

int cantidadDeBloquesLibresEnBitmap(t_bitarray * bitmap, int cantBloques) {
	int k;
	int i = 0;
	for (k = 0; k < cantBloques; k++) {
		bool as2 = !bitarray_test_bit(bitmap, k);
		if (as2 == true) {
			//printf("libre posicion %d\n",k);
			i++;
		} else {
			//printf("ocupado posicion %d\n",k);
		}
	}
	return i;
}

int sumatoriaDeBloquesLibres() {
	int suma = 0;
	void sumar(tablaBitmapXNodos * elemento) {
		int a = cantidadDeBloquesLibresEnBitmap(elemento->bitarray, elemento->cantidadBloques);
		suma = suma + a;
	}
	list_iterate(listaDeBitMap, (void*) sumar);
	return suma;
}
int sumatoriaDeBloquesTotal() {
	int suma = 0;
	void sumar(tablaBitmapXNodos * elemento) {
		int a = elemento->cantidadBloques;
		suma = suma + a;
	}
	list_iterate(listaDeBitMap, (void*) sumar);
	return suma;
}

int buscarPosicionLibre(t_bitarray * bitmap, int cantBloque) {
	int i;
	for (i = 0; i < cantBloque; i++) {
		if (!bitarray_test_bit(bitmap, i)) {
			//	log_info(logFs,"Se encontro la posicion %d en el bitmap libre.",i);
			return i;
		}
	}
	return -1;
	log_info(logFs, "No hay posicion libre");
}

tablaBitmapXNodos * obtenerNodoConMayorPosicionLibre() {

	double mayor = 0;
	void obtenerMayor(tablaBitmapXNodos * elemento) {
		int cantidadBloquesLibres = cantidadDeBloquesLibresEnBitmap(elemento->bitarray, elemento->cantidadBloques);
		int cantidadBLoques = elemento->cantidadBloques;
		double porcentaje = 100 * ((double) cantidadBloquesLibres / (double) cantidadBLoques);
		if (porcentaje > mayor) {
			mayor = porcentaje;
		}

	}
	list_iterate(listaDeBitMap, (void*) obtenerMayor);

	bool buscarNodoMasLibre(tablaBitmapXNodos * element) {
		int z = cantidadDeBloquesLibresEnBitmap(element->bitarray, element->cantidadBloques);
		int j = element->cantidadBloques;
		double porcentaje = 100 * ((double) z / (double) j);
		if (porcentaje == mayor) {
			return true;
		} else {
			return false;
		}

	}
	tablaBitmapXNodos * nodoMasLibre = list_find(listaDeBitMap, (void*) buscarNodoMasLibre);
	return nodoMasLibre;
}

tablaBitmapXNodos * buscarNodoPorNombreB(char * Nodo) {

	bool buscarEnLISTA(tablaBitmapXNodos * elemento) {
		return strcmp(elemento->nodo, Nodo) == 0;
	}
	tablaBitmapXNodos * nodoEncontrado = list_find(listaDeBitMap, (void*) buscarEnLISTA);
	return nodoEncontrado;
}

ContenidoXNodo * buscarNodoPorNombreS(char * Nodo) {

	bool buscarEnLISTA(ContenidoXNodo * elemento) {
		return strcmp(elemento->nodo, Nodo) == 0;
	}
	ContenidoXNodo * nodoEncontrado = list_find(tablaNodos, (void*) buscarEnLISTA);
	return nodoEncontrado;
}

tablaArchivo * buscarArchivoPorNombre(char * nombreArchivo) {

	bool buscarEnLISTA(tablaArchivo * elemento) {
		return strcmp(elemento->nombre, nombreArchivo) == 0;
	}
	tablaArchivo * archivoEncontrado = list_find(tablaArchivos, (void*) buscarEnLISTA);
	return archivoEncontrado;
}

void crearBitmap(char * PATH, char * nodoConectado, int cantBloques) {
	int FileD;
	tablaBitmapXNodos * aux3 = malloc(sizeof(tablaBitmapXNodos));
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
	aux3->bitarray = bitarray_create(bitmap2, ceil(((double) cantBloques) / 8.0));
	log_info(logFs, "[Configurar Todo]-Se creo correctamente el bitmap [%s]", nodoConectado);

	aux3->nodo = nodoConectado;
	aux3->cantidadBloques = cantBloques;
	list_add(listaDeBitMap, aux3);

}
//hacer recv de los nodos del nombre del nodo
void crearBitmapDeNodosConectados(char * NodoConectado, int cantBloques) {
	t_list * nodosConectados = list_create();
	list_add(nodosConectados, string_duplicate(NodoConectado));

	void crearBitmapXNodo(char * nodoConectado) {

		char * PATH_bitmap_xNOdo = string_new();
		string_append(&PATH_bitmap_xNOdo, "/home/utnso/workspace/tp-2017-2c-Mi-Grupo-1234/filesystem/bitmaps/");
		string_append(&PATH_bitmap_xNOdo, nodoConectado);
		string_append(&PATH_bitmap_xNOdo, ".dat");
		crearBitmap(PATH_bitmap_xNOdo, nodoConectado, cantBloques);

	}
	list_iterate(nodosConectados, (void*) crearBitmapXNodo);

}

void registrarNodo(int socketData) {
	int32_t header = deserializarHeader(socketData);

	int cantMensajes = protocoloCantidadMensajes[header];
	char ** arrayMensajesRecibidos = deserializarMensaje(socketData, cantMensajes);

	char *nombre = arrayMensajesRecibidos[0];
	int cantBloques = atoi(arrayMensajesRecibidos[1]);
	crearBitmapDeNodosConectados(nombre, cantBloques);
	tablaBitmapXNodos * nodoConBitmap = buscarNodoPorNombreB(arrayMensajesRecibidos[0]);
	ContenidoXNodo * nodo = malloc(sizeof(ContenidoXNodo));
	nodo->nodo = arrayMensajesRecibidos[0];
	nodo->ip = arrayMensajesRecibidos[2];
	nodo->libre = cantidadDeBloquesLibresEnBitmap(nodoConBitmap->bitarray, atoi(arrayMensajesRecibidos[1]));
	nodo->puerto = arrayMensajesRecibidos[3];
	nodo->socket = socketData;
	nodo->total = atoi(arrayMensajesRecibidos[1]);

	printf("%s\n", nodo->nodo);
	printf("%s\n", nodo->ip);
	printf("%d\n", nodo->libre);
	printf("%s\n", nodo->puerto);
	printf("%d\n", nodo->socket);
	printf("%d\n", nodo->total);
	/*if{

	 }*/

	list_add(tablaNodos, nodo);

}

char * conseguirNombreDePath(char * PATH) {
	char** pathDividido = NULL;
	pathDividido = string_split(PATH, "/");
	int q = 0;
	while (pathDividido[q] != NULL) {
		q++;
	}
	char * nombre = pathDividido[q - 1];
	return nombre;
}

void persistirArchivos(char * nombre) {

	tablaArchivo * elemento = buscarArchivoPorNombre(nombre);
	char * path = string_duplicate("/home/utnso/workspace/tp-2017-2c-Mi-Grupo-1234/filesystem/metadata/archivos/");
	string_append(&path, string_itoa(elemento->directorioPadre));
	string_append(&path, "/");
	string_append(&path, elemento->nombre);
	FILE * archivo = fopen(path, "a+");
	persistirArchivo = config_create(path);
	int j = 0;
	int i = 0;
	void persistirBloquesDeArchivo(ContenidoBloque * bloquesArchivo) {
		if ((j == 1) && (i == j)) {
			i = 0;
		}
		if (j > 1) {
			j = 0;
		}
		if ((j == 1) && (i > 2)) {
			j = 0;
		}

		char * bloque_copia = string_new();
		char * bloque_bytes = string_new();
		string_append(&bloque_copia, "BLOQUE");
		string_append(&bloque_copia, string_itoa(i));
		string_append(&bloque_copia, "COPIA");
		string_append(&bloque_copia, string_itoa(j));
		string_append(&bloque_bytes, "BLOQUE");
		string_append(&bloque_bytes, string_itoa(i));
		string_append(&bloque_bytes, "BYTES");
		char * array = string_duplicate("[");
		string_append(&array, bloquesArchivo->nodo);
		string_append(&array, ",");
		string_append(&array, string_itoa(bloquesArchivo->bloque));
		string_append(&array, "]");
		//	printf("%s\n",array);
		//	printf("%s\n",bloque_copia);
		//	printf("%s\n",bloque_bytes);

		config_set_value(persistirArchivo, bloque_copia, array);
		j++;
		config_set_value(persistirArchivo, bloque_bytes, string_itoa(bloquesArchivo->bytes));
		i++;
	}
	list_iterate(elemento->bloqueCopias, (void*) persistirBloquesDeArchivo);
	config_set_value(persistirArchivo, "NOMBRE", elemento->nombre);
	config_set_value(persistirArchivo, "TAMANIO", string_itoa(elemento->tamanio));
	config_set_value(persistirArchivo, "DIRECTORIO_PADRE", string_itoa(elemento->directorioPadre));
	config_set_value(persistirArchivo, "TIPO", string_itoa(elemento->tipo));
	config_save(persistirArchivo);
}

void partirArchivoBinario(char* PATH) {
	FILE* archivo = fopen(PATH, "r+");
	int fd = fileno(archivo);
	int tamano;
	struct stat buff;
	fstat(fd, &buff);
	tamano = buff.st_size;
	char * nombre = conseguirNombreDePath(PATH);
	tablaArchivo * nuevoArchivo = malloc(sizeof(tablaArchivo));
	nuevoArchivo->bloqueCopias = list_create();
	nuevoArchivo->nombre = nombre;
	nuevoArchivo->tamanio = tamano;
	nuevoArchivo->directorioPadre = 1;
	nuevoArchivo->tipo = 0;

	void * archivoABytes = malloc(buff.st_size);
	void * contenidoAEnviar = malloc(1048576);
	int i = 0;

	if (sumatoriaDeBloquesLibres() >= (cantidadBloquesAMandar(PATH)) * 2) {

		while (tamano >= 0) {
			ContenidoBloque * contenido = malloc(sizeof(ContenidoBloque));

			if (tamano > 0 && tamano < 1048576) {

				fread(contenidoAEnviar, tamano, 1, archivo);
				printf("%d\n", tamano);
				tablaBitmapXNodos * nodo2 = obtenerNodoConMayorPosicionLibre();
				ContenidoXNodo * nodoBuscado3 = buscarNodoPorNombreS(nodo2->nodo);
				int posicion1 = buscarPosicionLibre(nodo2->bitarray, nodo2->cantidadBloques);
				bitarray_set_bit(nodo2->bitarray, posicion1);
				printf("Guarde original en %s\n", nodo2->nodo);
				printf("\n");

				ContenidoBloque * contenido3 = malloc(sizeof(ContenidoBloque));
				contenido3->bytes = tamano;
				contenido3->bloque = posicion1;
				contenido3->nodo = nodo2->nodo;
				list_add(nuevoArchivo->bloqueCopias, contenido3);

				//SEND
				int cantStrings = 3;
				int NumeroDeBloqueDondeGuardar = posicion1;
				char * NumeroDeBloqueDondeGuardarString = intToArrayZerosLeft(NumeroDeBloqueDondeGuardar, 4);
				int tipoDeArchivo = 0;
				char * tipoDeArchivoString = intToArrayZerosLeft(tipoDeArchivo, 4);
				char **arrayMensajesSerializar = malloc(sizeof(char*) * cantStrings);
				if (!arrayMensajesSerializar)
					perror("error de malloc 1");

				int i = 0;
				arrayMensajesSerializar[i] = malloc(string_length(contenidoAEnviar) + 1);
				if (!arrayMensajesSerializar[i])
					perror("error de malloc 1");
				strcpy(arrayMensajesSerializar[i], contenidoAEnviar);
				i++;
				arrayMensajesSerializar[i] = malloc(string_length(NumeroDeBloqueDondeGuardarString) + 1);
				if (!arrayMensajesSerializar[i])
					perror("error de malloc 1");
				strcpy(arrayMensajesSerializar[i], NumeroDeBloqueDondeGuardarString);
				i++;
				arrayMensajesSerializar[i] = malloc(string_length(tipoDeArchivoString) + 1);
				if (!arrayMensajesSerializar[i])
					perror("error de malloc 1");
				strcpy(arrayMensajesSerializar[i], tipoDeArchivoString);
				i++;
				char *mensajeSerializado = serializarMensaje(TIPO_MSJ_ARCHIVO, arrayMensajesSerializar, cantStrings);
				//printf("%s\n",mensajeSerializado);
				int bytesEnviados = enviarMensaje(nodoBuscado3->socket, mensajeSerializado);
				printf("bytes enviados: %d\n", bytesEnviados);
				//SEND

				tablaBitmapXNodos * nodo3 = obtenerNodoConMayorPosicionLibre();
				ContenidoXNodo * nodobuscado4 = buscarNodoPorNombreS(nodo3->nodo);
				int posicion2 = buscarPosicionLibre(nodo3->bitarray, nodo3->cantidadBloques);
				bitarray_set_bit(nodo3->bitarray, posicion2);
				printf("Guarde copia en %s\n", nodo3->nodo);
				printf("\n");
				ContenidoBloque * contenido4 = malloc(sizeof(ContenidoBloque));
				contenido4->bytes = tamano;
				contenido4->bloque = posicion2;
				contenido4->nodo = nodo3->nodo;
				list_add(nuevoArchivo->bloqueCopias, contenido4);

				//SEND - Copia
				int cantStrings1 = 3;
				int NumeroDeBloqueDondeGuardar1 = posicion2;
				char * NumeroDeBloqueDondeGuardarString1 = intToArrayZerosLeft(NumeroDeBloqueDondeGuardar1, 4);
				int tipoDeArchivo1 = 1;
				char * tipoDeArchivoString1 = intToArrayZerosLeft(tipoDeArchivo1, 4);
				char **arrayMensajesSerializar1 = malloc(sizeof(char*) * cantStrings1);
				if (!arrayMensajesSerializar1)
					perror("error de malloc 1");

				i = 0;
				arrayMensajesSerializar1[i] = malloc(string_length(contenidoAEnviar) + 1);
				if (!arrayMensajesSerializar1[i])
					perror("error de malloc 1");
				strcpy(arrayMensajesSerializar1[i], contenidoAEnviar);
				i++;
				arrayMensajesSerializar1[i] = malloc(string_length(NumeroDeBloqueDondeGuardarString1) + 1);
				if (!arrayMensajesSerializar1[i])
					perror("error de malloc 1");
				strcpy(arrayMensajesSerializar1[i], NumeroDeBloqueDondeGuardarString1);
				i++;
				arrayMensajesSerializar1[i] = malloc(string_length(tipoDeArchivoString1) + 1);
				if (!arrayMensajesSerializar1[i])
					perror("error de malloc 1");
				strcpy(arrayMensajesSerializar1[i], tipoDeArchivoString1);
				i++;
				char *mensajeSerializado1 = serializarMensaje(TIPO_MSJ_ARCHIVO, arrayMensajesSerializar1, cantStrings1);
				//printf("%s\n",mensajeSerializado);
				int bytesEnviados1 = enviarMensaje(nodobuscado4->socket, mensajeSerializado1);
				printf("bytes enviados: %d\n", bytesEnviados1);
				//SEND - Copia

				break;
			} else {
				fread(contenidoAEnviar, 1048576, 1, archivo);

				tablaBitmapXNodos * nodo = obtenerNodoConMayorPosicionLibre();
				ContenidoXNodo * nodoBuscado = buscarNodoPorNombreS(nodo->nodo);
				int posicion0 = buscarPosicionLibre(nodo->bitarray, nodo->cantidadBloques);
				bitarray_set_bit(nodo->bitarray, posicion0);
				printf("Guarde original en %s\n", nodo->nodo);
				printf("\n");

				contenido->bytes = 1048576;
				contenido->bloque = posicion0;
				contenido->nodo = nodo->nodo;
				list_add(nuevoArchivo->bloqueCopias, contenido);

				log_info(logFs, "Archivo - bloque %d ", contenido->bloque);
				log_info(logFs, "Archivo  - nodo %s", contenido->nodo);

				//SEND
				int cantStrings = 3;
				int NumeroDeBloqueDondeGuardar = posicion0;
				char * NumeroDeBloqueDondeGuardarString = intToArrayZerosLeft(NumeroDeBloqueDondeGuardar, 4);
				int tipoDeArchivo = 1;
				char * tipoDeArchivoString = intToArrayZerosLeft(tipoDeArchivo, 4);
				char **arrayMensajesSerializar = malloc(sizeof(char*) * cantStrings);
				if (!arrayMensajesSerializar)
					perror("error de malloc 1");

				int i = 0;
				arrayMensajesSerializar[i] = malloc(string_length(contenidoAEnviar) + 1);
				if (!arrayMensajesSerializar[i])
					perror("error de malloc 1");
				strcpy(arrayMensajesSerializar[i], contenidoAEnviar);
				i++;
				arrayMensajesSerializar[i] = malloc(string_length(NumeroDeBloqueDondeGuardarString) + 1);
				if (!arrayMensajesSerializar[i])
					perror("error de malloc 1");
				strcpy(arrayMensajesSerializar[i], NumeroDeBloqueDondeGuardarString);
				i++;
				arrayMensajesSerializar[i] = malloc(string_length(tipoDeArchivoString) + 1);
				if (!arrayMensajesSerializar[i])
					perror("error de malloc 1");
				strcpy(arrayMensajesSerializar[i], tipoDeArchivoString);
				i++;
				char *mensajeSerializado = serializarMensaje(TIPO_MSJ_ARCHIVO, arrayMensajesSerializar, cantStrings);
				//printf("%s\n",mensajeSerializado);
				int bytesEnviados = enviarMensaje(nodoBuscado->socket, mensajeSerializado);
				printf("bytes enviados: %d\n", bytesEnviados);
				//SEND

				tablaBitmapXNodos * nodo1 = obtenerNodoConMayorPosicionLibre();
				ContenidoXNodo * nodoBuscado2 = buscarNodoPorNombreS(nodo1->nodo);
				int posicion7 = buscarPosicionLibre(nodo1->bitarray, nodo1->cantidadBloques);
				bitarray_set_bit(nodo1->bitarray, posicion7);
				printf("Guarde copia en %s\n", nodo1->nodo);
				printf("\n");

				ContenidoBloque * contenido2 = malloc(sizeof(ContenidoBloque));
				contenido2->bytes = 1048576;
				contenido2->bloque = posicion7;
				contenido2->nodo = nodo1->nodo;
				list_add(nuevoArchivo->bloqueCopias, contenido2);

				//SEND - Copia
				int cantStrings1 = 3;
				int NumeroDeBloqueDondeGuardar1 = posicion7;
				char * NumeroDeBloqueDondeGuardarString1 = intToArrayZerosLeft(NumeroDeBloqueDondeGuardar1, 4);
				int tipoDeArchivo1 = 1;
				char * tipoDeArchivoString1 = intToArrayZerosLeft(tipoDeArchivo1, 4);
				char **arrayMensajesSerializar1 = malloc(sizeof(char*) * cantStrings1);
				if (!arrayMensajesSerializar1)
					perror("error de malloc 1");

				i = 0;
				arrayMensajesSerializar1[i] = malloc(string_length(contenidoAEnviar) + 1);
				if (!arrayMensajesSerializar1[i])
					perror("error de malloc 1");
				strcpy(arrayMensajesSerializar1[i], contenidoAEnviar);
				i++;
				arrayMensajesSerializar1[i] = malloc(string_length(NumeroDeBloqueDondeGuardarString1) + 1);
				if (!arrayMensajesSerializar1[i])
					perror("error de malloc 1");
				strcpy(arrayMensajesSerializar1[i], NumeroDeBloqueDondeGuardarString1);
				i++;
				arrayMensajesSerializar1[i] = malloc(string_length(tipoDeArchivoString1) + 1);
				if (!arrayMensajesSerializar1[i])
					perror("error de malloc 1");
				strcpy(arrayMensajesSerializar1[i], tipoDeArchivoString1);
				i++;
				char *mensajeSerializado1 = serializarMensaje(TIPO_MSJ_ARCHIVO, arrayMensajesSerializar1, cantStrings1);
				//printf("%s\n",mensajeSerializado);
				int bytesEnviados1 = enviarMensaje(nodoBuscado2->socket, mensajeSerializado1);
				printf("bytes enviados: %d\n", bytesEnviados1);
				//SEND - Copia

				tamano -= 1048576;
			}
			i++;
		}
		list_add(tablaArchivos, nuevoArchivo);
		persistirArchivos(nombre);
		free(contenidoAEnviar);
		free(archivoABytes);
		fclose(archivo);
	} else {
		printf("No hay espacio suficiente para partir el archivo %s \n", nombre);
	}

}

void partirArchivoDeTexto(char * PATH) {

	FILE*archivo = fopen(PATH, "r+");
	int fd = fileno(archivo);
	struct stat buff;
	fstat(fd, &buff);
	int tamano = buff.st_size;
	char caracter;
	int a;
	int ultimoBarraN = 0;
	int ultimoBarraNAnterior = 0;
	int i = 1;
	int ultimoBarraNAntesDeMega = 0;
	void * contenidoAEnviar;
	int * posicionBarraN;
	int * posicionArchivoTerminado;
	char * nombre = conseguirNombreDePath(PATH);
	tablaArchivo * nuevoArchivo = malloc(sizeof(tablaArchivo));
	nuevoArchivo->bloqueCopias = list_create();

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
		nuevoArchivo->tamanio = tamano;
		nuevoArchivo->nombre = nombre;
		nuevoArchivo->directorioPadre = 1;
		nuevoArchivo->tipo = 1;
		log_info(logFs, "Archivo %s", nuevoArchivo->nombre);
		ContenidoBloque * contenido = malloc(sizeof(ContenidoBloque));

		if (posicionActual == 0) {
			contenidoAEnviar = malloc(*posicion);
			fread(contenidoAEnviar, *posicion, 1, archivo);

			tablaBitmapXNodos * nodo = obtenerNodoConMayorPosicionLibre();
			ContenidoXNodo * nodoBuscado = buscarNodoPorNombreS(nodo->nodo);
			int posicion0 = buscarPosicionLibre(nodo->bitarray, nodo->cantidadBloques);
			bitarray_set_bit(nodo->bitarray, posicion0);
			printf("Guarde original en %s\n", nodo->nodo);
			printf("\n");

			contenido->bytes = *posicion;
			contenido->bloque = posicion0;
			contenido->nodo = nodo->nodo;
			list_add(nuevoArchivo->bloqueCopias, contenido);

			log_info(logFs, "Archivo - bloque %d ", contenido->bloque);
			log_info(logFs, "Archivo  - nodo %s", contenido->nodo);

			//SEND
			int cantStrings = 3;
			int NumeroDeBloqueDondeGuardar = posicion0;
			char * NumeroDeBloqueDondeGuardarString = intToArrayZerosLeft(NumeroDeBloqueDondeGuardar, 4);
			int tipoDeArchivo = 1;
			char * tipoDeArchivoString = intToArrayZerosLeft(tipoDeArchivo, 4);
			char **arrayMensajesSerializar = malloc(sizeof(char*) * cantStrings);
			if (!arrayMensajesSerializar)
				perror("error de malloc 1");

			int i = 0;
			arrayMensajesSerializar[i] = malloc(string_length(contenidoAEnviar) + 1);
			if (!arrayMensajesSerializar[i])
				perror("error de malloc 1");
			strcpy(arrayMensajesSerializar[i], contenidoAEnviar);
			i++;
			arrayMensajesSerializar[i] = malloc(string_length(NumeroDeBloqueDondeGuardarString) + 1);
			if (!arrayMensajesSerializar[i])
				perror("error de malloc 1");
			strcpy(arrayMensajesSerializar[i], NumeroDeBloqueDondeGuardarString);
			i++;
			arrayMensajesSerializar[i] = malloc(string_length(tipoDeArchivoString) + 1);
			if (!arrayMensajesSerializar[i])
				perror("error de malloc 1");
			strcpy(arrayMensajesSerializar[i], tipoDeArchivoString);
			i++;
			char *mensajeSerializado = serializarMensaje(TIPO_MSJ_ARCHIVO, arrayMensajesSerializar, cantStrings);
			printf("nodoBuscado->socket adentro de partir: %d\n", nodoBuscado->socket);
			int bytesEnviados = enviarMensaje(nodoBuscado->socket, mensajeSerializado);
			printf("bytes enviados: %d\n", bytesEnviados);
			//SEND

			tablaBitmapXNodos * nodo1 = obtenerNodoConMayorPosicionLibre();
			ContenidoXNodo * nodoBuscado2 = buscarNodoPorNombreS(nodo1->nodo);
			int posicion7 = buscarPosicionLibre(nodo1->bitarray, nodo1->cantidadBloques);
			bitarray_set_bit(nodo1->bitarray, posicion7);
			printf("Guarde copia en %s\n", nodo1->nodo);
			printf("\n");

			ContenidoBloque * contenido2 = malloc(sizeof(ContenidoBloque));
			contenido2->bytes = *posicion;
			contenido2->bloque = posicion7;
			contenido2->nodo = nodo1->nodo;
			list_add(nuevoArchivo->bloqueCopias, contenido2);

			//SEND - Copia
			int cantStrings1 = 3;
			int NumeroDeBloqueDondeGuardar1 = posicion7;
			char * NumeroDeBloqueDondeGuardarString1 = intToArrayZerosLeft(NumeroDeBloqueDondeGuardar1, 4);
			int tipoDeArchivo1 = 1;
			char * tipoDeArchivoString1 = intToArrayZerosLeft(tipoDeArchivo1, 4);
			char **arrayMensajesSerializar1 = malloc(sizeof(char*) * cantStrings1);
			if (!arrayMensajesSerializar1)
				perror("error de malloc 1");

			i = 0;
			arrayMensajesSerializar1[i] = malloc(string_length(contenidoAEnviar) + 1);
			if (!arrayMensajesSerializar1[i])
				perror("error de malloc 1");
			strcpy(arrayMensajesSerializar1[i], contenidoAEnviar);
			i++;
			arrayMensajesSerializar1[i] = malloc(string_length(NumeroDeBloqueDondeGuardarString1) + 1);
			if (!arrayMensajesSerializar1[i])
				perror("error de malloc 1");
			strcpy(arrayMensajesSerializar1[i], NumeroDeBloqueDondeGuardarString1);
			i++;
			arrayMensajesSerializar1[i] = malloc(string_length(tipoDeArchivoString1) + 1);
			if (!arrayMensajesSerializar1[i])
				perror("error de malloc 1");
			strcpy(arrayMensajesSerializar1[i], tipoDeArchivoString1);
			i++;
			char *mensajeSerializado1 = serializarMensaje(TIPO_MSJ_ARCHIVO, arrayMensajesSerializar1, cantStrings1);
			printf("nodoBuscado2->socket adentro de partir: %d\n", nodoBuscado2->socket);
			int bytesEnviados1 = enviarMensaje(nodoBuscado2->socket, mensajeSerializado1);
			printf("bytes enviados: %d\n", bytesEnviados1);
			//SEND - Copia

			log_info(logFs, "Archivo - bloque %d ", contenido2->bloque);
			log_info(logFs, "Archivo  - nodo %s", contenido2->nodo);

		} else {

			int posicionAnterior = *((int*) list_get(posiciones, posicionActual - 1));
			contenidoAEnviar = malloc((*posicion) - posicionAnterior);
			fread(contenidoAEnviar, (*posicion) - posicionAnterior, 1, archivo);

			tablaBitmapXNodos * nodo2 = obtenerNodoConMayorPosicionLibre();
			ContenidoXNodo * nodoBuscado3 = buscarNodoPorNombreS(nodo2->nodo);
			int posicion1 = buscarPosicionLibre(nodo2->bitarray, nodo2->cantidadBloques);
			bitarray_set_bit(nodo2->bitarray, posicion1);
			printf("Guarde original en %s\n", nodo2->nodo);
			printf("\n");

			ContenidoBloque * contenido3 = malloc(sizeof(ContenidoBloque));
			contenido3->bytes = (*posicion) - posicionAnterior;
			contenido3->bloque = posicion1;
			contenido3->nodo = nodo2->nodo;
			list_add(nuevoArchivo->bloqueCopias, contenido3);

			//SEND
			int cantStrings = 3;
			int NumeroDeBloqueDondeGuardar = posicion1;
			char * NumeroDeBloqueDondeGuardarString = intToArrayZerosLeft(NumeroDeBloqueDondeGuardar, 4);
			int tipoDeArchivo = 1;
			char * tipoDeArchivoString = intToArrayZerosLeft(tipoDeArchivo, 4);
			char **arrayMensajesSerializar = malloc(sizeof(char*) * cantStrings);
			if (!arrayMensajesSerializar)
				perror("error de malloc 1");

			int i = 0;
			arrayMensajesSerializar[i] = malloc(string_length(contenidoAEnviar) + 1);
			if (!arrayMensajesSerializar[i])
				perror("error de malloc 1");
			strcpy(arrayMensajesSerializar[i], contenidoAEnviar);
			i++;
			arrayMensajesSerializar[i] = malloc(string_length(NumeroDeBloqueDondeGuardarString) + 1);
			if (!arrayMensajesSerializar[i])
				perror("error de malloc 1");
			strcpy(arrayMensajesSerializar[i], NumeroDeBloqueDondeGuardarString);
			i++;
			arrayMensajesSerializar[i] = malloc(string_length(tipoDeArchivoString) + 1);
			if (!arrayMensajesSerializar[i])
				perror("error de malloc 1");
			strcpy(arrayMensajesSerializar[i], tipoDeArchivoString);
			i++;
			char *mensajeSerializado = serializarMensaje(TIPO_MSJ_ARCHIVO, arrayMensajesSerializar, cantStrings);
			//printf("%s\n",mensajeSerializado);
			printf("nodoBuscado3->socket adentro de partir: %d\n", nodoBuscado3->socket);
			int bytesEnviados = enviarMensaje(nodoBuscado3->socket, mensajeSerializado);
			printf("bytes enviados: %d\n", bytesEnviados);
			//SEND

			log_info(logFs, "Archivo - bloque %d ", contenido3->bloque);
			log_info(logFs, "Archivo  - nodo %s", contenido3->nodo);

			tablaBitmapXNodos * nodo3 = obtenerNodoConMayorPosicionLibre();
			ContenidoXNodo * nodobuscado4 = buscarNodoPorNombreS(nodo3->nodo);
			int posicion2 = buscarPosicionLibre(nodo3->bitarray, nodo3->cantidadBloques);
			bitarray_set_bit(nodo3->bitarray, posicion2);
			printf("Guarde copia en %s\n", nodo3->nodo);
			printf("\n");
			ContenidoBloque * contenido4 = malloc(sizeof(ContenidoBloque));
			contenido4->bytes = (*posicion) - posicionAnterior;
			contenido4->bloque = posicion2;
			contenido4->nodo = nodo3->nodo;
			list_add(nuevoArchivo->bloqueCopias, contenido4);

			//SEND - Copia
			int cantStrings1 = 3;
			int NumeroDeBloqueDondeGuardar1 = posicion2;
			char * NumeroDeBloqueDondeGuardarString1 = intToArrayZerosLeft(NumeroDeBloqueDondeGuardar1, 4);
			int tipoDeArchivo1 = 1;
			char * tipoDeArchivoString1 = intToArrayZerosLeft(tipoDeArchivo1, 4);
			char **arrayMensajesSerializar1 = malloc(sizeof(char*) * cantStrings1);
			if (!arrayMensajesSerializar1)
				perror("error de malloc 1");

			i = 0;
			arrayMensajesSerializar1[i] = malloc(string_length(contenidoAEnviar) + 1);
			if (!arrayMensajesSerializar1[i])
				perror("error de malloc 1");
			strcpy(arrayMensajesSerializar1[i], contenidoAEnviar);
			i++;
			arrayMensajesSerializar1[i] = malloc(string_length(NumeroDeBloqueDondeGuardarString1) + 1);
			if (!arrayMensajesSerializar1[i])
				perror("error de malloc 1");
			strcpy(arrayMensajesSerializar1[i], NumeroDeBloqueDondeGuardarString1);
			i++;
			arrayMensajesSerializar1[i] = malloc(string_length(tipoDeArchivoString1) + 1);
			if (!arrayMensajesSerializar1[i])
				perror("error de malloc 1");
			strcpy(arrayMensajesSerializar1[i], tipoDeArchivoString1);
			i++;
			char *mensajeSerializado1 = serializarMensaje(TIPO_MSJ_ARCHIVO, arrayMensajesSerializar1, cantStrings1);
			//printf("%s\n",mensajeSerializado);
			printf("nodobuscado4->socket adentro de partir: %d\n", nodobuscado4->socket);
			int bytesEnviados1 = enviarMensaje(nodobuscado4->socket, mensajeSerializado1);
			printf("bytes enviados: %d\n", bytesEnviados1);
			//SEND - Copia

			log_info(logFs, "Archivo - bloque %d ", contenido4->bloque);
			log_info(logFs, "Archivo  - nodo %s", contenido4->nodo);

		}
		posicionActual++;
	}
	if (sumatoriaDeBloquesLibres() >= (cantidadBloquesAMandar(PATH)) * 2) {

		list_iterate(posiciones, (void*) partir);
		list_add(tablaArchivos, nuevoArchivo);
		persistirArchivos(nombre);
		fclose(archivo);
		free(contenidoAEnviar);
	} else {
		char * holi = conseguirNombreDePath(PATH);
		printf("No hay espacio para partir el archivo %s \n", holi);
	}
}

void almacenarArchivo(char * PATH, int TipoArchivo) {

	if (TipoArchivo == 1) {
		partirArchivoBinario(PATH);
	} else {
		partirArchivoDeTexto(PATH);
	}
}

void leerArchivo(char * PATH) {
	char * nombre = conseguirNombreDePath(PATH);
	tablaArchivo * archivoABuscar = buscarArchivoPorNombre(nombre);
	char *arrayDeOriginalesYcopias[100];
	int j = 0;
	void buscar(ContenidoBloque * elemento) {

		ContenidoXNodo * nodoEncontrado = buscarNodoPorNombreS(elemento->nodo);

		//mando el bloque que quiero leer
		int cantStrings = 1;
		int numeroDeBLoqueQueQuiero = elemento->bloque;
		printf(" numeroDeBLoqueQueQuiero  %d del nodo %s \n", numeroDeBLoqueQueQuiero, elemento->nodo);
		char * NumeroDeBloqueDondeGuardarString = intToArrayZerosLeft(numeroDeBLoqueQueQuiero, 4);
		char **arrayMensajesSerializar = malloc(sizeof(char*) * cantStrings);
		if (!arrayMensajesSerializar)
			perror("error de malloc 1");

		int i = 0;
		arrayMensajesSerializar[i] = malloc(string_length(NumeroDeBloqueDondeGuardarString) + 1);
		if (!arrayMensajesSerializar[i])
			perror("error de malloc 1");
		strcpy(arrayMensajesSerializar[i], NumeroDeBloqueDondeGuardarString);
		i++;
		char *mensajeSerializado = serializarMensaje(TIPO_MSJ_PEDIR_BLOQUES, arrayMensajesSerializar, cantStrings);
		//printf("%s\n",mensajeSerializado);
		printf("nodoEncontrado->socket adentro de partir: %d\n", nodoEncontrado->socket);
		//lock
		int bytesEnviados = enviarMensaje(nodoEncontrado->socket, mensajeSerializado);
		printf("bytes enviados: %d\n", bytesEnviados);
		////mando el bloque que quiero leer

		//recibo el buffer

		int32_t headerRecibo = deserializarHeader(nodoEncontrado->socket);
		printf("header adentro de leerARchivo(): %d\n", headerRecibo);
		int cantMensajesRecibidos = protocoloCantidadMensajes[headerRecibo];
		char ** arrayMensajesRecibidos = deserializarMensaje(nodoEncontrado->socket, cantMensajesRecibidos);
		//unlock
		//recibo el buffer
		//printf("buffer: %s\n", arrayMensajesRecibidos[0]);
	}
	pthread_mutex_lock(&mutex1);
	list_iterate(archivoABuscar->bloqueCopias, (void*) buscar);
	pthread_mutex_unlock(&mutex1);
	void printearArrayOriginal() {
		int i;
		int b = countSplit(arrayDeOriginalesYcopias);
		int c = (cantidadBloquesAMandar(PATH) * 2);
		for (i = 0; i < c; i++) {
			printf("%s\n", arrayDeOriginalesYcopias[i]);
			//getchar();
		}

	}
	//printearArrayOriginal();
	puts("leo archivo yamafs");
}

void enviarInfoBloques(int socketCliente, int headerId) {
//	int32_t headerId = deserializarHeader(socketCliente);
	printf("id: %d\n", headerId);
	printf("mensaje predefinido: %s\n", protocoloMensajesPredefinidos[headerId]);
	int cantidadMensajes = protocoloCantidadMensajes[headerId];
	char **arrayMensajes = deserializarMensaje(socketCliente, cantidadMensajes);
	char *archivo = malloc(string_length(arrayMensajes[0]) + 1);
	strcpy(archivo, arrayMensajes[0]);
	free(arrayMensajes);
	printf("archivo: %s\n", archivo);
	char * nombre = conseguirNombreDePath(archivo);
	tablaArchivo * archivoRecibido = buscarArchivoPorNombre(nombre);

	int cantBloquesFiles = cantidadBloquesAMandar(archivo);	//path completo yamafs:
	char *cantBloquesFileString = intToArrayZerosLeft(cantBloquesFiles, 4);
	int cantMensajesPorPedazoArchivo = 6;
	int cantStrings = 1 + cantMensajesPorPedazoArchivo * cantBloquesFiles;
	int i;
	char **arrayMensajesSerializar = malloc(sizeof(char*) * cantStrings);

	i = 0;
	arrayMensajesSerializar[i] = malloc(string_length(cantBloquesFileString) + 1);
	if (!arrayMensajesSerializar[i])
		perror("error de malloc 1");
	strcpy(arrayMensajesSerializar[i], cantBloquesFileString);
	i++;
	void impresion(ContenidoBloque * hola) {

		char *nodo = hola->nodo;
		//printf("%s\n", nodo);
		char *bloque = intToArrayZerosLeft(hola->bloque, 4);

		//printf("%s\n",bloque);
		char *bytes = intToArrayZerosLeft(hola->bytes, 8);
		//printf("%s\n",bytes);

		arrayMensajesSerializar[i] = malloc(string_length(nodo) + 1);
		if (!arrayMensajesSerializar[i])
			perror("error de malloc 2");
		strcpy(arrayMensajesSerializar[i], nodo);
		i++;

		arrayMensajesSerializar[i] = malloc(string_length(bloque) + 1);
		if (!arrayMensajesSerializar[i])
			perror("error de malloc 3");
		strcpy(arrayMensajesSerializar[i], bloque);
		i++;

		arrayMensajesSerializar[i] = malloc(string_length(bytes) + 1);
		if (!arrayMensajesSerializar[i])
			perror("error de malloc 4");
		strcpy(arrayMensajesSerializar[i], bytes);
		i++;
	}
	list_iterate(archivoRecibido->bloqueCopias, (void*) impresion);

	char *mensajeSerializado = serializarMensaje(TIPO_MSJ_METADATA_ARCHIVO, arrayMensajesSerializar, cantStrings);
	printf("mensaje serializado: %s\n", mensajeSerializado);
	int bytesEnviados = enviarMensaje(socketCliente, mensajeSerializado);
	printf("bytes enviados: %d\n", bytesEnviados);
}

void enviarInfoNodos(int socketCliente) {

	int cantidadNodos = list_size(tablaNodos);	//path completo yamafs:
	printf("cantBloquesFiles: %d\n", cantidadNodos);
	char * cantidadNodosString = intToArrayZerosLeft(cantidadNodos, 4);
	printf("cantBloquesFileString: %s\n", cantidadNodosString);
	int cantMensajePorNodo = 3;
	int cantStrings = 1 + cantMensajePorNodo * cantidadNodos;
	int i;
	char **arrayMensajesSerializar = malloc(sizeof(char*) * cantStrings);

	i = 0;
	arrayMensajesSerializar[i] = malloc(string_length(cantidadNodosString) + 1);
	if (!arrayMensajesSerializar[i])
		perror("error de malloc 1");
	strcpy(arrayMensajesSerializar[i], cantidadNodosString);
	printf("arrayMensajesSerializar[0]: %s\n", arrayMensajesSerializar[0]);
	i++;
	void impresion(ContenidoXNodo * elemento) {

		char * nodo = elemento->nodo;
		char * ip = elemento->ip;
		char * puerto = elemento->puerto;

		arrayMensajesSerializar[i] = malloc(string_length(nodo) + 1);
		if (!arrayMensajesSerializar[i])
			perror("error de malloc 2");
		strcpy(arrayMensajesSerializar[i], nodo);
		i++;

		arrayMensajesSerializar[i] = malloc(string_length(ip) + 1);
		if (!arrayMensajesSerializar[i])
			perror("error de malloc 3");
		strcpy(arrayMensajesSerializar[i], ip);
		i++;

		arrayMensajesSerializar[i] = malloc(string_length(puerto) + 1);
		if (!arrayMensajesSerializar[i])
			perror("error de malloc 4");
		strcpy(arrayMensajesSerializar[i], puerto);
		i++;
	}
	list_iterate(tablaNodos, (void*) impresion);

	char *mensajeSerializado = serializarMensaje(TIPO_MSJ_DATOS_CONEXION_NODOS, arrayMensajesSerializar, cantStrings);
	printf("mensaje serializado: %s\n", mensajeSerializado);
	int bytesEnviados = enviarMensaje(socketCliente, mensajeSerializado);
	printf("bytes enviados: %d\n", bytesEnviados);
}

void persistirNodosFuncion() {
	char * nombre = string_duplicate("[");
	FILE * archivo = fopen("/home/utnso/workspace/tp-2017-2c-Mi-Grupo-1234/filesystem/metadata/nodos.bin", "w+");
	fputs("TAMANO_TOTAL", archivo);
	fputs("=", archivo);
	fputs(string_itoa(sumatoriaDeBloquesTotal()), archivo);
	fputc('\n', archivo);
	fputs("TAMANO_LIBRE", archivo);
	fputs("=", archivo);
	fputs(string_itoa(sumatoriaDeBloquesLibres()), archivo);
	fputc('\n', archivo);

	void persistir(ContenidoXNodo * elemento) {
		string_append(&nombre, elemento->nodo);
		string_append(&nombre, ",");
		char * nombreLibre = string_duplicate(elemento->nodo);
		string_append(&nombreLibre, "Libre");
		char * nombreTotal = string_duplicate(elemento->nodo);
		string_append(&nombreTotal, "Total");
		tablaBitmapXNodos * NodoEncontrado = buscarNodoPorNombreB(elemento->nodo);

		fputs(nombreTotal, archivo);
		fputs("=", archivo);
		fputs(string_itoa(elemento->total), archivo);
		fputc('\n', archivo);
		fputs(nombreLibre, archivo);
		fputs("=", archivo);
		fputs(string_itoa(cantidadDeBloquesLibresEnBitmap(NodoEncontrado->bitarray, NodoEncontrado->cantidadBloques)), archivo);
		fputc('\n', archivo);

	}
	list_iterate(tablaNodos, (void*) persistir);
	string_append(&nombre, "]");
	fputs("NODOS", archivo);
	fputs("=", archivo);
	fputs(nombre, archivo);
	fputc('\n', archivo);

	fclose(archivo);

}

int countSplit(char ** array) {
	int size;
	for (size = 0; array[size] != NULL; size++)
		;
	return size;
}

void cargarListaDeNodosAnteriores() {
	char ** arrayDeNodosAnteriores = config_get_array_value(persistirNodos, "NODOS");

	int i = 0;
	for (; i < countSplit(arrayDeNodosAnteriores); i++) {
		list_add(listaDeNodosAnteriores, arrayDeNodosAnteriores[i]);
		printf("%s\n", arrayDeNodosAnteriores[i]);
	}
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
								enviarHeaderSolo(SocketYama,TIPO_MSJ_HANDSHAKE_RESPUESTA_OK);
								//enviarInfoNodos(SocketYama);
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
							if (estable) {

							} else {

							}

							registrarNodo(nuevoSocket);
							persistirNodosFuncion();
							//FD_CLR(nuevoSocket, &master);
							estable = true;
						}
							break;
						}
						if (nuevoSocket > fdmax) {
							fdmax = nuevoSocket;
						}
					}
				} else {
					//lock
					pthread_mutex_lock(&mutex1);
					int32_t headerId = deserializarHeader(i);
					printf("headerId: %d\n", headerId);
					pthread_mutex_unlock(&mutex1);
					if (headerId <= 0) { //error o desconexión de un cliente
						printf("cerró el socket :%d\n", i);
						cerrarCliente(i); // bye!
						FD_CLR(i, &master); // remove from master set
					} else {
						if (i == SocketYama) {
							enviarInfoBloques(SocketYama, headerId);
							enviarInfoNodos(SocketYama);
							puts("soy yama");
						}
						if (i == SocketWorker) {
							puts("soy worker");
						}
					}
					//unlock

//					int muere;
//					recv(i, &muere, sizeof(int), MSG_WAITALL);
//					printf("muere: %d\n", muere);
					//Devuelve -1 en error
//					if (muere <= 0) {
//						//partirArchivoDeTexto("/home/utnso/Escritorio/FileSystem.h");
//						//partirArchivoBinario("/home/utnso/Escritorio/FileSystem.h");
//						printf("Se murio socket %d \n", i); ///hacer log
//						close(i); //se cierra el socket conectado,si se comenta esto
//						//el socket no cierra de este lado por ende podria seguir recibiendo mensajes
//						FD_CLR(i, &master);
//					}
					//Devuelve -1 en error
					/* esto sirve para cuando ya estan conectados ,
					 si te envian mensajes la funcion
					 recibirmensaje era parte de mi serializacion
					 void * stream;
					 Mensaje mensaje = recibirMensaje(i, &stream);
					 if (mensaje.accion <= 0) {
					 if (mensaje.accion == 0) {*/
					//partirArchivoDeTexto("/home/utnso/Escritorio/Nuevo.txt");
//hacer eliminar algun dia		ContenidoXNodo * hola2 = list_remove_by_condition(tablaNodos,buscarNodoPorNombreS("NODO_1"));
					/*} else {
					 perror("Hubo un error que no deberia pasar");
					 }*/

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

char * devolverPathAbsoluto(char * pathYamaFS) {
	char * pathAbsoluto = string_new();
	string_append(&pathAbsoluto, "/home/utnso/workspace/tp-2017-2c-Mi-Grupo-1234/filesystem/yamafs/");
	string_append(&pathAbsoluto, pathYamaFS);
	return pathAbsoluto;
}
int main() {
	pthread_mutex_init(&mutex1, NULL);
	char * PUERTO;
	logFs = log_create("/home/utnso/workspace/tp-2017-2c-Mi-Grupo-1234/filesystem/FileSystem.log", "FileSystem", 0, 0);
	FILE * directorios = fopen("/home/utnso/workspace/tp-2017-2c-Mi-Grupo-1234/filesystem/directorios.dat", "w+");
	FILE * nodos = fopen("/home/utnso/workspace/tp-2017-2c-Mi-Grupo-1234/filesystem/nodo.bin", "wb");
	tablaNodos = list_create();
	tablaArchivos = list_create();
	listaDeBitMap = list_create();
	listaDeNodosAnteriores = list_create();

	pthread_t hiloConsola;
	configFs = config_create("/home/utnso/workspace/tp-2017-2c-Mi-Grupo-1234/config/configFilesystem.txt");
	persistirNodos = config_create("/home/utnso/workspace/tp-2017-2c-Mi-Grupo-1234/filesystem/metadata/nodos.bin");
	PUERTO = config_get_string_value(configFs, "PUERTO_PROPIO");
	bool estado = config_get_string_value(configFs, "ESTADO");
	config_set_value(configFs, "ESTADO", "false");
	config_save(configFs);
	pthread_create(&hiloConsola, NULL, (void*) IniciarConsola, NULL);

	//list_add(listaDeNodosAnteriores,"nodito");
	bool x = buscarEnListaAnterior("nodito");
	soyServidor(PUERTO);
	//pthread_join(hiloConsola, NULL);

	fclose(directorios);
	fclose(nodos);
	log_destroy(logFs);
	return 0;
}
