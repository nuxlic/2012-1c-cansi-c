/*
 * groupDescriptor.h
 *
 *  Created on: 29/04/2012
 *      Author: utnso
 */

#ifndef GROUPDESCRIPTOR_H_
#define GROUPDESCRIPTOR_H_
#include <stdint.h>

typedef struct groupDescriptor{
	uint32_t	bg_block_bitmap;		/* Blocks bitmap block */
	uint32_t	bg_inode_bitmap;		/* Inodes bitmap block */
	uint32_t	bg_inode_table;		/* Inodes table block */
	uint16_t	bg_free_blocks_count;	/* Free blocks count */
	int16_t	bg_free_inodes_count;	/* Free inodes count */
	uint16_t	bg_used_dirs_count;	/* Directories count */
	uint16_t	bg_pad;
	uint32_t	bg_reserved[3];
} __attribute__ ((packed)) gDescrip_t;

gDescrip_t* leerTablaDeDescriptoresDeGrupos();





#endif /* GROUPDESCRIPTOR_H_ */
