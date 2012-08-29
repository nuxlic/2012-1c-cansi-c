/*
 * inode.c
 *
 *  Created on: 06/05/2012
 *      Author: utnso
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "inode.h"
#include "groupDescriptor.h"
#include "superbloque.h"
#include "ext2_def.h"
#include "bitmap.h"
#include "commons/collections/list.h"

extern sbInfo SbI;
extern void* FSmap;

inode_t* leerTablaDeInodos(uint32_t numeroDeGrupo) {
	gDescrip_t* gdt = leerTablaDeDescriptoresDeGrupos();
	inode_t* tInodo = (inode_t*) (FSmap
			+ (gdt[numeroDeGrupo].bg_inode_table * SbI.blockSize));
	//printf("%d\n",gdt->bg_inode_table);
	return tInodo;
}

inode_t* leerInodo(uint32_t inodeNumber) {
	uint32_t blockGroup = (inodeNumber - 1) / SbI.inodosXGrupo;
	inode_t* tablaDeInodos = leerTablaDeInodos(blockGroup);
	//inode_t* tablaDeInodos=(inode_t*)(FSmap+5*1024);
	uint32_t inodeIndex = (inodeNumber - 1) % SbI.inodosXGrupo;
	return 	&tablaDeInodos[inodeIndex];

}

uint32_t* leerDirectos(uint32_t* arrayDirectsBlocks, uint32_t bloquesAleer) {
	uint32_t* direcciones = calloc(bloquesAleer, sizeof(uint32_t));
	if (bloquesAleer > 12 * SbI.blockSize)
		return NULL;
	memcpy(direcciones, arrayDirectsBlocks, bloquesAleer * sizeof(uint32_t));
	return direcciones;
}

uint32_t* leerIndireccionamientoSimple(uint32_t indirectoSimple, uint32_t level,
		uint32_t bloquesAleer) {
	uint32_t* pointer = (uint32_t*) (FSmap + indirectoSimple * SbI.blockSize);
	if (bloquesAleer > SbI.blockSize / sizeof(uint32_t))
		return NULL;
	uint32_t* direcciones = calloc(bloquesAleer, sizeof(uint32_t));
	memcpy(direcciones, pointer, bloquesAleer * sizeof(uint32_t));
	return direcciones;

}

uint32_t* leerIndireccionamientoDobleTriple(uint32_t indirecto, uint32_t level,
		uint32_t bloquesAleer) {

	uint32_t maxBlocksInThisLevel = pow((SbI.blockSize / sizeof(uint32_t)),
			level);

	uint32_t* (*leerIndireccionamiento)(uint32_t, uint32_t,
			uint32_t) = leerIndireccionamientoDobleTriple;

	if ((level - 1) == 1)
		leerIndireccionamiento = leerIndireccionamientoSimple;

	if (bloquesAleer > maxBlocksInThisLevel)
		return NULL;

	uint32_t* arrayOfNumbersOfBlocks = calloc(bloquesAleer, sizeof(uint32_t));

	if (arrayOfNumbersOfBlocks == NULL)
		return NULL;

	uint32_t blocksPerPosInBlockArray = pow((SbI.blockSize / sizeof(uint32_t)),
			level - 1);
	uint32_t entriesToReadInBlockArray = bloquesAleer
			/ blocksPerPosInBlockArray;
	uint32_t bloquesRestantes = bloquesAleer % blocksPerPosInBlockArray;

	uint32_t sizeInBytes = blocksPerPosInBlockArray * sizeof(uint32_t);

	uint32_t i;
	uint32_t pointer;
	uint32_t j = 0;
	uint32_t* indirectPointer = (uint32_t*) (FSmap + indirecto * SbI.blockSize);

	for (i = 0; i < entriesToReadInBlockArray; ++i) {
		pointer = indirectPointer[i];
		uint32_t* aux = leerIndireccionamiento(pointer, level - 1,
				blocksPerPosInBlockArray);

		if (aux == NULL)
			return NULL;

		memcpy(&arrayOfNumbersOfBlocks[j], aux, sizeInBytes);
		free(aux);

		j += blocksPerPosInBlockArray;

	}

	if (bloquesRestantes > 0) {
		pointer = indirectPointer[i];
		uint32_t* aux2 = leerIndireccionamiento(pointer, level - 1,
				bloquesRestantes);

		if (aux2 == NULL)
			return NULL;

		memcpy(&arrayOfNumbersOfBlocks[j], aux2,
				bloquesRestantes * sizeof(uint32_t));
		free(aux2);

	}

	return arrayOfNumbersOfBlocks;

}

uint32_t* ext2_buscarPunterosLibresIndSimple(uint32_t indS,uint32_t cantidad, uint32_t level,uint32_t* cantidadLibre){
	uint32_t* pointer=(uint32_t*)(FSmap+indS*SbI.blockSize);
	uint32_t cont=0;
	while(*pointer!=0){
		cont++;
		pointer+=1;

	}
	*cantidadLibre=pow((SbI.blockSize/sizeof(uint32_t)),1);
	return pointer;
}
bool leerIndireccion(uint32_t* address_to_copy, uint32_t indirect_address, uint32_t level, uint32_t cantidad){


	uint32_t* (*Read_Indirection) (uint32_t,uint32_t,uint32_t);

	if (level > 1)
		Read_Indirection = leerIndireccionamientoDobleTriple;
	else
		Read_Indirection = leerIndireccionamientoSimple;

	uint32_t* indirect_pointer = Read_Indirection(indirect_address,level,cantidad);

	if (indirect_pointer == NULL)
		return false;

	memcpy(address_to_copy,indirect_pointer,cantidad * sizeof(uint32_t));
	free(indirect_pointer);

	return true;
}
