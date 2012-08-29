/*
 * rfs.c
 *
 *  Created on: 15/05/2012
 *      Author: utnso
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <semaphore.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <netinet/in.h>
#include <assert.h>
#include <arpa/inet.h>
#include <libmemcached/memcached.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <errno.h>
#include <sys/inotify.h>

#include "commons/sockear.h"

#define MAXEVENTS 64

#include "fs_handler.h"
#include "rfsCommon.h"
#include "superbloque.h"
#include "groupDescriptor.h"
#include "bitmap.h"
#include "inode.h"
#include "ext2_def.h"
#include "ext2_operations.h"
#include "multiThreading.h"
#include "socketHandler.h"
#include "commons/sockear.h"
#include "commons/string.h"
#include "commons/serializadores.h"
#include "memcachear.h"
#include "commons/log.h"

#define EVENT_SIZE  ( sizeof (struct inotify_event) + 24 )
#define BUF_LEN     ( 1024 * EVENT_SIZE )

sbInfo SbI;
int32_t file_descriptor;
extern t_config* rfs_config;
uint32_t Retardo;
extern uint32_t maxClientes;
char* ipCache;
uint32_t portCache;
memcached_st* cache;
t_log* logueo;
int main(int argc, char **argv) {
	char* configPath = malloc(50);
	uint32_t port;
	uint32_t cantidadDeHilos;
	char* IP;
	//char* logPath; //Por ahora no la usamos
	char* diskFilePath;

	strcpy(configPath, argv[1]);

	readRFSConfig(configPath, &diskFilePath, &port, &cantidadDeHilos, &ipCache,
			&portCache);
	file_descriptor = abrirFS(diskFilePath);
	superbloque_t* sb = leer_superbloque();
	SbI = get_sbInfo(sb);
	maxClientes = 15; //levantarlo del arch de conf
	logueo = log_create("logRFS.txt", "RFS", true, LOG_LEVEL_DEBUG);

	structures_synchronizer_t* sLocks = structures_synchronizer_init();
	worker_t* worker;
	worker=calloc(1,sizeof(worker_t));
	worker->cola=calloc(1,sizeof(colaConSemaforo_t));
	worker->cola->colaDePedidos = queue_create();
	sem_init(&worker->cola->countOfElements, 0, 0);
	pthread_mutex_init(&worker->cola->access, NULL);

	pthread_t thPool[cantidadDeHilos];
		int i;
		for (i = 0; i < cantidadDeHilos; i++) {
			pthread_create(&thPool[i], NULL, (void*) workerJ,(void*) worker);
		}
///////////PROGRAMACION DEL EPOLL////////////////
	int sfd, s;
	int efd;
	struct epoll_event event;
	struct epoll_event *events;

	sfd = crear_socket();
	int yes = 1;
	if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int32_t))
			== -1) {
		perror("setsockopt");
		exit(1);
	}
	bindear_socket(sfd, port);
	if (sfd == -1)
		abort();

//	s = make_socket_non_blocking(sfd);
//	if (s == -1) {
//		abort();
//	}
	s = listen(sfd, SOMAXCONN);
	if (s == -1) {
		perror("listen");
		abort();
	}

	efd = epoll_create1(0);
	if (efd == -1) {
		perror("epoll_create");
		abort();
	}

	event.data.fd = sfd;
	event.events = EPOLLIN | EPOLLET;
	s = epoll_ctl(efd, EPOLL_CTL_ADD, sfd, &event);
	if (s == -1) {
		perror("epoll_ctl");
		abort();
	}

	/* Buffer where events are returned */
	events = calloc(MAXEVENTS, sizeof event);
	puts("Server Listening for incoming connections\n");
	puts("It's all in your hands\n");
	puts("Que la fuerza te acompañe en contra de los segmentation faults\n");
	/* The event loop */

	int inotify = inotify_init();
	if (inotify < 0) {
		perror("inotify_init");
	}

	// Creamos un monitor sobre un path indicando que eventos queremos escuchar
	inotify_add_watch(inotify, configPath, IN_MODIFY);
