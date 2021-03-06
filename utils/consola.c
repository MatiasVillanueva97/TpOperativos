#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <commons/string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <pthread.h>
#include "../filesystem/src/filesystem.h"

bool existeDirectorio(char* ruta);
#define maxSize 256
typedef struct {
  int tipo;
  char *nombre;     /* Nombre de la funcion */
} command;
pthread_mutex_t mutex1;
command comandos[] = {
 { 1,"format"},
 { 2,"rm"},
 { 3,"rename"},
 { 4,"mv"},
 { 5,"cat"},
 { 6,"mkdir"},
 { 7,"cpfrom"},
 { 8,"cpto"},
 { 9,"cpblock"},
 { 10,"md5"},
 { 11,"ls"},
 { 12,"info"},
 {13,"help"}
};



void analizarComando(char * linea){

  int i;
  int comandoNativo = 47;
  int limite = (sizeof(comandos)/sizeof(comandos[0]));
  command comandoAux;

  char ** comandoDesarmado = string_split(linea," ");
  char * primerPos = comandoDesarmado[0];

  for(i=0; i < limite; i++){
    comandoAux = comandos[i];

    if( strcmp(primerPos, comandoAux.nombre) == 0){
      comandoNativo = comandoAux.tipo;
      break;
      }
  }


  switch(comandoNativo){

      case 1:{

        printf("Formateando FileSystem..\n");
		printf("\n");
    	          	uint cantidadNodosSistemas=list_size(tablaNodos);
       if(formateado==0){
    	   if(estadoAnterior==0){
    	          	if(cantidadNodosSistemas>=2){
    	          		config_set_value(configFs,"ESTADO_ESTABLE","1");
    	          		config_save(configFs);
    	          		estadoEstable=config_get_int_value(configFs,"ESTADO_ESTABLE");
    	          		config_set_value(configFs,"FORMATEADO","1");
    	          		config_save(configFs);
    	          		formateado=config_get_int_value(configFs,"FORMATEADO");
    	          		printf("FileSystem Formateado\n");
    	          		printf("\n");
    	          	}else{
    	          		printf("No hay suficientes DataNode para dejar el FS en un estado Estable\n");
    	          	}
    	          }else{

    	        	  if(cantidadNodosSistemas>=2){

    	          	// eliminar todas las litas,printear formateo
    	        	  eliminarCarpetasParaformateo();
    	        	  eliminarListasParaFormateo();
    	        	  limpiarBitmapsYSetearCantidadLibre();
    	        	  inicializarCarpetasParaFormateo();
    	        	  persistirNodosFuncion();
    	        	  printf("Estructuras Eliminadas\n");
    	      		config_set_value(configFs,"FORMATEADO","1");
    	      		config_save(configFs);
    	      		formateado=config_get_int_value(configFs,"FORMATEADO");
    	      		printf("FileSystem Formateado\n");
    	      		    	          		printf("\n");
    	        	  }else{
      	          		printf("No hay suficientes DataNode para dejar el FS en un estado Estable\n");
      	          	}


    	          }

       }else {
    	   printf("El FileSystem ya se encuentra formateado \n");
       }





        break;
      }

      case 2:{
    	  printf("Comando en proceso! Todavia no se puede ejecutar! (rm)\n");
      }
      break;


      case 3:{
    	  printf("Comando en proceso! Todavia no se puede ejecutar! (rename)\n");

      }
      break;

      case 4:{
    	  printf("Comando en proceso! Todavia no se puede ejecutar! (mv)\n");

      }
      break;

      case 5:{
    	  if(estadoEstable==1){



    	     	        char * comandoNuevo = string_new();
    	     	        char * comandoNuevoRm = string_new();

    	     	        char * path_yafs = comandoDesarmado[1];

    	     	        if(path_yafs == NULL){
    	     	        	printf("Faltan parametros para ejecutar el comando md5sum\n");
    	     	        } else {
    	     	        	char * nombre = conseguirNombreDePath(path_yafs);
    	     	        	char * pathArchivo = string_duplicate("../metadata/");


    	     	        	tablaArchivo * archivoEncontrado = buscarArchivoPorNombreYRuta(nombre,path_yafs,0);
    	     	        	if(archivoEncontrado==NULL){
    	     	        		printf("No existe el archivo en ese directorio\n");
    	     	        	}else {

    	     				 pthread_mutex_lock(&mutex1);
    	     				 int a = leerArchivo(path_yafs,pathArchivo,1);
    	     				 pthread_mutex_unlock(&mutex1);
    	     				 if(a==0){
    	     				 ContenidoXNodo * elemento = list_get(tablaNodos,0);
    	     				 enviarHeaderSolo(elemento->socket,TIPO_MSJ_OK);
    	     				 string_append(&comandoNuevo,"cat ");
    	     				 string_append(&comandoNuevo,pathArchivo);
    	     				 string_append(&comandoNuevo,nombre);
    	     				 system(comandoNuevo);
    	     				 string_append(&comandoNuevoRm,"rm -f ");
    	     				 string_append(&comandoNuevoRm,pathArchivo);
    	     				 string_append(&comandoNuevoRm,nombre);
    	     				 system(comandoNuevoRm);


    	     				 }
    	     				 else {

    	     				 }
    	     	        	}
    	     	        }    	free(comandoNuevo);

    	     	  }else {

    	     		  printf("No se puede ejecutar este comando si no hay un establdo estable \n");

    	     	  }



      }
      break;

      case 6:{
    	  char* nuevoDirectorio = comandoDesarmado[1];
    	  if(nuevoDirectorio != NULL){
    		  if(!existeDirectorio(nuevoDirectorio)){
    			  if(crearDirectorio(nuevoDirectorio) == 1){
    			  }else{
    			  	  printf("No se pudo crear el directorio. Por favor vuelva a intentarlo\n");
    	  		  }
    		  }else{
    			  printf("No se pudo crear el directorio. El directorio ya existe.\n");
    		  }
    	  }else{
      		  printf("Asegurese de ingresar el nombre del directorio. Por favor vuelva a intentarlo\n");
      	  }
      }
      break;

      case 7: {

    	  if(estadoEstable==1){
    		  char * nombreArchivoViejo = comandoDesarmado[1];
    		      	   char * nombreArchivoNuevo = comandoDesarmado[2];
    		      	   char * flag = comandoDesarmado[3];
    		      	   if(nombreArchivoViejo != NULL && nombreArchivoNuevo != NULL && flag != NULL){
    		      		  almacenarArchivo(nombreArchivoViejo,nombreArchivoNuevo,atoi(flag));
    		      	   }else{
    		      		   printf("Faltan parametros para ejecutar el comando cpfrom\n");
    		      	   }

    	  }else
    	  {
    		  printf("No se puede ejecutar este comando si no hay un establdo estable ");
    	  }




      }
      break;

      case 8:{
    	  if(estadoEstable==1){

        	  char * nombreArchivoViejofs = comandoDesarmado[1];
        	  char * nombreArchivoNuevolocal = comandoDesarmado[2];
        	  if(nombreArchivoViejofs != NULL && nombreArchivoNuevolocal != NULL){
        			 pthread_mutex_lock(&mutex1);
        			 int a = leerArchivo(nombreArchivoViejofs,nombreArchivoNuevolocal,0);
        			 pthread_mutex_unlock(&mutex1);
        			 if(a==0){

        			 ContenidoXNodo * elemento = list_get(tablaNodos,0);
        			 enviarHeaderSolo(elemento->socket,TIPO_MSJ_OK);
        			 }else {

        			 }
        	      	   }else{
        	      		   printf("Faltan parametros para ejecutar el comando cpto\n");
        	      	   }

    	  }else {
    		  printf("No se puede ejecutar este comando si no hay un establdo estable \n");
    	  }


      }
      break;

      case 9:{
        printf("Comando en arreglo! Todavia no se puede ejecutar! (9)\n");
      }
      break;

      case 10:{

    	  if(estadoEstable==1){



    	        char * comandoNuevo = string_new();
    	        char * comandoNuevoRm = string_new();

    	        char * path_yafs = comandoDesarmado[1];

    	        if(path_yafs == NULL){
    	        	printf("Faltan parametros para ejecutar el comando md5sum\n");
    	        } else {
    	        	char * nombre = conseguirNombreDePath(path_yafs);
    	        	char * pathArchivo = string_duplicate("../metadata/");

    				 pthread_mutex_lock(&mutex1);
    				 int a = leerArchivo(path_yafs,pathArchivo,1);
    				 pthread_mutex_unlock(&mutex1);
    				 if(a==0){
    				 ContenidoXNodo * elemento = list_get(tablaNodos,0);
    				 enviarHeaderSolo(elemento->socket,TIPO_MSJ_OK);
    				 string_append(&comandoNuevo,"md5sum ");
    				 string_append(&comandoNuevo,pathArchivo);
    				 string_append(&comandoNuevo,nombre);
    				 string_append(&comandoNuevo," | awk '{print $1}'");
    				 system(comandoNuevo);
    				 string_append(&comandoNuevoRm,"rm -f ");
    				 string_append(&comandoNuevoRm,pathArchivo);
    				 string_append(&comandoNuevoRm,nombre);
    				 system(comandoNuevoRm);


    				 }
    				 else {

    				 }
    	        }    	free(comandoNuevo);

    	  }else {

    		  printf("No se puede ejecutar este comando si no hay un establdo estable \n");

    	  }


      }
      break;

      case 11:{
    	  printf("Comando en proceso! Todavia no se puede ejecutar! (ls)\n");
      }
      break;

      case 12:{
        char * nombreArchivoViejo = string_new();

        string_append(&nombreArchivoViejo, comandoDesarmado[1]);
        if(nombreArchivoViejo == NULL){
        	printf("Faltan parametros para ejecutar el comando info.\n");
        } else {

        	tablaArchivo * archivo =buscarArchivoPorNombreYRuta(conseguirNombreDePath(comandoDesarmado[1]),comandoDesarmado[1],0);
        	if(archivo==NULL){
        		printf("No existe el archivo %s el directorio %s \n",conseguirNombreDePath(comandoDesarmado[1]),comandoDesarmado[1]);
        	}else {
        		printf("\n");
            	printf("Nombre = %s\n",archivo->nombre);
            	printf("Tamaño = %d\n",archivo->tamanio);
            	int i=0;
            	int j=0;
            	int k=0;
            	void printear(ContenidoBloque * bloquesArchivo){
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
            			printf("\t %s = %s \t",bloque_copia,array);
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
            						printf("%s = %s \n",bloque_copia,array);

            		}
            	k++;}list_iterate(archivo->bloqueCopias,(void*)printear);
                printf("\n");

            }
        	}

      //  free(nombreArchivoViejo);

      }
      break;

      case 13:{
    	  printf("\n");
    	  		printf("format // Formatea el FileSystem\n");
    	  		printf("\n");
    	  		printf("rm [path_archivo] // Elimina Archivo\n");
    	  		printf("\n");
    	  		printf("rmDir [path_directorio] // Elimina un directorio\n");
    	  		printf("\n");
    	  		printf("rmBloq [path_archivo] [nro_bloque] [nro_copia] // Elimina un bloque\n");
    	  		printf("\n");
    	  		printf("rename [path_original] [nombre_final] // Renombra un Archivo o Directorio\n");
    	  		printf("\n");
    	  		printf("mv [path_original] [path_final] //Mueve un Archivo o Directorio\n");
    	  		printf("\n");
    	  		printf("cat [path_archivo] //Muestra el contenido del archivo como texto plano\n");
    	  		printf("\n");
    	  		printf("mkdir [path_dir] //Crea un directorio. Si el directorio ya existe\n");
    	  		printf("\n");
    	  		printf("cpfrom [path_archivo_origen] [directorio_yamafs] [tipo_de_archivo] //Copiar un archivo local al yamafs, siguiendo los lineamientos en la operaciòn Almacenar Archivo, de la Interfaz del FileSystem.\n");
    	  		printf("\n");
    	  		printf("cpto [path_archivo_yamafs] [directorio_filesystem] //Copiar un archivo local al yamafs\n");
    	  		printf("\n");
    	  		printf("cpblock [path_archivo] [nro_bloque] [id_nodo]  //Crea una copia de un bloque de un archivo en el nodo dado\n");
    	  		printf("\n");
    	  		printf("md5 [path_archivo_yamafs] //Solicitar el MD5 de un archivo en yamafs\n");
    	  		printf("\n");
    	  		printf("ls [path_directorio] //Lista los archivos de un directorio\n");
    	  		printf("\n");
    	  		printf("info [path_archivo] //Muestra toda la información del archivo\n");
    	  		printf("\n");
      }break;

      default:
    	  	printf("Comando no reconocido.\n");
            break;

    }

  	liberarArrayComando(comandoDesarmado);

}

void IniciarConsola(){
	signal(SIGINT,matarTodo);
  char * linea;
  char * barraDeComando = string_from_format("%s>", getlogin());

  	  puts("Iniciando Consola\n");
  	sleep(1);
  	puts("Consola lista\n");

  while(1) {
    linea = (char *) readline(barraDeComando);
    if(linea)
    	add_history(linea);
    if(string_is_empty(linea)){

    }else {
        if(!strncmp(linea, "exit", 4)) {
        	matarTodo();
           free(linea);
           free(barraDeComando);
           printf("FileSystem Terminado\n");
           exit(0);
        } else {
            analizarComando(linea);
        }
    }

  }

}
