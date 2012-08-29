/*
 * superbloque.c
 *
 *  Created on: 29/04/2012
 *      Author: utnso
 */

#include <stdio.h>
#include "superbloque.h"
#include <sys/types.h>
#include <stdint.h>

extern void* FSmap;

superbloque_t* leer_superbloque() {
	superbloque_t* sb = (superbloque_t*) (FSmap + 1024);
	return sb;
}



uint32_t cantidadDeGroupBlocks(superbloque_t* sb) {
	if((sb->s_blocks_count %sb->s_blocks_per_group)!=0)
		return 	(sb->s_blocks_count /sb->s_blocks_per_group)+1;
	else return (sb->s_blocks_count/sb->s_blocks_per_group);
}

sbInfo get_sbInfo(superbloque_t* sb){
	sbInfo sbi;
	sbi.blockSize=1024<<sb->s_log_block_size;
	sbi.bloquesXGrupo=sb->s_blocks_per_group;
	sbi.inodosXGrupo=sb->s_inodes_per_group;
	sbi.cantidadDeGroupBlocks=cantidadDeGroupBlocks(sb);
	sbi.freeInodesCount=sb->s_free_inodes_count;
	return sbi;
}
