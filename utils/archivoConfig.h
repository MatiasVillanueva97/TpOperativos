/*
 * archivoConfig.h
 *
 *  Created on: 18/9/2017
 *      Author: utnso
 */

#ifndef UTILS_ARCHIVOCONFIG_H_
#define UTILS_ARCHIVOCONFIG_H_

/*
 * lee el archivo de configuración y guarda los datos en un array
 * recibe el nombre del archivo de configuración, el puntero al array con las keys a leer y el puntero al array donde guardar los datos
 * devuelve 0 si OK o 1 si ERROR
 */
int leerArchivoConfig(char *nameArchivoConfig, char **keysConfig, char **datosConfig);

#endif /* UTILS_ARCHIVOCONFIG_H_ */
