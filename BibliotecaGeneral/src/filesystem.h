/*
 * filesystem.h
 *
 *  Created on: 16/9/2017
 *      Author: utnso
 */

#ifndef FILESYSTEM_SRC_FILESYSTEM_H_
#define FILESYSTEM_SRC_FILESYSTEM_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

t_log* logger;

void crearLogger(){
    char * archivoLog = strdup("FILESYSTEM_LOG.log");
    logger = log_create("FILESYSTEM_LOG.log", archivoLog, true, LOG_LEVEL_INFO);
    free(archivoLog);
    archivoLog = NULL;
}

#endif /* FILESYSTEM_SRC_FILESYSTEM_H_ */
