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
#include "string.h"

t_header* deserializadorHeader(char* data) {
	t_header *header = malloc(sizeof(t_header));
	memcpy(&header->type, data, 1); //type
	memcpy(&header->length, data + 1, 2); //payloadLength
	free(data);
	return header;
}

uint8_t*deserializar_handshake(char* data, int payloadlength)

{
	uint8_t *handshake = malloc(sizeof(uint8_t));

	memcpy(handshake, data, sizeof(uint8_t));
	free(data);
	return handshake;
}

char* deserializar_pedido_nombre(t_stream* stream) {
	int32_t offset = 0, tmp = 0;
	char* self;
	for (tmp = 1; (stream->data + offset)[tmp - 1] != '\0'; ++tmp)
		;
	self = malloc(tmp);
	memcpy(self, stream->data, tmp);
	free(stream->data);
	free(stream);
	return self;
}

t_pedido_escribir_archivo* deserializar_pedido_escribir_archivo(char* stream,
		int longitud) {
	//Devuelve un puntero a t_pedido_escibir_archivo que contiene los campos size, offset, buffer, y nombre_de_archivo
	t_pedido_escribir_archivo* pedido = calloc(1,
			sizeof(t_pedido_escribir_archivo));
	int32_t tmp = 0;
	int32_t offset = 0;
	memcpy(&pedido->size, stream, tmp = sizeof(size_t));
	offset += tmp;
	memcpy(&pedido->offset, stream + offset, tmp = sizeof(off_t));
	offset += tmp;
	pedido->buffer = calloc(pedido->size, sizeof(char));
	memcpy(pedido->buffer, stream + offset, tmp = pedido->size);
	offset += tmp;
	pedido->nombre_de_archivo = calloc(longitud - offset, sizeof(char));
	memcpy(pedido->nombre_de_archivo, stream + offset, longitud - offset);
	free(stream);
	return pedido;
}

t_stream* serializar_respuesta_escribir_archivo(int8_t tipo,
		size_t bytes_escritos) {
	//TIPO|PAYLOADLENGHT|BYTESESCRITOS
	t_stream* stream = calloc(1, sizeof(t_stream));
	uint8_t header_lenght = sizeof(int8_t) + sizeof(uint16_t);
	uint16_t payload_lenght = sizeof(size_t);

	stream->data = calloc(header_lenght + payload_lenght, sizeof(char));
	memcpy(stream->data, &tipo, sizeof(int8_t));
	memcpy(stream->data + sizeof(int8_t), &payload_lenght, sizeof(uint16_t));
	memcpy(stream->data + header_lenght, &bytes_escritos, sizeof(size_t));

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
	free(stream->data);
	free(stream);
	return file_write;
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
	free(stream->data);
	free(stream);
	return file_read;
}

t_pedido_offset_nombre* deserializar_pedido_offset_nombre(t_stream* stream,
		int longi) {
	t_pedido_offset_nombre* pedido = calloc(1, sizeof(t_pedido_offset_nombre));
	pedido->path = calloc(longi - sizeof(off_t), sizeof(char));
	memcpy(&pedido->offset, stream->data, sizeof(off_t));
	memcpy(pedido->path, stream->data + sizeof(off_t), longi - sizeof(off_t));
	return pedido;
	free(stream->data);
	free(stream);
	return pedido;
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
	free(stream->data);
	free(stream);
	return modo_nombre;
}

t_stream* serializar_handshake(uint8_t type) {

	t_stream *stream = malloc(sizeof(t_stream));
	stream->data = malloc(3);
	uint8_t headerlenght = sizeof(uint8_t) + sizeof(uint16_t);
	uint16_t payloadlength = 0;
	memcpy(stream->data, &type, sizeof(uint8_t));
	memcpy(stream->data + sizeof(uint8_t), &payloadlength, sizeof(uint16_t));

	stream->length = headerlenght;
	return stream;
}

