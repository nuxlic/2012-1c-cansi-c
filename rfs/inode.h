/*
 * inode.h
 *
 *  Created on: 06/05/2012
 *      Author: utnso
 */

#ifndef INODE_H_
#define INODE_H_

#include <stdint.h>
#include "commons/collections/list.h"


typedef struct ext2_inode {
	int16_t	i_mode;		/* File mode */
	int16_t	i_uid;		/* Low 16 bits of Owner Uid */
	int32_t	i_size;		/* Size in bytes */
	int32_t	i_atime;	/* Access time */
	int32_t	i_ctime;	/* Creation time */
	int32_t	i_mtime;	/* Modification time */
	int32_t	i_dtime;	/* Deletion Time */
	int16_t	i_gid;		/* Low 16 bits of Group Id */
	int16_t	i_links_count;	/* Links count */
	int32_t	i_blocks;	/* Blocks count */
	int32_t	i_flags;	/* File flags */
	union {
		struct {
			int32_t  l_i_reserved1;
		} linux1;
		struct {
			int32_t  h_i_translator;
		} hurd1;
		struct {
			int32_t  m_i_reserved1;
		} masix1;
	} osd1;				/* OS dependent 1 */
	uint32_t	i_blockD[12];
	uint32_t i_blockIDS;
	uint32_t i_blockIDD;
	uint32_t i_blockIDT;/* Pointers to blocks */
	int32_t	i_generation;	/* File version (for NFS) */
	int32_t	i_file_acl;	/* File ACL */
	int32_t	i_dir_acl;	/* Directory ACL */
	int32_t	i_faddr;	/* Fragment address */
	union {
		struct {
			uint8_t	l_i_frag;	/* Fragment number */
			uint8_t	l_i_fsize;	/* Fragment size */
			uint16_t	i_pad1;
			int16_t	l_i_uid_high;	/* these 2 fields    */
			int16_t	l_i_gid_high;	/* were reserved2[0] */
			uint32_t	l_i_reserved2;
		} linux2;
		struct {
			uint8_t	h_i_frag;	/* Fragment number */
			uint8_t	h_i_fsize;	/* Fragment size */
			int16_t	h_i_mode_high;
			int16_t	h_i_uid_high;
			int16_t	h_i_gid_high;
			int32_t	h_i_author;
		} hurd2;
		struct {
			uint8_t	m_i_frag;	/* Fragment number */
			uint8_t	m_i_fsize;	/* Fragment size */
			uint16_t	m_pad1;
			uint32_t	m_i_reserved2[2];
		} masix2;
	} osd2;				/* OS dependent 2 */
} __attribute__ ((packed)) inode_t;

#define EXT2_NAME_LEN 255
typedef struct ext2_dir_entry_2 {
	uint32_t	inode;			/* Inode number */
	uint16_t	rec_len;		/* Directory entry length */
	uint8_t	name_len;		/* Name length */
	uint8_t	file_type;
	char	name[];	/* File name */
}__attribute__ ((packed)) directorio_t;


enum {
	EXT2_FT_UNKNOWN		= 0,
	EXT2_FT_REG_FILE	= 1,
	EXT2_FT_DIR			= 2,
	EXT2_FT_CHRDEV		= 3,
	EXT2_FT_BLKDEV		= 4,
	EXT2_FT_FIFO		= 5,
	EXT2_FT_SOCK		= 6,
	EXT2_FT_SYMLINK		= 7,
	EXT2_FT_MAX
};

inode_t* leerTablaDeInodos(uint32_t);
inode_t* leerInodo(uint32_t);
uint32_t* leerDirectos(uint32_t* arrayDirectsBlocks, uint32_t bloquesAleer);
uint32_t* leerIndireccionamientoSimple(uint32_t indirectoSimple,uint32_t,uint32_t bloquesAleer);
uint32_t* leerIndireccionamientoDobleTriple(uint32_t indirecto,uint32_t level, uint32_t bloquesAleer);
bool leerIndireccion(uint32_t* address_to_copy, uint32_t indirect_address, uint32_t level, uint32_t cantidad);



#endif /* INODE_H_ */
