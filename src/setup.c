//-------------------------------------------------------------------------
/*
Copyright (C) 2010-2021 EDuke32 developers and contributors
Copyright (C) 2019 sirlemonhead, Nuke.YKT, Szilard Biro

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

#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>

#include <proto/exec.h>
#include <proto/graphics.h>
#include <proto/intuition.h>
#include <proto/gadtools.h>
#include <clib/alib_protos.h>

#include "compat.h"
#include "config.h"

// PCExhumed
int mouseaiming, aimmode, mouseflip;
int runkey_mode, auto_run;
int gFov;
int32_t bFullScreen;
int32_t screensize;
int32_t nGamma;

// JMACT dummy functions
boolean CONTROL_MousePresent;
boolean CONTROL_JoyPresent;
boolean CONTROL_MouseEnabled;
boolean CONTROL_JoystickEnabled;
void CONTROL_MapKey(int32 which, kb_scancode key1, kb_scancode key2) {}
void CONTROL_MapButton(int32 whichfunction, int32 whichbutton, boolean doubleclicked, controldevice device) {}
void CONTROL_DefineFlag(int32 which, boolean toggle) {}
void CONTROL_ClearAssignments(void) {}
void CONTROL_SetMouseSensitivity(int32 newsensitivity) {} // TODO maybe later with CONTROL_GetMouseSensitivity
void CONTROL_SetJoyAxisDead(int32 axis, int32 newdead) {}
void CONTROL_SetJoyAxisSaturate(int32 axis, int32 newsatur) {}
boolean CONTROL_Startup(controltype which, int32 (*TimeFunction)(void), int32 ticspersecond) { return 0; }
void CONTROL_MapAnalogAxis(int32 whichaxis, int32 whichanalog, controldevice device) {}
void CONTROL_MapDigitalAxis(int32 whichaxis, int32 whichfunction, int32 direction, controldevice device) {}
void CONTROL_SetAnalogAxisScale(int32 whichaxis, int32 axisscale, controldevice device) {}
// TODO implement these two! or include keyboard.c
/*
char *KB_ScanCodeToString(kb_scancode scancode)
{
	return "";
}
kb_scancode KB_StringToScanCode(char * string)
{
	return 0;
}
*/

// Build TODO
int pathsearchmode;
int totalclock;
int wm_ynbox(const char *name, const char *fmt, ...) { return 0; }
int OSD_CaptureKey(int sc) { return 0; }
void OSD_Printf(const char *fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	vprintf(fmt, va);
	va_end(va);
}
void buildprintf(const char *fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	vprintf(fmt, va);
	va_end(va);
}
void uninitengine(void) {}
int openfrompath(const char *fn, int flags, int mode)
{
	return open(fn, flags, mode);
}
boff_t Bfilelength(int fd)
{
	struct stat st;
	if (fstat(fd, &st) < 0) return -1;
	return(boff_t)(st.st_size);
}
unsigned char bgetchar(void) { return 0; }
int bkbhit(void) { return 0; }
void bflushchars(void) {}
char keystatus[256];
void setkeypresscallback(void (*callback)(int,int)) {}

int   _buildargc = 1;
const char **_buildargv = NULL;

// Amiga stuff

static struct Screen *mysc = NULL;
static struct Gadget *glist = NULL;
static struct Window *mywin = NULL;
static APTR vi = NULL;

struct TextAttr Topaz80 = {"topaz.font", 8, 0, 0};

enum
{
	GAD_RESOLUTION,
	GAD_FULLSCREEN,
	GAD_CONTROLS,
	GAD_FUNCTIONS,
	GAD_KEYS,
	// don't use this
	GAD_MAX
};

struct Gadget *gadgets[GAD_MAX];

STRPTR resolutionLabels[] = 
{
	"320x200",
	"320x240",
	"400x300",
	"480x360",
	"512x384",
	"640x400",
	"640x480",
	"800x600",
	"1024x768",
	"1152x864",
	"1280x720",
	"1280x800",
	"1280x960",
	"1280x1024",
	"1366x768",
	"1440x900",
	"1400x1050",
	"1600x900",
	"1600x1200",
	NULL
};

STRPTR controlLabels[] =
{
	"Keyboard",
	"Mouse",
	"Joystick",
	NULL
};

struct Node functionNodes[kMaxGameFunctions];
struct List functionList;
struct Node keyNodes[sc_LastScanCode];
struct List keyList;

void HandleGadgetEvent(struct Gadget *gad, UWORD code)
{
	switch (gad->GadgetID)
	{
		// TODO
	}
}

void prepareLabels(void)
{
	struct Node *node;
	int i, j;

	NewList(&functionList);
	for (i = 0; i < kMaxGameFunctions; i++)
	{
		node = &functionNodes[i];
		node->ln_Name = gamefunctions[i];
		AddTail(&functionList, node);
	}
	/*for (node = functionList.lh_Head; node->ln_Succ; node = node->ln_Succ)
		printf("%p %s\n", node, node->ln_Name);*/

	NewList(&keyList);
	j = 0;
	for (i = 0; i < sc_LastScanCode; i++)
	{
		char *name = KB_ScanCodeToString(i);
		if (!name || !*name) continue;
		node = &keyNodes[j++];
		node->ln_Name = name;
		node->ln_Pri = 127 - name[0];
		AddTail(&keyList, node);
		//Enqueue(&keyList, node);
	}
	/*for (node = keyList.lh_Head; node->ln_Succ; node = node->ln_Succ)
		printf("%p %s\n", node, node->ln_Name);*/
}

