/*
 * funcionesFuse.h
 *
 *  Created on: 08/06/2012
 *      Author: utnso
 */

#ifndef FUNCIONESFUSE_H_
#define FUNCIONESFUSE_H_
#include <fuse.h>


int dir_read (const char *path, void *buffer, fuse_fill_dir_t rellenar, off_t offset, struct fuse_file_info *info);

int create_file (const char *path,mode_t modo, struct fuse_file_info *info);

int file_read(const char *path, char *buffer, size_t tam, off_t offset, struct fuse_file_info *info);
int file_write (const char *path, const char *buffer, size_t tam, off_t offset, struct fuse_file_info *info);

int fuse_truncate(const char *path, off_t offset);

int create_dir (const char *path, mode_t modo);

int borrarDir (const char *path);
int file_unlink (const char *path);
int file_getattr(const char *path,struct stat *info);
int open_file(const char *path, struct fuse_file_info *info);
int release_file(const char *path, struct fuse_file_info *info);

#endif /* FUNCIONESFUSE_H_ */
