#ifndef STDIO_H
#define STDIO_H

#include "printf/printf.h"

typedef struct FILE FILE;

struct FILE {
};

extern FILE *const stdout;

static inline int getchar(void)
{
	return 0;
}

static inline void setbuf(FILE *stream, char *buf)
{
}

#endif
