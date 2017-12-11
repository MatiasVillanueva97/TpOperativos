/*
 * nroMasterJob.c
 *
 *  Created on: 24/11/2017
 *      Author: utnso
 */

uint16_t maxNroMaster = 0, maxNroJob = 0;

struct nodosUsadosPlanificacion{
	int numero;
	int cantidadVecesUsados;
};

typedef struct {
	uint16_t nroMaster;
	uint16_t nroJob;
	uint16_t nodoReduccGlobal;
	uint16_t cantBloquesArchivo;
	uint16_t cantNodosUsados;
	struct nodosUsadosPlanificacion *nodosUsados;
} nroMasterJob;	//datos JOB

//vector con los datos de un job y master conectados
//está indexada por FileDescriptor
//se actualiza cuando el select recibe una conexión nueva y crea el FD
//o cuando se desconecta el master y el select da de baja el FD
nroMasterJob listaNrosMasterJob[CANT_MAX_FD];

nroMasterJob* getNroMasterJobByFD(int fileDescriptor) {
	return &listaNrosMasterJob[fileDescriptor];
}

void asignarNroMasterJob(uint16_t nroMaster, uint16_t nroJob, int fileDescriptor) {
	listaNrosMasterJob[fileDescriptor].nroJob = nroJob;
	listaNrosMasterJob[fileDescriptor].nroMaster = nroMaster;
}

void asignarNodoReduccGlobal(uint16_t nodoReduccGlobal, int fileDescriptor) {
	listaNrosMasterJob[fileDescriptor].nodoReduccGlobal = nodoReduccGlobal;
}

uint16_t getNuevoNroMaster() {
	return (++maxNroMaster);
}

uint16_t getNuevoNroJob() {
	return (++maxNroJob);
}
