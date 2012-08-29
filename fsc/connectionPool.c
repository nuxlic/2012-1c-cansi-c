/*
 * connectionPool.c
 *
 *  Created on: 16/07/2012
 *      Author: utnso
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include<sys/socket.h>
#include<errno.h>
#include<stdio.h>
#include<string.h>
#include<netinet/in.h>
#include<stdlib.h>
#include<fcntl.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>

#include "sockear.h"
#include "memcachear.h"
#include "serializadores.h"
#include "connectionPool.h"
#include "commons/collections/list.h"

extern char* ip;
extern uint32_t port; // port del server
extern char* ipCache;
extern uint32_t portCache;

int32_t connectionPool_init(connections_structures_t* pools,
		uint32_t maximasConexiones) {
	pools->socketPool=list_create();
	pools->memcachedPool=list_create();
	pthread_mutex_init(&pools->mutexMemcachedPool,NULL);
	pthread_mutex_init(&pools->mutexSocketPool,NULL);
	sem_init(&pools->countSocketConnections,0,0);
	sem_init(&pools->countMemcachedConnections,0,0);
	int i;
	for (i = 0; i < maximasConexiones; ++i) {
		int sfd = crear_socket();
		conectar_socket(sfd, ip, port);
		uint8_t comunicacion = iniciar_comunicacion(sfd);
		if (comunicacion == OK) {
			list_add(pools->socketPool,(void*)sfd);
			sem_post(&pools->countSocketConnections);
		} else {
			abort();
		}
		memcached_return cache_remota;
		memcached_server_st* servers;
		memcached_st* memcached = memcachead_connect(&cache_remota, &servers,
				ipCache, portCache);
		list_add(pools->memcachedPool,memcached);
		sem_post(&pools->countMemcachedConnections);
	}
	return 0;
}

int connectionPool_extractAvaibleSocket(connections_structures_t* pools){
	sem_wait(&pools->countSocketConnections);
	pthread_mutex_lock(&pools->mutexSocketPool);
	void* sfd;
	sfd=list_remove(pools->socketPool,0);
	pthread_mutex_unlock(&pools->mutexSocketPool);
	return (int)sfd;
}

int connectionPool_depositeAvaibleSocket(connections_structures_t* pools,int sfd){
	pthread_mutex_lock(&pools->mutexSocketPool);
	list_add(pools->socketPool,(void*)sfd);
	pthread_mutex_unlock(&pools->mutexSocketPool);
	sem_post(&pools->countSocketConnections);
	return 0;
}

memcached_st* connectionPool_extractAvaibleMemcached(connections_structures_t* pools){
	sem_wait(&pools->countMemcachedConnections);
	pthread_mutex_lock(&pools->mutexMemcachedPool);
	memcached_st* memc=list_remove(pools->memcachedPool,0);
	pthread_mutex_unlock(&pools->mutexMemcachedPool);
	return memc;
}

int connectionPool_depositeAvaibleMemcached(connections_structures_t* pools,memcached_st* memc){
	pthread_mutex_lock(&pools->mutexMemcachedPool);
	list_add(pools->memcachedPool,memc);
	pthread_mutex_unlock(&pools->mutexMemcachedPool);
	sem_post(&pools->countMemcachedConnections);
	return 0;
}
