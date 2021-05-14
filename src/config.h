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

#ifndef __config_h__
#define __config_h__

#include "compat.h"

#include "_control.h"
#include "control.h"
#include "keyboard.h"

#define kMaxGameFuncLen     64

// KEEPINSYNC mact/include/_control.h, build/src/sdlayer.cpp
#define MAXJOYBUTTONS 32
#define MAXJOYBUTTONSANDHATS (MAXJOYBUTTONS+4)

// KEEPINSYNC mact/include/_control.h, build/src/sdlayer.cpp
#define MAXMOUSEAXES 2
#define MAXMOUSEDIGITAL (MAXMOUSEAXES*2)

// KEEPINSYNC mact/include/_control.h, build/src/sdlayer.cpp
#ifndef EDUKE32
#define MAXJOYAXES 12
#else
#define MAXJOYAXES 9
#endif
#define MAXJOYDIGITAL (MAXJOYAXES*2)

// default mouse scale
#define DEFAULTMOUSEANALOGUESCALE           65536

// default joystick settings
#define DEFAULTJOYSTICKANALOGUESCALE        65536
#define DEFAULTJOYSTICKANALOGUEDEAD         2000
#define DEFAULTJOYSTICKANALOGUESATURATE     9500

enum {
	gamefunc_Move_Forward,
	gamefunc_Move_Backward,
	gamefunc_Turn_Left,
	gamefunc_Turn_Right,
	gamefunc_Strafe,
	gamefunc_Strafe_Left,
	gamefunc_Strafe_Right,
	gamefunc_Run,
	gamefunc_Jump,
	gamefunc_Crouch,
	gamefunc_Fire,
	gamefunc_Open,
	gamefunc_Look_Up,
	gamefunc_Look_Down,
	gamefunc_Look_Straight,
	gamefunc_Aim_Up,
	gamefunc_Aim_Down,
	gamefunc_SendMessage,
	gamefunc_Weapon_1,
	gamefunc_Weapon_2,
	gamefunc_Weapon_3,
	gamefunc_Weapon_4,
	gamefunc_Weapon_5,
	gamefunc_Weapon_6,
	gamefunc_Weapon_7,
	gamefunc_Pause,
	gamefunc_Map,
	gamefunc_Zoom_In,
	gamefunc_Zoom_Out,
	gamefunc_Gamma_Correction,
	gamefunc_Escape,
	gamefunc_Shrink_Screen,
	gamefunc_Enlarge_Screen,
	gamefunc_Inventory,
	gamefunc_Inventory_Left,
	gamefunc_Inventory_Right,
    gamefunc_Show_Console,
	gamefunc_Mouse_Aiming,
	gamefunc_Toggle_Crosshair,
	gamefunc_Next_Weapon,
	gamefunc_Previous_Weapon,
	gamefunc_AutoRun,
    gamefunc_Map_Follow_Mode,
	gamefunc_Third_Person_View,
	kMaxGameFunctions
};

extern const char gamefunctions[kMaxGameFunctions][kMaxGameFuncLen];
extern const char keydefaults[kMaxGameFunctions * 2][kMaxGameFuncLen];
extern const char oldkeydefaults[kMaxGameFunctions * 2][kMaxGameFuncLen];

typedef struct {
    int32_t usejoystick;
    int32_t usemouse;
    int32_t fullscreen;
    int32_t xdim;
    int32_t ydim;
    int32_t bpp;
    int32_t forcesetup;
    int32_t noautoload;
} ud_setup_t;

#define kSetupFilename  "pcexhumed.cfg"
extern char setupfilename[BMAX_PATH];

#ifdef EDUKE32
extern hashtable_t h_gamefuncs;
#endif

void SetupInput();

void LoadConfig();
int CONFIG_ReadSetup();
void CONFIG_WriteSetup(uint32_t flags);

extern int lMouseSens;

extern ud_setup_t gSetup;
extern int32_t scripthandle;
extern int32_t setupread;
extern int32_t useprecache;
extern int32_t MouseDeadZone, MouseBias;

extern int32_t FXVolume;
extern int32_t MusicVolume;
extern int32_t SoundToggle;
extern int32_t MusicToggle;
extern int32_t MixRate;
extern int32_t MidiPort;
extern int32_t NumVoices;
extern int32_t NumChannels;
extern int32_t NumBits;
extern int32_t ReverseStereo;
extern int32_t MusicDevice;
extern int32_t FXDevice;

extern int32_t gShowCrosshair;

// JBF 20031211: Store the input settings because
// (currently) mact can't regurgitate them
extern int32_t MouseFunctions[MAXMOUSEBUTTONS][2];
extern int32_t MouseDigitalFunctions[MAXMOUSEAXES][2];
extern int32_t MouseAnalogueAxes[MAXMOUSEAXES];
extern int32_t MouseAnalogueScale[MAXMOUSEAXES];
extern int32_t JoystickFunctions[MAXJOYBUTTONSANDHATS][2];
extern int32_t JoystickDigitalFunctions[MAXJOYAXES][2];
extern int32_t JoystickAnalogueAxes[MAXJOYAXES];
extern int32_t JoystickAnalogueScale[MAXJOYAXES];
extern int32_t JoystickAnalogueInvert[MAXJOYAXES];
extern int32_t JoystickAnalogueDead[MAXJOYAXES];
extern int32_t JoystickAnalogueSaturate[MAXJOYAXES];
extern uint8_t KeyboardKeys[kMaxGameFunctions][2];

extern int32_t MAXCACHE1DSIZE;

#ifndef EDUKE32
void CONFIG_SetDefaultKeys(const char(*keyptr)[kMaxGameFuncLen], bool lazy);
#else
void CONFIG_SetDefaultKeys(const char(*keyptr)[kMaxGameFuncLen], bool lazy=false);
#endif
void CONFIG_MapKey(int which, kb_scancode key1, kb_scancode oldkey1, kb_scancode key2, kb_scancode oldkey2);
int32_t CONFIG_FunctionNameToNum(const char* func);

#endif
