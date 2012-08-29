/*
 * serializadores.c
 *
 *  Created on: 02/06/2012
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
#include "sockear.h"
#include"serializadores.h"

t_header* deserializadorHeader(char* data) {
	t_header *header = malloc(sizeof(t_header));
	memcpy(&header->type, data, 1); //type
	memcpy(&header->length, data + 1, 2); //payloadLength

	return header;
}

uint8_t*deserializar_handshake(char* data, int payloadlength)

  {uint8_t *handshake =malloc(sizeof(uint8_t));



  memcpy(handshake,data, sizeof(uint8_t));

  return handshake;
}

char* deserializar_pedido_nombre(t_stream* stream) {
	//char* self=malloc((uint16_t)(stream->data+1));
	int32_t offset=3,tmp = 0;
	char* self;
	for (tmp = 0; (stream->data+offset)[tmp - 1] != '\0'; tmp++);
	self = malloc(tmp);
	memcpy(self, stream->data, tmp);

	return self;
}

//t_stream* serializar_pedido_file_write(const char *path, const char *buffer,
//		size_t size, off_t desplazamiento, struct fuse_file_info *info) {
//
//	t_stream* stream = malloc(sizeof(t_stream));
//
//	//uint32_t payloadlenght = sizeof(size_t) + sizeof(off_t) + sizeof(size_t)
//	//		+ sizeof(uint8_t) + size + strlen(path) + 1;
//	uint32_t payloadlenght=strlen(path)+1+strlen(buffer)+1+sizeof(uint32_t)+sizeof(uint32_t);
//	uint32_t headerlenght = sizeof(uint8_t) + sizeof(uint16_t);
//
//	char *data = malloc(headerlenght + payloadlenght);
//
//	uint32_t tipo = WRITE_FILE;
//	uint32_t tmp = 0;
//
//	memcpy(stream->data, &tipo, tmp = sizeof(uint8_t));
//	uint32_t offset = tmp;
//	memcpy(stream->data + offset, &payloadlenght, tmp = sizeof(uint16_t));
//	offset = offset + tmp;
//	memcpy(stream->data + offset, &size, tmp = sizeof(size_t));
//	offset = offset + tmp;
//	memcpy(stream->data + offset, &desplazamiento, tmp = sizeof(off_t));
//	offset = offset + tmp;
//	memcpy(stream->data + offset, &info, tmp = sizeof(uint8_t));
//	offset = offset + tmp;
//	memcpy(stream->data + offset, buffer, tmp = size);
//	offset = offset + tmp;
//	memcpy(stream->data + offset, path, strlen(path) + 1);
//
//	stream->data = data;
//	stream->length = headerlenght + payloadlenght;
//
//	return stream;
//}

size_t deserializar_respuesta_escribir_archivo(char* stream, int longitud){
	//Devuelve un puntero a t_respuesta_escribir_archivo
	size_t bytes;
	memcpy(&bytes,stream,longitud);

	return bytes;
}

t_stream* serializar_pedido_escribir_archivo(const char *nombre_de_archivo, const char *buffer, size_t size, off_t desplazamiento){
	//TIPO|PAYLOADLENGHT|SIZE|DESPLAZAMIENTO|BUFFER|NOMBRE

	uint32_t header_lenght = sizeof(int8_t) + sizeof(uint16_t);
	uint32_t payload_lenght = sizeof(size_t) + sizeof(off_t) + size + strlen(nombre_de_archivo) + 1;

	t_stream* stream = calloc(1, sizeof(t_stream));
	stream->data = calloc(header_lenght + payload_lenght, sizeof(char));

	uint32_t tipo = WRITE_FILE;
	uint32_t tmp = 0;

	memcpy( stream->data, &tipo, tmp = sizeof(int8_t) );
	uint32_t offset = tmp;
	memcpy( stream->data + offset, &payload_lenght, tmp = sizeof(uint16_t) );
	offset += tmp;
	memcpy( stream->data + offset, &size, tmp = sizeof(size_t) );
	offset += tmp;
	memcpy( stream->data + offset, &desplazamiento, tmp = sizeof(off_t) );
	offset += tmp;
	memcpy( stream->data + offset , buffer, tmp = size);
	offset += tmp;
	memcpy( stream->data + offset, nombre_de_archivo, strlen(nombre_de_archivo) + 1);
	stream->length = header_lenght + payload_lenght;

	return stream;
}

t_pedido_file_write* deserializar_pedido_file_write(t_stream *stream) {
	t_pedido_file_write* file_write = malloc(sizeof(t_pedido_file_write));

	int32_t tmp = 0;
	int32_t offset = 0;
	memcpy(&file_write->size, stream->data, tmp = sizeof(size_t));
	offset = offset + tmp;
	memcpy(&file_write->offset, stream->data + offset, tmp = sizeof(off_t));
	offset = offset + tmp;
	memcpy(&file_write->direct_io, stream->data + offset, tmp =
			sizeof(uint8_t));
	offset = offset + tmp;
	file_write->buffer = malloc(file_write->size);
	memcpy(file_write->buffer, stream->data + offset, tmp = file_write->size);
	offset = offset + tmp;
	for (tmp = 1; (file_write->path + offset)[tmp - 1] != '\0'; tmp++)
		;
	file_write->path = malloc(tmp);
	memcpy(file_write->path, stream->data + offset, tmp);

	return file_write;
}

t_stream* serializar_pedido_file_read(const char *path, size_t size,
		off_t desplazamiento, struct fuse_file_info *info) {

	t_stream* stream = malloc(sizeof(t_stream));
	uint32_t payloadlenght = sizeof(size_t) + sizeof(off_t) + sizeof(uint8_t)
			+ strlen(path) + 1;
	uint32_t headerlenght = sizeof(uint8_t) + sizeof(uint16_t);

	char *data = malloc(headerlenght + payloadlenght);

	uint32_t tipo = READ_FILE;
	int offset = 0, tmp = 0;
	memcpy(data, &tipo, tmp = sizeof(uint8_t));
	offset += tmp;
	memcpy(data + offset, &payloadlenght, tmp = sizeof(uint16_t));
	offset += tmp;
	memcpy(data + offset, &size, tmp = sizeof(size_t));
	offset += tmp;
	memcpy(data + offset, &desplazamiento, tmp = sizeof(off_t));
	offset += tmp;
	memcpy(data + offset, &info, tmp = sizeof(uint8_t));
	offset += tmp;
	memcpy(data + offset, path, strlen(path) + 1);

	stream->data = data;
	stream->length = headerlenght + payloadlenght;

	return stream;
}

t_pedido_file_read* deserializar_pedido_file_read(t_stream *stream) {
	t_pedido_file_read* file_read = malloc(sizeof(t_pedido_file_read));
	int offset = 0, tmp = 0;
	memcpy(&file_read->size, stream->data, tmp = sizeof(size_t));
	offset += tmp;
	memcpy(&file_read->offset, stream->data + offset, tmp = sizeof(off_t));
	offset += tmp;
	memcpy(&file_read->direct_io, stream->data + offset, tmp = sizeof(uint8_t));
	offset += tmp;
	for (tmp = 1; (stream->data + offset)[tmp - 1] != '\0'; tmp++)
		;
	file_read->path = malloc(tmp);
	memcpy(file_read->path, stream->data + offset, tmp);

	return file_read;
}

t_stream* serializar_pedido_nombre(uint8_t type, const char*path) {
	char* data = malloc(sizeof(uint8_t) + strlen(path) + 1 + sizeof(uint16_t));
	t_stream* stream = malloc(sizeof(t_stream));
	int32_t offset = 0, tmp = 0;
	memcpy(data, &type, tmp = sizeof(uint8_t));
	offset = tmp;
	uint16_t paylodlen = strlen(path) + 1;
	memcpy(data + offset, &paylodlen, tmp = sizeof(uint16_t));
	offset += tmp;
	memcpy(data + offset, path, tmp = strlen(path) + 1);
	stream->data = data;
	stream->length = offset + tmp;

	return stream;
}

t_stream *serializar_pedido_offset_nombre(const char *path, off_t offset) {
	uint32_t payload_lenght = sizeof(off_t) + strlen(path) + 1;
		uint32_t header_lenght = sizeof(int8_t) + sizeof(uint16_t);

		t_stream* stream = calloc(1, sizeof(t_stream));
		stream->data = calloc(header_lenght + payload_lenght, sizeof(char));

		uint8_t tipo = TRUNCATE;

		memcpy( stream->data, &tipo, sizeof(uint8_t) );
		memcpy( stream->data + sizeof(int8_t), &payload_lenght, sizeof(uint16_t) );
		memcpy( stream->data + header_lenght, &offset, sizeof(off_t) );
		memcpy( stream->data + header_lenght + sizeof(off_t), path, strlen(path) + 1);

		stream->length = header_lenght + payload_lenght;

		return stream;
}

t_pedido_offset_nombre* deserializar_pedido_offset_nombre(t_stream* stream) {
	t_pedido_offset_nombre *offset_nombre = malloc(
			sizeof(t_pedido_offset_nombre));
	int offset = 0, tmp = 0;
	memcpy(&offset_nombre->offset, stream->data, tmp = sizeof(off_t)); //payload
	offset = +tmp;
	for (tmp = 1; (stream->data + offset)[tmp - 1] != '\0'; tmp++)
		;
	offset_nombre->path = malloc(tmp);
	memcpy(offset_nombre->path, stream->data + offset, tmp); //payload

	return offset_nombre;
}

t_stream* serializar_pedido_modo_nombre(uint8_t type,const char *path, mode_t mode) {
	char* data = malloc(
			sizeof(uint8_t) + sizeof(uint16_t) + strlen(path) + 1
					+ sizeof(mode_t));
	t_stream* stream = malloc(sizeof(t_stream));
	int offset = 0, tmp = 0;
	//uint8_t type = CREATE_DIR;
	uint16_t payloadlength = sizeof(mode_t) + strlen(path) + 1;
	memcpy(data, &type, tmp = sizeof(uint8_t));
	offset = tmp;
	memcpy(data + offset, &payloadlength, tmp = sizeof(uint16_t));
	offset += tmp;
	memcpy(data + offset, &mode, tmp = sizeof(mode_t));
	offset += tmp;
	memcpy(data + offset, path, tmp = strlen(path) + 1);

	stream->data = data;
	stream->length = offset + tmp;

	return stream;
}

t_pedido_modo_nombre* deserializar_pedido_modo_nombre(t_stream *stream) {
	t_pedido_modo_nombre *modo_nombre = malloc(sizeof(t_pedido_modo_nombre));
	int offset = 0, tmp = 0;
	memcpy(&modo_nombre->mode, stream->data + offset, tmp = sizeof(mode_t)); //payload
	offset += tmp;
	for (tmp = 1; (stream->data + offset)[tmp - 1] != '\0'; tmp++)
		;
	modo_nombre->path = malloc(tmp);
	memcpy(modo_nombre->path, stream->data + offset, tmp); //payload

	return modo_nombre;
}

t_stream* serializar_handshake(uint8_t type) {

	char* data = malloc(3);
	t_stream *stream = malloc(sizeof(t_stream));
	uint8_t headerlenght = sizeof(uint8_t) + sizeof(uint16_t);
	uint16_t payloadlength = 0;
	memcpy(data, &type, sizeof(uint8_t));
	memcpy(data + sizeof(uint8_t), &payloadlength, sizeof(uint16_t));

	stream->length = headerlenght;
	stream->data = data;

	return stream;
}

//t_stream* serializar_respuesta_file_read(uint8_t type,
//		const char*contenido_archivo) {
//	size_t tam = strlen(contenido_archivo) + 1;
//	uint32_t payloadlength = strlen(contenido_archivo) + 1 + sizeof(size_t);
//	uint32_t headerlenght = sizeof(uint8_t) + sizeof(uint16_t);
//	char *data = malloc(headerlenght + payloadlength);
//
//	t_stream *stream = malloc(sizeof(t_stream));
//
//	memcpy(data, &type, sizeof(uint8_t)); //type
//	memcpy(data + sizeof(uint8_t), &payloadlength, sizeof(uint16_t)); //patloadLength
//	memcpy(data + headerlenght, &tam, sizeof(size_t)); //payload
//	memcpy(data + headerlenght + sizeof(size_t), contenido_archivo, tam); //payload
//
//	stream->data = data;
//	stream->length = payloadlength + headerlenght;
//	return stream;
//}

t_respuesta_file_read * deserializar_respuesta_file_read(char *data,
		int payloadlength) {
	t_respuesta_file_read *file = malloc(sizeof(t_respuesta_file_read));
	int offset=0,tmp=0;
	memcpy(&file->tam,data,tmp=sizeof(size_t));
	offset=tmp;
	file->contenido=calloc(file->tam,sizeof(char));
	memcpy(file->contenido,data+offset,file->tam);

	return file;
}

//t_stream* serializar_respuesta_get_attr(uint8_t type, t_respuesta_get_attr attr) {
//
//	t_stream* stream = malloc(sizeof(t_stream));
//	uint8_t headerlenght = sizeof(uint8_t) + sizeof(uint16_t);
//	uint16_t payloadlenght = sizeof(t_respuesta_get_attr);
//	char* data = malloc(headerlenght + payloadlenght);
//
//	memcpy(data, &type, sizeof(uint8_t));
//	memcpy(data + sizeof(uint8_t), &payloadlenght, sizeof(uint16_t));
//
//	uint32_t tmp, offset = 0;
//
//	memcpy(data + headerlenght, &attr.type, tmp = sizeof(uint8_t));
//	offset = offset + tmp;
//	memcpy(data + headerlenght + offset, &attr.attr, tmp = sizeof(struct stat));
//	/*offset =offset + tmp;
//	 memcpy(data + headerlenght + offset, &attr.mode, tmp = sizeof(mode_t));
//	 offset =offset + tmp;
//	 memcpy(data + headerlenght + offset, &attr.nlinks, tmp = sizeof(nlink_t));
//	 offset =offset + tmp;
//	 memcpy(data + headerlenght + offset, &attr.size, tmp = sizeof(size_t));
//	 offset =offset + tmp;
//	 memcpy(data + headerlenght + offset, &attr.blocksize, tmp =
//	 sizeof(blksize_t));
//	 offset =offset + tmp;
//	 memcpy(data + headerlenght + offset, &attr.blockcount, tmp =
//	 sizeof(blkcnt_t));
//	 */
//	stream->length = headerlenght + payloadlenght;
//	stream->data = data;
//	return stream;
//}

