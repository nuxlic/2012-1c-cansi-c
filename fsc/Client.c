/*
 * Cliente.c
 *
 *  Created on: 10/05/2012
 *      Author: utnso
 */

#define FUSE_USE_VERSION 27
#define _FILE_OFFSET_BITS 64

#include <fuse.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "funcionesFuse.h"
#include "sockear.h"
#include "serializadores.h"
#include "fscCommons.h"
#include "commons/log.h"
#include "connectionPool.h"

char* ip; //ip del server
uint32_t port; // port del server
char* ipCache;
uint32_t portCache;
t_log_level logDetail;
t_log* teLogueo;
connections_structures_t* pools;

static struct fuse_operations funciones_fuse = {
	.create = create_file,
	.open = open_file,
	.read = file_read,
	.write = file_write,
	.release = release_file,
	.truncate = fuse_truncate,
	.unlink = file_unlink,
	.mkdir = create_dir,
	.readdir = dir_read,
	.rmdir = borrarDir,
	.getattr = file_getattr,};

int main(int argc, char **argv) {
char* logFilePath=malloc(40);
	//--------------levanto el archivo de configuracion----------//
	char* configPath = "../config/fsc.conf";
	uint32_t maxConnect;
	readFSCConfig(configPath, &ip, &port, &ipCache, &portCache,&logFilePath,&logDetail,&maxConnect);
	teLogueo=log_create(logFilePath,"File System Client",true,logDetail);
	pools=calloc(1,sizeof(connections_structures_t));
	connectionPool_init(pools,maxConnect);

	return fuse_main(argc, argv, &funciones_fuse, NULL);
}
