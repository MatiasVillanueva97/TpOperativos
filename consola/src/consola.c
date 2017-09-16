#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>

void main() {
  char * linea;
  puts("Ingrese una opción entre 1 para YAMA y 2 para MASTER,o exit para salir");
  while(1) {
    linea = readline("YAMA>");

    if(linea)
      add_history(linea);				//guardo en historial lo que escribi

    if(!strncmp(linea, "exit", 4)) {	//si escribo exit salgo
       free(linea);
       break;
    }
    if (!strncmp(linea, "YAMA", 4))					//Aca iria toda la funcionalidad de la consola
    	puts("usted ha elegido la opcion YAMA");
    if(!strncmp(linea, "MASTER", 5))
    	puts("usted ha elegido la opcion MASTER");
    }
  }

