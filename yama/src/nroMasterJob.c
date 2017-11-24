/*
 * nroMasterJob.c
 *
 *  Created on: 24/11/2017
 *      Author: utnso
 */

uint16_t maxNroMaster = 0, maxNroJob = 0;

typedef struct {
	uint16_t nroMaster;
	uint16_t nroJob;
} nroMasterJob;

//vector con los número de master y job asignados a cada master que se conecta
//está indexada por FileDescriptor
//se actualiza cuando el select recibe una conexión nueva y crea el FD
//o cuando se desconecta el master y el select da de baja el FD
nroMasterJob listaNrosMasterJob[100];

nroMasterJob getNroMasterJobByFD(int fileDescriptor) {
	return listaNrosMasterJob[fileDescriptor];
}

void asignarNroMasterJob(uint16_t nroMaster, uint16_t nroJob, int fileDescriptor) {
	listaNrosMasterJob[fileDescriptor].nroJob = nroJob;
	listaNrosMasterJob[fileDescriptor].nroMaster = nroMaster;
}

uint16_t getNuevoNroMaster() {
	return (++maxNroMaster);
}

uint16_t getNuevoNroJob() {
	return (++maxNroJob);
}
