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
        if(estadoAnterior==0){
        	uint cantidadNodosSistemas=list_size(tablaNodos);
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

        	if(estadoEstableFuncion()){
        		config_set_value(configFs,"ESTADO_ESTABLE","1");
        		config_save(configFs);
        		estadoEstable=config_get_int_value(configFs,"ESTADO_ESTABLE");
        		config_set_value(configFs,"FORMATEADO","1");
        		config_save(configFs);
        		formateado=config_get_int_value(configFs,"FORMATEADO");
        		printf("FileSystem Formateado\n");
        		printf("\n");
        	}else{
        		printf("No hay al menos una copia de cada archivo , Estado no estable\n");
        	}
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
        system(linea);
        printf("\n");
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

      case 7:{
    	   char * nombreArchivoViejo = comandoDesarmado[1];
    	   char * nombreArchivoNuevo = comandoDesarmado[2];
    	   char * flag = comandoDesarmado[3];
    	   if(nombreArchivoViejo != NULL && nombreArchivoNuevo != NULL && flag != NULL){
    		  almacenarArchivo(nombreArchivoViejo,nombreArchivoNuevo,atoi(flag));
    	   }else{
    		   printf("Faltan parametros para ejecutar el comando cpfrom\n");
    	   }

      }
      break;

      case 8:{
    	  char * nombreArchivoViejofs = comandoDesarmado[1];
    	  char * nombreArchivoNuevolocal = comandoDesarmado[2];
    	  if(nombreArchivoViejofs != NULL && nombreArchivoNuevolocal != NULL){
    			pthread_mutex_lock(&mutex1);
    			int a = leerArchivo(nombreArchivoViejofs,nombreArchivoNuevolocal);
    			 pthread_mutex_unlock(&mutex1);
    			 if(a==0){

    			 ContenidoXNodo * elemento = list_get(tablaNodos,0);
    			 printf("%s\n",elemento->nodo);
    			 enviarHeaderSolo(elemento->socket,TIPO_MSJ_OK);
    			 }else {

    			 }
    	      	   }else{
    	      		   printf("Faltan parametros para ejecutar el comando cpto\n");
    	      	   }
      }
      break;

      case 9:{
        printf("Comando en arreglo! Todavia no se puede ejecutar! (9)\n");
      }
      break;

      case 10:{
        char * comandoNuevo = string_new();

        char * nombreArchivoViejo = comandoDesarmado[1];

        if(nombreArchivoViejo == NULL){
        	printf("Faltan parametros para ejecutar el comando md5sum\n");
        } else {
        	string_append(&comandoNuevo,"md5sum ");
        	string_append(&comandoNuevo,nombreArchivoViejo);
        	string_append(&comandoNuevo," | awk '{print $1}'");
        	system(comandoNuevo);
        }

    	free(comandoNuevo);

      }
      break;

      case 11:{
        system(linea);
        printf("\n");
      }
      break;

      case 12:{
        char * nombreArchivoViejo = string_new();

        string_append(&nombreArchivoViejo, comandoDesarmado[1]);
        if(nombreArchivoViejo == NULL){
        	printf("Faltan parametros para ejecutar el comando info.\n");
        } else {
        	printf("%s\n comandoDesarmado[1]",comandoDesarmado[1]);
        	printf("%s\n nombreArchivoViejo[1]",nombreArchivoViejo);
        	tablaArchivo * archivo = buscarArchivoPorNombre(comandoDesarmado[1]);
        	if(archivo==NULL){
        		printf("puta te cabe \n");
        	}

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
        			printf("%s = %s \n",bloque_copia,array);
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
  char * linea;
  char * barraDeComando = string_from_format("%s>", getlogin());

  	  puts("Iniciando Consola\n");
  	sleep(1);
  	puts("Consola lista\n");

  while(1) {
    linea = (char *) readline(barraDeComando);

    if(linea)
    	add_history(linea);

    if(!strncmp(linea, "exit", 4)) {
       free(linea);
       free(barraDeComando);
       break;
    } else {
        analizarComando(linea);
    }

  }

}