//	s = make_socket_non_blocking(inotify);
//	if (s == -1)
//		abort();

	event.data.fd = inotify;
	event.events = EPOLLIN | EPOLLET;
	s = epoll_ctl(efd, EPOLL_CTL_ADD, inotify, &event);
	if (s == -1) {
		perror("epoll_ctl");
		abort();
	}

	while (1) {
		int n, done, i;

		n = epoll_wait(efd, events, MAXEVENTS, -1);
		for (i = 0; i < n; i++) {
			if (inotify == events[i].data.fd) {
				char buffer[BUF_LEN];
				int length = read(file_descriptor, buffer, BUF_LEN);
				if (length < 0) {
					perror("read");
				}
				destruirRFSConfig(rfs_config);
				rfs_config = config_create(configPath);
				void cargarRetardo(uint32_t* Retardo) {
					*Retardo = config_get_int_value(rfs_config, "RetardoDisco");
				}
				cargarRetardo(&Retardo);
				continue;
			}
			if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP)
					|| (!(events[i].events & EPOLLIN))) {
				/* An error has occured on this fd, or the socket is not
				 ready for reading (why were we notified then?) */
				fprintf(stderr, "epoll error\n");
				close(events[i].data.fd);
				continue;
			}

			else if (sfd == events[i].data.fd) {
				/* We have a notification on the listening socket, which
				 means one or more incoming connections. */
				//while (1) {
				int infd;
				infd = aceptar_clientes(sfd);
				if (infd == -1) {
					if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
						/* We have processed all incoming
						 connections. */
						break;
					} else {
						perror("accept");
						break;
					}
				}
				char* stream = malloc(3);
				recibir(infd, stream, 3);
				uint8_t* handshakeRecibidoDeserializado =
						deserializar_handshake(stream, 0);
				t_stream* respuesta;
				if (*handshakeRecibidoDeserializado == OK) {
					respuesta = serializar_handshake(OK);
					enviar_paquete(infd, respuesta->data, respuesta->length);
					free(respuesta->data);
					free(respuesta);
					free(handshakeRecibidoDeserializado);
				} else {
					printf("No paso el handshake\n");
					cerrar(events[i].data.fd);
					free(handshakeRecibidoDeserializado);
					break;
				}

				/* Make the incoming socket non-blocking and add it to the
				 list of fds to monitor. */
//				s = make_socket_non_blocking(infd);
//				if (s == -1)
//					abort();

				event.data.fd = infd;
				event.events = EPOLLIN | EPOLLET;
				s = epoll_ctl(efd, EPOLL_CTL_ADD, infd, &event);
				if (s == -1) {
					perror("epoll_ctl");
					abort();
				}
				//}
				continue;
			} else {
				/* We have data on the fd waiting to be read. Read and
				 display it. We must read whatever data is available
				 completely, as we are running in edge-triggered mode
				 and won't get a notification again for the same
				 data. */
				done = 0;

				//while (1) {

				/* Recibir el paquete */
				char* h = malloc(3);
				int error = recibir(events[i].data.fd, h, 3);

				if (error == -6) {
					/* If errno == EAGAIN, that means we have read all
					 data. So go back to the main loop. */
					continue;
					if (errno != EAGAIN) {
						perror("read");
						done = 1;
					}
					printf("Closed connection on descriptor %d\n",
							events[i].data.fd);

					/* Closing the descriptor will make epoll remove it
					 from the set of descriptors which are monitored. */
					close(events[i].data.fd);
					continue;
				} else if (error == -10) {
					/* End of file. The remote has closed the
					 connection. */
					done = 1;
					printf("Closed connection on descriptor %d\n",
							events[i].data.fd);

					/* Closing the descriptor will make epoll remove it
					 from the set of descriptors which are monitored. */
					close(events[i].data.fd);
					continue;
				}
				t_header* header = deserializadorHeader(h);
				if (header->type == TRUNCATE) {
					puts("aca pa");
					if (header->length > 30) {
						puts("Llego verdura y no se xq\n");
					}
				}
				t_stream* stream = malloc(sizeof(t_stream));
				stream->data = malloc(header->length);
				recibir(events[i].data.fd, stream->data, header->length);
				individualJob_t* job = functionIdentifyAndOperate(header,
						stream, events[i].data.fd, sLocks);
				free(header);
				pool_deposite_job_in_queue(job, worker->cola);
			}

			if (done) {
				printf("Closed connection on descriptor %d\n",
						events[i].data.fd);

				/* Closing the descriptor will make epoll remove it
				 from the set of descriptors which are monitored. */
				close(events[i].data.fd);
			}
			//}
		}
	}

	free(events);

	close(sfd);

	free(IP);
	free(configPath);

	closeFS(file_descriptor);
	destruirRFSConfig(rfs_config);
	return EXIT_SUCCESS;
}
