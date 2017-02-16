#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>

#include "d_main.h"
#include "v_video.h"

#define HEADER_SIZE   8
#define MAX_RECV_SIZE 1024

#define FRAME_ID    0x66726d65 // frme
#define PALETTE_ID  0x706c7474 // pltt
#define EVENTS_ID   0x65766e74 // evnt

static int recv_fd = -1;
static int send_fd = -1;
static unsigned char image_msg_buf[HEADER_SIZE + SCREENWIDTH * SCREENHEIGHT];

static boolean recv_full(void *buf, size_t size, boolean peek)
{
	for (size_t pos = 0; pos < size; ) {
		ssize_t len = read(recv_fd, buf + pos, size - pos);

		if (peek && pos == 0) {
			if (len < 0 && (errno == EAGAIN || errno == EINTR))
				return false;
		}

		if (len == 0)
			exit(0);

		if (len < 0) {
			if (errno == EAGAIN || errno == EINTR)
				continue;
			exit(1);
		}

		pos += len;
	}

	return true;
}

static void send_full(const void *data, size_t size)
{
	for (size_t pos = 0; pos < size; ) {
		ssize_t len = write(send_fd, data + pos, size - pos);

		if (len == 0)
			exit(0);

		if (len < 0) {
			if (errno == EINTR)
				continue;
			exit(1);
		}

		pos += len;
	}
}

static void send_packet(void *buf, size_t size, uint32_t type)
{
	uint32_t *header = buf;

	header[0] = size;
	header[1] = type;

	send_full(buf, size);
}

static int translate_key(int rc)
{
	switch (rc) {
	case 276: rc = KEY_LEFTARROW; break;
	case 275: rc = KEY_RIGHTARROW; break;
	case 274: rc = KEY_DOWNARROW; break;
	case 273: rc = KEY_UPARROW; break;
	case 27:  rc = KEY_ESCAPE; break;
	case 13:  rc = KEY_ENTER; break;
	case 9:   rc = KEY_TAB; break;
	case 282: rc = KEY_F1; break;
	case 283: rc = KEY_F2; break;
	case 284: rc = KEY_F3; break;
	case 285: rc = KEY_F4; break;
	case 286: rc = KEY_F5; break;
	case 287: rc = KEY_F6; break;
	case 288: rc = KEY_F7; break;
	case 289: rc = KEY_F8; break;
	case 290: rc = KEY_F9; break;
	case 291: rc = KEY_F10; break;
	case 292: rc = KEY_F11; break;
	case 293: rc = KEY_F12; break;

	case 8:
	case 127:
		rc = KEY_BACKSPACE;
		break;

	case 19:
		rc = KEY_PAUSE;
		break;

	case 272:
	case 61:
		rc = KEY_EQUALS;
		break;

	case 269:
	case 45:
		rc = KEY_MINUS;
		break;

	case 303:
	case 304:
		rc = KEY_RSHIFT;
		break;

	case 305:
	case 306:
		rc = KEY_RCTRL;
		break;

	case 308:
	case 310:
	case 307:
	case 309:
		rc = KEY_RALT;
		break;

	case 32:
		rc = ' ';
		break;

	default:
		//if (rc >= XK_space && rc <= XK_asciitilde)
		//    rc = rc - XK_space + ' ';

		if (rc >= 97 && rc <= 122)
			rc = rc - 97 + 'a';

		break;
	}

	return rc;
}

struct ev_data_t {
	char type;
	char button;
	union {
		struct {
			int key;
		};
		struct {
			short move_x;
			short move_y;
		};
	};
} __attribute__ ((packed));

