/*
 * ext2_operations.h
 *
 *  Created on: 26/05/2012
 *      Author: utnso
 */

#include <stdbool.h>
#include "commons/collections/list.h"
#include "inode.h"
#include "commons/serializadores.h"
#include "fs_handler.h"
#include "multiThreading.h"

#ifndef EXT2_OPERATIONS_H_
#define EXT2_OPERATIONS_H_

resultado_t* ext2_ReadFile(args_t* args);
t_list* ext2_ReadDir(char* path,structures_synchronizer_t*);
int32_t ext2_create_file(char* path,mode_t modo,structures_synchronizer_t*);
t_respuesta_get_attr ext2_getAttr(char* path,structures_synchronizer_t*);
resultado_t* ext2_writeFile(args_t*);
resultado_t* ext2_truncateFile(args_t*);
resultado_t* ext2_mkdir(args_t* params);
int32_t ext2_removeFile(args_t*);
int32_t ext2_removeDir(args_t*);

inode_t* ext2_get_Inode(char* path, uint32_t* nroInodo);

#endif /* EXT2_OPERATIONS_H_ */
