/*
 * Socket.c
 *
 *  Created on: 10/05/2012
 *      Author: utnso
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include<sys/socket.h>
#include<errno.h>
#include<stdio.h>
#include<string.h>
#include<netinet/in.h>
#include<stdlib.h>
#include<fcntl.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "sockear.h"
#include "serializadores.h"

#define MY_IP "127.0.0.1"
#define DEST_IP "127.0.0.1"
#define DEST_PORT 3493
#define MYPORT 3492
#define ERRORSOCKET -1
#define ERRORBIND -2
#define ERRORLISTEN -3
#define ERRORACCEPT -4
#define ERRORCONNECT -5
#define ERRORRECV -6
#define ERRORSEND -7
#define ERRORCLOSE -8
#define ERRORHNDSHK -9
#define ERRORCONNECTWASCLOSE -10

int32_t maxClientes;

/*SERVIDOR*/

/*Creacion del socket */
uint32_t crear_socket() {
	uint32_t descriptor = socket(AF_INET, SOCK_STREAM, 0);
	if (descriptor == -1) {
		perror("Error de creacion de socket");
		return ERRORSOCKET;
	} else
		return descriptor;
}

/*Dar nombre al socket*/
uint32_t bindear_socket(int descriptor, uint32_t port) {
	int exit = 0;
	struct sockaddr_in direccion;
	direccion.sin_family = AF_INET;/*el el tipo de coneccion, igual que el primer parametro de socket().*/
	direccion.sin_port = htons(port);/* es le numero correspondiente al puerto*/
	direccion.sin_addr.s_addr = INADDR_ANY;/* es la direccion del cliente al que queremos atender, colocando INADDR_ANY atendemos a cualquiera*/

	if (bind(descriptor, (struct sockaddr*) &direccion, sizeof(direccion))
			== -1) {
		perror("Error de puerto");
		return ERRORBIND;
	} else
		return exit;
}

/*Empezar a atender llamadas*/

uint32_t escuchar_llamadas(int descriptor) {
	int exit = 0;
	if (listen(descriptor, maxClientes) == -1)/*el segundo parametro corresponde al numero maximo de clientes encolados*/
	{
		perror("Error de listen");
		return ERRORLISTEN;
	} else
		return exit;
}

/*Aceptar Clientes en espera, este proceso deberia estar en un while infinito para que el servidor este siempre en espera*/

uint32_t aceptar_clientes(int descriptor) {
	struct sockaddr Cliente;
	int Descriptor_cliente;
	//int Longitud_cliente;
	int addrlen = sizeof(struct sockaddr_in);

	Descriptor_cliente = accept(descriptor, &Cliente, (void *) &addrlen);
	if (Descriptor_cliente == -1) {
		perror("Error al aceptar");
		return ERRORACCEPT;
	} else
		return Descriptor_cliente;
}

/*Para crear el socket usamos Crear_socket (igual que el servidor) */

uint32_t conectar_socket(int descriptor, char* ipServer, uint32_t portServer) {
	int exit = 0;
	struct sockaddr_in direccion;
	direccion.sin_family = AF_INET;/*el el tipo de coneccion, igual que el primer parametro de socket().*/
	direccion.sin_port = htons(portServer);/* es le numero correspondiente al puerto*/
	inet_aton(ipServer, &direccion.sin_addr);/* es la direccion del cliente al que queremos atender, colocando INADDR_ANY atendemos a cualquiera*/

	if (connect(descriptor, (struct sockaddr*) &direccion, sizeof(direccion))
			== -1) {
		perror("Error de coneccion");
		return ERRORCONNECT;
	} else
		return exit;
}

uint32_t enviar_paquete(uint32_t descriptor, void *mensaje, uint32_t length) {
	//Si ponemos la flag MSG_DONTWAIT se transforma en NO BLOQUEANTE. Hay que preguntar si conviene que se bloquee por el tema de si hay muchos clientes y los threads
	int32_t bytes_enviados = 0, aux = 0;
	while (aux < length) {
		if (bytes_enviados == -1) {
			perror("Error del send()");
			return ERRORSEND;
		}
		bytes_enviados = send(descriptor, mensaje+aux, length, 0);
		aux = aux + bytes_enviados;
	}

	return aux;
}
char* recibir_paquete(uint32_t server_descriptor, uint16_t length) {
	char* paquete = malloc(length);
	recibir(server_descriptor, paquete, length);
	return paquete;
}

uint32_t recibir(uint32_t descriptor, void *mensaje_recibido, uint16_t length) {
	int32_t bytes_recibidos;

	bytes_recibidos = recv(descriptor, mensaje_recibido, length, 0);
	if (bytes_recibidos < 0) {
		perror("Error en recv()");
		return ERRORRECV;
	}
	if (bytes_recibidos == 0) {
		perror("Cliente cerro la conexion");
		return ERRORCONNECTWASCLOSE;
	}
	int32_t aux = bytes_recibidos;
	while (aux < length) {
		bytes_recibidos = recv(descriptor, mensaje_recibido+aux, length, 0);
		aux += bytes_recibidos;
	}
	return aux;
}

uint32_t cerrar(uint32_t descriptor) {
	int exit = 0;
	if (close(descriptor) == EINTR) {
		perror("Error del close()");
		return ERRORCLOSE;
	}
	return exit;
}

uint8_t iniciar_comunicacion(int descriptor) {

	t_stream *handshakeEnviado = serializar_handshake(OK);
	enviar_paquete(descriptor, handshakeEnviado->data,
			handshakeEnviado->length);
	free(handshakeEnviado);
	char* paquete=malloc(3);
	recibir(descriptor,paquete,3);
	uint8_t* handSrecibido=deserializar_handshake(paquete,0);
	uint8_t hand=*handSrecibido;
	free(handSrecibido);
	return hand;
}

