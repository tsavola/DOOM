#include <string.h>

#include <gate.h>

#include "d_main.h"
#include "doomdef.h"
#include "gate_support.h"
#include "v_video.h"

static byte buffer[sizeof(struct gate_packet) + 3 * 256 + SCREENWIDTH * SCREENHEIGHT];
static struct gate_packet *const packet = (struct gate_packet *) buffer;
static byte *const palette = buffer + sizeof(struct gate_packet);
static byte *const pixels = buffer + sizeof(struct gate_packet) + 3 * 256;

static int translate_key(int rc)
{
	switch (rc) {
	case 39: rc = '0'; break;
	case 41: rc = KEY_ESCAPE; break;
	case 42: rc = KEY_BACKSPACE; break;
	case 43: rc = KEY_TAB; break;
	case 44: rc = ' '; break;
	case 46: rc = KEY_EQUALS; break;
	case 58: rc = KEY_F1; break;
	case 59: rc = KEY_F2; break;
	case 60: rc = KEY_F3; break;
	case 61: rc = KEY_F4; break;
	case 62: rc = KEY_F5; break;
	case 63: rc = KEY_F6; break;
	case 64: rc = KEY_F7; break;
	case 65: rc = KEY_F8; break;
	case 66: rc = KEY_F9; break;
	case 67: rc = KEY_F10; break;
	case 68: rc = KEY_F11; break;
	case 69: rc = KEY_F12; break;
	case 72: rc = KEY_PAUSE; break;
	case 79: rc = KEY_RIGHTARROW; break;
	case 80: rc = KEY_LEFTARROW; break;
	case 81: rc = KEY_DOWNARROW; break;
	case 82: rc = KEY_UPARROW; break;
	case 226: rc = KEY_LALT; break;
	case 229: rc = KEY_RSHIFT; break;
	case 230: rc = KEY_RALT; break;

	case 40:
	case 88:
		rc = KEY_ENTER;
		break;

	case 56:
	case 86:
		rc = KEY_MINUS;
		break;

	case 45:
	case 87:
		rc = '+';
		break;

	case 224:
	case 228:
		rc = KEY_RCTRL;
		break;

	default:
		if (rc >= 4 && rc <= 29)
			rc = rc - 4 + 'a';

		if (rc >= 30 && rc <= 38)
			rc = rc - 30 + '1';

		break;
	}

	return rc;
}

static void handle_events(uint64_t *events, int num_events)
{
	static boolean mouse_buttons[4]; // index 0 is dummy

	for (int i = 0; i < num_events; i++) {
		uint64_t data = events[i];
		event_t event = {0};

		switch (data & 255) {
		case 1: // quit
			gate_exit(0);
			break;

		case 2: // key press
			event.type = ev_keydown;
			event.data1 = translate_key((data >> 8) & 255);
			D_PostEvent(&event);
			break;

		case 3: // key release
			event.type = ev_keyup;
			event.data1 = translate_key((data >> 8) & 255);
			D_PostEvent(&event);
			break;
		}
	}
}

void I_InitGraphics(void)
{
	packet->size = sizeof buffer;
	packet->domain = GATE_PACKET_DOMAIN_CALL;

	screens[0] = pixels;

	rasterize(packet);
}

void I_StartTic(void)
{
}

void I_SetPalette(byte *new_palette)
{
	memcpy(palette, new_palette, 3 * 256);
}

void I_StartFrame(void)
{
}

void I_UpdateNoBlit(void)
{
}

void I_FinishUpdate(void)
{
	rasterize(packet);

	uint64_t events[(GATE_MAX_PACKET_SIZE - sizeof(struct gate_packet)) / 8];
	int num_events = receive_raster_events(events, sizeof events / sizeof events[0]);
	handle_events(events, num_events);
}

void I_ReadScreen(byte *scr)
{
	memcpy(scr, screens[0], SCREENWIDTH * SCREENHEIGHT);
}

void I_ShutdownGraphics(void)
{
	receive_raster_events(NULL, 0);
}
