/*
 * memcachear.h
 *
 *  Created on: 13/06/2012
 *      Author: utnso
 */

#ifndef MEMCACHEAR_H_
#define MEMCACHEAR_H_
#include <libmemcached/memcached.h>
memcached_st* memcachead_connect(memcached_return *rc,memcached_server_st** servers, char* ipCache, uint16_t portCache);
memcached_return memcached_addOrReplace(memcached_st* cache, char* key,
		char* value, uint32_t sizeOfValue);
memcached_return memcached_getValue(memcached_st* cache, char* key, char** value,
		uint32_t* sizeOfValue);
void memcached_finishHer(memcached_st* cache,memcached_server_st** server);

char* getFatherPath(const char* path);
#endif /* MEMCACHEAR_H_ */
