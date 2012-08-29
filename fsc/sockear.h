/*
 * Socket.h
 *
 *  Created on: 10/05/2012
 *      Author: utnso
 */

#ifndef SOCKET_H_
#define SOCKET_H_


uint32_t crear_socket();

uint32_t bindear_socket(int descriptor,uint32_t port);

uint32_t escuchar_llamadas(int descriptor);

uint32_t aceptar_clientes (int descriptor);

uint32_t conectar_socket(int descriptor,char* ip, uint32_t port);

uint32_t enviar_paquete(uint32_t descriptor, void *mensaje, uint32_t length);

char* recibir_paquete(uint32_t server_descriptor, uint16_t length);

uint32_t recibir(uint32_t descriptor, void* mensaje_recibido, uint16_t length);

char *recibir_cabecera(int descriptor);

uint32_t cerrar(uint32_t descriptor);

uint8_t iniciar_comunicacion(int descriptor);

#endif /* SOCKET_H_ */
