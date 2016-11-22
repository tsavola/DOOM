#ifndef UNISTD_H
#define UNISTD_H

#include <stddef.h>
#include <stdint.h>

#define O_BINARY 0
#define O_RDONLY 1

#define R_OK 0

#define SEEK_SET 0

int access(const char *pathname, int mode);
int close(int fd);
int lseek(int fd, int offset, int whence);
int open(const char *pathname, int flags, ...);
int read(int fd, void *buf, size_t count);
int usleep(uint64_t usec);

#endif
