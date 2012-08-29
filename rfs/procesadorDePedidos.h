/*
 * procesadorDePedidos.h
 *
 *  Created on: 16/07/2012
 *      Author: utnso
 */

#ifndef PROCESADORDEPEDIDOS_H_
#define PROCESADORDEPEDIDOS_H_

void procesarPedidoCreateFile(args_t* args);
void procesarPedidoCreateDir(args_t* args);
void procesarPedidoDeleteFile(args_t* args);
void procesarPedidoDeleteDir(args_t* args);
void procesarPedidoReadFile(args_t* args);
void procesarPedidoWriteFile(args_t* args);
void procesarPedidoGetAttr(args_t* args);
void procesarPedidoReadDir(args_t* args);
void procesarPedidoTruncate(args_t* args);
void procesarPedidoOpenFile(args_t* args);

#endif /* PROCESADORDEPEDIDOS_H_ */
