/*
 * ext2_operations.c
 *
 *  Created on: 26/05/2012
 *      Author: utnso
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/mman.h>

#include "superbloque.h"
#include "fs_handler.h"
#include "groupDescriptor.h"
#include "inode.h"
#include "bitmap.h"
#include "commons/bitarray.h"
#include "commons/collections/list.h"
#include "ext2_operations.h"
#include "ext2_def.h"
#include "commons/string.h"
#include "memcachear.h"

#define MAX_WRITINGS_PERSYNC 10

extern sbInfo SbI;
extern void* FSmap;
extern char* ipCache;
extern uint32_t portCache;
extern struct stat DatosDelArchivo;
extern uint32_t writings;
extern uint32_t Retardo;
//***********************EXT2 PRIVATE FUNCTIONS PROTOTYPES***********************//
static bool ext2_addEntryToDirectory(char* path, uint32_t inodeNumberOfNewFile,
		structures_synchronizer_t*, directorio_t*);
static bool ext2_deleteDirEntry(char* path, structures_synchronizer_t* locks);
//static char* getFatherPath(const char* path);
static char* ext2_get_fatherDirectoryAndNameOfNewFile(char* path, char** name);
inode_t* ext2_get_Inode(char* path, uint32_t* nroInodo);
static t_list* ext2_getblocksofdir(inode_t inodo);
static char* ext2_readBlocks(inode_t inode, uint32_t offset, uint32_t size);
static uint32_t* ext2_get_Blocks_of_an_offset(inode_t inode, uint32_t offset,
		uint32_t size);
static uint32_t* ext2_get_AllBlocks(inode_t* inode, uint32_t);
static int32_t ext2_addBlocks(inode_t* inode, int32_t cantidad,
		structures_synchronizer_t*);
static int32_t ext2_removeBlocks(inode_t* inode, int32_t cantidad, args_t*);
//***********************EXT2 FUNCTIONS******************************************//
resultado_t* ext2_truncateFile(args_t* args) {

	open_file_t* file = foundOrAddOpenFileInList(args->ped->path,
			args->lockeos->openFileList);
	inode_t* file_inode = ext2_get_Inode(args->ped->path, NULL);

	resultado_t* result = calloc(1, sizeof(resultado_t));
	result->path = strdup(args->ped->path);
	result->data = calloc(1, sizeof(uint32_t));

	if (file_inode == NULL) {
		open_file_finishHim(args->lockeos->openFileList, file);
		uint32_t err = 1;
		memcpy(result->data, &err, sizeof(uint32_t));
		return result;
	}

	uint32_t new_size = args->ped->offset;

	if (file_inode->i_mode & EXT2_IFREG) {

		pthread_rwlock_wrlock(&file->lock);

		int64_t diff = ceil((double) new_size / SbI.blockSize)
				- ceil((double) file_inode->i_size / SbI.blockSize);

		uint32_t not_attached = 0;

		if (diff > 0)
			not_attached = ext2_addBlocks(file_inode, diff, args->lockeos);
		else if (diff < 0)
			ext2_removeBlocks(file_inode, abs(diff), args);

		if (not_attached > 0) {
			file_inode->i_size += (diff - not_attached) * SbI.blockSize;
			file_inode->i_blocks = ceil(
					(double) file_inode->i_size / SbI.blockSize) / 512;
			pthread_rwlock_unlock(&file->lock);
		} else {

			file_inode->i_size = new_size;
			file_inode->i_blocks = ceil((double) new_size / SbI.blockSize)
					/ 512;
			pthread_rwlock_unlock(&file->lock);
		}
	} else {
		uint32_t err = 1;
		memcpy(result->data, &err, sizeof(uint32_t));
	}

	open_file_finishHim(args->lockeos->openFileList, file);
	return result;

}
resultado_t* ext2_writeFile(args_t* args) {

	open_file_t* file = foundOrAddOpenFileInList(args->ped->path,
			args->lockeos->openFileList);

	inode_t* file_inode = ext2_get_Inode(args->ped->path, NULL);

	resultado_t* result = calloc(1, sizeof(resultado_t));
	result->path = strdup(args->ped->path);

	uint32_t start_block = args->ped->offset / SbI.blockSize;
	uint32_t start_byte_in_start_block = args->ped->offset % SbI.blockSize;

	int64_t bytes_to_write = args->ped->bytes;

	if (file_inode == NULL) {
		free(args->ped->buffer);
		open_file_finishHim(args->lockeos->openFileList, file);
		return result;
	}

	if (file_inode->i_mode & EXT2_IFREG) {

		pthread_rwlock_wrlock(&file->lock);

		int64_t diff = args->ped->offset + bytes_to_write - file_inode->i_size;

		if ((args->ped->offset > file_inode->i_size)
				&& (file_inode->i_size % SbI.blockSize > 0)) {
			uint32_t* file_data = ext2_get_AllBlocks(file_inode,
					file_inode->i_size);
			uint32_t cantOfBlocks = ceil(
					(double) file_inode->i_size / SbI.blockSize);
			char* last_block = FSmap
					+ SbI.blockSize * file_data[cantOfBlocks - 1];

			uint32_t bytes_to_set = SbI.blockSize
					- file_inode->i_size % SbI.blockSize;

			memset(&last_block[file_inode->i_size % SbI.blockSize], 0,
					bytes_to_set);

			free(file_data);
		}

		if (ceil((double) (args->ped->offset + bytes_to_write) / SbI.blockSize)
				> ceil((double) file_inode->i_size / SbI.blockSize)) {
			ext2_addBlocks(file_inode, ceil((double) diff / SbI.blockSize),
					args->lockeos);
		}

		uint32_t* file_data = ext2_get_AllBlocks(file_inode,
				file_inode->i_size + diff);

		uint32_t count = ceil(
				(double) (args->ped->offset + bytes_to_write) / SbI.blockSize);

		if (start_block > count) {
			result->data = calloc(1, sizeof(uint32_t));
			free(args->ped->buffer);
			open_file_finishHim(args->lockeos->openFileList, file);
			return result;
		}

//		if (file_data[count - 1] == 0) {
//			result->data = calloc(1, sizeof(uint32_t));
//			free(args->ped->buffer);
//			open_file_finishHim(args->lockeos->openFileList, file);
//			return result;
//		}

		char* first_block = FSmap + file_data[start_block] * SbI.blockSize;

		uint32_t remaining_space = SbI.blockSize - start_byte_in_start_block;

		uint32_t bytes_to_copy =
				(remaining_space > bytes_to_write) ?
						bytes_to_write : remaining_space;

		memcpy(&first_block[start_byte_in_start_block], args->ped->buffer,
				bytes_to_copy);

		//log_info(logger,"%s %s %s %d","Archivo: ",params->ped->path, "Bloque escrito: ",file_data[start_block]);

		memc_set(args, start_block, file_data);

		bytes_to_write -= bytes_to_copy;

		uint32_t bytes_written = bytes_to_copy;

		if (bytes_to_write > 0) {

			for (uint32_t block_number = start_block + 1; bytes_to_write > 0;
					block_number++) {

				if (block_number > count) {
					result->data = calloc(1, sizeof(uint32_t));
					memcpy(result->data, &bytes_written, sizeof(uint32_t));
					free(args->ped->buffer);
					open_file_finishHim(args->lockeos->openFileList, file);
					return result;
				}

				bytes_to_copy =
						bytes_to_write > SbI.blockSize ?
								SbI.blockSize : bytes_to_write;

				void* block_to_overwrite = FSmap
						+ file_data[block_number] * SbI.blockSize;
				char* newBuffer = args->ped->buffer;
				memcpy(block_to_overwrite, &(newBuffer[bytes_written]),
						bytes_to_copy);

				//log_info(logger,"%s %s %s %d","Archivo: ",params->ped->path, "Bloque escrito: ",file_data[block_number]);

				memc_set(args, block_number, file_data);

				bytes_to_write -= bytes_to_copy;

				bytes_written += bytes_to_copy;

			}
		}

		result->data = calloc(1, sizeof(uint32_t));
		memcpy(result->data, &bytes_written, sizeof(uint32_t));

		if (args->ped->offset + bytes_written > file_inode->i_size) {
			uint32_t diff = args->ped->offset + bytes_written
					- file_inode->i_size;
			file_inode->i_size += diff;
			file_inode->i_blocks = ceil(
					(double) file_inode->i_size / SbI.blockSize)
					* (SbI.blockSize / 512);
		}
		pthread_mutex_lock(&args->lockeos->writesSyncLock);
		if (writings >= 9999999999999999) {
			msync(FSmap, DatosDelArchivo.st_size, MS_ASYNC);
			writings = 0;
		}
		writings++;
		pthread_mutex_unlock(&args->lockeos->writesSyncLock);
		if (Retardo != 0)
			usleep(Retardo * 1000);

		free(file_data);
	}

	pthread_rwlock_unlock(&file->lock);
	free(args->ped->buffer);
	open_file_finishHim(args->lockeos->openFileList, file);

	return result;

}

