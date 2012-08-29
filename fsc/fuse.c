/*
 * fuseClient.c
 *
 *  Created on: 16/05/2012
 *      Author: utnso
 */

/*Acá se deberian ejecutar las operaciones que el cliente quiere hacer
 * con el archivo.
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <libmemcached/memcached.h>
#include <fuse.h>

#include "sockear.h"
#include "serializadores.h"
#include "funcionesFuse.h"
#include "commons/string.h"
#include "commons/log.h"
#include "memcachear.h"
#include "connectionPool.h"

#define ERROR_CONNECTION -1

extern t_log* teLogueo;
extern connections_structures_t* pools;

int create_file(const char *path, mode_t mode, struct fuse_file_info *info) {
	int exit = 0;
	char key[255] = "LC";
	char* fatherPath = getFatherPath(path);
	strcat(key, fatherPath);
	memcached_st* memcached = connectionPool_extractAvaibleMemcached(pools);
	memcached_delete(memcached, (const char*) key, strlen(key), (time_t) 0);
	char mensajeCache[255];
	sprintf(mensajeCache, "Operacion: Actualizar Cache - path:%s", path);
	log_debug(teLogueo, mensajeCache);
	//memcached_finishHer(memcached, &servers);
	connectionPool_depositeAvaibleMemcached(pools, memcached);
	t_stream *stream = serializar_pedido_modo_nombre(CREATE_FILE, path, mode);
	int descriptor = connectionPool_extractAvaibleSocket(pools);
	enviar_paquete(descriptor, stream->data, stream->length);
	char mensaje[255];
	sprintf(mensaje, "Operacion: Crear Archivo - path:%s", path);
	log_debug(teLogueo, mensaje);
	char *mensaje_recibido = recibir_paquete(descriptor, 3);
	t_header *header = deserializadorHeader(mensaje_recibido);
	if (header->type == YA_EXISTE) {
		connectionPool_depositeAvaibleSocket(pools, descriptor);

		perror("No existe el directorio");
		return -EEXIST;
	}
	if (header->type == NAME_TOO_LONG) {
		connectionPool_depositeAvaibleSocket(pools, descriptor);

		perror("El nombre es demasiado largo");
		return -ENAMETOOLONG;
	}
	if (header->type == ESTA_LLENO) {
		connectionPool_depositeAvaibleSocket(pools, descriptor);
		perror("esta lleno\n");
		return -EPERM;
	}
	free(header);
	free(mensaje_recibido);
	free(stream);
	connectionPool_depositeAvaibleSocket(pools, descriptor);
	return exit;
}

int create_dir(const char *path, mode_t modo) {
	int exit = 0;
	char key[255] = "LC";
	char* fatherPath = getFatherPath(path);
	strcat(key, fatherPath);
	memcached_st* memcached = connectionPool_extractAvaibleMemcached(pools);
	memcached_delete(memcached, (const char*) key, strlen(key), (time_t) 0);
	char mensajeCache[255];
	sprintf(mensajeCache, "Operacion: Actualizar Cache - path:%s", path);
	log_debug(teLogueo, mensajeCache);
	//memcached_finishHer(memcached, &servers);
	connectionPool_depositeAvaibleMemcached(pools, memcached);
	t_stream *stream = serializar_pedido_modo_nombre(CREATE_DIR, path, modo);
	int descriptor = connectionPool_extractAvaibleSocket(pools);
	enviar_paquete(descriptor, stream->data, stream->length);
	char mensaje[255];
	sprintf(mensaje, "Operacion: Crear Directorio - path:%s", path);
	log_debug(teLogueo, mensaje);
	char *mensaje_recibido = recibir_paquete(descriptor, 3);
	t_header *header = deserializadorHeader(mensaje_recibido);
	if (header->type == NO_EXISTE) {
		connectionPool_depositeAvaibleSocket(pools, descriptor);
		perror("NO existe el directorio");
		return -ENOENT;
	}
	if (header->type == NAME_TOO_LONG) {
		connectionPool_depositeAvaibleSocket(pools, descriptor);
		perror("El nombre es demasiado largo");
		return -ENAMETOOLONG;
	}
	free(header);
	free(mensaje_recibido);
	free(stream);
	connectionPool_depositeAvaibleSocket(pools, descriptor);
	return exit;
}

int fuse_truncate(const char *path, off_t offset) {
	int exit = 0;
	char key[255] = "AC";
	strcat(key, path);

	printf("Entro a truncate\n");

	memcached_st* memcached = connectionPool_extractAvaibleMemcached(pools);
	memcached_delete(memcached, (const char*) key, strlen(key), (time_t) 0);
	char mensajeCache[255];
	sprintf(mensajeCache, "Operacion: Actualizar Cache - path:%s", path);
	log_debug(teLogueo, mensajeCache);
	//memcached_finishHer(memcached, &servers);
	connectionPool_depositeAvaibleMemcached(pools, memcached);
	t_stream *stream = serializar_pedido_offset_nombre(path, offset);
	int descriptor = connectionPool_extractAvaibleSocket(pools);
	enviar_paquete(descriptor, stream->data, stream->length);
	char mensaje[255];
	uint32_t off = (uint32_t) offset;
	sprintf(mensaje, "Operacion: Truncar Archivo - path:%s - offset:%u", path,
			off);
	log_debug(teLogueo, mensaje);
	puts("aca se bloquea");
	char *mensaje_recibido = recibir_paquete(descriptor, 3);
	puts("no se bloqueo");
	t_header *header = deserializadorHeader(mensaje_recibido);
	if (header->type == NO_EXISTE) {
		connectionPool_depositeAvaibleSocket(pools, descriptor);
		perror("Ya existe el archivo");
		return -ENOENT;
	}
	if (header->type == NAME_TOO_LONG) {
		connectionPool_depositeAvaibleSocket(pools, descriptor);
		perror("El nombre es demasiado largo");
		return -ENAMETOOLONG;
	}
	free(header);
	free(mensaje_recibido);
	free(stream);
	printf("El valor de exit es %d", exit);
	connectionPool_depositeAvaibleSocket(pools, descriptor);

	return exit;
}

int borrarDir(const char *path) {
	int exit = 0;
	char key[255] = "LC";
	char* fatherPath = getFatherPath(path);
	strcat(key, fatherPath);
	memcached_st* memcached = connectionPool_extractAvaibleMemcached(pools);
	memcached_delete(memcached, (const char*) key, strlen(key), (time_t) 0);
	char mensajeCacheActualizarEntradaDir[255];
	sprintf(mensajeCacheActualizarEntradaDir,
			"Operacion: Actualizar Cache - path:%s", path);
	log_debug(teLogueo, mensajeCacheActualizarEntradaDir);
	char key2[255] = "AC";
	strcat(key2, path);
	memcached_delete(memcached, (const char*) key2, strlen(key2), (time_t) 0);
	char mensajeCacheActualizarAtributos[255];
	sprintf(mensajeCacheActualizarAtributos,
			"Operacion: Actualizar Cache - path:%s", path);
	log_debug(teLogueo, mensajeCacheActualizarAtributos);
	//memcached_finishHer(memcached, &servers);
	connectionPool_depositeAvaibleMemcached(pools, memcached);
	t_stream *stream = serializar_pedido_nombre(BORRAR_DIR, path);
	int descriptor = connectionPool_extractAvaibleSocket(pools);
	enviar_paquete(descriptor, stream->data, stream->length);
	char mensaje[255];
	sprintf(mensaje, "Operacion: Borrar Directorio - path:%s", path);
	log_debug(teLogueo, mensaje);
	char *mensaje_recibido = recibir_paquete(descriptor, 3);
	t_header *header = deserializadorHeader(mensaje_recibido);
	if (header->type == NO_EXISTE) {
		connectionPool_depositeAvaibleSocket(pools, descriptor);
		perror("No existe el directorio");
		return -ENOENT;
	}
	if (header->type == NAME_TOO_LONG) {
		connectionPool_depositeAvaibleSocket(pools, descriptor);
		perror("El nombre es demasiado largo");
		return -ENAMETOOLONG;
	}
	if (header->type == NO_ESTA_VACIO) {
		connectionPool_depositeAvaibleSocket(pools, descriptor);
		perror("El dir no esta vacio");
		return -ENOTEMPTY;
	}
	free(header);
	free(mensaje_recibido);
	free(stream);
	connectionPool_depositeAvaibleSocket(pools, descriptor);
	return exit;
}

int file_unlink(const char *path) {
	int exit = 0;
	char key[255] = "LC";
	char* fatherPath = getFatherPath(path);
	strcat(key, fatherPath);
	memcached_st* memcached = connectionPool_extractAvaibleMemcached(pools);
	memcached_delete(memcached, (const char*) key, strlen(key), (time_t) 0);
	char mensajeCacheActualizarEntradaDir[255];
	sprintf(mensajeCacheActualizarEntradaDir,
			"Operacion: Actualizar Cache - path:%s", path);
	log_debug(teLogueo, mensajeCacheActualizarEntradaDir);
	char key2[255] = "AC";
	strcat(key2, path);
	memcached_delete(memcached, (const char*) key2, strlen(key2), (time_t) 0);
	char mensajeCacheActualizarAtributos[255];
	sprintf(mensajeCacheActualizarAtributos,
			"Operacion: Actualizar Cache - path:%s", path);
	log_debug(teLogueo, mensajeCacheActualizarAtributos);
	//memcached_finishHer(memcached, &servers);
	connectionPool_depositeAvaibleMemcached(pools, memcached);
	t_stream *stream = serializar_pedido_nombre(BORRAR_FILE, path);
	int descriptor = connectionPool_extractAvaibleSocket(pools);
	enviar_paquete(descriptor, stream->data, stream->length);
	char mensaje[255];
	sprintf(mensaje, "Operacion: Borrar Archivo - path:%s", path);
	log_debug(teLogueo, mensaje);
	char *mensaje_recibido = recibir_paquete(descriptor, 3);
	t_header *header = deserializadorHeader(mensaje_recibido);
	if (header->type == NO_EXISTE) {
		connectionPool_depositeAvaibleSocket(pools, descriptor);
		perror("No existe el directorio");
		return -ENOENT;
	}
	if (header->type == NAME_TOO_LONG) {
		connectionPool_depositeAvaibleSocket(pools, descriptor);
		perror("El nombre es demasiado largo");
		return -ENAMETOOLONG;
	}
	free(header);
	free(mensaje_recibido);
	free(stream);
	connectionPool_depositeAvaibleSocket(pools, descriptor);
	return exit;
}

int file_getattr(const char *path, struct stat *attr) {
	int exit = 0;
	memset(attr, 0, sizeof(struct stat));
	/* Me conecto con la Memcached	 */
	memcached_st* memcached = connectionPool_extractAvaibleMemcached(pools);
	/* Hago el GET */
	uint32_t size;
	char* value;
	//Para diferenciar las claves dependiendo si es un getattr o un readdir, se le agregara a la key los caracteres
	//AC, que indica ATRIBUTOS de CLIENTE.
	char* key = calloc(2 + strlen(path), sizeof(char));
	memcpy(key, "AC", strlen("AC"));
	memcpy(key + strlen("AC"), path, strlen(path));
	if (memcached_getValue(memcached, key, &value, &size)
			== MEMCACHED_SUCCESS) {
		char mensajeCacheGet[255];
		sprintf(mensajeCacheGet,
				"Operacion: Obtener un valor de la Cache - path:%s", path);
		log_debug(teLogueo, mensajeCacheGet);
		t_respuesta_get_attr* attrCache = deserializar_respuesta_get_attr(value,
				size);
		attr->st_ino = attrCache->nroInodo;
		attr->st_mode = attrCache->mode;
		attr->st_nlink = attrCache->hdlinks;
		attr->st_size = attrCache->size;
		attr->st_blksize = attrCache->blocksize;
		attr->st_blocks = attrCache->blocks;
		free(attrCache);
		//memcached_finishHer(memcached, &servers);
		free(value);
		free(key);
	} else {
		t_stream *stream = serializar_pedido_nombre(GET_ATTR, path);
		int descriptor = connectionPool_extractAvaibleSocket(pools);
		enviar_paquete(descriptor, stream->data, stream->length);
		char mensaje[255];
		sprintf(mensaje, "Operacion:Obtener Atributos - path:%s", path);
		log_debug(teLogueo, mensaje);
		char *cabecera_recibida = malloc(3);
		recibir(descriptor, cabecera_recibida, 3);
		t_header *header = deserializadorHeader(cabecera_recibida);
		if (header->type == NO_EXISTE) {
			connectionPool_depositeAvaibleMemcached(pools, memcached);
			connectionPool_depositeAvaibleSocket(pools, descriptor);
			printf("No existe el archivo %s\n", path);
			free(cabecera_recibida);
			return -ENOENT;
		}
		if (header->type == NAME_TOO_LONG) {
			connectionPool_depositeAvaibleMemcached(pools, memcached);
			connectionPool_depositeAvaibleSocket(pools, descriptor);
			perror("El nombre es demasiado largo");
			free(cabecera_recibida);
			return -ENAMETOOLONG;
		}
		char* mensaje_recibido = malloc(header->length);
		recibir(descriptor, mensaje_recibido, header->length);
		t_respuesta_get_attr *file_attr = deserializar_respuesta_get_attr(
				mensaje_recibido, header->length);
		attr->st_ino = file_attr->nroInodo;
		if ((file_attr->mode & S_IFMT) == S_IFDIR) {
			attr->st_mode = S_IFDIR | 0755;
		} else if ((file_attr->mode & S_IFMT) == S_IFREG) {
			attr->st_mode = S_IFREG | 0666;
		} else {
			printf("NO TENGO LA MAS PUTA IDEA QUE ES \n");
		}

		attr->st_nlink = file_attr->hdlinks;
		attr->st_size = file_attr->size;
		attr->st_blksize = file_attr->blocksize;
		attr->st_blocks = file_attr->blocks;

		memcached_addOrReplace(memcached, key, mensaje_recibido,
				header->length);
		char mensajeCacheActualizar[255];
		sprintf(mensajeCacheActualizar, "Operacion: Actualizar Cache - path:%s",
				path);
		log_debug(teLogueo, mensajeCacheActualizar);
		free(mensaje_recibido);
		free(cabecera_recibida);
		free(stream);
		connectionPool_depositeAvaibleSocket(pools, descriptor);
		//memcached_finishHer(memcached, &servers);
	}
	connectionPool_depositeAvaibleMemcached(pools, memcached);
	return exit;
}

