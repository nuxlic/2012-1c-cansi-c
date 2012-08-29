/*
 * memcachear.c
 *
 *  Created on: 13/06/2012
 *      Author: utnso
 */

#include <libmemcached/memcached.h>
#include <string.h>
#include "commons/string.h"
//extern char* ipCache;
//extern uint32_t portCache;

memcached_st* memcachead_connect(memcached_return *rc,
		memcached_server_st** servers, char* ipCache, uint16_t portCache) {
	*servers = NULL;
	memcached_st *memc;
	memc = memcached_create(NULL);
	memcached_behavior_set(memc,
				MEMCACHED_BEHAVIOR_BINARY_PROTOCOL, 1);
	*servers = memcached_server_list_append(*servers, ipCache, portCache, rc);
	*rc = memcached_server_push(memc, *servers);
	return memc;

}

memcached_return memcached_addOrReplace(memcached_st* cache, char* key,
		char* value, uint32_t sizeOfValue) {
	memcached_return rc = memcached_set(cache, key, strlen(key), value,
			sizeOfValue, (time_t) 0, (uint32_t) 0);
	return rc;

}

memcached_return memcached_getValue(memcached_st* cache, char* key,
		char** value, uint32_t* sizeOfValue) {

	uint32_t flags;
	memcached_return_t error;
	*value = memcached_get(cache, key, strlen(key),(size_t*) sizeOfValue, &flags,
			&error);
	return error;
}

void memcached_finishHer(memcached_st* cache, memcached_server_st** server) {
	free(*server);
	memcached_free(cache);
}

char* getFatherPath(const char* path) {
	char* path_last = strrchr(path, '/');
	if(strcmp(path,path_last)==0){
		return "/";
	}
		int size_splitted = strlen(path) - strlen(path_last);
		char* splitted_path= calloc(size_splitted+1, sizeof(char));
		memcpy(splitted_path, path, size_splitted);
		return splitted_path;
	}
