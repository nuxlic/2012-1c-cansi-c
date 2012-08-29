/*
 * blockBitmap.c
 *
 *  Created on: 04/05/2012
 *      Author: utnso
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include "bitmap.h"
#include "superbloque.h"
#include "groupDescriptor.h"
#include "commons/bitarray.h"
#include <string.h>
#include "inode.h"
#include "memcachear.h"

extern void* FSmap;
extern sbInfo SbI;
extern char* ipCache;
extern uint32_t portCache;

t_bitarray* leerBitmapDeBloques(uint32_t numeroDeGrupo) { //lee el bloque del bitmap y me devuelve un bitarray

	gDescrip_t* gdt = leerTablaDeDescriptoresDeGrupos();
	char* bitarray;
	bitarray = (char*) (FSmap
			+ (gdt[numeroDeGrupo].bg_block_bitmap * SbI.blockSize));
	t_bitarray* bitmap;
	bitmap = bitarray_create(bitarray, SbI.blockSize);

	return bitmap;
}

t_bitarray* leerBitmapDeInodos(uint32_t numeroDeGrupo) { //lee el bloque del bitmap y me devuelve un bitarray

	gDescrip_t* gdt = leerTablaDeDescriptoresDeGrupos();
	char* bitarray;
	bitarray = (char*) (FSmap
			+ (gdt[numeroDeGrupo].bg_inode_bitmap * SbI.blockSize));

	t_bitarray* bitmap;
	bitmap = bitarray_create(bitarray, SbI.blockSize);

	return bitmap;
}

uint32_t* buscarBloquesLibres(uint32_t cantidad,
		structures_synchronizer_t* locks) {
	uint32_t* libres = calloc(cantidad, sizeof(uint32_t));
	uint32_t aux, i, j, cont;
	cont = 0;
	for (i = 0; i < SbI.cantidadDeGroupBlocks && cont != cantidad; i++) {
		pthread_mutex_lock(&locks->writeBlocksLock[i]);
		t_bitarray* bitmap = leerBitmapDeBloques(i);
		for (j = 0; j < SbI.bloquesXGrupo && cont != cantidad; j++) {
			bool bit = bitarray_test_bit(bitmap, j);
			aux = j + i * SbI.bloquesXGrupo + 1;
			if (bit == 0) {
				memcpy(&libres[cont], &aux, sizeof(uint32_t));
				cont++;
			}
		}
		bitarray_destroy(bitmap);
		pthread_mutex_unlock(&locks->writeBlocksLock[i]);
	}
	if (cont == 0) {
		perror("Todos Ocupados");
		return NULL;
	}
	return libres;
}

int32_t buscarInodosLibres(structures_synchronizer_t* locks) {

	uint32_t i, j;
	uint32_t aux = 0;
//	superbloque_t* sb=leer_superbloque();
//	SbI.freeInodesCount=sb->s_free_inodes_count;
//	if(SbI.freeInodesCount==0){
//		return -1;
//	}
	for (i = 0; i < SbI.cantidadDeGroupBlocks; i++) {
		pthread_mutex_lock(&locks->writeInodesLock[i]);
		t_bitarray* bitmap = leerBitmapDeInodos(i);
		for (j = 0; j < SbI.inodosXGrupo; j++) {
			bool bit = bitarray_test_bit(bitmap, j);
			aux = j + i * SbI.inodosXGrupo + 1;
			if (bit == 0) {
				pthread_mutex_unlock(&locks->writeInodesLock[i]);
				bitarray_destroy(bitmap);
				return aux;
			}
		}
		bitarray_destroy(bitmap);
		pthread_mutex_unlock(&locks->writeInodesLock[i]);

	}
			perror("Todos Ocupados");
		return -1;

}

void asignarInodo(uint32_t inodeNumber, structures_synchronizer_t* locks) {
	uint32_t blockGroup = (inodeNumber - 1) / SbI.inodosXGrupo;
	pthread_mutex_lock(&locks->writeInodesLock[blockGroup]);
	t_bitarray* bitmap = leerBitmapDeInodos(blockGroup);
	uint32_t inodeIndex = (inodeNumber - 1) % SbI.inodosXGrupo;
	bitarray_set_bit(bitmap, inodeIndex);
	gDescrip_t* gdt = leerTablaDeDescriptoresDeGrupos();
	gdt[blockGroup].bg_free_inodes_count -= 1;
//	if(gdt[blockGroup].bg_free_inodes_count<0){
//		gdt[blockGroup].bg_free_inodes_count=0;
//	}
	pthread_mutex_unlock(&locks->writeInodesLock[blockGroup]);
	pthread_mutex_lock(&locks->sbLock);
	superbloque_t* sb = leer_superbloque();
	sb->s_free_inodes_count -= 1;
//	if (sb->s_free_inodes_count < 0) {
//		sb->s_free_inodes_count = 0;
//	}
	bitarray_destroy(bitmap);
	pthread_mutex_unlock(&locks->sbLock);
	//	actualizarFS(gdt[blockGroup].bg_block_bitmap,bitmap->bitarray);
}

void asignarBloque(uint32_t blockNumber, structures_synchronizer_t* locks) {
	uint32_t blockGroup = (blockNumber - 1) / SbI.bloquesXGrupo;
	pthread_mutex_lock(&locks->writeBlocksLock[blockGroup]);
	t_bitarray* bitmap = leerBitmapDeBloques(blockGroup);
	uint32_t blockIndex = (blockNumber - 1) % SbI.bloquesXGrupo;
	bitarray_set_bit(bitmap, blockIndex);
	gDescrip_t* gdt = leerTablaDeDescriptoresDeGrupos();
	gdt[blockGroup].bg_free_blocks_count -= 1;
	pthread_mutex_unlock(&locks->writeBlocksLock[blockGroup]);
	pthread_mutex_lock(&locks->sbLock);
	superbloque_t* sb = leer_superbloque();
	sb->s_free_blocks_count -= 1;
	pthread_mutex_unlock(&locks->sbLock);
	bitarray_destroy(bitmap);

	//	actualizarFS(gdt[blockGroup].bg_block_bitmap,bitmap->bitarray);
}

void desasignarBloque(int32_t blockNumber, args_t* args) {
	memc_t* mc=memc_connect(ipCache,portCache);
	args->ped->mc=mc->memcached;
	memc_delete(args,blockNumber);
	memc_free(&mc);
	uint32_t blockGroup = (blockNumber - 1) / SbI.bloquesXGrupo;
	pthread_mutex_lock(&args->lockeos->writeBlocksLock[blockGroup]);
	t_bitarray* bitmap = leerBitmapDeBloques(blockGroup);
	uint32_t blockIndex = (blockNumber - 1) % SbI.bloquesXGrupo;
	bitarray_clean_bit(bitmap, blockIndex);
	gDescrip_t* gdt = leerTablaDeDescriptoresDeGrupos();
	gdt[blockGroup].bg_free_blocks_count += 1;
	pthread_mutex_unlock(&args->lockeos->writeBlocksLock[blockGroup]);
	pthread_mutex_lock(&args->lockeos->sbLock);
	superbloque_t* sb = leer_superbloque();
	sb->s_free_blocks_count += 1;
	pthread_mutex_unlock(&args->lockeos->sbLock);
	bitarray_destroy(bitmap);

}



void desasignarInodo(uint32_t inodeNumber, structures_synchronizer_t* locks) {
	uint32_t blockGroup = (inodeNumber - 1) / SbI.inodosXGrupo;
	pthread_mutex_lock(&locks->writeInodesLock[blockGroup]);
	t_bitarray* bitmap = leerBitmapDeInodos(blockGroup);
	uint32_t inodeIndex = (inodeNumber - 1) % SbI.inodosXGrupo;
	bitarray_clean_bit(bitmap, inodeIndex);
	gDescrip_t* gdt = leerTablaDeDescriptoresDeGrupos();
	gdt[blockGroup].bg_free_inodes_count += 1;
	pthread_mutex_unlock(&locks->writeInodesLock[blockGroup]);
	pthread_mutex_lock(&locks->sbLock);
	superbloque_t* sb = leer_superbloque();
	sb->s_free_inodes_count++;
	pthread_mutex_unlock(&locks->sbLock);
	bitarray_destroy(bitmap);

}



