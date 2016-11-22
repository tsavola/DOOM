#ifndef STDLIB_H
#define STDLIB_H

#include <stddef.h>

#include <gate.h>

int atoi(const char *nptr);
void *realloc(void *ptr, size_t size);

static inline __attribute__((noreturn)) void abort(void)
{
	gate_exit(1);
}

static inline int abs(int j)
{
	return j >= 0 ? j : -j;
}

static inline __attribute__((noreturn)) void exit(int status)
{
	gate_exit(status);
}

static inline char *getenv(const char *name)
{
	return (char *) name;
}

static inline void *malloc(size_t size)
{
	return realloc(NULL, size);
}

#endif
