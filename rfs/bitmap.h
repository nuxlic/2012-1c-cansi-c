/*
 * blockBitmap.h
 *
 *  Created on: 04/05/2012
 *      Author: utnso
 */

#ifndef BLOCKBITMAP_H_
#define BLOCKBITMAP_H_
#include "commons/bitarray.h"
#include "inode.h"
#include "fs_handler.h"
#include "multiThreading.h"

t_bitarray* leerBitmapDeBloques( uint32_t);
t_bitarray* leerBitmapDeInodos( uint32_t);

uint32_t* buscarBloquesLibres(uint32_t cantidad,structures_synchronizer_t*);
int32_t buscarInodosLibres(structures_synchronizer_t*);
void asignarInodo(uint32_t inodeNumber,structures_synchronizer_t*);
void asignarBloque(uint32_t blockNumber,structures_synchronizer_t*);
void desasignarBloque(int32_t blockNumber,args_t*);
void desasignarUnArrayDeBloques(uint32_t indirecto,structures_synchronizer_t*);
void desasignarInodo(uint32_t inodeNumber,structures_synchronizer_t*);

#endif /* BLOCKBITMAP_H_ */
