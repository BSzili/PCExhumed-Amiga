//-------------------------------------------------------------------------
/*
Copyright (C) 2010-2019 EDuke32 developers and contributors
Copyright (C) 2019 sirlemonhead, Nuke.YKT

This file is part of PCExhumed.

PCExhumed is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License version 2
as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
//-------------------------------------------------------------------------

#ifndef __cd_h__
#define __cd_h__

#ifndef EDUKE32
//#include_next "cd.h"
// TODO include the original header
const char * CD_ErrorString(int code);
int  CD_Init(int SoundCard);
int  CD_GetCurrentDriver(void);
const char * CD_GetCurrentDriverName(void);
void CD_Shutdown(void);
int  CD_Play(int track, int loop);
void CD_Stop(void);
int  CD_IsPlaying(void);
void CD_SetVolume(int volume);
#define CD_Ok (0)
#endif

void setCDaudiovolume(int val);
bool playCDtrack(int nTrack, bool bLoop);
void StartfadeCDaudio();
int StepFadeCDaudio();
bool CDplaying();
void StopCD();
int fadecdaudio();

#endif
