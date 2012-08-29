/*
 * procesadorDePedidos.c
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

#include "commons/sockear.h"
#include"commons/serializadores.h"
#include "ext2_operations.h"
#include "multiThreading.h"
#include "commons/log.h"
#include "procesadorDePedidos.h"



///////////////////////////PRIVATE FUNCTIONS PROTOTYPES/////////////////////////////////////////////
static void enviarRespuestaCreateFile(const resultado_t* const res,
		int32_t client, structures_synchronizer_t* locks);
static void enviarRespuestaCreateDir(const resultado_t* const res,
		int32_t client, structures_synchronizer_t* locks);
static void enviarRespuestaDeleteDir(const resultado_t* const res,
		int32_t client, structures_synchronizer_t* locks);
static void enviarRespuestaDeleteFile(const resultado_t* const res,
		int32_t client, structures_synchronizer_t* locks);
static void enviarRespuestaReadFile(const resultado_t* const res,
		int32_t client, structures_synchronizer_t* locks);
static void enviarRespuestaWriteFile(const resultado_t* const res,
		int32_t client, args_t*);
static void enviarRespuestaReadDir(const resultado_t* const res, int32_t client,
		structures_synchronizer_t* locks);
static void enviarRespuestaGetAttr(const resultado_t* const res, int32_t client,
		structures_synchronizer_t* locks);
static void enviarRespuestaOpenFile(const resultado_t* const res,
		int32_t client, structures_synchronizer_t* locks);
static void enviarRespuestaTruncate(const resultado_t* const res,
		int32_t client, structures_synchronizer_t* locks);

//////////////////////////////////////PUBLIC FUNCTIONS/////////////////////////////////////////////
void procesarPedidoCreateFile(args_t* args) {
	resultado_t* res = malloc(sizeof(resultado_t));
	res->path = args->ped->path;

	if (strlen(args->ped->path) > 40) {
		res->data = NULL;
		res->error = NAME_TOO_LONG;
		//log_debug(logueo, "proceso un pedido de create File");
		enviarRespuestaCreateFile(res, args->client, args->lockeos);
		return;
	}
	inode_t* inode = ext2_get_Inode(args->ped->path, NULL);
	if (inode != NULL) {
		res->data = NULL;
		res->error = YA_EXISTE;
		enviarRespuestaCreateFile(res, args->client, args->lockeos);
		return;
	}
	int32_t error = ext2_create_file(args->ped->path, args->ped->mode,
			args->lockeos);

	if (error == -10) {
		res->error = ESTA_LLENO;
		res->data = NULL;
		res->path = args->ped->path;
		puts("esta lleno");
	}
	if (error == -1) {
		res->data = NULL;
		res->error = YA_EXISTE;
		res->path = args->ped->path;
	} else {
		res->data = &error;
		res->error = OK;
		res->path = strdup(args->ped->path);
	}
	enviarRespuestaCreateFile(res, args->client, args->lockeos);
	free(args->ped->path);
	free(args->ped);
	free(args);
}

void procesarPedidoCreateDir(args_t* args) {

	if (strlen(args->ped->path) > 40) {
		resultado_t* res = malloc(sizeof(resultado_t));
		res->data = NULL;
		res->error = NAME_TOO_LONG;
		//log_debug(logueo, "proceso un pedido de create File");
		enviarRespuestaCreateDir(res, args->client, args->lockeos);
		return;
	}
	inode_t* inode = ext2_get_Inode(args->ped->path, NULL);
	if (inode != NULL) {
		resultado_t* res = malloc(sizeof(resultado_t));
		res->data = NULL;
		res->error = YA_EXISTE;
		enviarRespuestaCreateDir(res, args->client, args->lockeos);
		return;
	}
	resultado_t* res = ext2_mkdir(args);

//	if(res==NULL){
//		res=calloc(1,sizeof(resultado_t));
//		res->data=NULL;
//		res->error=YA_EXISTE;
//	}else
	if (*((uint32_t*) res->data) != 0) {
		res->data = NULL;
		res->error = YA_EXISTE;
	} else {
		res->error = OK;
	}
	enviarRespuestaCreateDir(res, args->client, args->lockeos);
	free(args->ped->path);
	free(args->ped);
	free(args);

}
void procesarPedidoDeleteFile(args_t* args) {
	resultado_t* res = malloc(sizeof(resultado_t));
	res->path = strdup(args->ped->path);

	if (strlen(args->ped->path) > 40) {
		res->data = NULL;
		res->error = NAME_TOO_LONG;
		//log_debug(logueo, "proceso un pedido de create File");
		enviarRespuestaDeleteFile(res, args->client, args->lockeos);
		return;
	}

	int32_t error = ext2_removeFile(args);

	if (error == -1) {
		res->data = NULL;
		res->error = NO_EXISTE;
	} else {
		res->data = &error;
		res->error = OK;
	}
	enviarRespuestaDeleteFile(res, args->client, args->lockeos);
	free(args->ped->path);
	free(args->ped);
	free(args);

}

void procesarPedidoDeleteDir(args_t* args) {
	resultado_t* res = malloc(sizeof(resultado_t));

	res->path = strdup(args->ped->path);
	if (strlen(args->ped->path) > 40) {
		res->data = NULL;
		res->error = NAME_TOO_LONG;
		//log_debug(logueo, "proceso un pedido de create File");
		enviarRespuestaDeleteDir(res, args->client, args->lockeos);
		return;
	}

	int32_t error = ext2_removeDir(args);

	if (error == -2) {
		res->data = NULL;
		res->error = NO_EXISTE;
	} else if (error == -1) {
		res->data = NULL;
		res->error = NO_ESTA_VACIO;
	} else {
		res->data = &error;
		res->error = OK;
	}
	enviarRespuestaDeleteDir(res, args->client, args->lockeos);
	free(args->ped->path);
	free(args->ped);
	free(args);

}

void procesarPedidoReadFile(args_t* args) {

	if (strlen(args->ped->path) > 40) {
		resultado_t* res = malloc(sizeof(resultado_t));
		res->data = NULL;
		res->error = NAME_TOO_LONG;
	}

	if (args->ped->offset == 229376) {
		printf("aca se rompe\n");
	}

	resultado_t* res = ext2_ReadFile(args);

	if (res->data == NULL) {
		res->error = NO_EXISTE;
	} else {
		res->error = OK;
	}
	enviarRespuestaReadFile(res, args->client, args->lockeos);
	free(args->ped->path);
	free(args->ped);
	free(args);

}
void procesarPedidoWriteFile(args_t* args) {

	if (strlen(args->ped->path) > 40) {
		resultado_t* res = malloc(sizeof(resultado_t));
		res->data = NULL;
		res->error = NAME_TOO_LONG;
	}

	resultado_t* res = ext2_writeFile(args);

	if (res->data == NULL) {
		res->error = NO_EXISTE;
	}else if(((int32_t)res->data)==-10){
		res->data=NULL;
		res->error=ESTA_LLENO;
	}
		else {
		res->error = OK;
	}
	enviarRespuestaWriteFile(res, args->client, args);
	free(args->ped->path);
	free(args->ped);
	free(args);

}

void procesarPedidoOpenFile(args_t* args) {
	resultado_t* res = malloc(sizeof(resultado_t));

	if (strcmp(args->ped->path, "/prueba/pepito.txt") == 0) {
		printf("a ver");
	}
	inode_t* inode = ext2_get_Inode(args->ped->path, NULL);

	res->data = NULL;
	if (inode == NULL) {
		res->error = NO_EXISTE;
	} else {
		res->error = OPEN_FILE;
	}
	res->path = strdup(args->ped->path);
	enviarRespuestaOpenFile(res, args->client, args->lockeos);
	free(args->ped->path);
	free(args->ped);
	free(args);

}

void procesarPedidoGetAttr(args_t* args) {
	resultado_t* res = malloc(sizeof(resultado_t));

	if (strcmp(args->ped->path, "/prueba/pepito.txt") == 0) {
		printf("a ver");
	}
	t_respuesta_get_attr data = ext2_getAttr(args->ped->path, args->lockeos);

	res->data = calloc(1, sizeof(t_respuesta_get_attr));
	memcpy(res->data, &data, sizeof(t_respuesta_get_attr));
	res->error = data.type;
	res->path = strdup(args->ped->path);
	enviarRespuestaGetAttr(res, args->client, args->lockeos);
	free(args->ped->path);
	free(args->ped);
	free(args);
}
void procesarPedidoReadDir(args_t* args) {
	resultado_t* res = malloc(sizeof(resultado_t));

	if (strlen(args->ped->path) > 40) {
		res->data = NULL;
		res->error = NAME_TOO_LONG;
	}

	t_list* list = ext2_ReadDir(args->ped->path, args->lockeos);

	if (list == NULL) {
		res->data = NULL;
		res->error = NO_EXISTE;
	} else {
		res->data = list;
		res->error = OK;
	}
	res->path = strdup(args->ped->path);
	enviarRespuestaReadDir(res, args->client, args->lockeos);
	free(args->ped->path);
	free(args->ped);
	free(args);

}
void procesarPedidoTruncate(args_t* args) {

	if (strlen(args->ped->path) > 40) {
		resultado_t* res = malloc(sizeof(resultado_t));
		res->data = NULL;
		res->error = NAME_TOO_LONG;
		res->path = strdup(args->ped->path);
		enviarRespuestaTruncate(res, args->client, args->lockeos);
		free(args->ped->path);
		free(args->ped);
		free(args);
		return;

	}
	resultado_t* res = ext2_truncateFile(args);
	if ((int32_t)res->data == 1) {
		res->data = NULL;
		res->error = NO_EXISTE;
	} else {
		res->error = OK;
	}
	enviarRespuestaTruncate(res, args->client, args->lockeos);
	free(args->ped->path);
	free(args->ped);
	free(args);

}
///////////////////////////////////////PRIVATE FUNCTIOS//////////////////////////////////////////////
static void enviarRespuestaCreateDir(const resultado_t* const res,
		int32_t client, structures_synchronizer_t* locks) {
	open_file_t* file = foundOrAddOpenFileInList(res->path,
			locks->openFileList);
	pthread_rwlock_rdlock(&file->lock);
	t_stream* respuesta;
	if (res->data == NULL) {
		if (res->error == NAME_TOO_LONG) {
			respuesta = serializar_handshake(NAME_TOO_LONG);
		} else if (res->error == YA_EXISTE) {
			respuesta = serializar_handshake(YA_EXISTE);
		}
	} else {
		respuesta = serializar_handshake(OK);
		free(res->data);
	}
	enviar_paquete(client, respuesta->data, respuesta->length);
	free(respuesta->data);
	free(respuesta);
	free(res->path);
	free((void*) res);
	pthread_rwlock_unlock(&file->lock);
	open_file_finishHim(locks->openFileList, file);
}
static void enviarRespuestaDeleteFile(const resultado_t* const res,
		int32_t client, structures_synchronizer_t* locks) {
	open_file_t* file = foundOrAddOpenFileInList(res->path,
			locks->openFileList);
	pthread_rwlock_rdlock(&file->lock);
	t_stream* respuesta;
	if (res->data == NULL) {
		if (res->error == NAME_TOO_LONG) {
			respuesta = serializar_handshake(NAME_TOO_LONG);
		} else if (res->error == NO_EXISTE) {
			respuesta = serializar_handshake(NO_EXISTE);
		}
	} else {
		respuesta = serializar_handshake(OK);
		//free(res->data);
	}
	enviar_paquete(client, respuesta->data, respuesta->length);
	free(respuesta->data);
	free(respuesta);
	free(res->path);
	free((void*) res);
	pthread_rwlock_unlock(&file->lock);
	open_file_finishHim(locks->openFileList, file);
}
static void enviarRespuestaDeleteDir(const resultado_t* const res,
		int32_t client, structures_synchronizer_t* locks) {
	open_file_t* file = foundOrAddOpenFileInList(res->path,
			locks->openFileList);
	pthread_rwlock_rdlock(&file->lock);
	t_stream* respuesta;
	if (res->data == NULL) {
		if (res->error == NAME_TOO_LONG) {
			respuesta = serializar_handshake(NAME_TOO_LONG);
		} else if (res->error == NO_EXISTE) {
			respuesta = serializar_handshake(NO_EXISTE);
		} else if (res->error == NO_ESTA_VACIO) {
			respuesta = serializar_handshake(NO_ESTA_VACIO);
		}
	} else {
		respuesta = serializar_handshake(OK);
		//free(res->data);
	}
	enviar_paquete(client, respuesta->data, respuesta->length);
	free(respuesta->data);
	free(respuesta);
	free(res->path);
	free((void*) res);
	pthread_rwlock_unlock(&file->lock);
	open_file_finishHim(locks->openFileList, file);
}
static void enviarRespuestaCreateFile(const resultado_t* const res,
		int32_t client, structures_synchronizer_t* locks) {
	open_file_t* file = foundOrAddOpenFileInList(res->path,
			locks->openFileList);
	pthread_rwlock_rdlock(&file->lock);
	t_stream* respuesta;
	if (strcmp(res->path, "/354.test") == 0) {
		puts("miremos aca");
	}
	if (res->data == NULL) {
		if (res->error == NAME_TOO_LONG) {
			respuesta = serializar_handshake(NAME_TOO_LONG);
		} else if (res->error == YA_EXISTE) {
			respuesta = serializar_handshake(YA_EXISTE);
		}
		if (res->error == ESTA_LLENO) {
			respuesta = serializar_handshake(ESTA_LLENO);
		}
	} else {
		respuesta = serializar_handshake(OK);
		//free(res->data);
	}
	enviar_paquete(client, respuesta->data, respuesta->length);
	free(respuesta->data);
	free(respuesta);
	free(res->path);
	free((void*) res);
	pthread_rwlock_unlock(&file->lock);
	open_file_finishHim(locks->openFileList, file);
}

static void enviarRespuestaReadFile(const resultado_t* const res,
		int32_t client, structures_synchronizer_t* locks) {
	open_file_t* file = foundOrAddOpenFileInList(res->path,
			locks->openFileList);
	pthread_rwlock_rdlock(&file->lock);
	t_stream* respuesta;
	if (res->data == NULL) {
		if (res->error == NAME_TOO_LONG) {
			respuesta = serializar_handshake(NAME_TOO_LONG);
		} else if (res->error == NO_EXISTE) {
			respuesta = serializar_handshake(NO_EXISTE);
		}
	} else {
		respuesta = serializar_respuesta_file_read(READ_FILE,
				(char*) (res->data), res->bytes);
		free(res->data);
	}
	enviar_paquete(client, respuesta->data, respuesta->length);
	free(respuesta->data);
	free(respuesta);
	free(res->path);
	free((void*) res);
	pthread_rwlock_unlock(&file->lock);
	open_file_finishHim(locks->openFileList, file);
}

static void enviarRespuestaWriteFile(const resultado_t* const res,
		int32_t client, args_t* args) {
	open_file_t* file = foundOrAddOpenFileInList(res->path,
			args->lockeos->openFileList);
	pthread_rwlock_rdlock(&file->lock);
	t_stream* respuesta;
	if (res->data == NULL) {
		if (res->error == NAME_TOO_LONG) {
			respuesta = serializar_handshake(NAME_TOO_LONG);
		} else if (res->error == NO_EXISTE) {
			respuesta = serializar_handshake(NO_EXISTE);
		}else if(res->error==ESTA_LLENO){
			respuesta=serializar_handshake(ESTA_LLENO);
			puts("funco");
		}
	} else {
		respuesta = serializar_respuesta_escribir_archivo(OK, args->ped->bytes);
		free(res->data);
	}
	enviar_paquete(client, respuesta->data, respuesta->length);
	free(respuesta->data);
	free(respuesta);
	free(res->path);
	free((void*) res);
	pthread_rwlock_unlock(&file->lock);
	open_file_finishHim(args->lockeos->openFileList, file);
}

static void enviarRespuestaReadDir(const resultado_t* const res, int32_t client,
		structures_synchronizer_t* locks) {
	open_file_t* file = foundOrAddOpenFileInList(res->path,
			locks->openFileList);
	pthread_rwlock_rdlock(&file->lock);
	t_stream* respuesta;
	if (res->data == NULL) {
		if (res->error == NAME_TOO_LONG) {
			respuesta = serializar_handshake(NAME_TOO_LONG);
		} else if (res->error == NO_EXISTE) {
			respuesta = serializar_handshake(NO_EXISTE);
		}
	} else {
		respuesta = serializar_respuesta_dir_read(READ_DIR,
				(t_list*) (res->data));
	}
	enviar_paquete(client, respuesta->data, respuesta->length);
	free(respuesta->data);
	free(respuesta);
	free(res->path);
	free((void*) res);
	pthread_rwlock_unlock(&file->lock);
	open_file_finishHim(locks->openFileList, file);
}

static void enviarRespuestaGetAttr(const resultado_t* const res, int32_t client,
		structures_synchronizer_t* locks) {
	open_file_t* file = foundOrAddOpenFileInList(res->path,
			locks->openFileList);
	pthread_rwlock_rdlock(&file->lock);
	t_stream* respuesta;
	if (res->error == NO_EXISTE) {
		respuesta = serializar_handshake(NO_EXISTE);
	} else if (res->error == NAME_TOO_LONG) {
		respuesta = serializar_handshake(NAME_TOO_LONG);
	} else {
		respuesta = serializar_respuesta_get_attr(GET_ATTR,
				*((t_respuesta_get_attr*) res->data));
	}
		free(res->data);
	enviar_paquete(client, respuesta->data, respuesta->length);
	free(respuesta->data);
	free(respuesta);
	free(res->path);
	free((void*) res);
	pthread_rwlock_unlock(&file->lock);
	open_file_finishHim(locks->openFileList, file);
}

static void enviarRespuestaOpenFile(const resultado_t* const res,
		int32_t client, structures_synchronizer_t* locks) {
	open_file_t* file = foundOrAddOpenFileInList(res->path,
			locks->openFileList);
	pthread_rwlock_rdlock(&file->lock);
	t_stream* respuesta;
	if (res->error == NO_EXISTE) {
		respuesta = serializar_handshake(NO_EXISTE);
	} else if (res->error == NAME_TOO_LONG) {
		respuesta = serializar_handshake(NAME_TOO_LONG);
	} else {
		respuesta = serializar_handshake(OK);
		//free(res->data);
	}
	enviar_paquete(client, respuesta->data, respuesta->length);
	free(respuesta->data);
	free(respuesta);
	free(res->path);
	free((void*) res);
	pthread_rwlock_unlock(&file->lock);
	open_file_finishHim(locks->openFileList, file);
}

static void enviarRespuestaTruncate(const resultado_t* const res,
		int32_t client, structures_synchronizer_t* locks) {
	open_file_t* file = foundOrAddOpenFileInList(res->path,
			locks->openFileList);
	pthread_rwlock_rdlock(&file->lock);
	t_stream* respuesta;
	if (res->data == NULL) {
		if (res->error == NAME_TOO_LONG) {
			respuesta = serializar_handshake(NAME_TOO_LONG);
		} else if (res->error == NO_EXISTE) {
			respuesta = serializar_handshake(NO_EXISTE);
		} else if (res->error == OK) {
			respuesta = serializar_handshake(OK);
			//free(res->data);
		}
	} else {
		respuesta = serializar_handshake(res->error);
		//free(res->data);
	}
	enviar_paquete(client, respuesta->data, respuesta->length);
	free(respuesta->data);
	free(respuesta);
	free(res->path);
	free(res->data);
	free((void*) res);
	pthread_rwlock_unlock(&file->lock);
	open_file_finishHim(locks->openFileList, file);
}

