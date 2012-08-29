/*
 * socketHandler.c
 *
 *  Created on: 07/06/2012
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

extern t_log* logueo;

individualJob_t* functionIdentifyAndOperate(t_header* header, t_stream* strm,
		int32_t clientes, structures_synchronizer_t* locks) {
	switch (header->type) {
	case OPEN_FILE: {
//		t_stream* strm = malloc(strlen(data) + 1);
//		strm->data = data;
		char* pedido = deserializar_pedido_nombre(strm);
		individualJob_t* job = malloc(sizeof(individualJob_t));
		pedido_t* ped = malloc(sizeof(pedido_t));
		ped->path = strdup(pedido);
		ped->client = clientes;
		job->ped = ped;
		job->locks = locks;
		job->procesarPedido = procesarPedidoOpenFile;
		//log_debug(logueo, "me llego un pedido de getattr");
		free(pedido);
		char mensaje[255];
		sprintf(mensaje, "Open File: Path: %s", ped->path);
		log_debug(logueo, mensaje);
		return job;

		break;
	}
	case RELEASE_FILE: {

		break;
	}
	case WRITE_FILE: {
//		t_stream* strm = malloc(strlen(data) + 1);
//		strm->data = data;
		t_pedido_escribir_archivo* pedido =
				deserializar_pedido_escribir_archivo(strm->data,
						header->length);
		individualJob_t* job = malloc(sizeof(individualJob_t));
		pedido_t* ped = malloc(sizeof(pedido_t));
		ped->path = strdup(pedido->nombre_de_archivo);
		ped->offset = pedido->offset;
		ped->buffer = pedido->buffer;
		ped->size = pedido->size;
		ped->bytes = pedido->size;
		ped->client = clientes;
		job->locks = locks;
		job->ped = ped;
		job->procesarPedido = procesarPedidoWriteFile;
		char mensaje[255];
		sprintf(mensaje, "Write File: Path %s Offset %u Size %u", ped->path,
				ped->offset, ped->size);
		log_debug(logueo, mensaje);

		free(pedido->nombre_de_archivo);
		free(pedido);
		free(strm);
		return job;
		break;
	}
	case READ_FILE: {
//		t_stream* strm = malloc(strlen(data) + 1);
//		strm->data = data;
		t_pedido_file_read* pedido = deserializar_pedido_file_read(strm);
		individualJob_t* job = malloc(sizeof(individualJob_t));
		pedido_t* ped = malloc(sizeof(pedido_t));
		ped->path = strdup(pedido->path);
		ped->offset = pedido->offset;
		ped->size = pedido->size;
		ped->bytes = pedido->size;
		ped->client = clientes;
		job->locks = locks;
		job->ped = ped;
		job->procesarPedido = procesarPedidoReadFile;
		char mensaje[255];
		sprintf(mensaje, "Read File: Path %s Offset %u Size %u", ped->path,
				ped->offset, ped->size);
		log_debug(logueo, mensaje);

		free(pedido->path);
		free(pedido);
		return job;
		break;
	}
	case READ_DIR: {
//		t_stream* strm = malloc(strlen(data) + 1);
//		strm->data = data;
		char* pedido = deserializar_pedido_nombre(strm);
		individualJob_t* job = malloc(sizeof(individualJob_t));
		pedido_t* ped = malloc(sizeof(pedido_t));
		ped->path = strdup(pedido);
		ped->client = clientes;
		job->ped = ped;
		job->locks = locks;
		job->procesarPedido = procesarPedidoReadDir;
		char mensaje[255];
		sprintf(mensaje, "Read Dir: Path %s ", ped->path);
		log_debug(logueo, mensaje);

		free(pedido);
		return job;
		break;
	}
	case TRUNCATE: {

//		t_stream* strm = malloc(strlen(data) + 1);
//		strm->data = data;
		t_pedido_offset_nombre* pedido = deserializar_pedido_offset_nombre(strm,
				header->length);
		individualJob_t* job = malloc(sizeof(individualJob_t));
		pedido_t* ped = malloc(sizeof(pedido_t));
		ped->path = strdup(pedido->path);
		ped->offset = pedido->offset;
		ped->client = clientes;
		job->locks = locks;
		job->ped = ped;
		job->procesarPedido = procesarPedidoTruncate;
		char mensaje[255];
		sprintf(mensaje, "Truncate File: Path %s Offset %u ", ped->path,
				ped->offset);
		log_debug(logueo, mensaje);

		free(pedido->path);
		free(pedido);
		return job;
		break;
	}
	case BORRAR_FILE: {
//		t_stream* strm = malloc(strlen(data) + 1);
//		strm->data = data;
		char* pedido = deserializar_pedido_nombre(strm);
		individualJob_t* job = malloc(sizeof(individualJob_t));
		pedido_t* ped = malloc(sizeof(pedido_t));
		ped->path = strdup(pedido);
		ped->client = clientes;
		job->locks = locks;
		job->ped = ped;
		job->procesarPedido = procesarPedidoDeleteFile;
		char mensaje[255];
		sprintf(mensaje, "Remove File: Path %s ", ped->path);
		log_debug(logueo, mensaje);

		free(pedido);
		return job;
		break;
	}
	case BORRAR_DIR: {
//		t_stream* strm = malloc(strlen(data) + 1);
//		strm->data = data;
		char* pedido = deserializar_pedido_nombre(strm);
		individualJob_t* job = malloc(sizeof(individualJob_t));
		pedido_t* ped = malloc(sizeof(pedido_t));
		ped->path = strdup(pedido);
		ped->client = clientes;
		job->locks = locks;
		job->ped = ped;
		job->procesarPedido = procesarPedidoDeleteDir;
		char mensaje[255];
		sprintf(mensaje, "Remove Dir: Path %s ", ped->path);
		log_debug(logueo, mensaje);

		free(pedido);
		return job;
		break;
	}
	case CREATE_FILE: {
//		t_stream* strm = malloc(strlen(data) + 1);
//		strm->data = data;
		t_pedido_modo_nombre* pedido = deserializar_pedido_modo_nombre(strm);
		individualJob_t* job = malloc(sizeof(individualJob_t));
		pedido_t* ped = malloc(sizeof(pedido_t));
		ped->path = strdup(pedido->path);
		ped->client = clientes;
		ped->mode = pedido->mode;
		job->locks = locks;
		job->ped = ped;
		job->procesarPedido = procesarPedidoCreateFile;
		char mensaje[255];
		sprintf(mensaje, "Create File: Path %s ", ped->path);
		log_debug(logueo, mensaje);

		free(pedido->path);
		free(pedido);
		return job;
		break;
	}
	case CREATE_DIR: {
//		t_stream* strm = malloc(strlen(data) + 1);
//		strm->data = data;
		t_pedido_modo_nombre* pedido = deserializar_pedido_modo_nombre(strm);
		individualJob_t* job = malloc(sizeof(individualJob_t));
		pedido_t* ped = malloc(sizeof(pedido_t));
		ped->path = strdup(pedido->path);
		ped->client = clientes;
		ped->mode = pedido->mode;
		job->locks = locks;
		job->ped = ped;
		job->procesarPedido = procesarPedidoCreateDir;
		char mensaje[255];
		sprintf(mensaje, "Create Dir: Path %s ", ped->path);
		log_debug(logueo, mensaje);

		free(pedido->path);
		free(pedido);
		return job;
		break;
	}
	case GET_ATTR: {
//		t_stream* strm = malloc(strlen(data) + 1);
//		strm->data = data;
		char* pedido = deserializar_pedido_nombre(strm);
		individualJob_t* job = malloc(sizeof(individualJob_t));
		pedido_t* ped = malloc(sizeof(pedido_t));
		ped->path = strdup(pedido);
		ped->client = clientes;
		job->locks = locks;
		job->ped = ped;
		job->procesarPedido = procesarPedidoGetAttr;
		char mensaje[255];
		sprintf(mensaje, "Get Attr: Path %s ", ped->path);
		log_debug(logueo, mensaje);

		free(pedido);
		return job;
		break;
	}
	default:
		perror("Error al procesar pedido: Pedido Invalido");
		break;
	}
	return NULL;
}
