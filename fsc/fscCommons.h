/*
 * fscCommons.h
 *
 *  Created on: 14/06/2012
 *      Author: utnso
 */

#ifndef FSCCOMMONS_H_
#define FSCCOMMONS_H_
#include "commons/config.h"
#include "commons/log.h"
void readFSCConfig(char* configPath, char** IP, uint32_t* port, char** IPcache,
		uint32_t*  portCache,char** logFilePath,t_log_level* logLevelDetail,uint32_t*);
void destruirRFSConfig(t_config* rfs_config );


#endif /* FSCCOMMONS_H_ */