t_respuesta_get_attr ext2_getAttr(char* path, structures_synchronizer_t* locks) {
	uint32_t nroInodo;
	t_respuesta_get_attr attrToReturn;
	if (path == NULL) {
		attrToReturn.type = NO_EXISTE;
		return attrToReturn;
	}
	if (strlen(path) > 400) {
		attrToReturn.type = NAME_TOO_LONG;
		return attrToReturn;
	}
	open_file_t* file = foundOrAddOpenFileInList(path, locks->openFileList);
	pthread_rwlock_rdlock(&file->lock);
	inode_t* inode = ext2_get_Inode(path, &nroInodo);
	pthread_rwlock_unlock(&file->lock);
	if (inode == NULL) {
		attrToReturn.type = NO_EXISTE;
		return attrToReturn;
	} else {
		attrToReturn.type = GET_ATTR;
	}
	attrToReturn.nroInodo = nroInodo;
	attrToReturn.mode = inode->i_mode;
	attrToReturn.hdlinks = inode->i_links_count;
	attrToReturn.size = inode->i_size;
	attrToReturn.blocksize = SbI.blockSize;
	attrToReturn.blocks = inode->i_blocks;
	open_file_finishHim(locks->openFileList, file);
	return attrToReturn;
}

resultado_t* ext2_ReadFile(args_t* params) {

	open_file_t* file = foundOrAddOpenFileInList(params->ped->path,
			params->lockeos->openFileList);

	inode_t* file_inode = ext2_get_Inode(params->ped->path, NULL);

	resultado_t* result = calloc(1, sizeof(resultado_t));
	result->path = strdup(params->ped->path);

	if (file_inode == NULL) {
		open_file_finishHim(params->lockeos->openFileList, file);
		return result;
	}

	uint32_t offset = params->ped->offset;
	int64_t bytes_to_read = params->ped->bytes;

	uint32_t start_block = offset / SbI.blockSize;
	uint32_t start_byte_in_start_block = offset % SbI.blockSize;

	uint8_t* read_data = calloc(bytes_to_read, sizeof(uint8_t));

	if (file_inode->i_mode & EXT2_IFREG) {

		pthread_rwlock_rdlock(&file->lock);

		uint32_t* file_data = ext2_get_AllBlocks(file_inode,
				file_inode->i_size);

		if (file_inode->i_size <= offset + bytes_to_read)
			bytes_to_read = file_inode->i_size - offset;

		result->bytes = bytes_to_read;

		if (file_data == NULL) {
			char* a = "";
			result->data = calloc(1, 1);
			memcpy(result->data, a, strlen(a) + 1);
			pthread_rwlock_unlock(&file->lock);

			open_file_finishHim(params->lockeos->openFileList, file);
			return result;

		}

		uint32_t read_so_far = 0;
		uint32_t remaining_bytes_in_start_block = SbI.blockSize
				- start_byte_in_start_block;

		uint32_t bytes_to_copy =
				bytes_to_read > remaining_bytes_in_start_block ?
						remaining_bytes_in_start_block : bytes_to_read;

		memc_data_t* cache_fs_data = memc_get(params, start_block, file_data);

		//log_info(logger,"%s %d","Bloque leido del archivo: ",start_block);

		memcpy(&read_data[read_so_far],
				(cache_fs_data->data + start_byte_in_start_block),
				bytes_to_copy);
		bytes_to_read -= bytes_to_copy;
		read_so_far += bytes_to_copy;

		if (cache_fs_data->from_cache)
			free(cache_fs_data->data);
		free(cache_fs_data);

		if (bytes_to_read > remaining_bytes_in_start_block) {

			for (uint32_t block_number = start_block + 1; bytes_to_read > 0;
					++block_number) {
				bytes_to_copy =
						bytes_to_read > SbI.blockSize ?
								SbI.blockSize : bytes_to_read;

				memc_data_t* cache_fs_data = memc_get(params, block_number,
						file_data);

				//log_info(logger,"%s %d","Bloque leido del archivo: ",block_number);

				memcpy(&read_data[read_so_far], cache_fs_data->data,
						bytes_to_copy);

				if (cache_fs_data->from_cache)
					free(cache_fs_data->data);

				free(cache_fs_data);

				read_so_far += bytes_to_copy;
				bytes_to_read -= bytes_to_copy;

			}
		}

		result->data = read_data;
		free(file_data);
		pthread_rwlock_unlock(&file->lock);
	}
	open_file_finishHim(params->lockeos->openFileList, file);
	return result;
}

