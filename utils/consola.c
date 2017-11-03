/*
 * Consola2.c
 *
 *  Created on: 14/9/2017
 *      Author: utnso
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <commons/string.h>
#include <readline/readline.h>
#include <readline/history.h>


//Prototipados

void ayuda(char ** comandoConsola);
void format(char ** comandoConsola);
void rm(char ** comandoConsola);
void rmDir(char ** comandoConsola);
void rmBloq(char ** comandoConsola);
void renombrar(char ** comandoConsola);
void mv(char ** comandoConsola);
void cat(char ** comandoConsola);
void mk__dir(char ** comandoConsola);
void cpfrom(char ** comandoConsola);
void cpto(char ** comandoConsola);
void cpblock(char ** comandoConsola);
void md5(char ** comandoConsola);
void ls(char ** comandoConsola);
void info(char ** comandoConsola);

//FUNCIONES

void IniciarConsola() {
char * linea;
char * barraDeComando = string_from_format("%s>",getlogin());
puts("Iniciando Consola");
sleep(1);
puts("Consola lista");
while(1) {
linea =readline(barraDeComando);
if(linea)
	add_history(linea);
if(!strncmp(linea,"exit",4))
{ 	puts("Saliendo de consola...");
	free(linea);
break;
}
if(!strncmp(linea,"\n",1))
{ 	puts("");
	free(linea);
}
char** comandoConsola = NULL;//Esta variable es para cortar el mensaje en 2.
comandoConsola = string_split(linea, " "); // separa la entrada en un char**

//POLIMORFISMO
format(comandoConsola);
ayuda(comandoConsola);
rm(comandoConsola);
rmDir(comandoConsola);
rmBloq(comandoConsola);
renombrar(comandoConsola);
mv(comandoConsola);
cat(comandoConsola);
mk__dir(comandoConsola);
cpfrom(comandoConsola);
cpto(comandoConsola);
cpblock(comandoConsola);
md5(comandoConsola);
ls(comandoConsola);
info(comandoConsola);

		free(linea);
		//liberar vector comandoConsola
}
}
//USAR COMMONS string_split para despues
void ayuda(char ** comandoConsola){
	if(!strcmp(comandoConsola[0],"help")){
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



		}}
void format(char ** comandoConsola){
	if(!strcmp(comandoConsola[0],"format")){
		puts("formatearea algo");
	}
}

void rm(char ** comandoConsola){
	char buf[1024];
	if(!strcmp(comandoConsola[0],"rm")){
			if(comandoConsola[1]==NULL){
					puts("Faltan parametros");
					puts("escriba help si necesita ayuda");
				}else{
				snprintf(buf, sizeof(buf), "rm '%s'", comandoConsola[1]);
				system(buf);
		}}
	}
void rmDir(char ** comandoConsola){
	char buf[1024];
	if(!strcmp(comandoConsola[0],"rmDir")){
				if(comandoConsola[1]==NULL){
						puts("Faltan parametros");
						puts("escriba help si necesita ayuda");
					}else{
				snprintf(buf, sizeof(buf), "rm -r '%s'", comandoConsola[1]);
				system(buf);
			}}

}
void rmBloq(char ** comandoConsola){
	if(!strcmp(comandoConsola[0],"rmBloq")){
				if((comandoConsola[1]==NULL) || comandoConsola[2]==NULL || comandoConsola[3]==NULL){
						puts("Faltan parametros");
						puts("escriba help si necesita ayuda");
					}else{
				puts("rumuevo algo");
				puts("ruta");
				puts(comandoConsola[1]);
				puts("bloque");
				puts(comandoConsola[2]);
				puts("id");
				puts(comandoConsola[3]);
			}}

}
void renombrar(char ** comandoConsola){
	char buf[1024];
	if(!strcmp(comandoConsola[0],"rename")){
		if(comandoConsola[1]==NULL || comandoConsola[2]== NULL){
				puts("Faltan parametros");
				puts("escriba help si necesita ayuda");
			}else{
				snprintf(buf, sizeof(buf),"mv '%s' '%s'", comandoConsola[1],comandoConsola[2]);
				system(buf);
	}}
}

void mv(char ** comandoConsola){
	char buf[1024];
	if(!strcmp(comandoConsola[0],"mv")){
		if(comandoConsola[1]==NULL || comandoConsola[2]== NULL){
				puts("Faltan parametros");
				puts("escriba help si necesita ayuda");
			}else{
				puts(comandoConsola[1]);

				puts(comandoConsola[2]);
				snprintf(buf, sizeof(buf), "mv '%s' '%s'", comandoConsola[1],comandoConsola[2]);
				system(buf);
			}}
}
void cat(char ** comandoConsola){
	char buf[1024];
	if(!strcmp(comandoConsola[0],"cat")){
		if(comandoConsola[1]==NULL){
		puts("Faltan parametros");
		puts("escriba help si necesita ayuda");
	}else {
		snprintf(buf, sizeof(buf), "cat '%s'", comandoConsola[1]);
		system(buf);
	}
}}
void mk__dir(char ** comandoConsola){
	char buf[1024];
	if(!strcmp(comandoConsola[0],"mkdir")){
		if(comandoConsola[1]==NULL){
				puts("Faltan parametros");
				puts("escriba help si necesita ayuda");
			}else {
				snprintf(buf, sizeof(buf), "mkdir '%s'", comandoConsola[1]);
				system(buf);
	}
}}
void cpfrom(char ** comandoConsola){

	if(!strcmp(comandoConsola[0],"cpfrom")){
		if(comandoConsola[1]==NULL || comandoConsola[2]== NULL){
			puts("Faltan parametros");
			puts("escriba help si necesita ayuda");
		}else {
		puts(" Copiar un archivo local al yamafs, siguiendo los lineamientos en la operaciòn Almacenar Archivo, de la Interfaz del FileSystem.");
		puts("Path archivo origen");
		puts(comandoConsola[1]);
		puts("directorio filesystem");
		puts(comandoConsola[2]);
	}
}}

void cpto(char ** comandoConsola){
	char buf[1024];
	if(!strcmp(comandoConsola[0],"cpto")){

		if(comandoConsola[1]==NULL || comandoConsola[2]== NULL){
			puts("Faltan parametros");
			puts("escriba help si necesita ayuda");
		}else {
			snprintf(buf, sizeof(buf), "cp '%s''%s'", comandoConsola[1],comandoConsola[2]);
			system(buf);
	}
}}
void cpblock(char ** comandoConsola){
	if(!strcmp(comandoConsola[0],"cpblock")){
		if(comandoConsola[1]==NULL || comandoConsola[2]== NULL || comandoConsola[3] == NULL){
				puts("Faltan parametros");
				puts("escriba help si necesita ayuda");

	}else {
		puts("creo copias de bloques");
		puts("Path archivo");
		puts(comandoConsola[1]);
		puts("nro de bloque");
		puts(comandoConsola[2]);
		puts("id nodo");
		puts(comandoConsola[3]);

	}
}}
void md5(char ** comandoConsola){
	char buf[1024];
	if(!strcmp(comandoConsola[0],"md5")){
		if(comandoConsola[1]==NULL){
		puts("Faltan parametros");
		puts("escriba help si necesita ayuda");
	}else {
		snprintf(buf, sizeof(buf), "md5sum '%s'", comandoConsola[1]);
		system(buf);
	}
}}
void ls(char ** comandoConsola){
	char buf[1024];
	if(!strcmp(comandoConsola[0],"ls")){
		if(comandoConsola[1]==NULL){
			system("ls");
			}
		else{
		snprintf(buf, sizeof(buf), "ls '%s'", comandoConsola[1]);
		system(buf);
	}
}}
void info(char ** comandoConsola){
	if(!strcmp(comandoConsola[0],"info")){
		if(comandoConsola[1]==NULL){
		puts("Faltan parametros");
		puts("escriba help si necesita ayuda");
	}
		else{


		puts("informo cosas de archivos");
		puts("path del archivo");
		puts(comandoConsola[1]);
	}
	}}
