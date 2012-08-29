/*
 * multiThreading.h
 *
 *  Created on: 07/06/2012
 *      Author: utnso
 */

#ifndef MULTITHREADING_H_
#define MULTITHREADING_H_
#include "commons/collections/queue.h"
#include "stdint.h"
#include "semaphore.h"
#include "fs_handler.h"

typedef struct{
	char* path;
	uint32_t offset;
	uint32_t size;
	void* buffer;
	mode_t mode;
	int32_t client;
	memcached_st* mc;
	uint32_t bytes;
} pedido_t;

typedef struct{
	char* path;
	void* data;
	uint8_t error;
	uint32_t bytes;
} resultado_t;

typedef struct{
	pedido_t* ped;
	structures_synchronizer_t* lockeos;
	int32_t client;
}args_t;
typedef struct{
	void  (*procesarPedido)(args_t*);
	pedido_t* ped;
	structures_synchronizer_t* locks;

}individualJob_t;

typedef struct{
	t_queue* colaDePedidos;
	sem_t countOfElements;
	pthread_mutex_t access;
}colaConSemaforo_t;

typedef struct{
	colaConSemaforo_t* cola;
	args_t* args;
}worker_t;

void thread_detached(individualJob_t* jobToDo,structures_synchronizer_t* lockeos,int32_t client);

void pool_deposite_job_in_queue(individualJob_t* job,colaConSemaforo_t* cola);
individualJob_t* pool_extract_job_in_queue(colaConSemaforo_t* cola);
void workerJ(void* worker);
#endif /* MULTITHREADING_H_ */