t_list* ext2_ReadDir(char* path, structures_synchronizer_t* locks) {
	open_file_t* file = foundOrAddOpenFileInList(path, locks->openFileList);
	pthread_rwlock_rdlock(&file->lock);
	inode_t* inodo = ext2_get_Inode(path, NULL);
	if (inodo == NULL) {
		pthread_rwlock_unlock(&file->lock);
		open_file_finishHim(locks->openFileList, file);
		return NULL;
	}
	t_list* toReturn = ext2_getblocksofdir(*inodo);
	pthread_rwlock_unlock(&file->lock);
	open_file_finishHim(locks->openFileList, file);
	return toReturn;
}

int32_t ext2_create_file(char* path, mode_t mode,
		structures_synchronizer_t* locks) {
	int32_t inodoNuevo = buscarInodosLibres(locks);
	if (inodoNuevo == -1) {
		printf("no hay mas inodos disp\n");
		return -10;
	}
	char* name;
	char* father = ext2_get_fatherDirectoryAndNameOfNewFile(path, &name);
	//free(father);
	directorio_t* newEntry = malloc(sizeof(directorio_t) + strlen(name) + 1);
	newEntry->inode = inodoNuevo;
	newEntry->file_type = 1;
	strncpy(newEntry->name, name, strlen(name) + 1);
	newEntry->name_len = strlen(name);
	uint32_t newEntryrec_len = sizeof(directorio_t) + newEntry->name_len;
	if (newEntryrec_len % 4 != 0) {
		newEntry->rec_len = newEntryrec_len + 4 - newEntryrec_len % 4;
	} else {
		newEntry->rec_len = newEntryrec_len;
	}
	if (ext2_addEntryToDirectory(path, inodoNuevo, locks, newEntry) == false) {
		printf("no se pudo crear la entrada a directorio\n");
		return -1;
	}
	asignarInodo(inodoNuevo, locks);
	open_file_t* file = foundOrAddOpenFileInList(path, locks->openFileList);
	pthread_rwlock_wrlock(&file->lock);
	inode_t* inodo = leerInodo(inodoNuevo);
	memset(inodo, 0, 128);
	inodo->i_mode = mode;
	inodo->i_blocks = 0;
	//inodo->i_dtime=1;
	inodo->i_links_count = 1;
	free(newEntry);
	free(name);
	pthread_rwlock_unlock(&file->lock);
	open_file_finishHim(locks->openFileList, file);
	msync(FSmap, DatosDelArchivo.st_size, MS_SYNC);
	return 0;
}

resultado_t* ext2_mkdir(args_t* args) {
	int32_t inodoNuevo = buscarInodosLibres(args->lockeos);
	if (inodoNuevo == -1) {
		printf("no hay mas inodos disp\n");
		return NULL;
	}
	char* name;
	char* father = ext2_get_fatherDirectoryAndNameOfNewFile(args->ped->path,
			&name);
	directorio_t* newEntry = calloc(1, sizeof(directorio_t) + strlen(name));
	newEntry->inode = inodoNuevo;
	newEntry->file_type = 2;
	strncpy(newEntry->name, name, strlen(name));
	newEntry->name_len = strlen(name);
	uint32_t newEntryrec_len = sizeof(directorio_t) + newEntry->name_len;
	if (newEntryrec_len % 4 != 0) {
		newEntry->rec_len = newEntryrec_len + 4 - newEntryrec_len % 4;
	} else {
		newEntry->rec_len = newEntryrec_len;
	}
	if (ext2_addEntryToDirectory(args->ped->path, inodoNuevo, args->lockeos,
			newEntry) == false) {
		printf("no se pudo crear la entrada a directorio\n");
		return NULL;
	}
	asignarInodo(inodoNuevo, args->lockeos);
	open_file_t* file = foundOrAddOpenFileInList(args->ped->path,
			args->lockeos->openFileList);
	uint32_t blockGroup = (inodoNuevo - 1) / SbI.inodosXGrupo;
	pthread_mutex_lock(&args->lockeos->writeDirUsedCountLock[blockGroup]);
	gDescrip_t* gdt = leerTablaDeDescriptoresDeGrupos();
	gdt[blockGroup].bg_used_dirs_count++;
	pthread_mutex_unlock(&args->lockeos->writeDirUsedCountLock[blockGroup]);
	pthread_rwlock_wrlock(&file->lock);
	inode_t* inodo = leerInodo(inodoNuevo);
	memset(inodo, 0, 128);
	inodo->i_mode = args->ped->mode | EXT2_IFDIR;
	//inodo->i_dtime=1;
	inodo->i_size = SbI.blockSize;
	inodo->i_blocks = (SbI.blockSize / 512);
	uint32_t* libre = (buscarBloquesLibres(1, args->lockeos));
	inodo->i_blockD[0] = *libre;
	free(libre);
	memset(FSmap + inodo->i_blockD[0] * SbI.blockSize, 0, SbI.blockSize);
	asignarBloque(inodo->i_blockD[0], args->lockeos);
	inodo->i_links_count = 1;
	void* directorio = FSmap + inodo->i_blockD[0] * SbI.blockSize;
	directorio_t* entradaPunto = calloc(1, sizeof(directorio_t) + strlen("."));

	entradaPunto->inode = newEntry->inode;
	entradaPunto->file_type = newEntry->file_type;
	entradaPunto->name_len = 1;
	char* punto = ".";
	strncpy(entradaPunto->name, punto, 1);
	//entradaPunto->name=".";
	uint32_t entradaPuntorec_len = sizeof(directorio_t)
			+ entradaPunto->name_len;
	if (entradaPuntorec_len % 4 != 0) {
		entradaPunto->rec_len = entradaPuntorec_len + 4
				- entradaPuntorec_len % 4;
	} else {
		entradaPunto->rec_len = entradaPuntorec_len;
	}
	memcpy(directorio, entradaPunto, sizeof(directorio_t) + strlen("."));
	//free(newEntry);
	uint32_t aux_len = entradaPunto->rec_len;
	directorio += entradaPunto->rec_len;
	directorio_t* entradaPuntoPunto = calloc(1,
			sizeof(directorio_t) + strlen(".."));
	entradaPuntoPunto->file_type = 2;
	char* puntoPunto = "..";
	strncpy(entradaPuntoPunto->name, puntoPunto, 2);
	//entradaPuntoPunto->name="..";
	uint32_t nroInode;
	ext2_get_Inode(father, &nroInode);
	entradaPuntoPunto->inode = nroInode;
	entradaPuntoPunto->name_len = 2;
	entradaPuntoPunto->rec_len = SbI.blockSize - aux_len;
	memcpy(directorio, entradaPuntoPunto,
			sizeof(directorio_t) + entradaPuntoPunto->name_len);
	//free(entradaPuntoPunto);
	pthread_rwlock_unlock(&file->lock);
	open_file_finishHim(args->lockeos->openFileList, file);
	msync(FSmap, DatosDelArchivo.st_size, MS_SYNC);
	resultado_t* res = calloc(1, sizeof(resultado_t));
	res->data = calloc(1, sizeof(uint32_t));
	uint32_t ok = 0;
	memcpy(res->data, &ok, sizeof(uint32_t));
	res->path = strdup(args->ped->path);
	free(name);
	free(entradaPunto);
	free(entradaPuntoPunto);
	free(newEntry);
	return res;
}

