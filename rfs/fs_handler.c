/*
 * fs_handler.c
 *
 *  Created on: 03/05/2012
 *      Author: utnso
 */
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/mman.h>
#include <errno.h>
#include <sys/stat.h>
#include <libmemcached/memcached.h>

#include "fs_handler.h"
#include "groupDescriptor.h"
#include "superbloque.h"
#include "commons/config.h"
#include "memcachear.h"

#define MAX_WRITINGS_PERSYNC 5

extern sbInfo SbI;
struct stat DatosDelArchivo;
void* FSmap;
uint32_t writings;
extern uint32_t Retardo;
//extern memcached_st* cache;

uint32_t abrirFS(char* diskFilePath) { //abre el archivo ext2 y lo mapea
	int32_t file_descriptor;
	writings = 0;

	if ((file_descriptor = open(diskFilePath, O_RDWR)) == -1) {
		//log_error(Log,"Principal",strerror(errno));
		printf("Código de Error:%d Fallo al asociar el disco al proceso. %s\n",
				errno, strerror(errno));
		exit(1);
	}
	if(fstat(file_descriptor,&DatosDelArchivo)==-1)
		perror("Fallo el stat");
	FSmap = mmap(NULL, (DatosDelArchivo.st_size)+1, PROT_READ | PROT_WRITE, MAP_SHARED,
			file_descriptor, 0);
	if (FSmap == MAP_FAILED) {
		//log_error(Log,"Principal",strerror(errno));
		printf("Código de Error:%d Fallo en la funcion mmap. %s\n", errno,
				strerror(errno));
		exit(1);
	}
	posix_madvise(FSmap, DatosDelArchivo.st_size, POSIX_MADV_RANDOM);

	return file_descriptor;
}

void actualizarFS(uint32_t bloque, char* buf,structures_synchronizer_t* locks) { //Escribe un bloque
	uint32_t size;
	if((strlen(buf)+1)>=SbI.blockSize){
		size=SbI.blockSize;
	}else{
		size=strlen(buf)+1;
	}
	memcpy(FSmap + (bloque * SbI.blockSize), buf, size);
	//if(memcached_addOrReplace(cache,(char*)bloque,buf,SbI.blockSize)!=MEMCACHED_SUCCESS)
		//perror("no puedo guardar en la cache");
	pthread_mutex_lock(&locks->writesSyncLock);
	if (writings >= MAX_WRITINGS_PERSYNC) {
		msync(FSmap, DatosDelArchivo.st_size, MS_ASYNC);
		writings = 0;
	}
	writings++;
	pthread_mutex_unlock(&locks->writesSyncLock);
	if (Retardo != 0)
			usleep(Retardo*1000);

}

void leerUnBloque(uint32_t bloque, char* buf) { //lee un bloque

	memcpy(buf, FSmap + (bloque * SbI.blockSize), SbI.blockSize);
//	if (Retardo != 0)
//			usleep(Retardo*1000);
}

void closeFS(uint32_t file_descriptor){ //Desmapea y cierra el archivo

	if (munmap(FSmap, DatosDelArchivo.st_size) == -1) {
		//log_error(Log,"Principal",strerror(errno));
		printf("Código de Error:%d Fallo en la funcion munmap. %s\n",errno,strerror(errno));
		exit(1);
	}
	if(close(file_descriptor)<0){
		//log_error(Log,"Principal",strerror(errno));
		printf("Código de Error:%d Fallo al cerrar el disco. %s\n",errno,strerror(errno));
		exit(1);
	}
}
