#ifndef SYS_TIME_H
#define SYS_TIME_H

#include <stdint.h>

struct timeval {
	int64_t tv_sec;
	int tv_usec;
};

struct timezone {
	int tz_minuteswest;
	int tz_dsttime;
};

int gettimeofday(struct timeval *tv, struct timezone *tz);

#endif
