/*
 * serializadores.h
 *
 *  Created on: 03/06/2012
 *      Author: utnso
 */

#ifndef SERIALIZADORES_H_
#define SERIALIZADORES_H_
#include <fuse.h>
#include "collections/list.h"

typedef struct {
	uint32_t length;
	char *data;
} t_stream;

typedef struct {
	uint8_t type;
	uint16_t length;
} t_header;

typedef struct {
	char *path;
	off_t offset;
} t_pedido_offset_nombre;

typedef struct {
	char *path;
	mode_t mode;
} t_pedido_modo_nombre;

typedef struct {
	char *path;
	char *buffer;
	size_t size;
	off_t offset;
	uint8_t direct_io;
} t_pedido_file_write;

typedef struct {
	char *path;
	size_t size;
	off_t offset;
	uint8_t direct_io;
} t_pedido_file_read;

typedef struct {
	uint8_t type;
	uint32_t nroInodo;
	mode_t mode;
	uint32_t hdlinks;
	uint32_t size;
	uint32_t blocksize;
	uint32_t blocks;

}__attribute__ ((__packed__)) t_respuesta_get_attr;

typedef struct{
	char *nombre_de_archivo;
	char *buffer;
	size_t size;
	off_t offset;
} __attribute__ ((__packed__)) t_pedido_escribir_archivo;
t_pedido_escribir_archivo* deserializar_pedido_escribir_archivo(char* stream, int longitud);
t_stream* serializar_respuesta_escribir_archivo(int8_t tipo, size_t bytes_escritos);
typedef struct {
	char* path;
} t_pedido_nombre;

typedef struct {
	char* contenido;
	size_t tam;
} t_respuesta_file_read;

t_header* deserializadorHeader(char*);

char* deserializar_pedido_nombre(t_stream*);

uint8_t*deserializar_handshake(char* data, int payloadlength);

t_pedido_file_write* deserializar_pedido_file_write(t_stream *stream);

t_stream* serializar_respuesta_file_write(int8_t tipo, size_t bytes_escritos);

t_pedido_file_read* deserializar_pedido_file_read(t_stream *stream);

t_pedido_offset_nombre* deserializar_pedido_offset_nombre(t_stream* stream,int);

t_pedido_modo_nombre* deserializar_pedido_modo_nombre(t_stream *stream);

t_stream* serializar_handshake(uint8_t type);

t_stream* serializar_respuesta_file_read(uint8_t type,
		const char*contenido_archivo,size_t);


t_stream* serializar_respuesta_get_attr(uint8_t type, t_respuesta_get_attr);


t_stream* serializar_respuesta_dir_read(uint8_t type, t_list* dir);

typedef enum {
	OK,
	WRITE_FILE,
	READ_FILE,
	READ_DIR,
	TRUNCATE,
	BORRAR_FILE,
	BORRAR_DIR,
	CREATE_FILE,
	CREATE_DIR,
	GET_ATTR,
	OPEN_FILE,
	RELEASE_FILE,
	NO_EXISTE,
	YA_EXISTE,
	NAME_TOO_LONG,
	ERR,
	NO_ESTA_VACIO,
	ESTA_LLENO
} NIPC_type;

#endif /* SERIALIZADORES_H_ */
