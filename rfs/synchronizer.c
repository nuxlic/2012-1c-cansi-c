/*
 * synchronizer.c
 *
 *  Created on: 29/06/2012
 *      Author: utnso
 */

#include "synchronizer.h"

extern sbInfo SbI;

structures_synchronizer_t* structures_synchronizer_init() {
	structures_synchronizer_t* structuresLock = malloc(
			sizeof(structures_synchronizer_t));
	structuresLock->openFileList=openFileList_init();
	pthread_mutex_init(&structuresLock->sbLock, NULL);
	pthread_mutex_init(&structuresLock->writesSyncLock, NULL);
	structuresLock->writeBlocksLock = calloc(SbI.cantidadDeGroupBlocks,
			sizeof(pthread_mutex_t));
	structuresLock->writeInodesLock = calloc(SbI.cantidadDeGroupBlocks,
			sizeof(pthread_mutex_t));
	structuresLock->writeDirUsedCountLock = calloc(SbI.cantidadDeGroupBlocks,
			sizeof(pthread_mutex_t));
	for (int i = 0; i < SbI.cantidadDeGroupBlocks; ++i) {
		pthread_mutex_init(&structuresLock->writeBlocksLock[i], NULL);
		pthread_mutex_init(&structuresLock->writeInodesLock[i], NULL);
		pthread_mutex_init(&structuresLock->writeDirUsedCountLock[i], NULL);
	}
	return structuresLock;
}

void structures_synchronizer_finishHim(structures_synchronizer_t* sSynch) {
	pthread_mutex_destroy(&(sSynch->sbLock));
	pthread_mutex_destroy(&sSynch->writesSyncLock);
	for (int i = 0; i < SbI.cantidadDeGroupBlocks; ++i) {
		pthread_mutex_destroy(&sSynch->writeBlocksLock[i]);
		pthread_mutex_destroy(&sSynch->writeInodesLock[i]);
		pthread_mutex_destroy(&sSynch->writeDirUsedCountLock[i]);
	}

	free(sSynch->writeBlocksLock);
	free(sSynch->writeInodesLock);
	free(sSynch->writeDirUsedCountLock);

}

open_file_list_t* openFileList_init() {
	open_file_list_t* list = malloc(sizeof(open_file_list_t));
	list->open_file_list = list_create();
	pthread_mutex_init(&list->mutex, NULL);
	return list;
}

open_file_t* open_file_initialize(char* path) {
	open_file_t* file = malloc(sizeof(open_file_t));
	file->path = strdup(path);
	file->count = 1;
	pthread_rwlock_init(&file->lock, NULL);
	return file;
}

open_file_t* foundOrAddOpenFileInList(char* path, open_file_list_t* list) {
	open_file_t* element;
	pthread_mutex_lock(&list->mutex);

//auxiliar que compara los elementos de la lista para encontrar uno en particular
	bool cmpOfPaths(open_file_t* openFile) {
		if (strlen(path) == strlen(openFile->path)) {
			if (strncmp(path, openFile->path, strlen(path)) == 0) {
				return true;
			}
		}
		return false;
	}
	////////////////////////
	if ((element = list_find(list->open_file_list, (void*) cmpOfPaths)) != NULL) {
		pthread_rwlock_wrlock(&element->lock);
		element->count++;
		pthread_rwlock_unlock(&element->lock);
	} else {
		element = open_file_initialize(path);
		list_add(list->open_file_list, (void*) element);
	}
	pthread_mutex_unlock(&list->mutex);
	return element;
}

void open_file_destroyer(void* data) {
	open_file_t* fileToDestroy = (open_file_t*) data;
	free(fileToDestroy->path);
	pthread_rwlock_destroy(&fileToDestroy->lock);
	free(fileToDestroy);
}

void openFileListRemoveAndDestroy(open_file_list_t* list, open_file_t* file) {
	bool cmpOfPaths(open_file_t* openFile) {
		if (strlen(file->path) == strlen(openFile->path)) {
			if (strncmp(file->path, openFile->path, strlen(file->path) + 1)
					== 0) {
				return true;
			}
		}
		return false;
	}
	list_remove_and_destroy_by_condition(list->open_file_list, cmpOfPaths,
			open_file_destroyer);
}

void openFileListDestroy(open_file_list_t* list) {
	pthread_mutex_lock(&list->mutex);
	list_destroy_and_destroy_elements(list->open_file_list,
			open_file_destroyer);
	pthread_mutex_unlock(&list->mutex);
	pthread_mutex_destroy(&list->mutex);
	free(list);
}

void open_file_finishHim(open_file_list_t* list, open_file_t* file) {
	pthread_mutex_lock(&list->mutex);
	pthread_rwlock_wrlock(&file->lock);
	file->count--;
	pthread_rwlock_unlock(&file->lock);
	if (file->count == 0) {
		openFileListRemoveAndDestroy(list, file);
	}
	pthread_mutex_unlock(&list->mutex);
}
