#ifndef SYS_STAT_H
#define SYS_STAT_H

#include <stdint.h>

struct stat {
	int st_size;
};

int fstat(int fd, struct stat *statbuf);

static inline int mkdir(const char *pathname, int mode)
{
	return -1;
}

#endif
