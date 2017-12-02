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
  int comandoNativo;
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

        printf("Formateando FileSystem.\n");

        if(estadoAnterior==0){
        	uint cantidadNodosSistemas=list_size(tablaNodos);
        	if(cantidadNodosSistemas>=2){
        		config_set_value(configFs,"ESTADO_ESTABLE","1");
        		config_save(configFs);
        		estadoEstable=config_get_int_value(configFs,"ESTADO_ESTABLE");
        		printf("FileSystem Formateado\n");
        	}else{
        		printf("No hay suficientes DataNode para dejar el FS en un estado Estable\n");
        	}
        }else{

        	if(true){//hayUnEstadoEstable()
        		config_set_value(configFs,"ESTADO_ESTABLE","1");
        		config_save(configFs);
        		estadoEstable=config_get_int_value(configFs,"ESTADO_ESTABLE");
        	}else{
        		printf("No hay al menos una copia de cada archivo. Estado no estable\n");
        	}
        }


        break;
      }

      case 2:{
        if(strcmp(comandoDesarmado[1], "-d")==0){
        	int pudoBorrar = deleteDirectory(comandoDesarmado[2]);
        	if(pudoBorrar == 0){
        		printf("El directorio a borrar no existe.\n");
        	}else if(pudoBorrar == -1){
        		printf("El directorio a borrar tiene subdirectorios. No se puede borrar.\n");
        	}else{
        		char* comandoPConsola = string_new();
        		string_append(&comandoPConsola, "rmdir ");
        		string_append(&comandoPConsola, comandoDesarmado[2]);
        		system(comandoPConsola);
        		persistirDirectorio();
        		printf("Directorio borrado exitosamente.\n");
        		free(comandoPConsola);
        	}
        }else if(strcmp(comandoDesarmado[1], "-b")==0){
        	//BORRO BLOQUE
        }else if(comandoDesarmado[1]!=NULL){
        	//BORRO ARCHIVO
        }else{
        	//log_error
        }
      }
      break;


      case 3:{
    	  char * comandoNuevo = string_new();

    	  char * nombreArchivoViejo = comandoDesarmado[1];
    	  char * nombreArchivoNuevo = comandoDesarmado[2];

    	  if(nombreArchivoViejo == NULL || nombreArchivoNuevo == NULL){
    	  	  printf("Faltan parametros para ejecutar el comando rename\n");
    	  } else {

    		  renameDirectory(nombreArchivoViejo,nombreArchivoNuevo);

    		  string_append(&comandoNuevo,"rename ");
    		  string_append(&comandoNuevo,nombreArchivoViejo);
    		  string_append(&comandoNuevo," ");
    		  string_append(&comandoNuevo,nombreArchivoNuevo);

    		  system(comandoNuevo);
        	  printf("\n");

    	  }

    	  free(comandoNuevo);

      }
      break;

      case 4:{
    	  char * comandoNuevo = string_new();

    	  char * pathOriginal = comandoDesarmado[1];
    	  char * pathFinal = comandoDesarmado[2];

    	  if(pathOriginal == NULL || pathFinal == NULL){
    		  printf("Faltan parametros para ejecutar el comando mv\n");
    	  } else {

        	  moveDirectory(pathOriginal,pathFinal);

        	  string_append(&comandoNuevo,"mv ");
        	  string_append(&comandoNuevo,pathOriginal);
        	  string_append(&comandoNuevo," ");
        	  string_append(&comandoNuevo,pathFinal);

        	  system(comandoNuevo);
        	  printf("\n");

    	  }

    	  free(comandoNuevo);

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
    			      printf("\n");
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
    		  almacenarArchivo(nombreArchivoViejo,nombreArchivoNuevo,flag);
    		   printf("El archivo ha sido copiado exitosamente.\n");
    	   }else{
    		   printf("Faltan parametros para ejecutar el comando cpfrom\n");
    	   }

      }
      break;

      case 8:{
        //
        // Ver bien como hacer estos comandos despues
        //
        printf("Comando en arreglo! Todavia no se puede ejecutar! (8)\n");
      }
      break;

      case 9:{
        //
        // Ver bien como hacer estos comandos despues
        //
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
        	system(comandoNuevo);
        	printf("\n");
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
        char * comandoNuevo = string_new();
        char * nombreArchivoViejo = string_new();

        string_append(&nombreArchivoViejo, comandoDesarmado[1]);
        if(nombreArchivoViejo == NULL){
        	printf("Faltan parametros para ejecutar el comando info.\n");
        } else {

            string_append(&comandoNuevo,"ls -l -h ");
            string_append(&comandoNuevo,nombreArchivoViejo);
            system(comandoNuevo);
            printf("\n");

        }

        free(comandoNuevo);
        free(nombreArchivoViejo);

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
    	  		printf("cpfrom [path_archivo_origen] [directorio_yamafs] //Copiar un archivo local al yamafs, siguiendo los lineamientos en la operaciòn Almacenar Archivo, de la Interfaz del FileSystem.\n");
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
