#include "filesystem.h"
#include "../../utils/consola.c"

///

void persistirDirectorio(){
	char* path=string_new();
	char * puerto = config_get_string_value(configFs,"PATH_METADATA");
	string_append(&path,config_get_string_value(configFs,"PATH_METADATA"));
	string_append(&path,"directorios.dat");

	FILE* archivoDirectorio=fopen(path,"w+");
	int cont=0;
	int cantTotal=list_size(listaDirectorios);

	while(cont<cantTotal){
		fputs("DIRECTORIO",archivoDirectorio);
		fputs(string_itoa(cont),archivoDirectorio);
		fputs("=[",archivoDirectorio);
		tablaDeDirectorios* directorioSeleccionado=list_get(listaDirectorios,cont);
		fputs(string_itoa(directorioSeleccionado->index),archivoDirectorio);
		fputc(',',archivoDirectorio);
		fputs(directorioSeleccionado->nombre,archivoDirectorio);
		fputc(',',archivoDirectorio);
		fputs(string_itoa(directorioSeleccionado->padre),archivoDirectorio);
		fputc(']',archivoDirectorio);
		fputc('\n',archivoDirectorio);
		cont++;
	}
	fclose(archivoDirectorio);

}

void persistirRegistroArchivo(){
	char* path=string_new();
	string_append(&path,config_get_string_value(configFs,"PATH_ARCHIVOS"));
	string_append(&path,"registro.dat");

	FILE* archivoRegistro=fopen(path,"w+");
	int cont=0;
	int cantTotal=list_size(registroArchivos);
	while(cont<cantTotal){
		fputs("ARCHIVO",archivoRegistro);
		fputs(string_itoa(cont),archivoRegistro);
		fputs("=",archivoRegistro);
		char* pathArchivo=list_get(registroArchivos,cont);
		fputs(pathArchivo,archivoRegistro);
		fputc('\n',archivoRegistro);
		cont++;
	}
	fclose(archivoRegistro);

}

void persistirArchivos(tablaArchivo  * elemento){

	char* path=string_new();
	string_append(&path,config_get_string_value(configFs,"PATH_ARCHIVOS"));
	string_append(&path,string_itoa(elemento->directorioPadre));
	string_append(&path,"/");
	char* comando=string_new();
	string_append(&comando,"mkdir ");
	string_append(&comando,path);
	system(comando);
	string_append(&path,elemento->nombre);


	FILE* archivo=fopen(path,"w+");


	fputs("NOMBRE",archivo);
	fputs("=",archivo);
	fputs(elemento->nombre,archivo);
	fputc('\n',archivo);
	fputs("TAMANIO",archivo);
	fputs("=",archivo);
	fputs(string_itoa(elemento->tamanio),archivo);
	fputc('\n',archivo);
	fputs("DIRECTORIO_PADRE",archivo);
	fputs("=",archivo);
	fputs(string_itoa(elemento->directorioPadre),archivo);
	fputc('\n',archivo);
	fputs("TIPO",archivo);
	fputs("=",archivo);
	fputs(string_itoa(elemento->tipo),archivo);
	fputc('\n',archivo);
	int i=0;
	int j=0;
	int k=0;
	void persistirBloquesDeArchivo(ContenidoBloque * bloquesArchivo){
		if(j>=1){
			j=0;
		}
		if(k%2==0){
			char * bloque_copia =string_new();
			char * bloque_bytes = string_new();
			string_append(&bloque_copia,"BLOQUE");
			string_append(&bloque_copia,string_itoa(i));
			string_append(&bloque_copia,"COPIA");
			string_append(&bloque_copia,string_itoa(j));
			string_append(&bloque_bytes,"BLOQUE");
			string_append(&bloque_bytes,string_itoa(i));
			string_append(&bloque_bytes,"BYTES");
			char * array =string_duplicate("[");
			string_append(&array,bloquesArchivo->nodo);
			string_append(&array,",");
			string_append(&array,string_itoa(bloquesArchivo->bloque));
			string_append(&array,"]");
			fputs(bloque_copia,archivo);
			fputs("=",archivo);
			fputs(array,archivo);
			fputc('\n',archivo);
		}

		else{
			j++;
			char * bloque_copia =string_new();
						char * bloque_bytes = string_new();
						string_append(&bloque_copia,"BLOQUE");
						string_append(&bloque_copia,string_itoa(i));
						string_append(&bloque_copia,"COPIA");
						string_append(&bloque_copia,string_itoa(j));
						string_append(&bloque_bytes,"BLOQUE");
						string_append(&bloque_bytes,string_itoa(i));
						string_append(&bloque_bytes,"BYTES");
						char * array =string_duplicate("[");
						string_append(&array,bloquesArchivo->nodo);
						string_append(&array,",");
						string_append(&array,string_itoa(bloquesArchivo->bloque));
						string_append(&array,"]");
						i++;
						fputs(bloque_copia,archivo);
						fputs("=",archivo);
						fputs(array,archivo);
						fputc('\n',archivo);

		}
	k++;}
		list_iterate(elemento->bloqueCopias,(void*)persistirBloquesDeArchivo);
							fclose(archivo);}


void cargarEstructuraDirectorio(t_config* archivoDirectorio){
	uint32_t cantidadDeDirectorio=config_keys_amount(archivoDirectorio);
	uint32_t posicion=0;
	while(posicion<cantidadDeDirectorio){
		char* etiqueta=string_new();
		string_append(&etiqueta,"DIRECTORIO");
		string_append(&etiqueta,string_itoa(posicion));
		char** arrayDirectorioPosicion = config_get_array_value(archivoDirectorio, etiqueta);
		tablaDeDirectorios* entradaDirectorio=malloc(sizeof(tablaDeDirectorios));
		entradaDirectorio->nombre=string_new();
		entradaDirectorio->index=atoi(arrayDirectorioPosicion[0]);
		entradaDirectorio->nombre=arrayDirectorioPosicion[1];
		entradaDirectorio->padre=atoi(arrayDirectorioPosicion[2]);
		list_add(listaDirectorios,entradaDirectorio);
		posicion++;
	}
}

