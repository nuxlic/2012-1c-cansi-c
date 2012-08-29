/*
 * connectionPool.h
 *
 *  Created on: 16/07/2012
 *      Author: utnso
 */

#ifndef CONNECTIONPOOL_H_
#define CONNECTIONPOOL_H_
#include <libmemcached/memcached.h>
#include "commons/collections/list.h"
#include <semaphore.h>
typedef struct{
	t_list* socketPool;
	t_list* memcachedPool;
	sem_t countMemcachedConnections;
	sem_t countSocketConnections;
	pthread_mutex_t mutexSocketPool;
	pthread_mutex_t mutexMemcachedPool;
}connections_structures_t;

int32_t connectionPool_init(connections_structures_t* pools,
		uint32_t maximasConexiones);
int connectionPool_extractAvaibleSocket(connections_structures_t* pools);
int connectionPool_depositeAvaibleSocket(connections_structures_t* pools,int sfd);
memcached_st* connectionPool_extractAvaibleMemcached(connections_structures_t* pools);
int connectionPool_depositeAvaibleMemcached(connections_structures_t* pools,memcached_st* memc);


#endif /* CONNECTIONPOOL_H_ */
