/*
 * consola.c
 *
 *  Created on: 21/9/2017
 *      Author: utnso
 */


#include "consola.h"

int consola() {
  char * linea;
  puts("Ingrese un comando,o exit para salir");
  while(1) {
    linea = readline("YAMA>");

    if(linea)
      add_history(linea);				//guardo en historial lo que escribi

    if(!strncmp(linea, "exit", 4)) {	//si escribo exit salgo
       free(linea);
       return 1;
    }
    if (!strncmp(linea, "format", 5))					//Aca iria toda la funcionalidad de la consola
    	puts("usted ha elegido la opcion format");
    if(!strncmp(linea, "rm ", sizeof(linea)))
    	puts("usted ha elegido la opcion rm"); //aca deberia empezarse a parsear cual de todos los rm se ingreso
    }
  }