///

void liberarArrayComando(char** comandoDesarmado){
	int i = 0;
	while(comandoDesarmado[i]!=NULL){
		free(comandoDesarmado[i]);
		i++;
	}
	free(comandoDesarmado);
}

void liberarDirectorio(tablaDeDirectorios* directorio){
  free(directorio->nombre);
  free(directorio);
}


char* obtenerNombreDirectorio(char** rutaDesmembrada){
  int posicion = 0;
  char* nombreArchivo = string_new();
  while(1){
    if(rutaDesmembrada[posicion+1] == NULL){
      string_append(&nombreArchivo, rutaDesmembrada[posicion]);
      break;
    }
    posicion++;
  }
  return nombreArchivo;
}

int obtenerDirectorioPadre(char** rutaDesmembrada){
  char* fathersName = string_new();
  bool isMyFather(tablaDeDirectorios* directory){
    return strcmp(fathersName, directory->nombre) == 0;
  }
  int posicion = 0;
  while(1){
    if(rutaDesmembrada[posicion+1]!=NULL){
      if(rutaDesmembrada[posicion+2] == NULL){
        string_append(&fathersName, rutaDesmembrada[posicion]);
        tablaDeDirectorios* directory = list_find(listaDirectorios, (void*)isMyFather);
        if(directory == NULL){
        	free(fathersName);
        	return -2;
        }
        free(fathersName);
        return directory->index;
      }
    }else if(rutaDesmembrada[posicion+1]==NULL){
    	free(fathersName);
      return -1;
    }
    posicion++;
  }
}

tablaDeDirectorios* createDirectory(){
  tablaDeDirectorios* newDirectory = malloc(sizeof(tablaDeDirectorios));
  return newDirectory;
}

//MOVER DIRECTORIO

void moveDirectory(char* oldPath, char* newPath){
  char** rutaDesmembradaVieja = string_split(oldPath, "/");
  char** rutaDesmembradaNueva = string_split(newPath, "/");
  char* nombreDirectorio = obtenerNombreDirectorio(rutaDesmembradaVieja);
  int indexPadreNuevo = obtenerDirectorioPadre(rutaDesmembradaNueva);
  if(indexPadreNuevo == -1 || indexPadreNuevo==-2){
	  liberarArrayComando(rutaDesmembradaNueva);
    liberarArrayComando(rutaDesmembradaVieja);
    free(nombreDirectorio);
    printf("Error al encontrar el directorio padre del path final");
  }
  bool esDirectorio(tablaDeDirectorios* directorio){
    return strcmp(directorio->nombre, nombreDirectorio) == 0;
  }

  tablaDeDirectorios* directorioAModificar = list_find(listaDirectorios, (void*)esDirectorio);
  if(directorioAModificar == NULL){
	  liberarArrayComando(rutaDesmembradaNueva);
    liberarArrayComando(rutaDesmembradaVieja);
    free(nombreDirectorio);
    printf("No se ha encontrar el directorio en el sistema");
  }
  directorioAModificar->padre = indexPadreNuevo;
  persistirDirectorio();
  liberarArrayComando(rutaDesmembradaNueva);
  liberarArrayComando(rutaDesmembradaVieja);
  free(nombreDirectorio);
}

//EXISTE DIRECTORIO

bool existeDirectorio(char* ruta){
  char** rutaDesmembrada = string_split(ruta, "/");
  char* nombreDirectory = obtenerNombreDirectorio(rutaDesmembrada);
  bool esDirectorio(tablaDeDirectorios* directory){
    return strcmp(directory->nombre, nombreDirectory)==0;
  }
  bool yaExiste = list_any_satisfy(listaDirectorios, (void*)esDirectorio);
  //AVERIGUAR SI HAY QUE VERIFICAR TAMBIEN EL PADRE
  free(nombreDirectory);
  liberarArrayComando(rutaDesmembrada);
  return yaExiste;
}

//CREAR DIRECTORIO

int crearDirectorio(char* ruta){
  if(list_size(listaDirectorios)<=100){
      uint32_t indexDir = list_size(listaDirectorios);
      tablaDeDirectorios* newDirectory = createDirectory();
      char** rutaDesmembrada = string_split(ruta, "/");
      newDirectory->nombre = obtenerNombreDirectorio(rutaDesmembrada);
      newDirectory->padre = obtenerDirectorioPadre(rutaDesmembrada);
      newDirectory->index = indexDir;
      if(newDirectory->padre == -2){
    	  liberarArrayComando(rutaDesmembrada);
        liberarDirectorio(newDirectory);
        printf("Error de directorio padre de la ruta elegida");
        return 0;
      }
      list_add(listaDirectorios, newDirectory);
      liberarArrayComando(rutaDesmembrada);
      persistirDirectorio();
      printf("Se creo correctamente el directorio");
      return 1;
  }else{
	  printf("No se pudo crear el directorio, se llego al limite");
	  return -1;
  }
}

//RENOMBRAR DIRECTORIO

void renameDirectory(char* oldName, char* newName){
  char** rutaDesmembradaVieja = string_split(oldName, "/");
  char** rutaDesmembradaNueva = string_split(newName, "/");
  char* viejoNombre = obtenerNombreDirectorio(rutaDesmembradaVieja);
  char* nuevoNombre = obtenerNombreDirectorio(rutaDesmembradaNueva);
  bool encontrarPorNombre(tablaDeDirectorios* directorio){
    return strcmp(viejoNombre, directorio->nombre);
  }
  tablaDeDirectorios* directoryToChange = list_find(listaDirectorios, (void*)encontrarPorNombre);
  if(directoryToChange != NULL){
    free(directoryToChange->nombre);
    directoryToChange->nombre = string_new();
    string_append(&directoryToChange->nombre, nuevoNombre);
    liberarArrayComando(rutaDesmembradaNueva);
    liberarArrayComando(rutaDesmembradaVieja);

    persistirDirectorio();

    free(viejoNombre);
    free(nuevoNombre);
    printf("Se ha renombrado exitosamente el directori/archivo.");
  }else{
	  liberarArrayComando(rutaDesmembradaNueva);
    liberarArrayComando(rutaDesmembradaVieja);
    free(viejoNombre);
    free(nuevoNombre);
    printf("El directorio que se pide renombrar no existe.");
  }
}