int32_t ext2_removeFile(args_t* args) {

	uint32_t nroInodo;
	open_file_t* file = foundOrAddOpenFileInList(args->ped->path,
			args->lockeos->openFileList);
	pthread_rwlock_wrlock(&file->lock);
	inode_t* inodo = ext2_get_Inode(args->ped->path, &nroInodo);
	if (S_ISDIR(inodo->i_mode)) {
		return -1;
	}
	if (!ext2_deleteDirEntry(args->ped->path, args->lockeos)) {
		printf("Error: no se pudo borrar la entrada de directorio\n");
		return -1;
	}

	desasignarInodo(nroInodo, args->lockeos);
	if (inodo->i_size != 0) {
		uint32_t cantidad = ceil((double) inodo->i_size / SbI.blockSize);

		uint32_t* blocks = ext2_get_AllBlocks(inodo, inodo->i_size);
		if (blocks != NULL) {
			for (int i = 0; i < cantidad; ++i) {
				desasignarBloque(blocks[i], args);
			}
		}
		free(blocks);
	}
	memset(inodo, 0, sizeof(inode_t));
	pthread_rwlock_unlock(&file->lock);
	open_file_finishHim(args->lockeos->openFileList, file);

	return 0;
}

int32_t ext2_removeDir(args_t* args) {
	open_file_t* file = foundOrAddOpenFileInList(args->ped->path,
			args->lockeos->openFileList);
	pthread_rwlock_wrlock(&file->lock);
	t_list* readDir = ext2_ReadDir(args->ped->path, args->lockeos);
	if (readDir->elements_count > 2) {
		return -1;
	}
	list_destroy_and_destroy_elements(readDir, (void*) free);
	uint32_t nroInodo;
	inode_t* inodo = ext2_get_Inode(args->ped->path, &nroInodo);

	if (inodo == NULL) {
		return -2;
	}

	if (!ext2_deleteDirEntry(args->ped->path, args->lockeos)) {
		printf("Error: no se pudo borrar la entrada de directorio\n");
		return -1;
	}

	desasignarInodo(nroInodo, args->lockeos);
	if (inodo->i_size != 0) {
		uint32_t cantidad = ceil((double) inodo->i_size / SbI.blockSize);

		uint32_t* blocks = ext2_get_AllBlocks(inodo, inodo->i_size);
		if (blocks != NULL) {
			for (int i = 0; i < cantidad; ++i) {
				desasignarBloque(blocks[i], args);
			}
		}
		free(blocks);
	}
	memset(inodo, 0, sizeof(inode_t));
//pthread_rwlock_unlock(&file->lock);
	uint32_t blockGroup = (nroInodo - 1) / SbI.inodosXGrupo;
	pthread_mutex_lock(&args->lockeos->writeDirUsedCountLock[blockGroup]);
	gDescrip_t* gdt = leerTablaDeDescriptoresDeGrupos();
	gdt[blockGroup].bg_used_dirs_count--;
	pthread_mutex_unlock(&args->lockeos->writeDirUsedCountLock[blockGroup]);
	open_file_finishHim(args->lockeos->openFileList, file);

	return 0;
}

//****************************PRIVATE FUNCTIONS****************************************************//

static bool ext2_deleteDirEntry(char* path, structures_synchronizer_t* locks) {
	char* name;
	char* father = ext2_get_fatherDirectoryAndNameOfNewFile(path, &name);
	open_file_t* fatherLock = foundOrAddOpenFileInList(father,
			locks->openFileList);
	pthread_rwlock_wrlock(&fatherLock->lock);
	inode_t* fatherInode = ext2_get_Inode(father, NULL);
	bool deleted = false;
	uint32_t cantidad = ceil((double) fatherInode->i_size / SbI.blockSize);
	uint32_t i;
	uint32_t* blocks = ext2_get_AllBlocks(fatherInode, fatherInode->i_size);
	for (i = 0; i < cantidad; ++i) {
		void* offset = FSmap + blocks[i] * SbI.blockSize;
		void* aux = offset;
		while ((aux < offset + SbI.blockSize) && !deleted) {
			directorio_t* ant = (directorio_t*) aux;
			directorio_t* actual = (directorio_t*) (aux + ant->rec_len);
			//directorio_t* sig=(directorio_t*)(((void*)actual)+actual->rec_len);
			if (strncmp(name, actual->name, actual->name_len) == 0) {
				ant->rec_len += actual->rec_len;
				deleted = true;
				break;
			}
			aux += ant->rec_len;
		}
		if (deleted) {
			break;
		}
	}
	free(blocks);
	free(name);
//	free(father);
	pthread_rwlock_unlock(&fatherLock->lock);
	open_file_finishHim(locks->openFileList, fatherLock);
	return deleted;
}

