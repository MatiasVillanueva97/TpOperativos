/*
 * Sockets.c
 *
 *  Created on: 8/9/2017
 *      Author: utnso
 */


#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>

int crear_servidor(int puerto, void* (*funcion_nuevo_cliente)(void * socket)) {
    const int MAX_CONEXIONES = 30;

    int socket_escucha, socket_nueva_conexion;

    struct sockaddr_in socket_info;

    int yes = 1;
    if ((socket_escucha = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        return -1;
    }

    setsockopt(socket_escucha, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    memset(&socket_info, 0, sizeof(socket_info));
    socket_info.sin_family = AF_INET;
    socket_info.sin_addr.s_addr = INADDR_ANY;
    socket_info.sin_port = htons(puerto);

    if (bind(socket_escucha, (struct sockaddr*) &socket_info, sizeof(socket_info)) != 0) {
        perror("bind");
        return -1;
    }

    if (listen(socket_escucha, MAX_CONEXIONES) != 0) {
        perror("listen");
        return -1;
    }

    while (1) {

        if ((socket_nueva_conexion = accept(socket_escucha, NULL, 0)) < 0) {
            perror("accept");
            return -1;
        }

        int * socket_funcion = malloc(sizeof(int));
        *socket_funcion = socket_nueva_conexion;

        pthread_attr_t attr;
        pthread_t thread;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        pthread_create(&thread, &attr, funcion_nuevo_cliente, (void*) socket_funcion);
        pthread_attr_destroy(&attr);
    }
    return 0;
}

int conectar(char *ip, int port) {
    int socket_cliente;
    if ((socket_cliente = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        return -1;
    }

    struct sockaddr_in sockaddress;
    memset(&sockaddress, 0, sizeof(struct sockaddr_in));
    sockaddress.sin_family = AF_INET;
    sockaddress.sin_addr.s_addr = inet_addr(ip);
    sockaddress.sin_port = htons(port);

    if (connect(socket_cliente, (struct sockaddr*) &sockaddress, sizeof(struct sockaddr))) {
        perror("connect");
        return -1;
    }

    return socket_cliente;
}

int handshake_cliente(int socket, char id_cliente, char id_servidor) {
    msj_header cliente;
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
    return 0;
}

int handshake_servidor(int socket, char id_servidor, const char *clientes_aceptados) {
    msj_header cliente;
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

    return '\0';
}