//OBTENER INDEX DE LA TABLA DIRECTORIO

int obtenerIndexDirectorio(char* nombre){
  bool esDirectorio(tablaDeDirectorios* direct){
    return strcmp(nombre, direct->nombre);
  }
  tablaDeDirectorios* directorio = list_find(listaDirectorios,(void*) esDirectorio);
  if(directorio != NULL){
    return directorio->index;
  }
  return -2;

}

//BORRAR DIRECTORIO

int deleteDirectory(char* directoryToDelete){
  char** rutaDesmembrada = string_split(directoryToDelete, "/");
  char* directoryName = obtenerNombreDirectorio(rutaDesmembrada);
  int indexToDelete = obtenerIndexDirectorio(directoryName);
  if(indexToDelete == -2){
    free(directoryName);
    liberarArrayComando(rutaDesmembrada);

    return 0; //NO EXISTE DIRECTORIO
  }
  bool esDirectorio(tablaDeDirectorios* directorio){
    return strcmp(directorio->nombre, directoryName);
  }
  bool tieneHijos(tablaDeDirectorios* directorio){
    return directorio->padre == indexToDelete;
  }
  if(!list_any_satisfy(listaDirectorios, (void*)tieneHijos)){
    tablaDeDirectorios* directoryToRemove = list_remove_by_condition(listaDirectorios, (void*)esDirectorio);
    persistirDirectorio();

    void liberarArrayComando(char** comandoDesarmado){
    	int i = 0;
    	while(comandoDesarmado[i]!=NULL){
    		free(comandoDesarmado[i]);
    		i++;
    	}
    	free(comandoDesarmado);
    }

    void liberarDirectorio(tablaDeDirectorios* directorio){
      free(directorio->nombre);
      free(directorio);
    }


    char* obtenerNombreDirectorio(char** rutaDesmembrada){
      int posicion = 0;
      char* nombreArchivo = string_new();
      while(1){
        if(rutaDesmembrada[posicion+1] == NULL){
          string_append(&nombreArchivo, rutaDesmembrada[posicion]);
          break;
        }
        posicion++;
      }
      return nombreArchivo;
    }

    int obtenerDirectorioPadre(char** rutaDesmembrada){
      char* fathersName = string_new();
      bool isMyFather(tablaDeDirectorios* directory){
        return strcmp(fathersName, directory->nombre) == 0;
      }
      int posicion = 0;
      while(1){
        if(rutaDesmembrada[posicion+1]!=NULL){
          if(rutaDesmembrada[posicion+2] == NULL){
            string_append(&fathersName, rutaDesmembrada[posicion]);
            tablaDeDirectorios* directory = list_find(listaDirectorios, (void*)isMyFather);
            if(directory == NULL){
            	free(fathersName);
            	return -2;
            }
            free(fathersName);
            return directory->index;
          }
        }else if(rutaDesmembrada[posicion+1]==NULL){
        	free(fathersName);
          return -1;
        }
        posicion++;
      }
    }

    tablaDeDirectorios* createDirectory(){
      tablaDeDirectorios* newDirectory = malloc(sizeof(tablaDeDirectorios));
      return newDirectory;
    }

    //MOVER DIRECTORIO

    void moveDirectory(char* oldPath, char* newPath){
      char** rutaDesmembradaVieja = string_split(oldPath, "/");
      char** rutaDesmembradaNueva = string_split(newPath, "/");
      char* nombreDirectorio = obtenerNombreDirectorio(rutaDesmembradaVieja);
      int indexPadreNuevo = obtenerDirectorioPadre(rutaDesmembradaNueva);
      if(indexPadreNuevo == -1 || indexPadreNuevo==-2){
    	  liberarArrayComando(rutaDesmembradaNueva);
        liberarArrayComando(rutaDesmembradaVieja);
        free(nombreDirectorio);
        printf("Error al encontrar el directorio padre del path final");
      }
      bool esDirectorio(tablaDeDirectorios* directorio){
        return strcmp(directorio->nombre, nombreDirectorio) == 0;
      }

      tablaDeDirectorios* directorioAModificar = list_find(listaDirectorios, (void*)esDirectorio);
      if(directorioAModificar == NULL){
    	  liberarArrayComando(rutaDesmembradaNueva);
        liberarArrayComando(rutaDesmembradaVieja);
        free(nombreDirectorio);
        printf("No se ha encontrar el directorio en el sistema");
      }
      directorioAModificar->padre = indexPadreNuevo;
     persistirDirectorio();
      liberarArrayComando(rutaDesmembradaNueva);
      liberarArrayComando(rutaDesmembradaVieja);
      free(nombreDirectorio);
    }

    //EXISTE DIRECTORIO

    bool existeDirectorio(char* ruta){
      char** rutaDesmembrada = string_split(ruta, "/");
      char* nombreDirectory = obtenerNombreDirectorio(rutaDesmembrada);
      bool esDirectorio(tablaDeDirectorios* directory){
        return strcmp(directory->nombre, nombreDirectory)==0;
      }
      bool yaExiste = list_any_satisfy(listaDirectorios, (void*)esDirectorio);
      //AVERIGUAR SI HAY QUE VERIFICAR TAMBIEN EL PADRE
      free(nombreDirectory);
      liberarArrayComando(rutaDesmembrada);
      return yaExiste;
    }

    //CREAR DIRECTORIO

    int crearDirectorio(char* ruta){
      if(list_size(listaDirectorios)<=100){
          uint32_t indexDir = list_size(listaDirectorios);
          tablaDeDirectorios* newDirectory = createDirectory();
          char** rutaDesmembrada = string_split(ruta, "/");
          newDirectory->nombre = obtenerNombreDirectorio(rutaDesmembrada);
          newDirectory->padre = obtenerDirectorioPadre(rutaDesmembrada);
          newDirectory->index = indexDir;
          if(newDirectory->padre == -2){
        	  liberarArrayComando(rutaDesmembrada);
            liberarDirectorio(newDirectory);
            printf("Error de directorio padre de la ruta elegida");
            return 0;
          }
          list_add(listaDirectorios, newDirectory);
          liberarArrayComando(rutaDesmembrada);
         persistirDirectorio();
          printf("Se creo correctamente el directorio");
          return 1;
      }else{
    	  printf("No se pudo crear el directorio, se llego al limite");
    	  return -1;
      }
    }

    //RENOMBRAR DIRECTORIO

    void renameDirectory(char* oldName, char* newName){
      char** rutaDesmembradaVieja = string_split(oldName, "/");
      char** rutaDesmembradaNueva = string_split(newName, "/");
      char* viejoNombre = obtenerNombreDirectorio(rutaDesmembradaVieja);
      char* nuevoNombre = obtenerNombreDirectorio(rutaDesmembradaNueva);
      bool encontrarPorNombre(tablaDeDirectorios* directorio){
        return strcmp(viejoNombre, directorio->nombre);
      }
      tablaDeDirectorios* directoryToChange = list_find(listaDirectorios, (void*)encontrarPorNombre);
      if(directoryToChange != NULL){
        free(directoryToChange->nombre);
        directoryToChange->nombre = string_new();
        string_append(&directoryToChange->nombre, nuevoNombre);
        liberarArrayComando(rutaDesmembradaNueva);
        liberarArrayComando(rutaDesmembradaVieja);

       persistirDirectorio();

        free(viejoNombre);
        free(nuevoNombre);
        printf("Se ha renombrado exitosamente el directori/archivo.");
      }else{
    	  liberarArrayComando(rutaDesmembradaNueva);
        liberarArrayComando(rutaDesmembradaVieja);
        free(viejoNombre);
        free(nuevoNombre);
        printf("El directorio que se pide renombrar no existe.");
      }
    }

    //OBTENER INDEX DE LA TABLA DIRECTORIO

    int obtenerIndexDirectorio(char* nombre){
      bool esDirectorio(tablaDeDirectorios* direct){
        return strcmp(nombre, direct->nombre);
      }
      tablaDeDirectorios* directorio = list_find(listaDirectorios,(void*) esDirectorio);
      if(directorio != NULL){
        return directorio->index;
      }
      return -2;

    }

    //BORRAR DIRECTORIO

    int deleteDirectory(char* directoryToDelete){
      char** rutaDesmembrada = string_split(directoryToDelete, "/");
      char* directoryName = obtenerNombreDirectorio(rutaDesmembrada);
      int indexToDelete = obtenerIndexDirectorio(directoryName);
      if(indexToDelete == -2){
        free(directoryName);
        liberarArrayComando(rutaDesmembrada);

        return 0; //NO EXISTE DIRECTORIO
      }
      bool esDirectorio(tablaDeDirectorios* directorio){
        return strcmp(directorio->nombre, directoryName);
      }
      bool tieneHijos(tablaDeDirectorios* directorio){
        return directorio->padre == indexToDelete;
      }
      if(!list_any_satisfy(listaDirectorios, (void*)tieneHijos)){
        tablaDeDirectorios* directoryToRemove = list_remove_by_condition(listaDirectorios, (void*)esDirectorio);

      persistirDirectorio();

        liberarDirectorio(directoryToRemove);
        liberarArrayComando(rutaDesmembrada);
        free(directoryName);
        return 1;
      }else{
        free(directoryName);
        liberarArrayComando(rutaDesmembrada);

        return -1; //EL DIRECTORIO TIENE SUBDIRECTORIOS
      }
    }

    liberarDirectorio(directoryToRemove);
    liberarArrayComando(rutaDesmembrada);
    free(directoryName);
    return 1;
  }else{
    free(directoryName);
    liberarArrayComando(rutaDesmembrada);

    return -1; //EL DIRECTORIO TIENE SUBDIRECTORIOS
  }
}