static int32_t ext2_removeInLevelOne(uint32_t* indirecto, uint32_t level,
		int32_t* cantidad, args_t* args) {
	if (*indirecto == 0) {
		//perror("Ya no estaba en uso cuando se quiso remover");
		return -1;
	}
	uint32_t* pointer = (uint32_t*) (FSmap + (*indirecto) * SbI.blockSize);
	if (*cantidad >= (SbI.blockSize / sizeof(uint32_t))) {
		desasignarBloque(*indirecto, args);
		*indirecto = 0;
	}
	int32_t i;
	//int32_t contador=0;
	for (i = (SbI.blockSize / sizeof(uint32_t)) - 1; i >= 0 && (*cantidad) > 0;
			i--) {
		if (pointer[i] != 0) {
			(*cantidad)--;
			//contador++;
			desasignarBloque(pointer[i], args);
			pointer[i] = 0;
		}

	}
//	if(contador==(SbI.blockSize/sizeof(uint32_t))){
//		desasignarBloque(*indirecto,locks);
//	}
	return *cantidad;
}

static int32_t ext2_removeInLevelDoubleOrTriple(uint32_t* indirecto,
		uint32_t level, int32_t* cantidad, args_t* args) {
	if (*indirecto == 0) {
		//perror("Ya no estaba en uso cuando se quiso remover");
		return -1;
	}
	int32_t (*ext2_removeInThisLevel)(uint32_t*, uint32_t, int32_t*,
			args_t*)=ext2_removeInLevelDoubleOrTriple;

	if ((level - 1) == 1) {
		ext2_removeInThisLevel = ext2_removeInLevelOne;
	}
	uint32_t* pointer = (uint32_t*) (FSmap + (*indirecto) * SbI.blockSize);
	if (*cantidad >= pow((SbI.blockSize / sizeof(uint32_t)), level)) {
		desasignarBloque(*indirecto, args);
		*indirecto = 0;
	}

	int32_t i = 0;
	//uint32_t cantAux=*cantidad;
	for (i = (SbI.blockSize / sizeof(uint32_t)) - 1; i >= 0 && (*cantidad) > 0;
			i--) {
		ext2_removeInThisLevel(&pointer[i], level - 1, cantidad, args);
//		if(cantAux!=*cantidad && level!=3){
//			desasignarBloque(*indirecto,locks);
//			cantAux=*cantidad;
//		}
	}
	return *cantidad;
}

static int32_t ext2_removeBlocks(inode_t* inode, int32_t cantidad, args_t* args) {
	memc_t* mc = memc_connect(ipCache, portCache);
	uint32_t cantAux = cantidad;
	if (ext2_removeInLevelDoubleOrTriple(&inode->i_blockIDT, 3, &cantidad, args)
			== 0) {
	} else if (ext2_removeInLevelDoubleOrTriple(&inode->i_blockIDD, 2,
			&cantidad, args) == 0) {
	} else if (ext2_removeInLevelOne(&inode->i_blockIDS, 1, &cantidad, args)
			== 0) {
	} else {
		uint32_t i;
		for (i = 11; i >= 0 && cantidad > 0; i--) {
			if (inode->i_blockD[i] != 0) {
				cantidad--;
				desasignarBloque(inode->i_blockD[i], args);
				inode->i_blockD[i] = 0;

			}
		}
	}
	if (inode->i_size - (cantAux * SbI.blockSize)
			<= (12 + (SbI.blockSize / sizeof(uint32_t))
					+ pow((SbI.blockSize / sizeof(uint32_t)), 2))
					* SbI.blockSize) {
		if (inode->i_blockIDT == 0) {
			memc_free(&mc);
			return cantidad;
		}
		desasignarBloque(inode->i_blockIDT, args);
		inode->i_blockIDT = 0;
	}
	memc_free(&mc);
	return cantidad;
}

static int32_t ext2_addInLevelOne(uint32_t* indirecto, int32_t* cantidad,
		structures_synchronizer_t* locks) {
	uint32_t j = 0;
	if (*indirecto == 0) {
		*indirecto =*(buscarBloquesLibres(1, locks));
		asignarBloque(*indirecto, locks);
		memset(FSmap + (*indirecto) * SbI.blockSize, 0, SbI.blockSize);
	}
	uint32_t* arrayOfAvaibles = buscarBloquesLibres(*cantidad, locks);
	uint32_t* pointer = (uint32_t*) (FSmap + (*indirecto) * SbI.blockSize);
	uint32_t i;
	for (i = 0; (i < (SbI.blockSize / sizeof(uint32_t))) && ((*cantidad) > 0);
			i++) {
		if (pointer[i] == 0 && (*cantidad) != 0) {
			(*cantidad)--;
			pointer[i] = arrayOfAvaibles[j];
			asignarBloque(pointer[i], locks);
			j++;
			memset(FSmap + pointer[i] * SbI.blockSize, 0, SbI.blockSize);
		}
	}
	free(arrayOfAvaibles);
//actualizarFS(*indirecto,(char*)pointer);
	return *cantidad;
}

static int32_t ext2_addInLevelDouble(uint32_t* indirecto, int32_t* cantidad,
		structures_synchronizer_t* locks) {
	if (*indirecto == 0) {
		*indirecto = *(buscarBloquesLibres(1, locks));
		asignarBloque(*indirecto, locks);
		memset(FSmap + (*indirecto) * SbI.blockSize, 0, SbI.blockSize);

	}

	uint32_t* pointerDouble = (uint32_t*) (FSmap + (*indirecto) * SbI.blockSize);
	uint32_t i;
	for (i = 0; i < (SbI.blockSize / sizeof(uint32_t)); i++) {
		if (pointerDouble[i] == 0) {
			if (i != 0) {

				i--;
			}
			break;
		}
	}

	if (i == (SbI.blockSize / sizeof(uint32_t))) {
		//perror("Estaba lleno, llegaste tarde papa!");
		return -1;
	}

	for (; (*cantidad) > 0 && i < (SbI.blockSize / sizeof(uint32_t)); i++) {

		ext2_addInLevelOne(&pointerDouble[i], cantidad, locks);
	}
	return *cantidad;

}

static int32_t ext2_addInLevelTriple(uint32_t* indirecto, int32_t* cantidad,
		structures_synchronizer_t* locks) {

	if (*indirecto == 0) {
		*indirecto = *(buscarBloquesLibres(1, locks));
		asignarBloque(*indirecto, locks);
		memset(FSmap + (*indirecto) * SbI.blockSize, 0, SbI.blockSize);

	}
	uint32_t* pointerTriple = (uint32_t*) (FSmap + (*indirecto) * SbI.blockSize);
	uint32_t i;
	for (i = 0;
			i < (SbI.blockSize / sizeof(uint32_t)) && pointerTriple[i + 1] != 0;
			i++)
		;
//	if (i == (SbI.blockSize / sizeof(uint32_t))) {
//		perror("Estaba lleno, llegaste tarde papa!");
//		return -1;
//	}
	int32_t cantidadRestante = ext2_addInLevelDouble(&pointerTriple[i],
			cantidad, locks);

	while (cantidadRestante == -1 || cantidadRestante > 0) {
		i++;
		if (cantidadRestante == 0) {
			break;
		}
		uint32_t* indirectoTriple = malloc(sizeof(uint32_t));

		*indirectoTriple = *(buscarBloquesLibres(1, locks));
		asignarBloque(*indirectoTriple, locks);
		pointerTriple[i] = *indirectoTriple;
		memset(FSmap + (*indirectoTriple) * SbI.blockSize, 0, SbI.blockSize);
		cantidadRestante = ext2_addInLevelDouble(&pointerTriple[i + 1],
				cantidad, locks);
		free(indirectoTriple);
	}
	return 0;
}

