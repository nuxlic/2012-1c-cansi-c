/*
 * memcachear.h
 *
 *  Created on: 13/06/2012
 *      Author: utnso
 */

#ifndef MEMCACHEAR_H_
#define MEMCACHEAR_H_
#include <libmemcached/memcached.h>
#include "multiThreading.h"

typedef struct{
	memcached_st* memcached;
	memcached_server_st* servers;
	memcached_return response;
} memc_t ;

typedef struct{
	uint32_t len;
	char* key;
}memc_key_t;


typedef struct{
	void* data;
	bool from_cache;
}memc_data_t ;

memc_t* memc_connect(char* memc_ip, uint16_t memc_port);
void memc_free(memc_t** memcached);
memc_data_t* memc_get(const args_t* args, uint32_t block,uint32_t*);
void memc_set(const args_t* args, uint32_t block,uint32_t*);
void memc_delete(const args_t* args, uint32_t block);

#endif /* MEMCACHEAR_H_ */