t_stream* serializar_respuesta_file_read(uint8_t type,
		const char*contenido_archivo,size_t tam) {
	uint32_t payloadlength = tam + sizeof(size_t);
	uint32_t headerlenght = sizeof(uint8_t) + sizeof(uint16_t);

	t_stream *stream = malloc(sizeof(t_stream));
	stream->data = malloc(headerlenght + payloadlength);

	memcpy(stream->data, &type, sizeof(uint8_t)); //type
	memcpy(stream->data + sizeof(uint8_t), &payloadlength, sizeof(uint16_t)); //patloadLength
	memcpy(stream->data + headerlenght, &tam, sizeof(size_t)); //payload
	memcpy(stream->data + headerlenght + sizeof(size_t), contenido_archivo,
			tam); //payload

	stream->length = payloadlength + headerlenght;
	return stream;
}

t_stream* serializar_respuesta_file_write(int8_t tipo, size_t bytes_escritos) {

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

t_stream* serializar_respuesta_get_attr(uint8_t type, t_respuesta_get_attr attr) {

	t_stream* stream = malloc(sizeof(t_stream));
	uint8_t headerlenght = sizeof(uint8_t) + sizeof(uint16_t);
	uint16_t payloadlenght = sizeof(t_respuesta_get_attr);
	stream->data = calloc(1,headerlenght + payloadlenght);

	memcpy(stream->data, &type, sizeof(uint8_t));
	memcpy(stream->data + sizeof(uint8_t), &payloadlenght, sizeof(uint16_t));

	uint32_t tmp, offset = 0;

	//memcpy(stream->data + headerlenght, &attr.type, tmp = sizeof(uint8_t));
	//offset = offset + tmp;
	//memcpy(stream->data + headerlenght + offset, &attr, tmp =
	//	sizeof(struct stat));
	//offset =offset + tmp;
	memcpy(stream->data + headerlenght + offset, &attr.nroInodo, tmp =
			sizeof(uint32_t));
	offset += tmp;
	memcpy(stream->data + headerlenght + offset, &attr.mode, tmp =
			sizeof(mode_t));
	offset = offset + tmp;
	memcpy(stream->data + headerlenght + offset, &attr.hdlinks, tmp =
			sizeof(uint32_t));
	offset = offset + tmp;
	memcpy(stream->data + headerlenght + offset, &attr.size, tmp =
			sizeof(uint32_t));
	offset = offset + tmp;
	memcpy(stream->data + headerlenght + offset, &attr.blocksize, tmp =
			sizeof(uint32_t));
	offset = offset + tmp;
	memcpy(stream->data + headerlenght + offset, &attr.blocks, tmp =
			sizeof(uint32_t));

	stream->length = headerlenght + payloadlenght;
	return stream;
}

t_stream* serializar_respuesta_dir_read(uint8_t type, t_list* dir) {
	if (dir->elements_count == 0 || dir == NULL) {
		printf("sos un boludo la lista esta vacia gil!\n");
		abort();
	}
	t_stream* stream = malloc(sizeof(t_stream));
	uint8_t headerlenght = sizeof(uint8_t) + sizeof(uint16_t);
	uint32_t j;
	uint32_t sumatoriaDeStrlen = 0;
	for (j = 0; list_get(dir, j) != NULL; j++) {
		sumatoriaDeStrlen += strlen(list_get(dir, j)) + 1;
	}
	sumatoriaDeStrlen += 1;
	uint16_t payloadlenght = sumatoriaDeStrlen;

	stream->data = malloc(headerlenght + payloadlenght);

	memcpy(stream->data, &type, sizeof(uint8_t));
	memcpy(stream->data + sizeof(uint8_t), &payloadlenght, sizeof(uint16_t));

	uint32_t offset = 0;
	uint32_t tmp = 0, i = 0;
	do {
		memcpy(stream->data + headerlenght + offset, list_get(dir, i), tmp =
				strlen(list_get(dir, i)));
		offset += tmp;
		i++;
		if (list_get(dir, i) != NULL) {
			memcpy(stream->data + headerlenght + offset, "#", 1);
			offset += 1;
		}
	} while (list_get(dir, i) != NULL);
	memcpy(stream->data + headerlenght + offset, "#", 2);

	stream->length = headerlenght + payloadlenght;
	list_destroy_and_destroy_elements(dir,(void*)free);
	return stream;
}

