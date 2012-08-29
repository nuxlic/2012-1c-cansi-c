/*
 * rfsCommon.h
 *
 *  Created on: 15/05/2012
 *      Author: utnso
 */

#ifndef RFSCOMMON_H_
#define RFSCOMMON_H_
#include "commons/config.h"

void readRFSConfig(char* configPath,char** diskFilePath,uint32_t* port,uint32_t* cantidadDeHilos,char** IPcache,uint32_t* portCache);
void destruirRFSConfig(t_config*);


#endif /* RFSCOMMON_H_ */