////
int countSplit(char ** array){
		int size;
		for (size = 0; array[size] != NULL; size++);
		return size;
	}
void liberarArray2(char ** lista){
	int i = 0;
	for (i = 0; i < countSplit(lista); i++)
			free(lista[i]);
	free(lista);
}
void liberarArray(char ** lista,int cantidad){
	int i = 0;
	for (i = 0; i < cantidad; i++)
			free(lista[i]);
	free(lista);
}
	void cargarListaDeNodosAnteriores(){
		char ** arrayDeNodosAnteriores = config_get_array_value(persistirNodos,"NODOS");

			int i = 0;
					for(;i<countSplit(arrayDeNodosAnteriores);i++){
						char * nombreNodo = string_new();
						string_append(&nombreNodo,arrayDeNodosAnteriores[i]);
						list_add(listaDeNodosDeEstadoAnterior,nombreNodo);
						printf("%s\n",arrayDeNodosAnteriores[i]);
					}
						liberarArray2(arrayDeNodosAnteriores);
	}

bool buscarEnListaAnterior(char * nombre){
	bool buscarEnLISTA(char * nombreDentroDeLista) {
			return strcmp(nombreDentroDeLista, nombre) == 0;
		}
		return list_any_satisfy(listaDeNodosDeEstadoAnterior,(void*)buscarEnLISTA);
}
bool buscarEnListaDeFormat(char * nombre){
	bool buscarEnLISTA(char * nombreDentroDeLista) {
			return strcmp(nombreDentroDeLista, nombre) == 0;
		}
		return list_any_satisfy(listaDeNodosDeFormateo,(void*)buscarEnLISTA);
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
ContenidoXNodo * buscarNodoPorSocketS(int socket) {

	bool buscarEnLISTA(ContenidoXNodo * elemento) {
		return elemento->socket==socket;
	}
	ContenidoXNodo * nodoEncontrado = list_find(tablaNodos, (void*) buscarEnLISTA);
	return nodoEncontrado;
}

void eliminarPorNombreS(char * Nodo) {

	bool buscarEnLISTA(ContenidoXNodo * elemento) {
		return strcmp(elemento->nodo, Nodo) == 0;
	}
	list_remove_and_destroy_by_condition(tablaNodos,(void*)buscarEnLISTA,free);

}
void eliminarPorNombreB(char * Nodo) {

	bool buscarEnLISTA(tablaBitmapXNodos * elemento) {
		return strcmp(elemento->nodo, Nodo) == 0;
	}
	list_remove_and_destroy_by_condition(listaDeBitMap,(void*)buscarEnLISTA,free);

}
void eliminarDeLasListas(int socket){
	ContenidoXNodo * nodoEncontrad = buscarNodoPorSocketS(socket);
	eliminarPorNombreB(nodoEncontrad->nodo);;
	eliminarPorNombreS(nodoEncontrad->nodo);
}

tablaArchivo * buscarArchivoPorNombre(char * nombreArchivo) {

	bool buscarEnLISTA(tablaArchivo * elemento) {
		return strcmp(elemento->nombre, nombreArchivo) == 0;
	}
	tablaArchivo * archivoEncontrado = list_find(tablaArchivos, (void*) buscarEnLISTA);
	return archivoEncontrado;
}

void persistirNodosFuncion(){
	char * nombre = string_duplicate("[");
	FILE * archivo = fopen ("metadata/nodos.bin","w+");
	fputs("TAMANO_TOTAL",archivo);
	fputs("=",archivo);
	fputs(string_itoa(sumatoriaDeBloquesTotal()),archivo);
	fputc('\n',archivo);
	fputs("TAMANO_LIBRE",archivo);
	fputs("=",archivo);
	fputs(string_itoa(sumatoriaDeBloquesLibres()),archivo);
	fputc('\n',archivo);

	void persistir(ContenidoXNodo * elemento){
		string_append(&nombre,elemento->nodo);
		string_append(&nombre,",");
		char * nombreLibre =string_duplicate(elemento->nodo);
		string_append(&nombreLibre,"Libre");
		char * nombreTotal =string_duplicate(elemento->nodo);
		string_append(&nombreTotal,"Total");
		tablaBitmapXNodos * NodoEncontrado	= buscarNodoPorNombreB(elemento->nodo);

	fputs(nombreTotal,archivo);
	fputs("=",archivo);
	fputs(string_itoa(elemento->total),archivo);
	fputc('\n',archivo);
	fputs(nombreLibre,archivo);
	fputs("=",archivo);
	fputs(string_itoa(cantidadDeBloquesLibresEnBitmap(NodoEncontrado->bitarray,NodoEncontrado->cantidadBloques)),archivo);
	fputc('\n',archivo);


	}
	list_iterate(tablaNodos,(void*)persistir);
	string_append(&nombre,"]");
	fputs("NODOS",archivo);
	fputs("=",archivo);
	fputs(nombre,archivo);
	fputc('\n',archivo);

	fclose(archivo);

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
		string_append(&PATH_bitmap_xNOdo, "bitmaps/");
		string_append(&PATH_bitmap_xNOdo, nodoConectado);
		string_append(&PATH_bitmap_xNOdo, ".dat");
		crearBitmap(PATH_bitmap_xNOdo, nodoConectado, cantBloques);

	}
	list_iterate(nodosConectados, (void*) crearBitmapXNodo);

}

