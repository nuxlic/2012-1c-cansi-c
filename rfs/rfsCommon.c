/*
 * rfsCommon.c
 *
 *  Created on: 15/05/2012
 *      Author: utnso
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <semaphore.h>
#include <errno.h>

#include "commons/config.h"
#include "fs_handler.h"

t_config* rfs_config;
extern uint32_t Retardo;

void readRFSConfig(char* configPath,char** diskFilePath,uint32_t* port,uint32_t* cantidadDeHilos,char** IPcache,uint32_t* portCache){ //se tiene que ampliar para leer la ip el puerto y otras cosas mas
	rfs_config = config_create(configPath);
	*diskFilePath=config_get_string_value(rfs_config,"DiskFilePath");
	Retardo=config_get_int_value(rfs_config,"RetardoDisco");
	*port=config_get_int_value(rfs_config,"Port");
	*cantidadDeHilos=config_get_int_value(rfs_config,"CantidadMaximaDeHilos");
	*portCache=config_get_int_value(rfs_config,"PortCache");
	*IPcache = malloc(strlen(config_get_string_value(rfs_config, "IPcache"))+1);

		strncpy(*IPcache, config_get_string_value(rfs_config, "IPcache"),
				strlen(config_get_string_value(rfs_config, "IPcache"))+1);
}
void destruirRFSConfig(t_config* rfs_config ){

	config_destroy(rfs_config);
}
