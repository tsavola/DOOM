#include "gate_support.h"

#include <stdbool.h>
#include <string.h>

static char recvbuf[GATE_MAX_RECV_SIZE];
static size_t recvlen;

static struct gate_packet *receive_packet(void)
{
	while (recvlen < sizeof(struct gate_packet))
		recvlen += gate_recv(recvbuf + recvlen, sizeof recvbuf - recvlen, GATE_IO_WAIT);

	struct gate_packet *header = (struct gate_packet *) recvbuf;
	size_t aligned_size = GATE_ALIGN_PACKET(header->size);

	if (aligned_size > GATE_MAX_RECV_SIZE) {
		__gate_debug_str("bug: received packet size exceeds maximum packet size\n");
		gate_exit(1);
	}

	while (recvlen < aligned_size)
		recvlen += gate_recv(recvbuf + recvlen, sizeof recvbuf - recvlen, GATE_IO_WAIT);

	return header;
}

static void consume_packet(struct gate_packet *packet)
{
	if ((char *) packet != recvbuf) {
		__gate_debug_str("bug: bad consume_packet call\n");
		gate_exit(1);
	}

	size_t aligned_size = GATE_ALIGN_PACKET(packet->size);

	memcpy(recvbuf, recvbuf + aligned_size, recvlen - aligned_size);
	recvlen -= aligned_size;
}

static void send_packet(const void *sendbuf, size_t sendbufsize)
{
	size_t sendlen = 0;

	while (sendlen < sendbufsize) {
		struct gate_iovec recv = {recvbuf + recvlen, sizeof recvbuf - recvlen};
		struct gate_iovec send = {(void *)sendbuf + sendlen, sendbufsize - sendlen};

		if (recv.iov_len == 0) {
			__gate_debug_str("TODO: receive buffer full while sending\n");
			gate_exit(1);
		}

		size_t recvn;
		size_t sendn;
		gate_io(&recv, 1, &recvn, &send, 1, &sendn, GATE_IO_WAIT);
		recvlen += recvn;
		sendlen += sendn;
	}
}

static int16_t origin = -1;
static int16_t raster = -1;

static int32_t stream = -1;

void init_gate(void)
{
	struct discover {
		struct gate_service_name_packet header;
		char names[6 + 1 + 20 + 1];
	};

	union {
		struct discover packet;
		char padding[GATE_ALIGN_PACKET(sizeof(struct discover))];
	} buf;

	memset(&buf, 0, sizeof buf);
	buf.packet.header.header.size = sizeof(struct discover);
	buf.packet.header.header.code = GATE_PACKET_CODE_SERVICES;
	buf.packet.header.count = 2;
	memcpy(buf.packet.header.names, "origin\0gate.computer/raster", sizeof buf.packet.names);

	send_packet(&buf, sizeof buf);

	struct gate_packet *packet = receive_packet();

	if (packet->code != GATE_PACKET_CODE_SERVICES) {
		__gate_debug_str("error: expected reply packet from services\n");
		gate_exit(1);
	}

	struct gate_service_state_packet *discovery = (struct gate_service_state_packet *) packet;

	if (discovery->count > 0 && discovery->states[0] & GATE_SERVICE_STATE_AVAIL)
		origin = 0;
	else
		gate_debug("origin service unavailable\n");

	if (discovery->count > 1 && discovery->states[1] & GATE_SERVICE_STATE_AVAIL)
		raster = 1;
	else
		gate_debug("raster service unavailable\n");

	consume_packet(packet);

	const struct gate_packet accept = {
		.size = sizeof accept,
		.code = origin,
		.domain = GATE_PACKET_DOMAIN_CALL,
	};

	send_packet(&accept, sizeof accept);

	packet = receive_packet();

	if (packet->code != origin || packet->domain != GATE_PACKET_DOMAIN_CALL || packet->size != sizeof(struct gate_packet) + 8) {
		__gate_debug_str("error: expected accept reply packet from origin\n");
		gate_exit(1);
	}

	stream = ((int32_t *) (packet + 1))[0];
	int32_t error = ((int32_t *) (packet + 1))[1];
	if (error != 0) {
		__gate_debug_str("error: origin stream accept error\n");
		gate_exit(1);
	}
}

int read_origin(void *buf, size_t bufsize)
{
	if (origin < 0 || stream < 0)
		return -1;

	struct {
		struct gate_flow_packet header;
		struct gate_flow flow;
	} flow = {
		.header = {
			.header = {
				.size = sizeof flow,
				.code = origin,
				.domain = GATE_PACKET_DOMAIN_FLOW,
			},
		},
		.flow = {
			.id = stream,
			.increment = bufsize,
		},
	};

	send_packet(&flow, sizeof flow);

	int offset = 0;
	bool eof = false;

	do {
		struct gate_packet *packet = receive_packet();

		if (packet->code != origin) {
			__gate_debug_str("TODO: unrelated packet received while reading from origin\n");
			gate_exit(1);
		}

		if (packet->domain == GATE_PACKET_DOMAIN_DATA) {
			const struct gate_data_packet *datapacket = (void *) packet;
			if (datapacket->id != stream) {
				__gate_debug_str("TODO: received packet from origin with unrelated stream\n");
				gate_exit(1);
			}

			int datalen = packet->size - sizeof(struct gate_data_packet);
			if (datalen > bufsize - offset) {
				__gate_debug_str("received too much data from origin\n");
				gate_exit(1);
			}

			memcpy(buf + offset, (char *) packet + sizeof(struct gate_data_packet), datalen);
			offset += datalen;

			if (datalen == 0)
				eof = true;
		}

		consume_packet(packet);
	} while (!eof);

	return offset;
}

void rasterize(struct gate_packet *call)
{
	if (raster < 0)
		return;

	call->code = raster;
	send_packet(call, call->size);
}

int receive_raster_events(uint64_t *events, int max_events)
{
	if (raster < 0)
		return 0;

	int n = -1;

	do {
		struct gate_packet *reply = receive_packet();

		if (reply->code == raster && reply->domain == GATE_PACKET_DOMAIN_CALL) {
			n = (reply->size - sizeof(struct gate_packet)) / sizeof events[0];
			if (n > max_events)
				n = max_events;

			memcpy(events, (char *) reply + sizeof(struct gate_packet), n * sizeof events[0]);
		}

		consume_packet(reply);
	} while (n < 0);

	return n;
}
