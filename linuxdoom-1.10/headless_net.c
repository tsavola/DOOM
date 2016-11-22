#include "doomstat.h"
#include "i_net.h"

static doomcom_t doomcom_storage;

void I_InitNetwork(void)
{
	doomcom = &doomcom_storage;
	doomcom->ticdup = 1;
	doomcom->id = DOOMCOM_ID;
	doomcom->numplayers = 1;
	doomcom->numnodes = 1;
}

void I_NetCmd(void)
{
}