void registrarNodo(int socketData ) {
		int32_t header = deserializarHeader(socketData);
		int cantMensajes=protocoloCantidadMensajes[header];
		char ** arrayMensajesRecibidos = deserializarMensaje(socketData,cantMensajes);
		char *nombre = arrayMensajesRecibidos[0];
		int cantBloques=atoi(arrayMensajesRecibidos[1]);


		if(estadoAnterior==0){
		if(formateado==0){//dejo conectar a cualquiera

		crearBitmapDeNodosConectados(nombre,cantBloques);
		tablaBitmapXNodos * nodoConBitmap = buscarNodoPorNombreB(arrayMensajesRecibidos[0]);
		ContenidoXNodo * nodo = malloc(sizeof(ContenidoXNodo));
		nodo->nodo = arrayMensajesRecibidos[0];
		nodo->ip = arrayMensajesRecibidos[2];
		nodo->libre = cantidadDeBloquesLibresEnBitmap(nodoConBitmap->bitarray, atoi(arrayMensajesRecibidos[1]));
		nodo->puerto = arrayMensajesRecibidos[3];
		nodo->socket = socketData;
		nodo->total = atoi(arrayMensajesRecibidos[1]);

		list_add(tablaNodos, nodo);
		list_add(listaDeNodosDeFormateo,arrayMensajesRecibidos[0]);
		}
		if(formateado==1 && estadoEstable==1){
			if(buscarEnListaDeFormat(arrayMensajesRecibidos[0])){

						crearBitmapDeNodosConectados(nombre,cantBloques);
						tablaBitmapXNodos * nodoConBitmap = buscarNodoPorNombreB(arrayMensajesRecibidos[0]);
						ContenidoXNodo * nodo = malloc(sizeof(ContenidoXNodo));
						nodo->nodo = arrayMensajesRecibidos[0];
						nodo->ip = arrayMensajesRecibidos[2];
						nodo->libre = cantidadDeBloquesLibresEnBitmap(nodoConBitmap->bitarray, atoi(arrayMensajesRecibidos[1]));
						nodo->puerto = arrayMensajesRecibidos[3];
						nodo->socket = socketData;
						nodo->total = atoi(arrayMensajesRecibidos[1]);
						list_add(tablaNodos, nodo);
			}else {
					enviarHeaderSolo(socketData,TIPO_MSJ_HANDSHAKE_RESPUESTA_DENEGADO);
			}

		}
		}
		if(estadoAnterior==1){
			if(buscarEnListaAnterior(arrayMensajesRecibidos[0])){

						crearBitmapDeNodosConectados(nombre,cantBloques);
						tablaBitmapXNodos * nodoConBitmap = buscarNodoPorNombreB(arrayMensajesRecibidos[0]);
						ContenidoXNodo * nodo = malloc(sizeof(ContenidoXNodo));
						nodo->nodo = arrayMensajesRecibidos[0];
						nodo->ip = arrayMensajesRecibidos[2];
						nodo->libre = cantidadDeBloquesLibresEnBitmap(nodoConBitmap->bitarray, atoi(arrayMensajesRecibidos[1]));
						nodo->puerto = arrayMensajesRecibidos[3];
						nodo->socket = socketData;
						nodo->total = atoi(arrayMensajesRecibidos[1]);
						list_add(tablaNodos, nodo);
			}else {
				enviarHeaderSolo(socketData,TIPO_MSJ_HANDSHAKE_RESPUESTA_DENEGADO);
			}
		}

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


void partirArchivoBinario(char* PATH,char * PathDirectorio) {
	FILE* archivo = fopen(PATH, "r+");
	if(archivo == NULL){
		printf("Error al tratar de abrir el archivo en almacenar archivo.\n");
		exit(-1);
	}
	if(existeDirectorio(PathDirectorio)==false){
		printf("El directorio que se ingreso no existe\n");
	}
	int fd = fileno(archivo);
	int tamano;
	struct stat buff;
	fstat(fd, &buff);
	tamano = buff.st_size;
	tablaArchivo * nuevoArchivo = malloc(sizeof(tablaArchivo));

	char** rutaArchivo = string_split(PATH,"/");
	nuevoArchivo->nombre=obtenerNombreDirectorio(rutaArchivo);
	string_append(&PathDirectorio, nuevoArchivo->nombre);

	char** rutaDirectorio = string_split(PathDirectorio,"/");

	printf("Se procede a almacenar el archivo %s en %s.\n", nuevoArchivo->nombre, PathDirectorio);

	nuevoArchivo->bloqueCopias = list_create();
	nuevoArchivo->tamanio = tamano;
	nuevoArchivo->directorioPadre = 1;
	nuevoArchivo->tipo = 0;

	void * archivoABytes = malloc(buff.st_size);
	void * contenidoAEnviar = malloc(1048576);
	int i = 0;

	if(sumatoriaDeBloquesLibres()>=(cantidadBloquesAMandar(PATH))*2){

	while (tamano >= 0) {
		ContenidoBloque * contenido = malloc(sizeof(ContenidoBloque));

		if (tamano > 0 && tamano < 1048576) {

			fread(contenidoAEnviar, tamano, 1, archivo);
			printf("%d\n",tamano);
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
			int cantStrings=3;
			int NumeroDeBloqueDondeGuardar = posicion1;
			char * NumeroDeBloqueDondeGuardarString = intToArrayZerosLeft(NumeroDeBloqueDondeGuardar,4);
			int tipoDeArchivo = 0;
			char * tipoDeArchivoString = intToArrayZerosLeft(tipoDeArchivo,4);
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
						liberarArray(arrayMensajesSerializar,cantStrings);
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
						int cantStrings1=3;
						int NumeroDeBloqueDondeGuardar1 = posicion2;
						char * NumeroDeBloqueDondeGuardarString1 = intToArrayZerosLeft(NumeroDeBloqueDondeGuardar1,4);
						int tipoDeArchivo1 = 1;
						char * tipoDeArchivoString1 = intToArrayZerosLeft(tipoDeArchivo1,4);
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
									liberarArray(arrayMensajesSerializar1,cantStrings1);
									//SEND - Copia

			break;
		} else {
			fread(contenidoAEnviar, 1048576, 1, archivo);

			tablaBitmapXNodos * nodo = obtenerNodoConMayorPosicionLibre();
			ContenidoXNodo * nodoBuscado=buscarNodoPorNombreS(nodo->nodo);
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
			int cantStrings=3;
			int NumeroDeBloqueDondeGuardar = posicion0;
			char * NumeroDeBloqueDondeGuardarString = intToArrayZerosLeft(NumeroDeBloqueDondeGuardar,4);
			int tipoDeArchivo = 1;
			char * tipoDeArchivoString = intToArrayZerosLeft(tipoDeArchivo,4);
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
						liberarArray(arrayMensajesSerializar,cantStrings);
			//SEND


			tablaBitmapXNodos * nodo1 = obtenerNodoConMayorPosicionLibre();
			ContenidoXNodo * nodoBuscado2 =buscarNodoPorNombreS(nodo1->nodo);
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
			int cantStrings1=3;
			int NumeroDeBloqueDondeGuardar1 = posicion7;
			char * NumeroDeBloqueDondeGuardarString1 = intToArrayZerosLeft(NumeroDeBloqueDondeGuardar1,4);
			int tipoDeArchivo1 = 1;
			char * tipoDeArchivoString1 = intToArrayZerosLeft(tipoDeArchivo1,4);
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
						liberarArray(arrayMensajesSerializar1,cantStrings1);
						//SEND - Copia


			tamano -= 1048576;
		}
		i++;
	}
	list_add(tablaArchivos, nuevoArchivo);
	persistirArchivos(nuevoArchivo);
	persistirNodosFuncion();
	free(contenidoAEnviar);
	free(archivoABytes);
	fclose(archivo);
}else {
	printf("No hay espacio suficiente para partir el archivo %s \n",nuevoArchivo->nombre);
}

}



void partirArchivoDeTexto(char* PATH,char * PathDirectorio) {

	FILE* archivo = fopen(PATH, "r+");
		if(archivo == NULL){
			printf("Error al tratar de abrir el archivo en almacenar archivo.\n");
			exit(-1);
		}
		if(existeDirectorio(PathDirectorio)==false){
				printf("El directorio que se ingreso no existe\n");
			}
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

	char** rutaArchivo = string_split(PATH,"/");
		nuevoArchivo->nombre=obtenerNombreDirectorio(rutaArchivo);
		string_append(&PathDirectorio, nuevoArchivo->nombre);

		char** rutaDirectorio = string_split(PathDirectorio,"/");

		printf("Se procede a almacenar el archivo %s en %s.\n", nuevoArchivo->nombre, PathDirectorio);


	if(sumatoriaDeBloquesLibres()>=(cantidadBloquesAMandar(PATH))*2){

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
		nuevoArchivo->directorioPadre = obtenerDirectorioPadre(rutaDirectorio);
		nuevoArchivo->tipo = 0;
		log_info(logFs, "Archivo %s", nuevoArchivo->nombre);
		ContenidoBloque * contenido = malloc(sizeof(ContenidoBloque));

		if (posicionActual == 0) {
			contenidoAEnviar = malloc(*posicion);
			fread(contenidoAEnviar, *posicion, 1, archivo);

			tablaBitmapXNodos * nodo = obtenerNodoConMayorPosicionLibre();
			ContenidoXNodo * nodoBuscado=buscarNodoPorNombreS(nodo->nodo);
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
			int cantStrings=3;
			int NumeroDeBloqueDondeGuardar = posicion0;
			char * NumeroDeBloqueDondeGuardarString = intToArrayZerosLeft(NumeroDeBloqueDondeGuardar,4);
			int tipoDeArchivo = 1;
			char * tipoDeArchivoString = intToArrayZerosLeft(tipoDeArchivo,4);
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
						liberarArray(arrayMensajesSerializar,cantStrings);
			//SEND

			tablaBitmapXNodos * nodo1 = obtenerNodoConMayorPosicionLibre();
			ContenidoXNodo * nodoBuscado2 =buscarNodoPorNombreS(nodo1->nodo);
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
			int cantStrings1=3;
			int NumeroDeBloqueDondeGuardar1 = posicion7;
			char * NumeroDeBloqueDondeGuardarString1 = intToArrayZerosLeft(NumeroDeBloqueDondeGuardar1,4);
			int tipoDeArchivo1 = 1;
			char * tipoDeArchivoString1 = intToArrayZerosLeft(tipoDeArchivo1,4);
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
						liberarArray(arrayMensajesSerializar1,cantStrings1);
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
			int cantStrings=3;
			int NumeroDeBloqueDondeGuardar = posicion1;
			char * NumeroDeBloqueDondeGuardarString = intToArrayZerosLeft(NumeroDeBloqueDondeGuardar,4);
			int tipoDeArchivo = 1;
			char * tipoDeArchivoString = intToArrayZerosLeft(tipoDeArchivo,4);
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
						liberarArray(arrayMensajesSerializar,cantStrings);
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
			int cantStrings1=3;
			int NumeroDeBloqueDondeGuardar1 = posicion2;
			char * NumeroDeBloqueDondeGuardarString1 = intToArrayZerosLeft(NumeroDeBloqueDondeGuardar1,4);
			int tipoDeArchivo1 = 1;
			char * tipoDeArchivoString1 = intToArrayZerosLeft(tipoDeArchivo1,4);
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
						liberarArray(arrayMensajesSerializar1,cantStrings1);
						//SEND - Copia

			log_info(logFs, "Archivo - bloque %d ", contenido4->bloque);
			log_info(logFs, "Archivo  - nodo %s", contenido4->nodo);


		}
		posicionActual++;
	}

	list_iterate(posiciones, (void*) partir);
	list_add(tablaArchivos, nuevoArchivo);

	persistirArchivos(nuevoArchivo);
	persistirNodosFuncion();
	fclose(archivo);
	free(contenidoAEnviar);
	}
	else {
		char * holi = conseguirNombreDePath(PATH);
		printf("No hay espacio suficiente para guardar el archivo %s \n",holi);
	}
}