static void receive_events(void)
{
	static boolean mouse_buttons[4]; // index 0 is dummy
	static uint64_t ticks;

	struct timeval tv;
	if (gettimeofday(&tv, NULL) < 0)
		exit(1);

	uint64_t curr_ticks = (uint64_t) tv.tv_sec * 1000000UL + (uint64_t) tv.tv_usec;

	if (ticks == 0)
		ticks = curr_ticks;

	while (ticks <= curr_ticks)
		ticks += 1000000UL / 35UL;

	while (ticks > curr_ticks) {
		fd_set rfds;
		FD_ZERO(&rfds);
		FD_SET(recv_fd, &rfds);

		tv.tv_sec = 0;
		tv.tv_usec = ticks - curr_ticks;

		int count = select(recv_fd+1, &rfds, NULL, NULL, &tv);
		if (count < 0)
			exit(1);

		if (count == 0 || !FD_ISSET(recv_fd, &rfds))
			break;

		while (1) {
			uint32_t header[2];
			if (!recv_full(header, sizeof (header), true))
				break;

			uint32_t size = header[0];
			uint32_t type = header[1];
			if (size < sizeof (header) || size > MAX_RECV_SIZE)
				exit(1);

			uint32_t payload_size = size - sizeof (header);
			char payload[payload_size];
			recv_full(payload, payload_size, false);

			if (type == EVENTS_ID) {
				const struct ev_data_t *events = (struct ev_data_t *) payload;
				int num_events = payload_size / sizeof (struct ev_data_t);

				for (int i = 0; i < num_events; i++) {
					const struct ev_data_t *ev = &events[i];

					event_t event = { 0 };

					switch (ev->type) {
					case 0: // key press
						event.type = ev_keydown;
						event.data1 = translate_key(ev->key);
						D_PostEvent(&event);
						break;

					case 1: // key release
						event.type = ev_keyup;
						event.data1 = translate_key(ev->key);
						D_PostEvent(&event);
						break;

					case 2: // button press
						mouse_buttons[(int) ev->button] = true;

						event.type = ev_mouse;
						if (mouse_buttons[1]) event.data1 |= 1;
						if (mouse_buttons[2]) event.data1 |= 2;
						if (mouse_buttons[3]) event.data1 |= 4;

						D_PostEvent(&event);
						break;

					case 3: // button release
						mouse_buttons[(int) ev->button] = false;

						event.type = ev_mouse;
						if (mouse_buttons[1]) event.data1 |= 1;
						if (mouse_buttons[2]) event.data1 |= 2;
						if (mouse_buttons[3]) event.data1 |= 4;

						D_PostEvent(&event);
						break;

					case 4: // motion notify
						event.type = ev_mouse;
						if (mouse_buttons[1]) event.data1 |= 1;
						if (mouse_buttons[2]) event.data1 |= 2;
						if (mouse_buttons[3]) event.data1 |= 4;

						event.data2 = ev->move_x << 2;
						event.data3 = -ev->move_y << 2;

						D_PostEvent(&event);
						break;
					}
				}
			}
		}

		if (gettimeofday(&tv, NULL) < 0)
			exit(1);

		curr_ticks = (uint64_t) tv.tv_sec * 1000000UL + (uint64_t) tv.tv_usec;
	}
}

void I_ShutdownGraphics(void)
{
}

void I_StartFrame(void)
{
}

void I_StartTic(void)
{
}

void I_UpdateNoBlit(void)
{
}

void I_FinishUpdate(void)
{
	send_packet(image_msg_buf, sizeof (image_msg_buf), FRAME_ID);

	receive_events();
}

void I_ReadScreen(byte *scr)
{
	memcpy(scr, screens[0], SCREENWIDTH*SCREENHEIGHT);
}

void I_SetPalette(byte *palette)
{
	char buf[HEADER_SIZE + 256 * 3];

	memcpy(buf + HEADER_SIZE, palette, 256 * 3);
	send_packet(buf, sizeof (buf), PALETTE_ID);
}

void I_InitGraphics(void)
{
	recv_fd = atoi(getenv("RECV_FD"));
	if (recv_fd < 3)
		exit(1);

	int flags = fcntl(recv_fd, F_GETFL);
	if (flags < 0)
		exit(1);
	if (fcntl(recv_fd, F_SETFL, flags|O_NONBLOCK) < 0)
		exit(1);

	send_fd = atoi(getenv("SEND_FD"));
	if (send_fd < 3)
		exit(1);

	screens[0] = image_msg_buf + HEADER_SIZE;
}
