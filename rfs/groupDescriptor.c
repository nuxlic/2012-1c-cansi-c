/*
 * groupDescriptor.c
 *
 *  Created on: 03/05/2012
 *      Author: utnso
 */

#include <stdio.h>
#include "groupDescriptor.h"
#include <stdint.h>
#include "superbloque.h"

extern void* FSmap;
extern sbInfo SbI;

gDescrip_t* leerTablaDeDescriptoresDeGrupos() { //me devuelve un puntero a la gdt del primer grupo
	gDescrip_t* gdt;
	 if (SbI.blockSize == 1024)
	    gdt = (gDescrip_t*) (FSmap + 2048);
	 else gdt = (gDescrip_t*) (FSmap + SbI.blockSize);

	return gdt;
}