int file_write(const char *path, const char *buffer, size_t size, off_t offset,
		struct fuse_file_info *info) {
	t_stream *stream = serializar_pedido_escribir_archivo(path, buffer, size,
			offset);
	int descriptor = connectionPool_extractAvaibleSocket(pools);
	enviar_paquete(descriptor, stream->data, stream->length);
	char mensaje[255];
	uint32_t off = (uint32_t) offset;
	uint32_t siz = (uint32_t) size;
	sprintf(mensaje,
			"Operacion: Escribir Archivo - path:%s - size:%u - offset:%u", path,
			siz, off);
	log_debug(teLogueo, mensaje);
	char *cabecera_recibida = recibir_paquete(descriptor, 3);
	t_header *header = deserializadorHeader(cabecera_recibida);
	if (header->type == NO_EXISTE) {
		connectionPool_depositeAvaibleSocket(pools, descriptor);
		perror("No existe el directorio");
		return -ENOENT;
	}
	if (header->type == NAME_TOO_LONG) {
		connectionPool_depositeAvaibleSocket(pools, descriptor);
		perror("El nombre es demasiado largo");
		return -ENAMETOOLONG;
	}
	if (header->type == ESTA_LLENO) {
		connectionPool_depositeAvaibleSocket(pools, descriptor);

		return -EPERM;
	}
	char* mensaje_recibido = recibir_paquete(descriptor, header->length);
	size_t respuesta = deserializar_respuesta_escribir_archivo(mensaje_recibido,
			header->length);
//	size_t bytes;
//	memcpy(&bytes, &respuesta->bytes_escritos, sizeof(size_t));
	//free(stream);
	//free(header);
	//free(cabecera_recibida);
	//free(mensaje_recibido);
	connectionPool_depositeAvaibleSocket(pools, descriptor);
	return respuesta;

}

