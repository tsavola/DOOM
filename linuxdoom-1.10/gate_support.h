#ifndef GATE_SUPPORT_H
#define GATE_SUPPORT_H

#include <stddef.h>

#include <gate.h>

void init_gate(void);
int read_origin(void *buf, size_t bufsize);
void rasterize(struct gate_packet *call);
int receive_raster_events(uint64_t *events, int max_events);

#endif
