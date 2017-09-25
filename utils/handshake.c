/*
 * handshake.c
 *
 *  Created on: 18/9/2017
 *      Author: utnso
 */

int handshake_cliente(int socket, char id_cliente, char id_servidor) {
 /*   msj_header cliente;
    cliente.codigo = id_cliente;
    if (send(socket, &cliente, sizeof(msj_header), 0) != sizeof(msj_header)) {
        perror("send");
        return -1;
    }

    msj_header servidor;
    if (recv(socket, &servidor, sizeof(msj_header), 0) != sizeof(msj_header)) {
        perror("recv");
        return -1;
    }
    if (servidor.codigo != id_servidor) {
        printf("Error: el servidor no es kernel.\n");
        return -2;
    }
   */ return 0;
}

int handshake_servidor(int socket, char id_servidor, const char *clientes_aceptados) {
   /* msj_header cliente;
    if (recv(socket, &cliente, sizeof(msj_header), 0) != sizeof(msj_header)) {
        perror("recv");
        return '\0';
    }

    while (*clientes_aceptados) {
        if (cliente.codigo == *clientes_aceptados++) {
            msj_header servidor;
            servidor.codigo = id_servidor;
            if ((send(socket, &servidor, sizeof(msj_header), 0)) != sizeof(msj_header)) {
                perror("send");
                return '\0';
            }
            return cliente.codigo;
        }
    }

    */return '\0';
}