int file_read(const char *path, char *buffer, size_t size, off_t offset,
		struct fuse_file_info *info) {
	t_stream *stream = serializar_pedido_file_read(path, size, offset, info);
	int descriptor = connectionPool_extractAvaibleSocket(pools);
	enviar_paquete(descriptor, stream->data, stream->length);
	char mensaje[255];
	uint32_t off = (uint32_t) offset;
	uint32_t siz = (uint32_t) size;
	sprintf(mensaje, "Operacion: Leer Archivo - path:%s - size:%u - offset:%u",
			path, siz, off);
	log_debug(teLogueo, mensaje);
	char *cabecera_recibida = recibir_paquete(descriptor, 3);
	t_header *header = deserializadorHeader(cabecera_recibida);
	if (header->type == NO_EXISTE) {
		connectionPool_depositeAvaibleSocket(pools, descriptor);
		perror("No existe el archivo");
		return -ENOENT;
	}
	if (header->type == NAME_TOO_LONG) {
		connectionPool_depositeAvaibleSocket(pools, descriptor);
		perror("El nombre es demasiado largo");
		return -ENAMETOOLONG;
	}
	char* mensaje_recibido = recibir_paquete(descriptor, header->length);
	t_respuesta_file_read *file = deserializar_respuesta_file_read(
			mensaje_recibido, header->length);
	size_t tam;
	memcpy(&tam, &file->tam, sizeof(size_t));
	memcpy(buffer, file->contenido, tam);
	free(mensaje_recibido);
	free(header);
	free(cabecera_recibida);
	free(stream);
	free(file);
	connectionPool_depositeAvaibleSocket(pools, descriptor);
	return tam;
}
int open_file(const char *path, struct fuse_file_info *info) {
	int exit = 0;
//	t_stream *stream = serializar_pedido_nombre(OPEN_FILE, path);
//	int descriptor = connectionPool_extractAvaibleSocket(pools);
//	char mensaje[255];
//	sprintf(mensaje,"Operacion: Abrir Archivo - path:%s",path);
//	log_debug(teLogueo, mensaje);
//	enviar_paquete(descriptor, stream->data, stream->length);
//	char *mensaje_recibido = recibir_paquete(descriptor, 3);
//	t_header *header = deserializadorHeader(mensaje_recibido);
//	if (header->type == NO_EXISTE) {
//		connectionPool_depositeAvaibleSocket(pools, descriptor);
//		perror("No existe el archivo");
//		return -ENOENT;
//	}
//	if (header->type == NAME_TOO_LONG) {
//		connectionPool_depositeAvaibleSocket(pools, descriptor);
//		perror("El nombre es demasiado largo");
//		return -ENAMETOOLONG;
//	}
//	free(header);
//	free(stream);
//	free(mensaje_recibido);
//	connectionPool_depositeAvaibleSocket(pools, descriptor);
	return exit;
}