int main(int argc, char *argv[])
{
	_buildargc = argc;
	_buildargv = (const char **)argv;

	if ((mysc = LockPubScreen(NULL)))
	{
		if ((vi = GetVisualInfo(mysc, TAG_DONE)))
		{
			struct Gadget *gad;
			if ((gad = CreateContext(&glist)))
			{
				struct NewGadget ng;
				WORD topBorder = mysc->WBorTop + (mysc->Font->ta_YSize + 1);
				WORD leftBorder = mysc->WBorLeft;
				WORD gap = 10;

				prepareLabels();

				ng.ng_VisualInfo = vi;
				ng.ng_TextAttr = NULL;
				ng.ng_UserData = NULL;
				ng.ng_Flags = 0;
				ng.ng_TextAttr = &Topaz80;

				ng.ng_Flags = PLACETEXT_LEFT;
				ng.ng_TopEdge = topBorder + gap;
				ng.ng_LeftEdge = leftBorder + gap;
				ng.ng_Width = 100;
				ng.ng_Height = 14;
				ng.ng_GadgetText = "Resolution:";
				ng.ng_GadgetID = GAD_RESOLUTION;
				gadgets[GAD_RESOLUTION] = gad = CreateGadget(CYCLE_KIND, gad, &ng,
					GTCY_Labels, (IPTR)resolutionLabels,
					//GTCY_Active, 3,
					TAG_DONE);
				gad->LeftEdge += IntuiTextLength(gad->GadgetText) + 10;

				ng.ng_Flags = PLACETEXT_RIGHT;
				ng.ng_LeftEdge = gad->LeftEdge + gad->Width + gap;
				ng.ng_GadgetText = "Fullscreen";
				ng.ng_GadgetID = GAD_FULLSCREEN;
				gadgets[GAD_FULLSCREEN] = gad = CreateGadget(CHECKBOX_KIND, gad, &ng,
					GTCB_Checked, FALSE,
					TAG_DONE);

				ng.ng_Flags = PLACETEXT_LEFT;
				ng.ng_LeftEdge = leftBorder + gap + 80;
				ng.ng_TopEdge = gad->TopEdge + gad->Height + Topaz80.ta_YSize + gap;
				ng.ng_GadgetID = GAD_CONTROLS;
				gadgets[GAD_CONTROLS] = gad = CreateGadget(MX_KIND, gad, &ng,
					GTMX_Labels, (IPTR)controlLabels,
					//GTMX_Active, 0,
					GTMX_Spacing, 4,
					TAG_DONE);
				//gad->LeftEdge += IntuiTextLength(gad->GadgetText) + 10;

				ng.ng_Flags = PLACETEXT_ABOVE;
				ng.ng_LeftEdge = gad->LeftEdge + gad->Width + gap;
				ng.ng_TopEdge = gad->TopEdge;
				ng.ng_Width = 170;
				ng.ng_Height = 120;
				ng.ng_GadgetText = "Functions:";
				ng.ng_GadgetID = GAD_FUNCTIONS;
				gadgets[GAD_FUNCTIONS] = gad = CreateGadget(LISTVIEW_KIND, gad, &ng,
					GTLV_Labels, (IPTR)&functionList,
					GTLV_ScrollWidth, 18,
					TAG_DONE);

				ng.ng_Flags = PLACETEXT_ABOVE;
				ng.ng_LeftEdge = gad->LeftEdge + gad->Width + gap*3;
				ng.ng_TopEdge = gad->TopEdge;
				ng.ng_Width = 170;
				ng.ng_Height = 120;
				ng.ng_GadgetText = "Keys:";
				ng.ng_GadgetID = GAD_KEYS;
				gadgets[GAD_KEYS] = gad = CreateGadget(LISTVIEW_KIND, gad, &ng,
					GTLV_Labels, (IPTR)&keyList,
					GTLV_ScrollWidth, 18,
					TAG_DONE);

				if ((mywin = OpenWindowTags(NULL,
					WA_IDCMP, IDCMP_MOUSEBUTTONS | IDCMP_GADGETUP | IDCMP_GADGETDOWN | IDCMP_MOUSEMOVE | IDCMP_CLOSEWINDOW /*| IDCMP_REFRESHWINDOW*/ | IDCMP_INTUITICKS,
					WA_Flags, WFLG_ACTIVATE | WFLG_DRAGBAR /*| WFLG_SIZEGADGET*/ | WFLG_DEPTHGADGET | WFLG_CLOSEGADGET /*| WFLG_SIMPLE_REFRESH*/,
					WA_Gadgets, (ULONG)glist,
					WA_Title, (ULONG)"PCExhumed Setup",
					WA_InnerWidth, 630,
					WA_InnerHeight, 180,
					WA_AutoAdjust, TRUE,
					WA_PubScreen, (ULONG)mysc,
					TAG_DONE)))
				{
					struct IntuiMessage *imsg;
					BOOL terminated = FALSE;

					GT_RefreshWindow(mywin, NULL);

					while (!terminated)
					{
						WaitPort(mywin->UserPort);
						while (!terminated && (imsg = GT_GetIMsg(mywin->UserPort)))
						{
							gad = (struct Gadget *)imsg->IAddress;
							switch (imsg->Class)
							{
							case IDCMP_MOUSEMOVE:
							case IDCMP_GADGETUP:
							case IDCMP_GADGETDOWN:
								HandleGadgetEvent(gad, imsg->Code);
								break;

							case IDCMP_CLOSEWINDOW:
								terminated = TRUE;
								break;
							}
							GT_ReplyIMsg(imsg);
						}
					}
					CloseWindow(mywin);
				}
			}
			FreeGadgets(glist);
			FreeVisualInfo(vi);
		}
		UnlockPubScreen(NULL, mysc);
	}

	return 0;
}

