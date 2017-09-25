# TP 2017 2ºC - Mi Grupo 1234 - UTN FRBA

Repositorio del TP de SO - UTN - FRBA.
Grupo **Mi Grupo 1234** - 2ºC 2017.


## Integrantes

Nombre | Padrón | Mail |
:---: | :---: | :---: |
Agustín de la Dedicación | 1472161 | Agusdld@gmail.com |
Hernán Domingo | 1306315 | hernan.domingo.22@gmail.com |
Guillermo Fernández | 1163206 | gafernandez2525@gmail.com |
Nahuel Oyhanarte | 1545668 | noyhanarte@gmail.com |
Mathias Sanchez | 1527575 | mathisoad2@gmail.com |


## Requisitos

1. Tener instalado gcc
2. Tener instaladas la commons library ([link](https://github.com/sisoputnfrba/so-commons-library))
3. Tener instaladas la pthreads library
4. Tener instaladas la readline library (sudo apt-get install libreadline6-dev)


## Instrucciones

1. Clonar este repositorio: `git clone https://github.com/sisoputnfrba/tp-2017-2c-Mi-Grupo-1234`
2. Entrar al directorio: `cd tp-2017-2c-Mi-Grupo-1234`
3. Compilar usando el Makefile provisto: `make`
4. Ejecutar el proceso FileSystem: `filesystem/bin/filesystem`
5. Ejecutar el proceso DataNode: `datanode/bin/datanode` (TO DO)
6. Ejecutar el proceso Worker: `worker/bin/worker` (TO DO)
7. Ejecutar el proceso YAMA: `yama/bin/yama`
8. Ejecutar el proceso Master: `master/bin/master`

> Para trabajar en Eclipse, importar el proyecto haciendo: File -> New -> Makefile Project with existing code -> Destildar "C++"" -> Browse a la carpeta raíz del TP -> Seleccionar "Linux GCC"


## Guía de Estilo

* Constantes: todo en mayúsculas con guíones bajos. Ej.: `#define NUM_PROCESO_MASTER 1`

* Nombres de funciones y variables: lowerCamelCase (inicial de cada palabra en mayúscula excepto la primera)   
Ej.: `preparadoRecibir, iniciarPrograma()`


## Reglas

1) Por favor, **NO** subas cosas sin probar; nos complicás y retrasás a todos.
Si querés subir algo que no llegaste a revisar, q está _in progress_ y/o que no estás seguro si funciona para que los demás lo vean hacelo en un branch aparte de master.

2) No compiles a mano o usando cosas raras. Sea en Eclipse o en la consola hacelo usando el Makefile provisto, así nos aseguramos que todos lo hacemos de la **misma** manera.

3) No subas binarios ni archivos de proyecto/configuración: son innecesarios, contaminan el repo y confunden. 
Los primeros se deben generar automáticamente cuando compiles (sino quedan obsoletos), y los segundos son particulares de cada persona/entorno.

> Si no cumplís las reglas -> HAY TABLA. 


## Configuración

Proceso | IP | Puerto |
:---: | :---: | :---: |
FileSystem | 127.0.0.1 | #### |
DataNode | 127.0.0.1 | #### |
Worker | 127.0.0.1 | #### |
YAMA | 127.0.0.1 | #### |
Master | 127.0.0.1 | #### |


## Arquitectura

![ARQUITECTURA](https://sisoputnfrba.gitbooks.io/yama-tp-2c2017/content/assets/arquitectura.png)

Link: https://sisoputnfrba.gitbooks.io/yama-tp-2c2017/content/introduccion-y-arquitectura.html


## Protocolo de Comunicación v1.1

* struct headerProtocolo{
	int id;
	int tamPayload;
};

* Del lado que envía:
	* enviarHeader(socketServidor,header)
	* enviarMensaje(socketServidor,nombreArchivoRequerido)