int release_file(const char *path, struct fuse_file_info *info) {/*Aca faltaria verificar que el archivo este en la lista de archivos abiertos*/
	int exit = 0;
	char mensaje[255];
	sprintf(mensaje, "Operation: Cerrar Archivo - path: %s", path);
	log_debug(teLogueo, mensaje);
//	t_stream *stream = serializar_pedido_nombre(RELEASE_FILE, path);
//	int descriptor = connectionPool_extractAvaibleSocket(pools);
//
//	enviar_paquete(descriptor, stream->data, stream->length);
//
//	char *mensaje_recibido = recibir_paquete(descriptor, 3);
//	t_header *header = deserializadorHeader(mensaje_recibido);
//	if (header->type == NO_EXISTE) {
//		connectionPool_depositeAvaibleSocket(pools, descriptor);
//
//		perror("No existe el archivo");
//		return -ENOENT;
//	}
//	if (header->type == NAME_TOO_LONG) {
//		connectionPool_depositeAvaibleSocket(pools, descriptor);
//
//		perror("El nombre es demasiado largo");
//		return -ENAMETOOLONG;
//	}
//	free(stream);
//	free(mensaje_recibido);
//	free(header);
//	connectionPool_depositeAvaibleSocket(pools, descriptor);
	return exit;
}

