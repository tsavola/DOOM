#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

#include <gate.h>

#include "gate_support.h"

static char heap[16 * 1024 * 1024];
static size_t allocated;

void *realloc(void *oldptr, size_t size)
{
	size_t aligned = (size + 7) & ~7;
	if (allocated + 8 + aligned > sizeof heap) {
		gate_debug("out of memory\n");
		gate_exit(1);
	}
	char *newptr = heap + allocated;
	allocated += 8 + aligned;
	*(uint64_t *) newptr = size;
	newptr += 8;
	if (oldptr)
		memcpy(newptr, oldptr, *(uint64_t *) (oldptr - 8));
	return newptr;
}

int gettimeofday(struct timeval *tv, struct timezone *tz)
{
	uint64_t t = gate_clock_realtime();
	tv->tv_sec = t / 1000000000;
	tv->tv_usec = (t / 1000) % 1000000;
	return 0;
}

void *memcpy(void *dest, const void *src, size_t n)
{
	for (size_t i = 0; i < n; i++)
		((char *) dest)[i] = ((char *) src)[i];
	return dest;
}

void *memset(void *s, int c, size_t n)
{
	for (size_t i = 0; i < n; i++)
		((char *) s)[i] = c;
	return s;
}

char *strcat(char *dest, const char *src)
{
	strcpy(dest + strlen(dest), src);
	return dest;
}

int strcmp(const char *s1, const char *s2)
{
	if (strlen(s1) != strlen(s2))
		return 1;

	while (*s1) {
		if (*s1++ != *s2++)
			return 1;
	}

	return 0;
}

int strcasecmp(const char *s1, const char *s2)
{
	if (strlen(s1) != strlen(s2))
		return 1;

	while (*s1 && *s2) {
		if (toupper(*s1++) != toupper(*s2++))
			return 1;
	}

	return 0;
}

char *strcpy(char *dest, const char *src)
{
	char *ret = dest;
	do {
		*dest++ = *src;
	} while (*src++);
	return ret;
}

size_t strlen(const char *s)
{
	size_t n = 0;
	while (*s++)
		n++;
	return n;
}

int strncasecmp(const char *s1, const char *s2, size_t n)
{
	if (strlen(s1) < n || strlen(s2) < n) {
		if (strlen(s1) != strlen(s2))
			return 1;
	}

	for (size_t i = 0; *s1 && *s2 && i < n; i++) {
		if (toupper(*s1++) != toupper(*s2++))
			return 1;
	}

	return 0;
}

int strncmp(const char *s1, const char *s2, size_t n)
{
	if (strlen(s1) < n || strlen(s2) < n) {
		if (strlen(s1) != strlen(s2))
			return 1;
	}

	for (size_t i = 0; *s1 && *s2 && i < n; i++) {
		if (*s1++ != *s2++)
			return 1;
	}

	return 0;
}

char *strncpy(char *dest, const char *src, size_t n)
{
	memset(dest, 0, n);
	char *ret = dest;
	for (size_t i = 0; i < n; i++) {
		*dest++ = *src;
		if (*src++ == 0)
			break;
	}
	return ret;
}

int toupper(int c)
{
	if (c >= 'a' && c <= 'z')
		c += 'A' - 'a';
	return c;
}

int atoi(const char *nptr)
{
	int i = 0;
	while (*nptr) {
		if (*nptr >= '0' && *nptr <= '9')
			i = (i * 10) + (*nptr - '0');
		else
			return 0;
		nptr++;
	}
	return i;
}

int access(const char *pathname, int mode)
{
	if (strcmp(pathname, "DOOMWADDIR/doom1.wad") != 0)
		return -1;

	return 0;
}

static char filebuf[4196020]; // doom1.wad
static int filelen = -1;
static int filepos;

int open(const char *pathname, int flags, ...)
{
	if (filelen >= 0)
		return -1;

	if (strcmp(pathname, "DOOMWADDIR/doom1.wad") != 0)
		return -1;

	filelen = read_origin(filebuf, sizeof filebuf);
	if (filelen < 0)
		return -1;

	filepos = 0;
	return 0;
}

int lseek(int fd, int offset, int whence)
{
	if (fd != 0 || filelen < 0)
		return -1;

	filepos = offset; // whence is always SEEK_SET
	return filepos;
}

int read(int fd, void *buf, size_t count)
{
	if (fd != 0 || filelen < 0)
		return -1;

	int n = count;
	if (filepos + n > filelen) {
		n = filelen - filepos;
		if (n <= 0)
			return 0;
	}

	memcpy(buf, filebuf + filepos, n);
	filepos += n;
	return n;
}

int fstat(int fd, struct stat *buf)
{
	if (fd != 0 || filelen < 0)
		return -1;

	buf->st_size = filelen;
	return 0;
}

int close(int fd)
{
	if (fd != 0 || filelen < 0)
		return -1;

	filelen = -1;
	return 0;
}

__attribute__((noinline)) int usleep(uint64_t usec)
{
	static volatile int dummy;
	while (dummy == 0) {
	}
	return 0;
}

void _putchar(char c)
{
	gate_debug_data(&c, 1);
}

FILE *const stdout;