void almacenarArchivo(char * PATH,char*pathDirectorio, int TipoArchivo) {

	if (TipoArchivo == 1) {
		partirArchivoBinario(PATH,pathDirectorio);
	} else {
		partirArchivoDeTexto(PATH,pathDirectorio);
	}
}

void leerArchivo(char * PATH) {
	char * nombre = conseguirNombreDePath(PATH);
	tablaArchivo * archivoABuscar = buscarArchivoPorNombre(nombre);
	char **arrayDeOriginalesYcopias;
	int j ;
		void buscar(ContenidoBloque * elemento ){

	ContenidoXNodo * nodoEncontrado = buscarNodoPorNombreS(elemento->nodo);

	//mando el bloque que quiero leer
	int cantStrings=1;
	int numeroDeBLoqueQueQuiero = elemento->bloque;
	printf(" numeroDeBLoqueQueQuiero  %d del nodo %s \n",numeroDeBLoqueQueQuiero,elemento->nodo);
	char * NumeroDeBloqueDondeGuardarString = intToArrayZerosLeft(numeroDeBLoqueQueQuiero,4);
	char **arrayMensajesSerializar = malloc(sizeof(char*) * cantStrings);
	if (!arrayMensajesSerializar)
	perror("error de malloc 1");

	int i = 0;
	arrayMensajesSerializar[i] = malloc(string_length(NumeroDeBloqueDondeGuardarString) + 1);
	if (!arrayMensajesSerializar[i])
	perror("error de malloc 1");
	strcpy(arrayMensajesSerializar[i], NumeroDeBloqueDondeGuardarString);
		i++;
		char *mensajeSerializado = serializarMensaje(TIPO_MSJ_BLOQUE_DESDE_DATANODE, arrayMensajesSerializar, cantStrings);
		//printf("%s\n",mensajeSerializado);
		int bytesEnviados = enviarMensaje(nodoEncontrado->socket, mensajeSerializado);
		printf("bytes enviados: %d\n", bytesEnviados);
		////mando el bloque que quiero leer


		//recibo el buffer

		int32_t headerRecibo = deserializarHeader(nodoEncontrado->socket);
		int cantMensajesRecibidos =protocoloCantidadMensajes[headerRecibo];
		char ** arrayMensajesRecibidos = deserializarMensaje(nodoEncontrado->socket,cantMensajesRecibidos);
		printf("llegue aca \n");
		//arrayDeOriginalesYcopias[j] = string_substring(arrayMensajesRecibidos[0],0,elemento->bytes);
		printf("%s",arrayMensajesRecibidos[0]);
		//getchar();
		//recibo el buffer
		j++;
	}
		list_iterate(archivoABuscar->bloqueCopias,(void*)buscar);
		void printearArrayOriginal(){
			int i = 0;
			int c = (cantidadBloquesAMandar(PATH)*2);
				for(;i<c;i++){
					printf("%s\n",arrayDeOriginalesYcopias[i]);
					getchar();
					i++;
				}

		}
		//printearArrayOriginal();
}