static uint32_t level_indirection(inode_t* inode) {
	uint32_t indS = SbI.blockSize / sizeof(uint32_t);
	uint32_t indD = pow(indS, 2);
	uint32_t indT = pow(indS, 3);
	uint32_t level = 1 + (inode->i_size > indS * SbI.blockSize)
			+ (inode->i_size > indD * SbI.blockSize)
			+ (inode->i_size > indT * SbI.blockSize);
	return level - 1;
}

static int32_t ext2_addBlocks(inode_t* inode, int32_t cantidad,
		structures_synchronizer_t* locks) {
	uint32_t level = level_indirection(inode);
	switch (level) {
	case 0: {
		uint32_t* arrayOfAvaibles = buscarBloquesLibres(12, locks);
		uint32_t i;
		uint32_t j = 0;
		for (i = 0; i < 12 && cantidad != 0; i++) {
			if (inode->i_blockD[i] == 0) {
				cantidad--;
				inode->i_blockD[i] = arrayOfAvaibles[j];
				asignarBloque(inode->i_blockD[i], locks);
				j++;
				memset(FSmap + inode->i_blockD[i] * SbI.blockSize, 0,
						SbI.blockSize);
			}
		}
		free(arrayOfAvaibles);
		if (cantidad == 0) {
			return 0;
		} else {
			if (ext2_addInLevelOne(&inode->i_blockIDS, &cantidad, locks) == 0) {
				return 0;
			} else if (ext2_addInLevelDouble(&inode->i_blockIDD, &cantidad,
					locks) == 0) {
				return 0;
			} else if (ext2_addInLevelTriple(&inode->i_blockIDT, &cantidad,
					locks) == 0) {
				return 0;
			}
		}
		break;
	}
	case 1: {
		if (ext2_addInLevelOne(&inode->i_blockIDS, &cantidad, locks) == 0) {

			return 0;
		} else if (ext2_addInLevelDouble(&inode->i_blockIDD, &cantidad, locks)
				== 0) {

			return 0;
		} else if (ext2_addInLevelTriple(&inode->i_blockIDT, &cantidad, locks)
				== 0) {

			return 0;
		}
		break;
	}
	case 2: {
		if (ext2_addInLevelDouble(&inode->i_blockIDD, &cantidad, locks) == 0) {
			return 0;
		} else if (ext2_addInLevelTriple(&inode->i_blockIDT, &cantidad, locks)
				== 0) {
			return 0;
		}
		break;
	}
	case 3: {
		if (ext2_addInLevelTriple(&inode->i_blockIDT, &cantidad, locks) == 0) {

			return 0;
		}
		break;
	}
	}
	return cantidad;
}

//static uint32_t* ext2_get_AllBlocks(inode_t inode) {
//	uint32_t maxBlockPerIndirectBlock = (SbI.blockSize / sizeof(uint32_t));
//	uint32_t cantidad = ceil((double) inode.i_size / SbI.blockSize);
//	if (cantidad == 0) {
//		return NULL;
//	}
//	uint32_t* arrayOfNumbersOfBlock = calloc(cantidad, sizeof(uint32_t));
//	uint32_t i;
//	for (i = 0; inode.i_blockD[i] != 0 && i < 12 && i < cantidad; ++i) {
//		arrayOfNumbersOfBlock[i] = inode.i_blockD[i];
//	}
//	if (i < 12 && inode.i_blockIDD == 0)
//		return arrayOfNumbersOfBlock;
//	if (inode.i_blockIDT != 0) {
//		uint32_t* dirS = leerIndireccionamientoSimple(inode.i_blockIDS, 1,
//				maxBlockPerIndirectBlock);
//		uint32_t* dirD = leerIndireccionamientoDobleTriple(inode.i_blockIDD, 2,
//				pow(maxBlockPerIndirectBlock, 2));
//		uint32_t bloquesRestantes = ceil((double) inode.i_size / SbI.blockSize)
//				- 12 - maxBlockPerIndirectBlock
//				- pow(maxBlockPerIndirectBlock, 2);
//		uint32_t* dirT = leerIndireccionamientoDobleTriple(inode.i_blockIDT, 3,
//				bloquesRestantes);
//		memcpy(&arrayOfNumbersOfBlock[12], dirS,
//				maxBlockPerIndirectBlock * sizeof(uint32_t));
//		memcpy(&arrayOfNumbersOfBlock[12 + maxBlockPerIndirectBlock], dirD,
//				pow(maxBlockPerIndirectBlock, 2) * sizeof(uint32_t));
//
//		uint32_t j = 12 + maxBlockPerIndirectBlock
//				+ pow(maxBlockPerIndirectBlock, 2);
//		memcpy(&arrayOfNumbersOfBlock[j], dirT,
//				bloquesRestantes * sizeof(uint32_t));
//		free(dirS);
//		free(dirD);
//		free(dirT);
//		return arrayOfNumbersOfBlock;
//	} else if (inode.i_blockIDT == 0 && inode.i_blockIDD != 0) {
//		uint32_t bloquesRestantes = ceil((double) inode.i_size / SbI.blockSize)
//				- 12 - maxBlockPerIndirectBlock;
//		uint32_t* dirS = leerIndireccionamientoSimple(inode.i_blockIDS, 1,
//				maxBlockPerIndirectBlock);
//		uint32_t* dirD = leerIndireccionamientoDobleTriple(inode.i_blockIDD, 2,
//				pow(maxBlockPerIndirectBlock, 2));
//		memcpy(&arrayOfNumbersOfBlock[12], dirS,
//				maxBlockPerIndirectBlock * sizeof(uint32_t));
//		memcpy(&arrayOfNumbersOfBlock[12 + maxBlockPerIndirectBlock], dirD,
//				bloquesRestantes * sizeof(uint32_t));
//		free(dirS);
//		free(dirD);
//		return arrayOfNumbersOfBlock;
//	} else if (inode.i_blockIDD == 0 && inode.i_blockIDS != 0) {
//		uint32_t bloquesRestantes = ceil((double) inode.i_size / SbI.blockSize)
//				- 12;
//		uint32_t* dirS = leerIndireccionamientoSimple(inode.i_blockIDS, 1,
//				maxBlockPerIndirectBlock);
//		memcpy(&arrayOfNumbersOfBlock[12], dirS,
//				bloquesRestantes * sizeof(uint32_t));
//		free(dirS);
//		return arrayOfNumbersOfBlock;
//	}
//	return arrayOfNumbersOfBlock;
//}