t_respuesta_get_attr *deserializar_respuesta_get_attr(char *data,
		int payloadlength) {
	t_respuesta_get_attr *atributos = malloc(sizeof(t_respuesta_get_attr));
	int tmp, offset = 0;
	//memcpy(&atributos->type, data, tmp = sizeof(uint8_t));
	//offset = offset + tmp;
	//memcpy(&atributos->attr, data, tmp = sizeof(struct stat));
	//offset = offset + tmp;
	memcpy(&atributos->nroInodo,data+offset,tmp=sizeof(uint32_t));
	offset+=tmp;
	memcpy(&atributos->mode, data + offset, tmp = sizeof(mode_t));
	offset = offset + tmp;
	memcpy(&atributos->hdlinks, data + offset, tmp = sizeof(uint32_t));
	offset = offset + tmp;
	memcpy(&atributos->size, data + offset, tmp = sizeof(uint32_t));
	offset = offset + tmp;
	memcpy(&atributos->blocksize, data + offset, tmp = sizeof(uint32_t));
	offset = offset + tmp;
	memcpy(&atributos->blocks, data + offset, tmp = sizeof(uint32_t));

	return atributos;
}

t_stream* serializar_respuesta_dir_read(uint8_t type, t_list* dir) {
	t_stream* stream = malloc(sizeof(t_stream));
	uint16_t payloadlenght = 0;
	uint8_t headerlenght = sizeof(uint8_t) + sizeof(uint16_t);
	payloadlenght = list_size(dir);

	char *data = malloc(headerlenght + payloadlenght);

	memcpy(data, &type, sizeof(uint8_t));
	memcpy(data + sizeof(uint8_t), &payloadlenght, sizeof(uint16_t));

	uint32_t i = 0;
	int offset = headerlenght;
	int tmp = 0;
	while (!list_is_empty(dir)) {
		char* nombreDir = list_get(dir, i);
		memcpy(data + offset, nombreDir + '#', tmp = strlen(nombreDir) + 2);
		offset = offset + tmp;
		i++;
	}

	stream->length = headerlenght + payloadlenght;
	stream->data = data;

	return stream;
}
t_respuesta_file_write* deserializar_respuesta_file_write(char* stream, int length){

	t_respuesta_file_write* respuesta= malloc(sizeof(t_respuesta_file_write));

	memcpy(&respuesta->bytes, stream, length);

	return respuesta;
}

t_stream* serializar_respuesta_file_write(int8_t tipo, size_t bytes_escritos){

	t_stream* stream = malloc(sizeof(t_stream));
	uint8_t header_lenght = sizeof(int8_t) + sizeof(uint16_t);
	uint16_t payload_lenght = sizeof(size_t);

	stream->data = malloc(sizeof(header_lenght) + sizeof(payload_lenght));
	memcpy(stream->data, &tipo, sizeof(int8_t));
	memcpy(stream->data + sizeof(int8_t), &payload_lenght, sizeof(uint16_t));
	memcpy(stream->data + header_lenght, &bytes_escritos, sizeof(size_t));

	stream->length = header_lenght + payload_lenght;

	return stream;
}