void enviarInfoBloques(int socketCliente,int headerId) {
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

		char *nodo=hola->nodo;
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
	puts("presionar ENTER");
	getchar();
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
									if (estadoEstable) {
										SocketYama = nuevoSocket;
										enviarHeaderSolo(SocketYama,TIPO_MSJ_HANDSHAKE_RESPUESTA_OK);
									} else {
										enviarHeaderSolo(SocketYama,TIPO_MSJ_HANDSHAKE_RESPUESTA_DENEGADO);
									}
								}
									break;
								case worker: {
									SocketWorker = nuevoSocket;
								}
									break;
								case datanode: {
									registrarNodo(nuevoSocket);
									persistirNodosFuncion();
								}
									break;
								}
								if (nuevoSocket > fdmax) {
									fdmax = nuevoSocket;
								}
							}
						} else {
							int32_t headerId = deserializarHeader(i);
							if (headerId <= 0) { //error o desconexión de un cliente
								eliminarDeLasListas(i);
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
						}
					}
				}
			}
		}

	char *  devolverPathAbsoluto(char * pathYamaFS){
		char * pathAbsoluto = string_new();
		string_append(&pathAbsoluto,"/home/utnso/workspace/tp-2017-2c-Mi-Grupo-1234/filesystem/yamafs/");
		string_append(&pathAbsoluto,pathYamaFS);
		return pathAbsoluto;
		free(pathAbsoluto);
	}
	void matarTodo(){
		printf("te mate gato \n");
	}
	void inicializarCarpetas(){
		tablaDeDirectorios * nuevoDirectorio = malloc(sizeof(tablaDeDirectorios));
		char * metadata=string_new();
		char * archivos=string_new();
		char * bitmaps=string_new();
		string_append(&metadata,"mkdir ");
		string_append(&metadata,"metadata");
		string_append(&archivos,"mkdir ");
		string_append(&archivos,"metadata/archivos");
		string_append(&bitmaps,"mkdir ");
		string_append(&bitmaps,"bitmaps");
		system(metadata);
		system(archivos);
		system(bitmaps);
		crearDirectorio("yamafs:");
		persistirDirectorio();
		free(metadata);
		free(archivos);
		free(bitmaps);
	}
