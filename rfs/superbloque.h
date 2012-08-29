/*
 * superbloque.h
 *
 *  Created on: 29/04/2012
 *      Author: utnso
 */

#ifndef SUPERBLOQUE_H_
#define SUPERBLOQUE_H_
#include <stdint.h>

struct ext2_super_block {
	uint32_t	s_inodes_count;		/* Inodes count */
	uint32_t	s_blocks_count;		/* Blocks count */
	uint32_t	s_r_blocks_count;	/* Reserved blocks count */
	uint32_t	s_free_blocks_count;	/* Free blocks count */
	int32_t	s_free_inodes_count;	/* Free inodes count */
	uint32_t	s_first_data_block;	/* First Data Block */
	uint32_t	s_log_block_size;	/* Block size */
	uint32_t	s_log_frag_size;	/* Fragment size */
	uint32_t	s_blocks_per_group;	/* # Blocks per group */
	uint32_t	s_frags_per_group;	/* # Fragments per group */
	uint32_t	s_inodes_per_group;	/* # Inodes per group */
	uint32_t	s_mtime;		/* Mount time */
	uint32_t	s_wtime;		/* Write time */
	uint16_t	s_mnt_count;		/* Mount count */
	uint16_t	s_max_mnt_count;	/* Maximal mount count */
	uint16_t	s_magic;		/* Magic signature */
	uint16_t	s_state;		/* File system state */
	uint16_t	s_errors;		/* Behaviour when detecting errors */
	uint16_t	s_minor_rev_level; 	/* minor revision level */
	uint32_t	s_lastcheck;		/* time of last check */
	uint32_t	s_checkinterval;	/* max. time between checks */
	uint32_t	s_creator_os;		/* OS */
	uint32_t	s_rev_level;		/* Revision level */
	uint16_t	s_def_resuid;		/* Default uid for reserved blocks */
	uint16_t	s_def_resgid;		/* Default gid for reserved blocks */

}typedef superbloque_t;

struct{
	uint32_t blockSize;
	uint32_t bloquesXGrupo;
	uint32_t inodosXGrupo;
	uint32_t cantidadDeGroupBlocks;
	uint32_t freeInodesCount;
}typedef sbInfo;

superbloque_t* leer_superbloque();

uint32_t cantidadDeGroupBlocks(superbloque_t*);
sbInfo get_sbInfo(superbloque_t*);


#endif /* SUPERBLOQUE_H_ */