static uint32_t* ext2_get_AllBlocks(inode_t* anInode, uint32_t file_size) {

	uint32_t cantidadDeBloques =
			ceil((double) file_size / SbI.blockSize) > 1 ?
					ceil((double) (file_size) / SbI.blockSize) : 1;
	uint32_t cantidadPunterosABloques = SbI.blockSize / sizeof(uint32_t);
	uint32_t bloquesDirectos = 12;
	uint32_t bloquesIndirectos = 0;
	uint32_t level;
	uint32_t* punteroDirecto = anInode->i_blockD;
	uint32_t* listaDeBloques = calloc(cantidadDeBloques + 1, sizeof(uint32_t));
	int ubicacion = 0;

	if (cantidadDeBloques > 12) {
		bloquesDirectos = 12;
		cantidadDeBloques -= bloquesDirectos;
		memcpy(&listaDeBloques[0], punteroDirecto,
				sizeof(uint32_t) * bloquesDirectos);
		ubicacion += bloquesDirectos;
	} else {
		memcpy(&listaDeBloques[0], punteroDirecto,
				sizeof(uint32_t) * cantidadDeBloques);
		return listaDeBloques;
	}

	if (cantidadDeBloques > cantidadPunterosABloques) {
		bloquesIndirectos = cantidadPunterosABloques;
		cantidadDeBloques -= bloquesIndirectos;
		level = 1;

		if (leerIndireccion(&listaDeBloques[ubicacion], anInode->i_blockIDS,
				level, bloquesIndirectos))
			ubicacion += bloquesIndirectos;
		else
			return NULL;

	} else {
		bloquesIndirectos = cantidadDeBloques;
		level = 1;
		if (leerIndireccion(&listaDeBloques[ubicacion], anInode->i_blockIDS,
				level, bloquesIndirectos))
			return listaDeBloques;
		else
			return NULL;
	}

	if (cantidadDeBloques > pow(cantidadPunterosABloques, 2)) {

		bloquesIndirectos = pow(cantidadPunterosABloques, 2);

		cantidadDeBloques -= bloquesIndirectos;
		level = 2;

		if (leerIndireccion(&listaDeBloques[ubicacion], anInode->i_blockIDD,
				level, bloquesIndirectos))
			ubicacion += bloquesIndirectos;
		else
			return NULL;

	} else {
		bloquesIndirectos = cantidadDeBloques;
		level = 2;
		if (leerIndireccion(&listaDeBloques[ubicacion], anInode->i_blockIDD,
				level, bloquesIndirectos))
			return listaDeBloques;
		else
			return NULL;

	}

	if (cantidadDeBloques <= pow(cantidadPunterosABloques, 3)) {
		bloquesIndirectos = cantidadDeBloques;

		level = 3;

		if (leerIndireccion(&listaDeBloques[ubicacion], anInode->i_blockIDT,
				level, bloquesIndirectos))
			return listaDeBloques;

		else
			return NULL;
	}

	return NULL;
}

static uint32_t* ext2_get_Blocks_of_an_offset(inode_t inode, uint32_t offset,
		uint32_t size) {
	uint32_t* arrayOfBlocks = ext2_get_AllBlocks(&inode, inode.i_size);
	uint32_t firstpos = offset / SbI.blockSize;
	uint32_t entriesToRead = ceil((double) size / SbI.blockSize);
	uint32_t* blocksToRead;
	if (entriesToRead == 1) {
		blocksToRead = calloc(1, sizeof(uint32_t));
	} else {
		blocksToRead = calloc(entriesToRead, sizeof(uint32_t));
	}
	memcpy(blocksToRead, &arrayOfBlocks[firstpos],
			entriesToRead * sizeof(uint32_t));
	free(arrayOfBlocks);
	return blocksToRead;
}

static char* ext2_readBlocks(inode_t inode, uint32_t offset, uint32_t size) {
	uint32_t* blocksToRead = ext2_get_Blocks_of_an_offset(inode, offset, size);
	if (*blocksToRead == 0) {
		return NULL;
	}
	uint32_t cantOfBlocks = ceil((double) size / SbI.blockSize);
	char* arrayOfBlocks = calloc(cantOfBlocks + 1, SbI.blockSize);
	uint32_t i;
	char* buf = malloc(SbI.blockSize);
	for (i = 0; i < cantOfBlocks; i++) {
		leerUnBloque(blocksToRead[i], buf);
		if (i == cantOfBlocks - 1) {
			uint32_t resto = size - (i * SbI.blockSize);
			memcpy(arrayOfBlocks + i * SbI.blockSize, buf, resto);
		} else {

			memcpy(arrayOfBlocks + i * SbI.blockSize, buf, SbI.blockSize);
		}
	}
	free(buf);
	free(blocksToRead);
	uint32_t resto = offset % SbI.blockSize;
	return (char*) (arrayOfBlocks + resto);
}

static t_list* ext2_getblocksofdir(inode_t inodo) {
	char* contentOfBlocks = ext2_readBlocks(inodo, 0, inodo.i_size);
	uint8_t* aux = (uint8_t*) (contentOfBlocks);
	directorio_t* directorio = (directorio_t*) (aux);
	t_list* lCadenas = list_create();
	for (;
			(directorio->inode != 0
					&& directorio->inode
							< SbI.inodosXGrupo * SbI.cantidadDeGroupBlocks)/*||directorio->rec_len!=0*/;
			) {
		if (!S_ISDIR(inodo.i_mode)) {
			perror("No es un directorio.. que so vo jacker?");
			return NULL;
		}
		list_add(lCadenas,
				(char*) strndup(directorio->name, directorio->name_len));
		aux += directorio->rec_len;
		directorio = (directorio_t*) (aux);
	}
	free(contentOfBlocks);
	return lCadenas;
}

