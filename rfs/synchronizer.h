/*
 * synchronizer.h
 *
 *  Created on: 29/06/2012
 *      Author: utnso
 */

#ifndef SYNCHRONIZER_H_
#define SYNCHRONIZER_H_
#include <pthread.h>
#include <stdlib.h>
#include <semaphore.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>
#include <libmemcached/memcached.h>

//#include "fs_handler.h"
#include "superbloque.h"
#include "commons/collections/queue.h"
#include "commons/collections/list.h"
#include "commons/string.h"

typedef struct{
	char* path;
	uint32_t count;
	pthread_rwlock_t lock;
} open_file_t;

typedef struct{
	t_list* open_file_list;
	pthread_mutex_t mutex;
} open_file_list_t;

typedef struct{
	open_file_list_t* openFileList;
	pthread_mutex_t sbLock; //para el contados de bloques e inodos libres
	pthread_mutex_t writesSyncLock;//para el contador de escrituras
	pthread_mutex_t* writeBlocksLock;//para el contador de bloques libres del descriptor de grupo
	pthread_mutex_t* writeInodesLock;//para el contador de inodos libres del descriptor de grupo
	pthread_mutex_t* writeDirUsedCountLock;// para el contador de directorios en uso del descriptor de grupo
} structures_synchronizer_t;

structures_synchronizer_t* structures_synchronizer_init();
void structures_synchronizer_finishHim(structures_synchronizer_t* sSynch);

open_file_list_t* openFileList_init();
open_file_t* foundOrAddOpenFileInList(char* path,open_file_list_t* list);
void openFileListRemoveAndDestroy(open_file_list_t* list, open_file_t* file);
void openFileListDestroy(open_file_list_t* list);
void open_file_finishHim(open_file_list_t* list, open_file_t* file);

#endif /* SYNCHRONIZER_H_ */
