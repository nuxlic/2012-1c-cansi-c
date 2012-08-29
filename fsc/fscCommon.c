/*
 * fscCommon.c
 *
 *  Created on: 14/06/2012
 *      Author: utnso
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <semaphore.h>
#include <errno.h>

#include "commons/config.h"
#include "commons/log.h"

t_config* fsc_config;

void readFSCConfig(char* configPath, char** IP, uint32_t* port, char** IPcache,
		uint32_t* portCache,char** logFilePath,t_log_level* logLevelDetail,uint32_t* cantidadDeConexiones) { //se tiene que ampliar para leer la ip el puerto y otras cosas mas
	fsc_config = config_create(configPath);
	*IP = calloc(1,strlen(config_get_string_value(fsc_config, "IP"))+1);

	strncpy(*IP, config_get_string_value(fsc_config, "IP"),
			strlen(config_get_string_value(fsc_config, "IP"))+1);
	*port = config_get_int_value(fsc_config, "Port");
	*IPcache = malloc(strlen(config_get_string_value(fsc_config, "IPcache"))+1);

	strncpy(*IPcache, config_get_string_value(fsc_config, "IPcache"),
			strlen(config_get_string_value(fsc_config, "IPcache"))+1);
	*portCache = config_get_int_value(fsc_config, "PortCache");
	*logFilePath= config_get_string_value(fsc_config,"ArchivoLog");
	*logLevelDetail=config_get_int_value(fsc_config,"DetalleLog");
	*cantidadDeConexiones=config_get_int_value(fsc_config,"CantidadDeConexiones");
}
void destruirRFSConfig(t_config* rfs_config) {

	config_destroy(rfs_config);
}

