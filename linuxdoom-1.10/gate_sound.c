#include <stdio.h>

#include "i_sound.h"
#include "sounds.h"

char *sndserver_filename;

void I_SetChannels(void)
{
}

void I_SetMusicVolume(int volume)
{
}

int I_GetSfxLumpNum(sfxinfo_t *sfx)
{
	return 0;
}

int I_StartSound(int id, int vol, int sep, int pitch, int priority)
{
	return 1;
}

void I_StopSound(int handle)
{
}

int I_SoundIsPlaying(int handle)
{
	return 0;
}

void I_UpdateSound(void)
{
}

void I_SubmitSound(void)
{
}

void I_UpdateSoundParams(int handle, int vol, int sep, int pitch)
{
}

void I_ShutdownSound(void)
{
}

void I_InitSound()
{
}

void I_InitMusic(void)
{
}

void I_ShutdownMusic(void)
{
}

void I_PlaySong(int handle, int looping)
{
}

void I_PauseSong(int handle)
{
}

void I_ResumeSong(int handle)
{
}

void I_StopSong(int handle)
{
}

void I_UnRegisterSong(int handle)
{
}

int I_RegisterSong(void* data)
{
	return 1;
}