int dir_read(const char *path, void *buffer, fuse_fill_dir_t rellenar,
		off_t offset, struct fuse_file_info *info) {

	int exit = 0;
	char key[255] = "LC";
	strcat(key, path);
	int i;
	/*  Me conecto con la Memcached*/
	memcached_st* memcached = connectionPool_extractAvaibleMemcached(pools);
	/* Hago el GET */
	uint32_t size;
	char* value;
	/* Deserializo los directorios. Recordar que se separan con # */
	if (memcached_getValue(memcached, key, &value, &size)
			== MEMCACHED_SUCCESS) {
		char mensajeCacheGet[255];
		sprintf(mensajeCacheGet, "Operacion: Actualizar Cache - path:%s", path);
		log_debug(teLogueo, mensajeCacheGet);
		char** vector_memcached;
		vector_memcached = string_split(value, "#");
		for (i = 0; vector_memcached[i] != NULL; i++) {
			rellenar(buffer, vector_memcached[i], NULL, 0);
		}
		for (i = 0; vector_memcached[i] != NULL; i++) {
			free(vector_memcached[i]);
		}
		//memcached_finishHer(memcached, &servers);
		free(vector_memcached);
		free(value);
	} else {
		/*
		 * Si return_value es MEMCACHED_NOTFOUND
		 */

		t_stream *stream = serializar_pedido_nombre(READ_DIR, path);
		int descriptor = connectionPool_extractAvaibleSocket(pools);
		enviar_paquete(descriptor, stream->data, stream->length);
		char mensaje[255];
		sprintf(mensaje, "Operacion: Leer Directorio - path:%s", path);
		log_debug(teLogueo, mensaje);
		char *cabecera_recibida = recibir_paquete(descriptor, 3);
		t_header *header = deserializadorHeader(cabecera_recibida);
		if (header->type == NO_EXISTE) {
			connectionPool_depositeAvaibleMemcached(pools, memcached);
			connectionPool_depositeAvaibleSocket(pools, descriptor);
			perror("No existe el archivo");
			return -ENOENT;
		}
		if (header->type == NAME_TOO_LONG) {
			connectionPool_depositeAvaibleMemcached(pools, memcached);
			connectionPool_depositeAvaibleSocket(pools, descriptor);
			perror("El nombre es demasiado largo");
			return -ENAMETOOLONG;
		}
		char* mensaje_recibido = recibir_paquete(descriptor, header->length);
		/* Accedo a la cache y le envio el payload	*/
		memcached_addOrReplace(memcached, key, mensaje_recibido,
				header->length);
		char mensajeCacheActualizar[255];
		sprintf(mensajeCacheActualizar, "Operacion: Actualizar Cache - path:%s",
				path);
		log_debug(teLogueo, mensajeCacheActualizar);
		char** vector;
		vector = string_split(mensaje_recibido, "#");
		for (i = 0; vector[i] != NULL; i++) {
			rellenar(buffer, vector[i], NULL, 0);
		}
		free(vector);
		free(cabecera_recibida);
		free(stream);
		free(mensaje_recibido);
		free(header);
		//memcached_finishHer(memcached, &servers);
		connectionPool_depositeAvaibleSocket(pools, descriptor);
	}
	connectionPool_depositeAvaibleMemcached(pools, memcached);
	return exit;
}