inode_t* ext2_get_Inode(char* path, uint32_t* nroInodo) {
	inode_t* inodo = leerInodo(2);
	if (strcmp(path, "/") == 0) {
		if (nroInodo != NULL) {

			*nroInodo = 2;
		}
		return inodo;
	}
	char** arrayOfNameDirs = string_split(path, "/");
	int32_t i = -1;
	directorio_t* directorio;
	char* toFree;
	do {
		i++;

		char* contentOfBlocks = ext2_readBlocks(*inodo, 0, inodo->i_size);
		uint8_t* aux = (uint8_t*) (contentOfBlocks);
		directorio = (directorio_t*) (aux);
		for (;
				(directorio->inode != 0
						&& directorio->inode
								< SbI.inodosXGrupo * SbI.cantidadDeGroupBlocks) /*|| directorio->rec_len !=0*/;
				) {
			if (strncmp(arrayOfNameDirs[i], directorio->name,
					directorio->name_len) == 0
					&& strlen(arrayOfNameDirs[i]) == directorio->name_len) {
				if (nroInodo != NULL) {
					*nroInodo = directorio->inode;
				}
				inodo = leerInodo(directorio->inode);
				break;

			}
			aux += directorio->rec_len;
			directorio = (directorio_t*) (aux);
		}
		if (directorio->inode == 0) {
			//perror("no fue econtrado");
			for (i = 0; arrayOfNameDirs[i] != NULL; i++) {
				free(arrayOfNameDirs[i]);
			}
			free(arrayOfNameDirs);
			free(contentOfBlocks);
			return NULL;
			break;
		}
		inodo = leerInodo(directorio->inode);
		if (nroInodo != NULL) {
			*nroInodo = directorio->inode;
		}
		if (!string_ends_with(path, arrayOfNameDirs[i])) {

			free(contentOfBlocks);
		} else {
			toFree = contentOfBlocks;
		}
	} while (!string_ends_with(path, arrayOfNameDirs[i]));
	if (arrayOfNameDirs[i] == NULL) {
		i--;
	}
	if (strncmp(arrayOfNameDirs[i], directorio->name, directorio->name_len) != 0
			|| strlen(arrayOfNameDirs[i]) != directorio->name_len) {
		for (i = 0; arrayOfNameDirs[i] != NULL; i++) {
			free(arrayOfNameDirs[i]);
		}
		free(arrayOfNameDirs);
		free(toFree);

		return NULL;
	}
	for (i = 0; arrayOfNameDirs[i] != NULL; i++) {
		free(arrayOfNameDirs[i]);
	}
	free(arrayOfNameDirs);
	free(toFree);
	return inodo;

}

static char* ext2_get_fatherDirectoryAndNameOfNewFile(char* path, char** name) {

	char** arrayOfnameDirectories = string_split(path, "/");
	uint32_t i;
	for (i = 0; arrayOfnameDirectories[i] != NULL; i++) {

	}
	*name = strdup(arrayOfnameDirectories[i - 1]);
	char* path_last = strrchr(path, '/');
	if (strcmp(path, path_last) == 0) {
		for (i = 0; arrayOfnameDirectories[i] != NULL; i++) {
			free(arrayOfnameDirectories[i]);
		}
		free(arrayOfnameDirectories);
		return "/";
	}
	int size_splitted = strlen(path) - strlen(path_last);
	char* splitted_path = calloc(size_splitted + 1, sizeof(char));
	memcpy(splitted_path, path, size_splitted);
	for (i = 0; arrayOfnameDirectories[i] != NULL; i++) {
		free(arrayOfnameDirectories[i]);
	}
	free(arrayOfnameDirectories);
	return splitted_path;

}
//TODO testear
static bool ext2_addEntryToDirectory(char* path, uint32_t inodeNumberOfNewFile,
		structures_synchronizer_t* locks, directorio_t* newEntry) {

	bool added = false;
	while (!added) {
		char* name;
		char* father = ext2_get_fatherDirectoryAndNameOfNewFile(path, &name);
		uint32_t nroInodo;
		open_file_t* fatherLock = foundOrAddOpenFileInList(father,
				locks->openFileList);
		pthread_rwlock_wrlock(&fatherLock->lock);
		inode_t* inode = ext2_get_Inode(father, &nroInodo);
		uint32_t* blocks = ext2_get_AllBlocks(inode, inode->i_size);
		uint32_t cantidad = ceil((double) inode->i_size / SbI.blockSize);

		for (int i = 0; i < cantidad; ++i) {

			void* dataWithOffset = FSmap + blocks[i] * SbI.blockSize;
			void* aux = dataWithOffset;
			uint32_t cont = 0;

			while ((aux < dataWithOffset + SbI.blockSize) && !added) {
				directorio_t* dir = (directorio_t*) aux;

				uint32_t sinPaddear_size = sizeof(directorio_t) + dir->name_len;
				uint32_t conPadding_size;
				if ((sinPaddear_size % 4) > 0) {
					conPadding_size =
							(sinPaddear_size + 4 - sinPaddear_size % 4);
				} else {
					conPadding_size = sinPaddear_size;
				}
				if (dir->rec_len == 0) {
					dir->rec_len = SbI.blockSize;

				}
				if (dir->rec_len > conPadding_size) {
					uint32_t freeSpace = dir->rec_len - conPadding_size;
					if (freeSpace > newEntry->rec_len) {

						if (conPadding_size > 8) {
							dir->rec_len = conPadding_size;
							dir = (directorio_t*) (aux + conPadding_size);
							newEntry->rec_len = freeSpace;
						} else {

							newEntry->rec_len = dir->rec_len;
						}
						dir->inode = newEntry->inode;
						dir->rec_len = newEntry->rec_len;
						dir->name_len = newEntry->name_len;
						dir->file_type = newEntry->file_type;
						memcpy(dir->name, newEntry->name, newEntry->name_len);
						printf("added");
						added = true;
					}
				}

				aux += dir->rec_len;
				cont += dir->rec_len;

			}

			if (added)
				break;

		}

		if (!added) {
			if (ext2_addBlocks(inode, 1, locks) != 0) {
				pthread_rwlock_unlock(&fatherLock->lock);
				open_file_finishHim(locks->openFileList, fatherLock);
				free(blocks);
				//free(father);
				free(name);
				return added;
			}
		}
	pthread_rwlock_unlock(&fatherLock->lock);
	open_file_finishHim(locks->openFileList, fatherLock);
	free(blocks);
	free(name);
	//free(father);
	}

	return added;
}
