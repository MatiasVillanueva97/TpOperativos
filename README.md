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
Matias Villanueva | 1559837 | matias.nahuel.villanueva@hotmail.com |


## Requisitos

1. Tener instalado gcc
2. Tener instaladas la commons library ([link](https://github.com/sisoputnfrba/so-commons-library))
3. Tener instaladas la pthreads library
4. Tener instaladas la readline library (sudo apt-get install libreadline6-dev)
5. Tener instalado Ruby si se corre algún script (Transformador o Reductor) de tipo `.rb` (sudo apt-get install ruby)

## Instrucciones

1. Clonar este repositorio: `git clone https://github.com/sisoputnfrba/tp-2017-2c-Mi-Grupo-1234`
2. Entrar al directorio: `cd tp-2017-2c-Mi-Grupo-1234`
3. Compilar usando el Makefile provisto: `make`
4. Ejecutar todos los procesos juntos: `make run`

> Ojo! Si lo corrés varias veces seguidas falla porq las conexiones quedan abiertas, para resolverlo guardá los cambios y ejecutá lo siguiente :)

`sudo sysctl -w net.ipv4.tcp_tw_recycle=1 && sudo sysctl -w net.ipv4.tcp_tw_reuse=1 && sudo reboot`


> Opc.: Para ejecutar cada proceso por separado
* FileSystem: `cd filesystem/bin && ./filesystem`
* DataNode: `cd datanode/bin && ./datanode` (TO DO)
* Worker: `cd worker/bin && ./worker` (TO DO)
* YAMA: `cd yama/bin && ./yama`
* Master: `cd master/bin && ./master`


> Para trabajar en Eclipse, importá el proyecto haciendo: File -> New -> Makefile Project with existing code -> Destildar "C++"" -> Browse a la carpeta raíz del TP -> Seleccionar "Linux GCC"


## Guía de Estilo

* Constantes: todo en mayúsculas con guíones bajos. Ej.: `#define NUM_PROCESO_MASTER 1`

* Nombres de funciones y variables: lowerCamelCase (inicial de cada palabra en mayúscula excepto la primera)
Ej.: `preparadoRecibir, iniciarPrograma()`

* Asignaciones, enum's, etc. con espacios!
Ej.: `foo = bar, foo2 = bar2;`

> Si usás el comando *Format* de Eclipse (Shift+Ctrl+F) cambiate la siguiente config.:
Windows -> Preferences -> C/C++ -> Code Style -> Formatter -> Edit -> Line Wrapping -> Poné "Do not wrap" en c/u de los 5 tipos "padres" (Class, Function, etc.) -> Renombrá el profile y guardá


## Reglas

1) Por favor, **NO** subas cosas sin probar; nos complicás y retrasás a todos.
Si querés subir algo que no llegaste a revisar, q está _in progress_ y/o que no estás seguro si funciona para que los demás lo vean hacelo en un branch aparte de master.

2) No compiles a mano o usando cosas raras. Sea en Eclipse o en la consola hacelo usando el Makefile provisto, así nos aseguramos que todos lo hacemos de la **misma** manera.

3) No subas binarios ni archivos de proyecto/configuración: son innecesarios, contaminan el repo y confunden.
Los primeros se deben generar automáticamente cuando compiles (sino quedan obsoletos), y los segundos son particulares de cada persona/entorno.

> Si no cumplís las reglas -> HAY TABLA.

> Si querés evitar la tabla -> FACTURAS

## Configuración

Proceso | Archivo | IP | Puerto |
:---: | :---: | :---: | :---: |
FileSystem | configFilesystem.txt | 127.0.0.1 | 5000 |
YAMA | configYama.txt | 127.0.0.1 | 5100 |
Master | configMaster.txt | 127.0.0.1 | 5200 |
Nodo (Worker + DataNode) | configNodo.txt | 127.0.0.1 | 5300 |


## Arquitectura

![ARQUITECTURA](https://sisoputnfrba.gitbooks.io/yama-tp-2c2017/content/assets/arquitectura.png)

Link: https://sisoputnfrba.gitbooks.io/yama-tp-2c2017/content/


## Protocolo de Comunicación Serializado

* el header tiene un id de 4 bytes que indica cuántos mensajes se enviaron

* Del lado que envía:
	* mensajeSerializado=serializarMensaje(int id,char *arrayMensajes[])
	* enviarMensaje(socketServidor,mensajeSerializado)

* Del lado que recibe:
	* int32_t headerId = deserializarHeader(socketCliente)	//recibe el id del header para saber qué esperar
	* int cantidadMensajes = protocoloCantidadMensajes[headerId]	//averigua la cantidad de mensajes que le van a llegar
	* char **arrayMensajes = deserializarMensaje(socketCliente, cantidadMensajes)	//recibe los mensajes en un array de strings



