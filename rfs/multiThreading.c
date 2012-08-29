/*
 * multiThreading.c
 *
 *  Created on: 07/06/2012
 *      Author: utnso
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include <stdint.h>

#include "multiThreading.h"
#include "socketHandler.h"
#include "commons/collections/queue.h"



void pool_deposite_job_in_queue(individualJob_t* job, colaConSemaforo_t* cola) {
	pthread_mutex_lock(&cola->access);
	queue_push(cola->colaDePedidos, job);
	pthread_mutex_unlock(&cola->access);
	sem_post(&cola->countOfElements);
}

individualJob_t* pool_extract_job_in_queue(colaConSemaforo_t* cola) {
	sem_wait(&cola->countOfElements);
	pthread_mutex_lock(&cola->access);
	individualJob_t* job = queue_pop(cola->colaDePedidos);
	pthread_mutex_unlock(&cola->access);
	return job;
}

void workerJ(void* w) {
	while (1) {
		worker_t* wr=(worker_t*)w;
		individualJob_t* job = pool_extract_job_in_queue(wr->cola);
		wr->args=calloc(1,sizeof(args_t));
		wr->args->ped = job->ped;
		wr->args->client = job->ped->client;
		wr->args->lockeos = job->locks;
		job->procesarPedido(wr->args);
		free(job);
	}
}

