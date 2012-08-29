/*
 * fs_handler.h
 *
 *  Created on: 03/05/2012
 *      Author: utnso
 */

#ifndef FS_HANDLER_H_
#define FS_HANDLER_H_
#include <stdint.h>
#include "synchronizer.h"
uint32_t abrirFS(char* diskFilePath);
void actualizarFS(uint32_t bloque, char* buf,structures_synchronizer_t* locks);
void leerUnBloque(uint32_t bloque, char* buf);
void closeFS(uint32_t file_descriptor);


#endif /* FS_HANDLER_H_ */
