/*
 * memcachear.c
 *
 *  Created on: 13/06/2012
 *      Author: utnso
 */

#include <libmemcached/memcached.h>
#include <string.h>
#include <unistd.h>

#include "memcachear.h"

extern void* FSmap;
extern sbInfo SbI;
extern char* ipCache;
extern uint32_t portCache;
extern uint32_t Retardo;

static memc_key_t* generate_key(const char* path, uint32_t block);

memc_t* memc_connect(char* memc_ip, uint16_t memc_port) {
	memc_t* memcached = calloc(1, sizeof(memc_t));
	memcached->memcached = memcached_create(NULL);
	memcached_behavior_set(memcached->memcached,
			MEMCACHED_BEHAVIOR_BINARY_PROTOCOL, 1);
	memcached->servers = memcached_server_list_append(NULL, memc_ip, memc_port,
			&memcached->response);
	memcached->response = memcached_server_push(memcached->memcached,
			memcached->servers);
	return memcached;
}

void memc_free(memc_t** memcached) {
	free((*memcached)->servers);
	memcached_free((*memcached)->memcached);
	free(*memcached);
}

memc_data_t* memc_get(const args_t* args, uint32_t block, uint32_t* file_data) {

	memcached_return return_status;
	uint32_t flags;

	uint32_t block_size;
	memc_key_t* my_key = generate_key(args->ped->path, block);
	memc_t* mc = memc_connect(ipCache, portCache);
	void* cache_data = memcached_get(mc->memcached, my_key->key, my_key->len,
			&block_size, &flags, &return_status);

	memc_data_t* block_data = calloc(1, sizeof(memc_data_t));
	block_data->from_cache = false;
	if (return_status == MEMCACHED_SUCCESS) {
		block_data->data = cache_data;
		block_data->from_cache = true;
	} else {
		block_data->data = FSmap + file_data[block] * SbI.blockSize;
		if (Retardo != 0) {
			usleep(1000*Retardo);
		}
		memcached_set(mc->memcached, my_key->key, my_key->len, block_data->data,
				SbI.blockSize, (time_t) 0, (uint32_t) 0);
	}
	free(my_key->key);
	free(my_key);
	memc_free(&mc);
	return block_data;

}

void memc_set(const args_t* args, uint32_t block, uint32_t* file_data) {

	memc_key_t* my_key = generate_key(args->ped->path, block);

	void* data = FSmap + file_data[block] * SbI.blockSize;
	memc_t * mc = memc_connect(ipCache, portCache);
	memcached_set(mc->memcached, my_key->key,
			my_key->len, data, SbI.blockSize, (time_t) 0, (uint32_t) 0);
	if(Retardo!=0){
		usleep(1000*Retardo);
	}
	free(my_key->key);
	free(my_key);
	memc_free(&mc);
	return;

}

void memc_delete(const args_t* args, uint32_t block) {
	memc_key_t* my_key = generate_key(args->ped->path, block);

	memcached_delete(args->ped->mc, my_key->key, my_key->len, 0);

	free(my_key->key);
	free(my_key);
	return;
}

static memc_key_t* generate_key(const char* path, uint32_t block) {

	memc_key_t* super_key = calloc(1, sizeof(memc_key_t));
	super_key->len = strlen(path) + sizeof(uint32_t);
	super_key->key = calloc(super_key->len, sizeof(char));
	memcpy(super_key->key, path, strlen(path));
	memcpy(&super_key->key[strlen(path)], &block, 4);
	return super_key;
}
