/*
 * socketHandler.h
 *
 *  Created on: 07/06/2012
 *      Author: utnso
 */

#include "commons/serializadores.h"
#include "multiThreading.h"
#include "synchronizer.h"
#ifndef SOCKETHANDLER_H_
#define SOCKETHANDLER_H_


individualJob_t* functionIdentifyAndOperate(t_header* header,t_stream*,int32_t clientes,structures_synchronizer_t*);
#endif /* SOCKETHANDLER_H_ */