int main(int argc, char *argv[]) {

	//signal(SIGINT,matarTodo);

	tablaNodos = list_create();
	tablaArchivos = list_create();
	listaDeBitMap = list_create();
	listaDeNodosDeEstadoAnterior = list_create();
	listaDeNodosDeFormateo=list_create();
	listaDirectorios=list_create();
	char * PUERTO;
	logFs = log_create("../FileSystem.log", "FileSystem", 0, 0);
	configFs = config_create("../../config/configFilesystem.txt");
	persistirNodos =config_create("metadata/nodos.bin");
	directorios =config_create("metadata/directorios.dat.bin");
	pthread_t hiloConsola;
	estadoAnterior = config_get_int_value(configFs,"ESTADO_ANTERIOR");
	estadoEstable = config_get_int_value(configFs,"ESTADO_ESTABLE");

	if(argv[1]==NULL){
		if(estadoAnterior==1){
			//cargar estado anterior
			//cargar directorios
			//cargar archivos
			cargarListaDeNodosAnteriores();
			FILE * directorios = fopen("metadata/directorios.dat", "a+");
			FILE * nodos = fopen("metadata/nodos.bin", "a+");
			PUERTO = config_get_string_value(configFs, "PUERTO_PROPIO");
			pthread_create(&hiloConsola,NULL,(void*)IniciarConsola,NULL);
			soyServidor(PUERTO);
			pthread_join(hiloConsola, NULL);

		}else{
			//se Ejecuta por primera vez
			config_set_value(configFs,"ESTADO_ANTERIOR","1");
			config_save(configFs);
			inicializarCarpetas();
			FILE * directorios = fopen("metadata/directorios.dat", "w+");
			FILE * nodos = fopen("metadata/nodos.bin", "wb");
			PUERTO = config_get_string_value(configFs, "PUERTO_PROPIO");
			pthread_create(&hiloConsola,NULL,(void*)IniciarConsola,NULL);
			soyServidor(PUERTO);
			pthread_join(hiloConsola, NULL);
		}
	}
	else{
		if(!strcmp(argv[1], "--clean")){

		printf("limpiame gato\n");
		//elimino estructuras,carpetas y levanto

		config_set_value(configFs,"ESTADO_ANTERIOR","0");
		config_set_value(configFs,"ESTADO_ESTABLE","0");
		config_save(configFs);
		PUERTO = config_get_string_value(configFs, "PUERTO_PROPIO");
		pthread_create(&hiloConsola,NULL,(void*)IniciarConsola,NULL);
		soyServidor(PUERTO);
		pthread_join(hiloConsola, NULL);
		}
		else {
			printf("Argumento no valido para ejecutar el programa\n");
			EXIT_FAILURE;
		}
	}

	return 0;
}
