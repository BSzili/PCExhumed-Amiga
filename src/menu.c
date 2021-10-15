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

#include "compat.h"
#include "build.h"
#include "exhumed.h"
#include "typedefs.h"
#include "player.h"
#include "sequence.h"
#include "menu.h"
#include "names.h"
#include "engine.h"
#include "keyboard.h"
#include "status.h"
#include "random.h"
#include "sound.h"
#include "names.h"
#include "init.h"
#include "input.h"
#include "gun.h"
#include "view.h"
#include "object.h"
#include "light.h"
#include "cd.h"
#include "config.h"
#ifdef EDUKE32
#include <string>
#endif

#include <assert.h>

#ifdef __WATCOMC__
#include <stdlib.h>
#endif

#define kSaveFileName       "savgamea.sav"
#define kMaxSaveSlots		5
#define kMaxSaveSlotChars	25

GameStat GameStats;

short nCinemaSeen[30];

#ifdef EDUKE32
// this might be static within the DoPlasma function?
uint8_t plasmaBuffer[25600];
#endif

uint8_t energytile[66 * 66] = {0};

uint8_t cinemapal[768];
short nLeft[50] = {0};
int line;

short SavePosition = -1;

uint8_t *cur;
uint8_t *dest;

unsigned int nSmokeBottom;
unsigned int nSmokeRight;
unsigned int nSmokeTop;
unsigned int nSmokeLeft;

unsigned int nRandom = 0x41C6167E;
int dword_9AB57 = 0x1F;
short word_9AB5B = 0;

int keytimer = 0;

int plasma_A[5] = {0};
int plasma_B[5] = {0};
int plasma_C[5] = {0};

short nMenuKeys[] = { sc_N, sc_L, sc_M, sc_V, sc_Q, sc_None }; // select a menu item using the keys. 'N' for New Gane, 'V' for voume etc. 'M' picks Training for some reason...

int zoomsize = 0;

void menu_ResetKeyTimer();

enum {
    kMenuNewGame = 0,
    kMenuLoadGame,
    kMenuTraining,
    kMenuVolume,
    kMenuQuitGame,
    kMenuMaxItems
};


void ClearCinemaSeen()
{
    memset(nCinemaSeen, 0, sizeof(nCinemaSeen));
}

unsigned int menu_RandomBit2()
{
    unsigned int result = nRandom & 1;

    if ( --dword_9AB57 > 0 )
    {
        nRandom = (result << 31) | (nRandom >> 1);
    }
    else
    {
        dword_9AB57 = 31;
        nRandom ^= nRandom >> 4;
    }
    return result;
}

int menu_RandomLong2()
{
    int randLong = 0;

    for (int i = 0; i < 32; i++)
    {
        int val = menu_RandomBit2();
        randLong *= 2;
        randLong |= val;
    }

    return randLong;
}

void InitEnergyTile()
{
    memset(energytile, 96, sizeof(energytile));
    for (int i = 0; i < 16; i++)
    {
        int nTile = kClockSymbol1 + i;
        if (waloff[nTile] == 0) tileLoad(nTile);
    }
}

void DoEnergyTile()
{
    if (!waloff[kEnergy1]) tileLoad(kEnergy1);
    if (!waloff[kEnergy2]) tileLoad(kEnergy2);

    nButtonColor += nButtonColor < 0 ? 8 : 0;

    uint8_t *ptr1 = (uint8_t*)(waloff[kEnergy1] + 1984);
    uint8_t *ptr2 = (uint8_t*)(waloff[kEnergy1] + 2048);

    short nColor = nButtonColor + 161;

    int i, j;

    for (i = 0; i < 32; i++)
    {
        memset(ptr1, nColor, 64);
        memset(ptr2, nColor, 64);

        ptr1 -= 64;
        ptr2 += 64;

        nColor++;

        if (nColor >= 168) {
            nColor = 160;
        }
    }

    tileInvalidate(kEnergy1, -1, -1);

    if (nSmokeSparks)
    {
        uint8_t *c = &energytile[67]; // skip a line
        uint8_t *ptrW = (uint8_t*)waloff[kEnergy2];

        for (i = 0; i < 64; i++)
        {
            for (j = 0; j < 64; j++)
            {
                uint8_t val = *c;

                if (val != 96)
                {
                    if (val > 158) {
                        *ptrW = val - 1;
                    }
                    else {
                        *ptrW = 96;
                    }
                }
                else
                {
                    if (menu_RandomBit2()) {
                        *ptrW = *c;
                    }
                    else
                    {
                        uint8_t al = *(c + 1);
                        uint8_t ah = *(c - 1);

                        if (al <= ah) {
                            al = ah;
                        }

                        uint8_t cl = al;

                        al = *(c - 66);
                        if (cl <= al) {
                            cl = al;
                        }

                        al = *(c + 66);
                        if (cl <= al) {
                            cl = al;
                        }

                        al = *(c + 66);
                        if (cl <= al) {
                            cl = al;
                        }

                        al = *(c + 66);
                        if (cl <= al) {
                            cl = al;
                        }

                        al = *(c - 65);
                        if (cl <= al) {
                            cl = al;
                        }

                        al = *(c - 67);
                        if (cl > al) {
                            al = cl;
                        }

                        cl = al;

                        if (al <= 159) {
                            *ptrW = 96;
                        }
                        else
                        {
                            if (!menu_RandomBit2()) {
                                cl--;
                            }

                            *ptrW = cl;
                        }
                    }     
                }

                c++;
                ptrW++;
            }

            c += 2;
        }

        c = &energytile[67];
        ptrW = (uint8_t*)waloff[kEnergy2];

        // copy back to energytile[]
        for (i = 0; i < 64; i++)
        {
            memcpy(c, ptrW, 64);
            c += 66;
            ptrW += 64;
        }

        ptrW = (uint8_t*)waloff[kEnergy2];

        // kEnergy2 is 64 x 64
        for (i = 0; i < 4096; i++)
        {
            if (ptrW[i] == 96) {
                ptrW[i] = 255; // -1?
            }
        }

        word_9AB5B--;
        if (word_9AB5B <= 0)
        {
            int randSize  = (RandomSize(5) & 0x1F) + 16;
            int randSize2 = (RandomSize(5) & 0x1F) + 16;

            int val = randSize << 5;
            val += randSize;
            val *= 2;
            val += randSize2;

            assert(val < 4356);

            energytile[val] = 175;
            word_9AB5B = 1;
        }
        tileInvalidate(kEnergy2, -1, -1);
    }
}

int nPlasmaTile = kTile4092;
int nLogoTile;

#define kPlasmaWidth	320
#define kPlasmaHeight	80

void menu_DoPlasma()
{
    if (!nLogoTile)
        nLogoTile = EXHUMED ? kExhumedLogo : kPowerslaveLogo;
    if (waloff[kTile4092] == 0)
    {
        tileCreate(kTile4092, kPlasmaWidth, kPlasmaHeight);

        memset((void*)waloff[kTile4092], 96, kPlasmaWidth*kPlasmaHeight);

#ifndef EDUKE32
        tileCreate(kTile4093, kPlasmaWidth, kPlasmaHeight);
        memset((void*)waloff[kTile4093], 96, kPlasmaWidth*kPlasmaHeight);
#else
        waloff[kTile4093] = (intptr_t)plasmaBuffer;
        memset(plasmaBuffer, 96, sizeof(plasmaBuffer));
#endif

#ifndef EDUKE32
        nSmokeLeft = 160 - tilesizx[nLogoTile] / 2;
        nSmokeRight = nSmokeLeft + tilesizx[nLogoTile];

        tilesizx[kTile4093] = kPlasmaWidth;
        tilesizy[kTile4093] = kPlasmaHeight;

        nSmokeTop    = 40 - tilesizy[nLogoTile] / 2;
        nSmokeBottom = nSmokeTop + tilesizy[nLogoTile] - 1;
#else
        nSmokeLeft = 160 - tilesiz[nLogoTile].x / 2;
        nSmokeRight = nSmokeLeft + tilesiz[nLogoTile].x;

        tilesiz[kTile4093].x = kPlasmaWidth;
        tilesiz[kTile4093].y = kPlasmaHeight;

        nSmokeTop    = 40 - tilesiz[nLogoTile].y / 2;
        nSmokeBottom = nSmokeTop + tilesiz[nLogoTile].y - 1;
#endif

        //uint32_t t = time(0) << 16;
        //uint32_t t2 = time(0) | t;
        nRandom = timerGetPerformanceCounter();

        for (int i = 0; i < 5; i++)
        {
#ifndef EDUKE32
            int logoWidth = tilesizx[nLogoTile];
#else
            int logoWidth = tilesiz[nLogoTile].x;
#endif
#if 1
            plasma_C[i] = (nSmokeLeft + wrand() % logoWidth) << 16;
            plasma_B[i] = (menu_RandomLong2() % 327680) + 0x10000;
#else
            int r = wrand();
            int rand2 = menu_RandomLong2();

            __asm {
                mov		ebx, i
                mov		ecx, logoWidth
                mov     eax, r
                mov		edx, eax
                sar     edx, 31
                idiv    ecx

                add     edx, nSmokeLeft
                shl     edx, 16
                mov     ecx, 327680
                mov     plasma_C[ebx * 4], edx
                xor     edx, edx
                mov		eax, rand2
//				call    menu_RandomLong2
                div     ecx
                add     edx, 10000h
                mov     plasma_B[ebx * 4], edx
            };
#endif

            if (menu_RandomBit2()) {
                plasma_B[i] = -plasma_B[i];
            }

            plasma_A[i] = menu_RandomBit2();
        }
    }

#ifdef __AMIGA__
    int time = (int)totalclock + 4;
#endif

    videoClearScreen(overscanindex);

    uint8_t *r_ebx = (uint8_t*)waloff[nPlasmaTile] + 81;
    uint8_t *r_edx = (uint8_t*)waloff[nPlasmaTile ^ 1] + 81; // flip between value of 4092 and 4093 with xor

    for (int x = 0; x < kPlasmaWidth - 2; x++)
//	for (int x = 1; x < 318; x++)
    {
//		for (int y = 1; y < 79; y++)
        for (int y = 0; y < kPlasmaHeight - 2; y++)
        {
            uint8_t al = *r_edx;

            if (al != 96)
            {
                if (al > 158) {
                    *r_ebx = al - 1;
                }
                else {
                    *r_ebx = 96;
                }
            }
            else
            {
                if (menu_RandomBit2()) {
                    *r_ebx = *r_edx;
                }
                else
                {
                    uint8_t al = *(r_edx + 1);
                    uint8_t cl = *(r_edx - 1);

                    if (al <= cl) {
                        al = cl;
                    }

                    cl = al;
                    al = *(r_edx - 80);
                    if (cl <= al) {
                        cl = al;
                    }

                    al = *(r_edx + 80);
                    if (cl <= al) {
                        cl = al;
                    }

                    al = *(r_edx + 80);
                    if (cl <= al) {
                        cl = al;
                    }

                    al = *(r_edx + 80);
                    if (cl <= al) {
                        cl = al;
                    }

                    al = *(r_edx - 79);
                    if (cl > al) {
                        al = cl;
                    }

                    cl = *(r_edx - 81);
                    if (al <= cl) {
                        al = cl;
                    }

                    cl = al;

                    if (al <= 159) {
                        *r_ebx = 96;
                    }
                    else
                    {
                        if (!menu_RandomBit2()) {
                            cl--;
                        }

                        *r_ebx = cl;
                    }
                }
            }

            // before restarting inner loop
            r_edx++;
            r_ebx++;
        }

        // before restarting outer loop
        r_edx += 2;
        r_ebx += 2;
    }

    if (!waloff[nLogoTile]) tileLoad(nLogoTile);

    for (int j = 0; j < 5; j++)
    {
        int pB = plasma_B[j];
        int pC = plasma_C[j];
        int badOffset =  (pC>>16) < nSmokeLeft || (pC>>16) >= nSmokeRight;

#ifndef EDUKE32
        uint8_t *ptr3 = (uint8_t*)(waloff[nLogoTile] + ((pC >> 16) - nSmokeLeft) * tilesizy[nLogoTile]);
#else
        uint8_t *ptr3 = (uint8_t*)(waloff[nLogoTile] + ((pC >> 16) - nSmokeLeft) * tilesiz[nLogoTile].y);
#endif

        plasma_C[j] += plasma_B[j];

        if ((pB > 0 && (plasma_C[j] >> 16) >= nSmokeRight) || (pB < 0 && (plasma_C[j] >> 16) <= nSmokeLeft))
        {
            int esi = plasma_A[j];
            plasma_B[j] = -plasma_B[j];
            plasma_A[j] = esi == 0;
        }

        if (badOffset)
            continue;

        unsigned int nSmokeOffset = 0;

        if (plasma_A[j])
        {
            nSmokeOffset = nSmokeTop;

            while (nSmokeOffset < nSmokeBottom)
            {
                uint8_t al = *ptr3;
                if (al != 255 && al != 96) {
                    break;
                }

                nSmokeOffset++;
                ptr3++;
            }
        }
        else
        {
            nSmokeOffset = nSmokeBottom;

#ifndef EDUKE32
            ptr3 += tilesizy[nLogoTile] - 1;
#else
            ptr3 += tilesiz[nLogoTile].y - 1;
#endif

            while (nSmokeOffset > nSmokeTop)
            {
                uint8_t al = *ptr3;
                if (al != 255 && al != 96) {
                    break;
                }

                nSmokeOffset--;
                ptr3--;
            }
        }

        uint8_t *v28 = (uint8_t*)(80 * (plasma_C[j] >> 16) + waloff[nPlasmaTile]);
        v28[nSmokeOffset] = 175;
    }

    tileInvalidate(nPlasmaTile,-1,-1);

    overwritesprite(0,   0,  nPlasmaTile,  0, 2, kPalNormal);
    overwritesprite(160, 40, nLogoTile, 0, 3, kPalNormal);

    // flip between tile 4092 and 4093
    if (nPlasmaTile == kTile4092) {
        nPlasmaTile = kTile4093;
    }
    else if (nPlasmaTile == kTile4093) {
        nPlasmaTile = kTile4092;
    }

    // draw the fire urn/lamp thingies
    int dword_9AB5F = ((int)totalclock/16) & 3;

    overwritesprite(50,  150, kTile3512 + dword_9AB5F, 0, 3, kPalNormal);
    overwritesprite(270, 150, kTile3512 + ((dword_9AB5F + 2) & 3), 0, 3, kPalNormal);

    // TEMP
#ifndef __AMIGA__
    int time = (int)totalclock + 4;
#endif
    while ((int)totalclock < time) {
        HandleAsync();
    }
}


int8_t MapLevelOffsets[] = { 0, 50, 10, 20, 0, 45, -20, 20, 5, 0, -10, 10, 30, -20, 0, 20, 0, 0, 0, 0 };

struct TILEFRAMEDEF
{
    short nTile;
    short xOffs;
    short yOffs;
};
#ifndef EDUKE32
typedef struct TILEFRAMEDEF TILEFRAMEDEF;
#endif

// 22 bytes
struct MapNamePlaque
{
    short xPos;
    short yPos;
    TILEFRAMEDEF tiles[2];
    TILEFRAMEDEF text;
};
#ifndef EDUKE32
typedef struct MapNamePlaque MapNamePlaque;
#endif

MapNamePlaque mapNamePlaques[] = {
    { 100, 170, {{kTile3376, 0, 0}, {kTile3377, 0, 0}}, {kTile3411, 18, 6} },
    { 230, 10,  {{kTile3378, 0, 0}, {kTile3379, 0, 0}}, {kTile3414, 18, 6} }, // DENDUR (level 2)
    { 180, 125, {{kTile3380, 0, 0}, {kTile3381, 0, 0}}, {kTile3417, 18, 6} }, // Kalabash
    { 10,  95,  {{kTile3382, 0, 0}, {kTile3383, 0, 0}}, {kTile3420, 18, 6} },
    { 210, 160, {{kTile3384, 0, 0}, {kTile3385, 0, 0}}, {kTile3423, 18, 6} },
    { 10,  110, {{kTile3371, 0, 0}, {kTile3386, 0, 0}}, {kTile3426, 18, 6} },
    { 10,  50,  {{kTile3387, 0, 0}, {kTile3388, 0, 0}}, {kTile3429, 18, 6} },
    { 140, 0,   {{kTile3389, 0, 0}, {kTile3390, 0, 0}}, {kTile3432, 18, 6} },
    { 30,  20,  {{kTile3391, 0, 0}, {kTile3392, 0, 0}}, {kTile3435, 18, 6} },
    { 200, 150, {{kTile3409, 0, 0}, {kTile3410, 0, 0}}, {kTile3418, 20, 4} },
    { 145, 170, {{kTile3393, 0, 0}, {kTile3394, 0, 0}}, {kTile3438, 18, 6} },
    { 80,  80,  {{kTile3395, 0, 0}, {kTile3396, 0, 0}}, {kTile3441, 18, 6} },
    { 15,  0,   {{kTile3397, 0, 0}, {kTile3398, 0, 0}}, {kTile3444, 18, 5} },
    { 220, 35,  {{kTile3399, 0, 0}, {kTile3400, 0, 0}}, {kTile3447, 18, 6} },
    { 190, 40,  {{kTile3401, 0, 0}, {kTile3402, 0, 0}}, {kTile3450, 18, 6} },
    { 20,  130, {{kTile3403, 0, 0}, {kTile3404, 0, 0}}, {kTile3453, 19, 6} },
    { 220, 160, {{kTile3405, 0, 0}, {kTile3406, 0, 0}}, {kTile3456, 18, 6} },
    { 20,  10,  {{kTile3407, 0, 0}, {kTile3408, 0, 0}}, {kTile3459, 18, 6} },
    { 200, 10,  {{kTile3412, 0, 0}, {kTile3413, 0, 0}}, {kTile3419, 18, 5} },
    { 20,  10,  {{kTile3415, 0, 0}, {kTile3416, 0, 0}}, {kTile3421, 19, 4} }
};

// 3 different types of fire, each with 4 frames
TILEFRAMEDEF FireTiles[3][4] = {
    {{ kTile3484,0,3 },{ kTile3485,0,0 },{ kTile3486,0,3 },{ kTile3487,0,0 }},
    {{ kTile3488,1,0 },{ kTile3489,1,0 },{ kTile3490,0,1 },{ kTile3491,1,1 }},
    {{ kTile3492,1,2 },{ kTile3493,1,0 },{ kTile3494,1,2 },{ kTile3495,1,0 }}
};

struct Fire
{
    short nFireType;
    short xPos;
    short yPos;
};
#ifndef EDUKE32
typedef struct Fire Fire;
#endif

// 20 bytes
struct MapFire
{
    short nFires;
    Fire fires[3];
};
#ifndef EDUKE32
typedef struct MapFire MapFire;
#endif

/*
 level 1 - 3 fires
 level 2 - 3 fires
 level 3 - 1 fire

*/

MapFire MapLevelFires[] = {
    {3, {{0, 107, 95}, {1, 58,  140}, {2, 28,   38}}},
    {3, {{2, 240,  0}, {0, 237,  32}, {1, 200,  30}}},
    {2, {{2, 250, 57}, {0, 250,  43}, {2, 200,  70}}},
    {2, {{1, 82,  59}, {2, 84,   16}, {0, 10,   95}}},
    {2, {{2, 237, 50}, {1, 215,  42}, {1, 210,  50}}},
    {3, {{0, 40,   7}, {1, 75,    6}, {2, 100,  10}}},
    {3, {{0, 58,  61}, {1, 85,   80}, {2, 111,  63}}},
    {3, {{0, 260, 65}, {1, 228,   0}, {2, 259,  15}}},
    {2, {{0, 81,  38}, {2, 58,   38}, {2, 30,   20}}},
    {3, {{0, 259, 49}, {1, 248,  76}, {2, 290,  65}}},
    {3, {{2, 227, 66}, {0, 224,  98}, {1, 277,  30}}},
    {2, {{0, 100, 10}, {2, 48,   76}, {2, 80,   80}}},
    {3, {{0, 17,   2}, {1, 29,   49}, {2, 53,   28}}},
    {3, {{0, 266, 42}, {1, 283,  99}, {2, 243, 108}}},
    {2, {{0, 238, 19}, {2, 240,  92}, {2, 190,  40}}},
    {2, {{0, 27,   0}, {1, 70,   40}, {0, 20,  130}}},
    {3, {{0, 275, 65}, {1, 235,   8}, {2, 274,   6}}},
    {3, {{0, 75,  45}, {1, 152, 105}, {2, 24,   68}}},
    {3, {{0, 290, 25}, {1, 225,  63}, {2, 260, 110}}},
    {0, {{1, 20,  10}, {1, 20,   10}, {1, 20,   10}}}
};

int menu_DrawTheMap(int nLevel, int nLevelNew, int nLevelBest)
{
    int i;
    int x = 0;
    int var_2C = 0;
    int nIdleSeconds = 0;
    int bFadeDone = kFalse;

    int startTime = (int)totalclock;

    ClearAllKeys();
    UnMaskStatus();
    videoSetViewableArea(0, 0, xdim - 1, ydim - 1);

    // 0-offset the level numbers
    nLevel--;
    nLevelNew--;
    nLevelBest--;

    if (nLevel >= kMap20) { // max single player levels
        return -1;
    }

    if (nLevelNew >= kMap20) {
        return -1;
    }

    if (nLevel < 0) {
        nLevel = 0;
    }

    if (nLevelNew < 0) {
        nLevelNew = nLevel;
    }

    int curYPos = MapLevelOffsets[nLevel] + (200 * (nLevel / 2));
    int destYPos = MapLevelOffsets[nLevelNew] + (200 * (nLevelNew / 2));

    if (curYPos < destYPos) {
        var_2C = 2;
    }

    if (curYPos > destYPos) {
        var_2C = -2;
    }

    int runtimer = (int)totalclock;

    // Trim smoke in widescreen
#ifndef EDUKE32
    vec2_t mapwinxy1 = {windowx1,windowy1};
    vec2_t mapwinxy2 = {windowx2,windowy2};
#else
    vec2_t mapwinxy1 = windowxy1, mapwinxy2 = windowxy2;
#endif
    int32_t width = mapwinxy2.x - mapwinxy1.x + 1, height = mapwinxy2.y - mapwinxy1.y + 1;
    if (3 * width > 4 * height)
    {
        mapwinxy1.x += (width - 4 * height / 3) / 2;
        mapwinxy2.x -= (width - 4 * height / 3) / 2;
    }

    // User has 12 seconds to do something on the map screen before loading the current level
    while (nIdleSeconds < 12)
    {
        HandleAsync();

#ifndef EDUKE32
        if (widescreen)
#endif
        videoClearScreen(overscanindex); // fix hall of mirrors when console renders offscreen in widescreen resolutions.

        if (((int)totalclock - startTime) / kTimerTicks)
        {
            nIdleSeconds++;
            startTime = (int)totalclock;
        }

        int tileY = curYPos;

        // Draw the background screens
        for (i = 0; i < 10; i++)
        {
            overwritesprite(x, tileY, kTile3353 + i, 0, 2, kPalNormal);
            tileY -= 200;
        }

        // for each level - drawing the 'level completed' on-fire smoke markers
        for (i = 0; i < kMap20; i++)
        {
            int screenY = (i >> 1) * -200;

            if (nLevelBest >= i) // check if the player has finished this level
            {
                for (int j = 0; j < MapLevelFires[i].nFires; j++)
                {
                    int nFireFrame = (((int)totalclock >> 4) & 3);
                    assert(nFireFrame >= 0 && nFireFrame < 4);

                    int nFireType = MapLevelFires[i].fires[j].nFireType;
                    assert(nFireType >= 0 && nFireType < 3);

                    int nTile = FireTiles[nFireType][nFireFrame].nTile;
                    int smokeX = MapLevelFires[i].fires[j].xPos + FireTiles[nFireType][nFireFrame].xOffs;
                    int smokeY = MapLevelFires[i].fires[j].yPos + FireTiles[nFireType][nFireFrame].yOffs + curYPos + screenY;

                    // Use rotatesprite to trim smoke in widescreen
                    rotatesprite(smokeX << 16, smokeY << 16, 65536L, 0,
                                 nTile, 0, kPalNormal, 16 + 2, mapwinxy1.x, mapwinxy1.y, mapwinxy2.x, mapwinxy2.y);
//                    overwritesprite(smokeX, smokeY, nTile, 0, 2, kPalNormal);
                }
            }

            int t = ((((int)totalclock & 16) >> 4));

            int nTile = mapNamePlaques[i].tiles[t].nTile;

            int nameX = mapNamePlaques[i].xPos + mapNamePlaques[i].tiles[t].xOffs;
            int nameY = mapNamePlaques[i].yPos + mapNamePlaques[i].tiles[t].yOffs + curYPos + screenY;

            // Draw level name plaque
            overwritesprite(nameX, nameY, nTile, 0, 2, kPalNormal);

            int8_t shade = 96;

            if (nLevelNew == i)
            {
                shade = (Sin(16 * (int)totalclock) + 31) >> 8;
            }
            else if (nLevelBest >= i)
            {
                shade = 31;
            }

            int textY = mapNamePlaques[i].yPos + mapNamePlaques[i].text.yOffs + curYPos + screenY;
            int textX = mapNamePlaques[i].xPos + mapNamePlaques[i].text.xOffs;
            nTile = mapNamePlaques[i].text.nTile;

            // draw the text, alternating between red and black
            overwritesprite(textX, textY, nTile, shade, 2, kPalNormal);
        }

        videoNextPage();
        if (!bFadeDone)
        {
            bFadeDone = kTrue;
            FadeIn();
        }

        if (curYPos == destYPos)
        {
            if (KB_KeyDown[sc_UpArrow])
            {
                KB_KeyDown[sc_UpArrow] = 0;

                if (nLevelNew <= nLevelBest)
                {
                    nLevelNew++;
                    assert(nLevelNew < 20);

                    destYPos = MapLevelOffsets[nLevelNew] + (200 * (nLevelNew / 2));

                    if (curYPos <= destYPos) {
                        var_2C = 2;
                    }
                    else {
                        var_2C = -2;
                    }

                    nIdleSeconds = 0;
                }
            }

            if (KB_KeyDown[sc_DownArrow])
            {
                KB_KeyDown[sc_DownArrow] = 0;

                if (nLevelNew > 0)
                {
                    nLevelNew--;
                    assert(nLevelNew >= 0);

                    destYPos = MapLevelOffsets[nLevelNew] + (200 * (nLevelNew / 2));

                    if (curYPos <= destYPos) {
                        var_2C = 2;
                    }
                    else {
                        var_2C = -2;
                    }

                    nIdleSeconds = 0;
                }
            }

            if (KB_KeyDown[sc_Escape] || KB_KeyDown[sc_Space] || KB_KeyDown[sc_Return])
            {
                KB_KeyDown[sc_Escape] = 0;
                KB_KeyDown[sc_Return] = 0;
                KB_KeyDown[sc_Space] = 0;
                return nLevelNew + 1;
            }
        }
        else
        {
            // scroll the map every couple of ms
            if (totalclock - runtimer >= (kTimerTicks / 32)) {
                curYPos += var_2C;
                runtimer = (int)totalclock;
            }

            if (KB_KeyDown[sc_Escape] || KB_KeyDown[sc_Space] || KB_KeyDown[sc_Return])
            {
                if (var_2C < 8) {
                    var_2C *= 2;
                }

                KB_KeyDown[sc_Escape] = 0;
                KB_KeyDown[sc_Return] = 0;
                KB_KeyDown[sc_Space] = 0;
            }

            if (curYPos > destYPos&& var_2C > 0) {
                curYPos = destYPos;
            }

            if (curYPos < destYPos && var_2C < 0) {
                curYPos = destYPos;
            }

            nIdleSeconds = 0;
        }
    }

    MySetView(nViewLeft, nViewTop, nViewRight, nViewBottom);
    return nLevelNew + 1;
}

void menu_AdjustVolume()
{
    int nOption = 1;
    int var_8 = 0;

    while (1)
    {
        HandleAsync();

        menu_DoPlasma();

        overwritesprite(80, 50, kMenuMusicTile, (Sin((int)totalclock << 4) >> 9) * (nOption == 0), 2, kPalNormal);
        overwritesprite(55, 75, kMenuBlankTitleTile, 0, 2, kPalNormal);

        seq_DrawGunSequence(
            SeqOffsets[kSeqSlider], // eax
            MusicVolume % 3, // pick one of 3 frames?
            (MusicVolume >> 1) - 93, // ebx. must be x???
            -22,
            0,
            0);

        overwritesprite(80, 110, kMenuSoundFxTile, (Sin((int)totalclock << 4) >> 9) * (nOption == 1), 2, kPalNormal);
        overwritesprite(55, 135, kMenuBlankTitleTile, 0, 2, kPalNormal);

        seq_DrawGunSequence(
            SeqOffsets[kSeqSlider],
            FXVolume % 3,
            (FXVolume / 2) - 93,
            38,
            0,
            0);

        int y = (60 * nOption) + 38;

        overwritesprite(60,  y, kMenuCursorTile, 0, 2, kPalNormal);
        overwritesprite(206, y, kMenuCursorTile, 0, 10, kPalNormal);

        videoNextPage();

        if (KB_KeyDown[sc_Escape] || KB_KeyDown[sc_Return] || KB_KeyDown[sc_Space])
        {
            PlayLocalSound(StaticSound[kSoundSwitchFoot], 0);
            KB_KeyDown[sc_Escape] = 0;
            KB_KeyDown[sc_Space]  = 0;
            KB_KeyDown[sc_Return] = 0;
            return;
        }

        if (KB_KeyDown[sc_UpArrow])
        {
            if (nOption > 0)
            {
                nOption--;
                PlayLocalSound(StaticSound[kSoundSwitchWtr1], 0);
            }

            KB_KeyDown[sc_UpArrow] = 0;
        }

        if (KB_KeyDown[sc_DownArrow])
        {
            if (nOption < 1)
            {
                nOption++;
                PlayLocalSound(StaticSound[kSoundSwitchWtr1], 0);
            }

            KB_KeyDown[sc_DownArrow] = 0;
        }

        if ((int)totalclock <= var_8) {
            continue;
        }

        var_8 = (int)totalclock + 5;

        if (KB_KeyDown[sc_LeftArrow])
        {
            switch (nOption)
            {
                case 0:
                {
                    if (MusicVolume > 3) {
                        MusicVolume -= 4;
                    }

// TODO				SetMusicVolume();
    				setCDaudiovolume(MusicVolume);
                    continue;
                }

                case 1:
                {
                    if (FXVolume > 3) {
                        FXVolume -= 4;
                    }

                    SetMasterFXVolume(FXVolume);

                    if (LocalSoundPlaying()) {
                        UpdateLocalSound();
                    }
                    else {
                        PlayLocalSound(StaticSound[kSoundAmbientStone], 0);
                    }
                    continue;
                }
            }
        }

        if (KB_KeyDown[sc_RightArrow])
        {
            switch (nOption)
            {
                case 0:
                {
                    if (MusicVolume < 252) {
                        MusicVolume += 4;
                    }

//  				SetMusicVolume();
    				setCDaudiovolume(MusicVolume);
                    continue;
                }

                case 1:
                {
                    if (FXVolume < 252) {
                        FXVolume += 4;
                    }

                    SetMasterFXVolume(FXVolume);

                    if (LocalSoundPlaying()) {
                        UpdateLocalSound();
                    }
                    else {
                        PlayLocalSound(StaticSound[kSoundAmbientStone], 0);
                    }
                    continue;
                }
            }
        }

        if (GetLocalSound() != 23) {
            continue;
        }
        else {
            StopLocalSound();
        }
    }
}

int menu_NewGameMenu()
{
    const char endMark = 0xF;
    char nameList[kMaxSaveSlots][kMaxSaveSlotChars];
    int nameListSize = sizeof(nameList);

    int nNameOffset = 0; // char index into slot name string
    int nSlot = 0;

#ifndef EDUKE32
    int arg_3E = tilesizx[kMenuBlankTitleTile] - 10;
#else
    int arg_3E = tilesiz[kMenuBlankTitleTile].x - 10;
#endif

    FILE *fp = fopen(kSaveFileName, "rb");
    if (fp == NULL)
    {
        memset(nameList,   0, nameListSize);
        memset(&GameStats, 0, sizeof(GameStat));

        fp = fopen(kSaveFileName, "wb+");
        if (fp != NULL)
        {
            // write new blank save file
            fwrite(nameList, nameListSize, 1, fp);
            for (int i = 0; i < kMaxSaveSlots; i++) {
                fwrite(&GameStats, sizeof(GameStats), 1, fp);
            }

            fwrite(&endMark, sizeof(endMark), 1, fp);
            fclose(fp);
        }
    }
    else
    {
        int nRead = fread(nameList, 1, nameListSize, fp);
        if (nRead != nameListSize)
        {
            memset(nameList, 0, nameListSize);
        }

        fclose(fp);
    }

    //	while (1)
    {
        ClearAllKeys();

        while (1)
        {
            HandleAsync();
            menu_DoPlasma();

#ifndef EDUKE32
            int y = (tilesizy[kMenuBlankTitleTile] - (tilesizy[kMenuBlankTitleTile] / 2) / 2) + 65;
#else
            int y = (tilesiz[kMenuBlankTitleTile].y - (tilesiz[kMenuBlankTitleTile].y / 2) / 2) + 65;
#endif
            rotatesprite(160 << 16, y << 16, 0x10000, 0, kMenuNewGameTile, 0, 0, 2, 0, 0, xdim, ydim);

            int edi = 0;

            int arg_4A = 90;
            int arg_4E = 98;

            // Loop #3
            for (int i = 0; i < 5; i++)
            {
                int8_t shade = ((Sin((int)totalclock << 4) >> 9) * (i == nSlot)) + ((i != nSlot) * 31);

                overwritesprite(55, arg_4A, kMenuBlankTitleTile, shade, 2, kPalNormal);
                myprintext(63, arg_4E, nameList[i], 0);

                arg_4E += 22;
                arg_4A += 22;

                edi++;
            }

            edi = nSlot * 22;

            // draw selection markers
            overwritesprite(35, edi + 78, kMenuCursorTile, 0, 2, kPalNormal);
            overwritesprite(233, edi + 78, kMenuCursorTile, 0, 10, kPalNormal);
            videoNextPage();

            //nPages--;
            //if (nPages > 0) {
            //    continue;
            //}

            if (KB_KeyDown[sc_Escape])
            {
                PlayLocalSound(StaticSound[kSoundSwitchFoot], 0);
                KB_KeyDown[sc_Escape] = 0;
                return -1;
            }

            if (KB_KeyDown[sc_UpArrow])
            {
                PlayLocalSound(StaticSound[kSoundSwitchWtr1], 0);
                if (nSlot <= 0) {
                    nSlot = 4;
                }
                else {
                    nSlot--;
                }

                KB_KeyDown[sc_UpArrow] = 0;
                ClearAllKeys();
                continue;
            }

            if (KB_KeyDown[sc_DownArrow])
            {
                PlayLocalSound(StaticSound[kSoundSwitchWtr1], 0);
                if (nSlot >= 4) {
                    nSlot = 0;
                }
                else {
                    nSlot++;
                }

                KB_KeyDown[sc_DownArrow] = 0;
                ClearAllKeys();
                continue;
            }

            if (KB_KeyDown[sc_Return] || KB_KeyWaiting())
            {
                break;
            }
        }
    }

    PlayLocalSound(StaticSound[kSoundSwitchFoot], 0);
    if (KB_KeyDown[sc_Return]) {
        ClearAllKeys();
    }

    char *pName = nameList[nSlot];
    int nNameLength = strlen(pName);

#ifdef __AMIGA__
    if (nNameLength > 0) // TODO why do I need this?!
#endif
    memset(pName, 0, nNameLength);

    menu_DoPlasma();
    overwritesprite(55, (nSlot * 22) + 90, kMenuBlankTitleTile, 0, 2, kPalNormal);

    int arg_5A = 90;
    int arg_52 = 98;

    for (int i = 0; i < 5; i++)
    {
        overwritesprite(55, arg_5A, kMenuBlankTitleTile, (i != nSlot) * 31, 2, kPalNormal);
        myprintext(63, arg_52, nameList[i], 0);

        arg_52 += 22;
        arg_5A += 22;
    }

    int x = 35;
    int y = (nSlot * 22) + 78;

    while (1)
    {
        HandleAsync();

        overwritesprite(x, y, kMenuCursorTile, 0, 2, kPalNormal);
        overwritesprite(233, y, kMenuCursorTile, 0, 10, kPalNormal);
        videoNextPage();

        char ch = 0;

check_keys:
        if (KB_KeyWaiting())
        {
            HandleAsync();

            ch = KB_GetCh();
            if (!ch)
            {
                KB_GetCh();
                goto check_keys;
            }

            // handle key input
            if (ch == asc_Enter)
            {
                // loc_39ACA:
                nameList[nSlot][nNameOffset] = 0;

                PlayLocalSound(StaticSound[kSoundSwitchFoot], 0);
                KB_KeyDown[sc_Return] = 0;

                if (nameList[nSlot][0] == 0) {
                    return -1;
                }

                if (nNameLength) // does the save slot already exist?
                {
                    menu_DoPlasma();
                    if (Query(2, 4, "Overwrite existing game?", "Y/N", 'Y', 13, 'N', 27) >= 2) {
                        return -1;
                    }
                }

                FILE *fp = fopen(kSaveFileName, "rb+");
                if (fp == NULL) {
                    return -1;
                }

                memset(&GameStats, 0, sizeof(GameStat));
                GameStats.nWeapons = 1;
                GameStats.nMap = 1;

                fwrite(nameList, sizeof(nameList), 1, fp);
                fseek(fp, sizeof(nameList), SEEK_SET);
                fseek(fp, nSlot * sizeof(GameStat), SEEK_CUR);
                fwrite(&GameStats, sizeof(GameStat), 1, fp);
                fclose(fp);
                return nSlot;
            }
            else
            {
                // Enter wasn't pressed
                PlayLocalSound(4, 0); // ??

                if (ch == asc_BackSpace)
                {
                    nameList[nSlot][nNameOffset] = 0;

                    if (nNameOffset > 0) {
                        nNameOffset--;
                    }

                    nameList[nSlot][nNameOffset] = 0;
                }
                else if (ch == asc_Escape)
                {
                    PlayLocalSound(StaticSound[kSoundSwitchFoot], 0);
                    KB_ClearKeysDown();
                    KB_FlushKeyboardQueue();
                    KB_KeyDown[sc_Escape] = 0;
                    return -1;
                }
                else
                {
                    // check if a slot name is being typed
                    if ((ch >= '0' && ch <= '9')
                    ||  (ch >= 'A' && ch <= 'Z')
                    ||  (ch >= 'a' && ch <= 'z')
                    ||  (ch == ' '))
                    {
                        ch = toupper(ch);
                        if (nNameOffset < 24) // n chars per slot name
                        {
                            nameList[nSlot][nNameOffset] = ch;
                            nNameOffset++;
                            nameList[nSlot][nNameOffset] = '\0'; // null terminate in the new offset

                            int nLen = MyGetStringWidth(nameList[nSlot]);
                            if (nLen > arg_3E)
                            {
                                nNameOffset--;
                                nameList[nSlot][nNameOffset] = '\0';
                            }
                        }
                    }
                }
            }
        }

        // loc_399FD:
        menu_DoPlasma();

        int arg_5E = ((int)totalclock / 30) & 1;

        int y = 90;
        int arg_42 = 98;

        for (int i = 0; i < 5; i++)
        {
            overwritesprite(55, y, kMenuBlankTitleTile, (i != nSlot) * 31, 2, kPalNormal);
            int nTextWidth = myprintext(63, arg_42, nameList[i], 0);

            // flash a full-stop to show the current typing position
            if (arg_5E != 0 && nSlot == i)
            {
                myprintext(nTextWidth, arg_42, ".", 0);
            }

            arg_42 += 22;
            y += 22;
        }
    }
}

int menu_LoadGameMenu()
{
    char nameList[5][25];

    int nSlot = 0;

    FILE *fp = fopen(kSaveFileName, "rb");
    if (fp == NULL)
    {
        memset(nameList, 0, sizeof(nameList));
    }
    else
    {
        fread(nameList, sizeof(nameList), 1, fp);
        fclose(fp);
    }

    while (1)
    {
        menu_DoPlasma();

        HandleAsync();

        overwritesprite(80, 65, kMenuLoadGameTile, 0, 2, kPalNormal);

        int spriteY = 90;
        int textY = 98;

        for (int i = 0; i < kMaxSaveSlots; i++)
        {
            int8_t shade = ((Sin((int)totalclock << 4) >> 9)* (i == nSlot)) + ((i != nSlot) * 31);
            overwritesprite(55, spriteY, kMenuBlankTitleTile, shade, 2, kPalNormal);

            myprintext(63, textY, nameList[i], 0);
            textY += 22;
            spriteY += 22;
        }

        int y = (nSlot * 22) + 78;

        overwritesprite(35,  y, kMenuCursorTile, 0, 2, kPalNormal);
        overwritesprite(233, y, kMenuCursorTile, 0, 10, kPalNormal);
        videoNextPage();

        if (KB_KeyDown[sc_Escape])
        {
            PlayLocalSound(StaticSound[kSoundSwitchFoot], 0);
            KB_KeyDown[sc_Escape] = 0;
            return -1;
        }

        if (KB_KeyDown[sc_UpArrow])
        {
            PlayLocalSound(StaticSound[kSoundSwitchWtr1], 0);
            if (nSlot > 0) {
                nSlot--;
            }
            else {
                nSlot = kMaxSaveSlots - 1;
            }

            KB_KeyDown[sc_UpArrow] = 0;
        }

        if (KB_KeyDown[sc_DownArrow]) // checkme - is 0x5b in disassembly
        {
            PlayLocalSound(StaticSound[kSoundSwitchWtr1], 0);
            if (nSlot < kMaxSaveSlots - 1) {
                nSlot++;
            }
            else {
                nSlot = 0;
            }

            KB_KeyDown[sc_DownArrow] = 0;
        }

        if (!KB_KeyDown[sc_Return]) {
            continue;
        }

        PlayLocalSound(StaticSound[kSoundSwitchFoot], 0);
        KB_KeyDown[sc_Return] = 0;
        KB_ClearKeysDown();
        KB_FlushKeyboardQueue();

        if (nameList[nSlot][0] != '\0')
        {
            PlayLocalSound(StaticSound[33], 0);
            return nSlot;
        }

        PlayLocalSound(4, 0);
    }
}

void menu_ResetKeyTimer()
{
    keytimer = (int)totalclock + 2400;
}

void menu_GameLoad2(buildvfs_kfd hFile, bool bIsDemo)
{
    if (bIsDemo)
    {
        demo_header header;
        kread(hFile, &header, sizeof(demo_header));

        GameStats.nMap = header.nMap;
        GameStats.nWeapons = B_LITTLE16(header.nWeapons);
        GameStats.nCurrentWeapon = B_LITTLE16(header.nCurrentWeapon);
        GameStats.clip = B_LITTLE16(header.clip);
        GameStats.items = B_LITTLE16(header.items);
        GameStats.player.nHealth = B_LITTLE16(header.nHealth);
        GameStats.player.nFrame = B_LITTLE16(header.nFrame);
        GameStats.player.nAction = B_LITTLE16(header.nAction);
        GameStats.player.nSprite = B_LITTLE16(header.nSprite);
        GameStats.player.bIsMummified = B_LITTLE16(header.bIsMummified);
        GameStats.player.someNetVal = B_LITTLE16(header.someNetVal);
        GameStats.player.invincibility = B_LITTLE16(header.invincibility);
        GameStats.player.nAir = B_LITTLE16(header.nAir);
        GameStats.player.nSeq = B_LITTLE16(header.nSeq);
        GameStats.player.nMaskAmount = B_LITTLE16(header.nMaskAmount);
        GameStats.player.keys = B_LITTLE16(header.keys);
        GameStats.player.nMagic = B_LITTLE16(header.nMagic);
        Bmemcpy(GameStats.player.items, header.item, sizeof(header.item));

        for (int i = 0; i < 7; i++)
        {
            GameStats.player.nAmmo[i] = B_LITTLE16(header.nAmmo[i]);
        }

        Bmemcpy(GameStats.player.pad, header.pad, sizeof(header.pad));
        GameStats.player.nCurrentWeapon = B_LITTLE16(header.nCurrentWeapon2);
        GameStats.player.nWeaponFrame = B_LITTLE16(header.nWeaponFrame);
        GameStats.player.bIsFiring = B_LITTLE16(header.bIsFiring);
        GameStats.player.nNewWeapon = B_LITTLE16(header.nNewWeapon);
        GameStats.player.nWeaponState = B_LITTLE16(header.nWeaponState);
        GameStats.player.nLastWeapon = B_LITTLE16(header.nLastWeapon);
        GameStats.player.nRun = B_LITTLE16(header.nRun);
        GameStats.nLives = B_LITTLE16(header.nLives);
    }
    else {
        kread(hFile, &GameStats, sizeof(GameStats));
    }

    nPlayerWeapons[nLocalPlayer] = GameStats.nWeapons;

    PlayerList[nLocalPlayer].nCurrentWeapon = GameStats.nCurrentWeapon;
    nPlayerClip[nLocalPlayer] = GameStats.clip;

    int nPistolBullets = PlayerList[nLocalPlayer].nAmmo[kWeaponPistol];
    if (nPistolBullets >= 6) {
        nPistolBullets = 6;
    }

    nPistolClip[nLocalPlayer] = nPistolBullets;

    memcpy(&PlayerList[nLocalPlayer], &GameStats.player, sizeof(Player));

    nPlayerItem[nLocalPlayer]  = GameStats.items;
    nPlayerLives[nLocalPlayer] = GameStats.nLives;

    SetPlayerItem(nLocalPlayer, nPlayerItem[nLocalPlayer]);
    CheckClip(nLocalPlayer);
}

short menu_GameLoad(int nSlot)
{
    memset(&GameStats, 0, sizeof(GameStats));

    buildvfs_kfd hFile = kopen4loadfrommod(kSaveFileName, 0);
    if (hFile < 0) {
        return 0;
    }

    klseek(hFile, 125, SEEK_SET);
    klseek(hFile, nSlot * sizeof(GameStats), SEEK_CUR);

#ifndef EDUKE32
    menu_GameLoad2(hFile, false);
#else
    menu_GameLoad2(hFile);
#endif
    kclose(hFile);

    return GameStats.nMap;
}

void menu_DemoGameSave(FILE* fp)
{
    demo_header dh;
    memset(&dh, 0, sizeof(dh));

    Player* pPlayer = &PlayerList[nLocalPlayer];

    dh.nMap = (uint8_t)levelnew;
    dh.nWeapons = B_LITTLE16(nPlayerWeapons[nLocalPlayer]);
    dh.nCurrentWeapon = B_LITTLE16(pPlayer->nCurrentWeapon);
    dh.clip   = B_LITTLE16(nPlayerClip[nLocalPlayer]);
    dh.items  = B_LITTLE16(nPlayerItem[nLocalPlayer]);
    dh.nLives = B_LITTLE16(nPlayerLives[nLocalPlayer]);

    dh.nHealth = B_LITTLE16(pPlayer->nHealth);
    dh.nFrame = B_LITTLE16(pPlayer->nFrame);
    dh.nAction = B_LITTLE16(pPlayer->nAction);
    dh.nSprite = B_LITTLE16(pPlayer->nSprite);
    dh.bIsMummified = B_LITTLE16(pPlayer->bIsMummified);
    dh.someNetVal = B_LITTLE16(pPlayer->someNetVal);
    dh.invincibility = B_LITTLE16(pPlayer->invincibility);
    dh.nAir = B_LITTLE16(pPlayer->nAir);
    dh.nSeq = B_LITTLE16(pPlayer->nSeq);
    dh.nMaskAmount = B_LITTLE16(pPlayer->nMaskAmount);
    dh.keys = B_LITTLE16(pPlayer->keys);
    dh.nMagic = B_LITTLE16(pPlayer->nMagic);

    Bmemcpy(dh.item, pPlayer->items, sizeof(pPlayer->items));
    for (int i = 0; i < 7; i++)
    {
        dh.nAmmo[i] = B_LITTLE16(pPlayer->nAmmo[i]);
    }
    Bmemcpy(dh.pad, pPlayer->pad, sizeof(pPlayer->pad));

    dh.nCurrentWeapon2 = B_LITTLE16(pPlayer->nCurrentWeapon);
    dh.nWeaponFrame = B_LITTLE16(pPlayer->nWeaponFrame);
    dh.bIsFiring = B_LITTLE16(pPlayer->bIsFiring);
    dh.nNewWeapon  = B_LITTLE16(pPlayer->nNewWeapon);
    dh.nWeaponState  = B_LITTLE16(pPlayer->nWeaponState);
    dh.nLastWeapon  = B_LITTLE16(pPlayer->nLastWeapon);
    dh.nRun = B_LITTLE16(pPlayer->nRun);

    fwrite(&dh, sizeof(dh), 1, fp);
}

void menu_GameSave2(FILE *fp)
{
    memset(&GameStats, 0, sizeof(GameStats));

    GameStats.nMap = (uint8_t)levelnew;
    GameStats.nWeapons = nPlayerWeapons[nLocalPlayer];
    GameStats.nCurrentWeapon = PlayerList[nLocalPlayer].nCurrentWeapon;
    GameStats.clip   = nPlayerClip[nLocalPlayer];
    GameStats.items  = nPlayerItem[nLocalPlayer];
    GameStats.nLives = nPlayerLives[nLocalPlayer];

    memcpy(&GameStats.player, &PlayerList[nLocalPlayer], sizeof(GameStats.player));

    fwrite(&GameStats, sizeof(GameStats), 1, fp);
}

void menu_GameSave(int nSaveSlot)
{
    if (nSaveSlot < 0) {
        return;
    }

    FILE *fp = fopen(kSaveFileName, "rb+");
    if (fp != NULL)
    {
        fseek(fp, 125, SEEK_SET); // skip save slot names
        fseek(fp, sizeof(GameStat) * nSaveSlot, SEEK_CUR);
        menu_GameSave2(fp);
        fclose(fp);
    }
}

void menu_ResetZoom()
{
    zoomsize = 0;
    PlayLocalSound(StaticSound[kSoundItemUse], 0);
}

int menu_Menu(int bInLevelMenus)
{
    GrabPalette();

    int var_1C = 0;

    videoSetViewableArea(0, 0, xdim - 1, ydim - 1);

    KB_KeyDown[sc_Escape] = 0;

    StopAllSounds();
    StopLocalSound();

    menu_ResetKeyTimer();

    KB_FlushKeyboardQueue();
    KB_ClearKeysDown();

    menu_ResetZoom();

    short ptr[5];
    memset(ptr, 1, sizeof(ptr));

    // disable new game and load game if in multiplayer?
    if (nNetPlayerCount)
    {
        ptr[1] = 0;
        ptr[0] = 0;
    }

    // denote which menu item we've currently got selected
    int nMenu = 0;

    while (1)
    {
        HandleAsync();
        OSD_DispatchQueued();

        // skip any disabled menu items so we're selecting the first active one
        while (!ptr[nMenu])
        {
            nMenu++;
            if (nMenu == 5) {
                nMenu = 0;
            }
        }

        // handle the menu zoom-in
        if (zoomsize < 0x10000)
        {
            zoomsize += 4096;
            if (zoomsize >= 0x10000) {
                zoomsize = 0x10000;
            }
        }

        // menu idle timer - will play the demo file if no keys pressed after timer runs out
        if (!bInLevelMenus && (int)totalclock > keytimer) {
            return 9;
        }

        // loc_39F54:
        menu_DoPlasma();

#ifndef EDUKE32
        int y = 65 - tilesizy[kMenuNewGameTile] / 2;
#else
        int y = 65 - tilesiz[kMenuNewGameTile].y / 2;
#endif

        // YELLOW loop - Draw the 5 menu options (NEW GAME, TRAINING etc)
        for (int j = 0; j < 5; j++)
        {
            int8_t shade;

            if (nMenu == j) { // currently selected menu item
                shade = Sin((int)totalclock << 4) >> 9;
            }
            else if (ptr[j]) {
                shade = 0;
            }
            else {
                shade = 25;
            }

#ifndef EDUKE32
            picanm[j + kMenuNewGameTile] &= 0xff0000ff;
            rotatesprite(160 << 16, (y + tilesizy[j + kMenuNewGameTile]) << 16, zoomsize, 0, kMenuNewGameTile + j, shade, 0, 2, 0, 0, xdim, ydim);
#else
            picanm[j + kMenuNewGameTile].xofs = 0;
            picanm[j + kMenuNewGameTile].yofs = 0;
            rotatesprite(160 << 16, (y + tilesiz[j + kMenuNewGameTile].y) << 16, zoomsize, 0, kMenuNewGameTile + j, shade, 0, 2, 0, 0, xdim, ydim);
#endif

            y += 22;
        }

        // tilesizx is 51
        // tilesizy is 33

        int markerY = (22 * nMenu) + 53;
        overwritesprite(62,       markerY, kMenuCursorTile, 0, 2, kPalNormal);
        overwritesprite(62 + 146, markerY, kMenuCursorTile, 0, 10, kPalNormal);

        videoNextPage();

        int l = 0; // edi

        // ORANGE loop
        for (l = 0; ; l++)
        {
            int nKey = nMenuKeys[l];
            if (!nKey) {
                break;
            }

            if (KB_KeyDown[nKey])
            {
                goto LABEL_21; // TEMP
            }
        }

        // loc_3A0A7
        while (KB_KeyDown[sc_Escape])
        {
            HandleAsync();

            PlayLocalSound(StaticSound[kSoundSwitchFoot], 0);
            KB_KeyDown[sc_Escape] = 0;

            if (bInLevelMenus)
            {
                StopAllSounds();
                PlayLocalSound(StaticSound[kSoundSwitchFoot], 0);
                MySetView(nViewLeft, nViewTop, nViewRight, nViewBottom);
                return -1;
            }

            l = 4;
LABEL_21:

            menu_ResetKeyTimer();

            if (l != nMenu)
            {
                PlayLocalSound(StaticSound[kSoundSwitchWtr1], 0);
                KB_KeyDown[nMenuKeys[l]] = 0;
                nMenu = l;
            }
        }

        if (KB_KeyDown[sc_Space] || KB_KeyDown[sc_Return])
        {
            var_1C = 1;
        }
        else if (var_1C)
        {
            var_1C = 0;

            PlayLocalSound(StaticSound[kSoundSwitchFoot], 0);

            switch (nMenu) // TODO - change var name?
            {
                case kMenuNewGame:
                {
                    if (nTotalPlayers > 1) {
                        menu_ResetZoom();
                        menu_ResetKeyTimer();
                        break;
                    }

                    SavePosition = menu_NewGameMenu();
                    if (SavePosition == -1) {
                        menu_ResetZoom();
                        menu_ResetKeyTimer();
                        break;
                    }

                    FadeOut(1);
                    StopAllSounds();

                    StopAllSounds();
                    PlayLocalSound(StaticSound[kSoundSwitchFoot], 0);
                    MySetView(nViewLeft, nViewTop, nViewRight, nViewBottom);
                    return 1;
                }

                case kMenuLoadGame:
                {
                    if (nTotalPlayers > 1) {
                        menu_ResetZoom();
                        menu_ResetKeyTimer();
                        break;
                    }

                    SavePosition = menu_LoadGameMenu();

                    if (SavePosition == -1) {
                        menu_ResetZoom();
                        menu_ResetKeyTimer();
                        break;
                    }

                    StopAllSounds();

                    StopAllSounds();
                    PlayLocalSound(StaticSound[kSoundSwitchFoot], 0);
                    MySetView(nViewLeft, nViewTop, nViewRight, nViewBottom);
                    return 2;
                }

                case kMenuTraining:
                {
                    if (nTotalPlayers > 1) {
                        menu_ResetZoom();
                        menu_ResetKeyTimer();
                        break;
                    }

                    StopAllSounds();
                    PlayLocalSound(StaticSound[kSoundSwitchFoot], 0);
                    MySetView(nViewLeft, nViewTop, nViewRight, nViewBottom);
                    return 3;
                }

                case kMenuVolume:
                {
                    menu_AdjustVolume();
                    menu_ResetZoom();
                    menu_ResetKeyTimer();
                    break;
                }

                case kMenuQuitGame:
                {
                    StopAllSounds();
                    StopAllSounds();
                    PlayLocalSound(StaticSound[kSoundSwitchFoot], 0);
                    MySetView(nViewLeft, nViewTop, nViewRight, nViewBottom);
                    return 0;
                }

                default:
                    menu_ResetZoom();
                    menu_ResetKeyTimer();
                    break;
            }
        }

        if (KB_KeyDown[sc_UpArrow])
        {
            PlayLocalSound(StaticSound[kSoundSwitchWtr1], 0);
            if (nMenu <= 0) {
                nMenu = 4;
            }
            else {
                nMenu--;
            }

            KB_KeyDown[sc_UpArrow] = 0;
            menu_ResetKeyTimer();
        }

        if (KB_KeyDown[sc_DownArrow]) // FIXME - is this down arrow? value is '5B' in disassembly
        {
            PlayLocalSound(StaticSound[kSoundSwitchWtr1], 0);
            if (nMenu >= 4) {
                nMenu = 0;
            }
            else {
                nMenu++;
            }

            KB_KeyDown[sc_DownArrow] = 0;
            menu_ResetKeyTimer();
        }

        // TODO - change to #defines
        if (KB_KeyDown[0x5c]) {
            KB_KeyDown[0x5c] = 0;
        }

        if (KB_KeyDown[0x5d]) {
            KB_KeyDown[0x5d] = 0;
        }
    }

    return 0;// todo
}

#define kMaxCinemaPals	16
const char *cinpalfname[kMaxCinemaPals] = {
    "3454.pal",
    "3452.pal",
    "3449.pal",
    "3445.pal",
    "set.pal",
    "3448.pal",
    "3446.pal",
    "hsc1.pal",
    "2972.pal",
    "2973.pal",
    "2974.pal",
    "2975.pal",
    "2976.pal",
    "heli.pal",
    "2978.pal",
    "terror.pal"
};

int linecount;
int nextclock;
short nHeight;
short nCrawlY;
short cinematile;


// TODO - moveme
int LoadCinemaPalette(int nPal)
{
    nPal--;

    if (nPal < 0 || nPal >= kMaxCinemaPals) {
        return -2;
    }

    // original code strcpy'd into a buffer first...

    int hFile = kopen4loadfrommod(cinpalfname[nPal], 0);
    if (hFile < 0) {
        return -2;
    }

    kread(hFile, cinemapal, sizeof(cinemapal));

#ifdef EDUKE32
    for (auto &c : cinemapal)
        c <<= 2;
#endif

    kclose(hFile);

    return nPal;
}

//int IncrementCinemaFadeIn()
//{
//    dest = cinemapal;
//    cur = curpal;
//
//    int ebx = 0;
//
//    for (int i = 0; i < 768; i++)
//    {
//        ebx++;
//
//        if (*cur < *dest)
//        {
//            (*cur)++;
//        }
//        else if (*cur == *dest)
//        {
//            ebx--;
//        }
//        else
//        {
//            (*cur)--;
//        }
//
//        cur++;
//        dest++;
//    }
//
//    MySetPalette(curpal);
//    return ebx;
//}

void CinemaFadeIn()
{
    BlackOut();

    paletteSetColorTable(ANIMPAL, cinemapal);
    videoSetPalette(0, ANIMPAL, 2+8);

#ifdef USE_OPENGL
    if (videoGetRenderMode() >= REND_POLYMOST)
    {
        videoNextPage();
        return;
    }
#endif

    int val;

    do
    {
        val = DoFadeIn();
        WaitTicks(2);

        // need to page flip in each iteration of the loop for non DOS version
        videoNextPage();

    } while (val > 0);
}

void ComputeCinemaText(int nLine)
{
    linecount = 0;

    while (1)
    {
        if (!strcmp(gString[linecount + nLine], "END")) {
            break;
        }

        int nWidth = MyGetStringWidth(gString[linecount + nLine]);
        nLeft[linecount] = 160 - nWidth / 2;

        linecount++;
    }

    nCrawlY = 199;
    nHeight = linecount * 10;

    ClearAllKeys();
}

void ReadyCinemaText(uint16_t nVal)
{
    line = FindGString("CINEMAS");
    if (line < 0) {
        return;
    }

    while (nVal)
    {
        while (strcmp(gString[line], "END")) {
            line++;
        }

        line++;
        nVal--;
    }

    ComputeCinemaText(line);
}

bool AdvanceCinemaText()
{
    bool bDoText = nHeight + nCrawlY > 0;

    if (bDoText || CDplaying())
    {
        nextclock = (int)totalclock + 15; // NOTE: Value was 14 in original code but seems a touch too fast now

        if (bDoText)
        {
            short y = nCrawlY;
            int i = 0;

            while (i < linecount && y <= 199)
            {
                if (y >= -10) {
                    myprintext(nLeft[i], y, gString[line + i], 0);
                }

                i++;
                y += 10;
            }

            nCrawlY--;
        }

        while (1)
        {
            HandleAsync();

            if (KB_KeyDown[sc_Escape] || KB_KeyDown[sc_Return] || KB_KeyDown[sc_Space]) {
                break;
            }

            if (nextclock <= (int)totalclock) {
                return true;
            }
        }
    }

    return false;
}

void DoCinemaText(short nVal)
{
    ReadyCinemaText(nVal);

    bool bContinue = true;

    while (bContinue)
    {
        overwritesprite(0, 0, cinematile, 0, 2, kPalNormal);

        bContinue = AdvanceCinemaText();

        WaitVBL();
        videoNextPage();
    }
}

void GoToTheCinema(int nVal)
{
    UnMaskStatus();

    switch (nVal - 1)
    {
        default:
            return;

        case 0:
        {
            LoadCinemaPalette(1);
            cinematile = 3454;
            break;
        }

        case 1:
        {
            LoadCinemaPalette(2);
            cinematile = 3452;
            break;
        }

        case 2:
        {
            LoadCinemaPalette(3);
            cinematile = 3449;
            break;
        }

        case 3:
        {
            LoadCinemaPalette(4);
            cinematile = 3445;
            break;
        }

        case 4:
        {
            LoadCinemaPalette(5);
            cinematile = 3451;
            break;
        }

        case 5:
        {
            LoadCinemaPalette(6);
            cinematile = 3448;
            break;
        }

        case 6:
        {
            LoadCinemaPalette(7);
            cinematile = 3446;
            break;
        }
    }

    if (ISDEMOVER) {
        if (!waloff[cinematile]) {
            tileCreate(cinematile, 320, 200);
        }
    }

    FadeOut(kFalse);
    StopAllSounds();
    NoClip();

    overwritesprite(0, 0, kMovieTile, 100, 2, kPalNormal);
    videoNextPage();

//	int386(16, (const union REGS *)&val, (union REGS *)&val)

    overwritesprite(0, 0, cinematile, 0, 2, kPalNormal);
    videoNextPage();

    CinemaFadeIn();
    ClearAllKeys();

    int ebx = -1;
    int edx = -1;

    switch (nVal - 1)
    {
        default:
            WaitAnyKey(10);
            break;

        case 0:
            ebx = 4;
            edx = ebx;
            break;

        case 1:
            ebx = 0;
            break;

        case 2:
            ebx = 2;
            edx = ebx;
            break;

        case 3:
            ebx = 7;
            break;

        case 4:
            ebx = 3;
            edx = ebx;
            break;

        case 5:
            ebx = 8;
            edx = ebx;
            break;

        case 6:
            ebx = 6;
            edx = ebx;
            break;
    }

    if (ebx != -1)
    {
        if (edx != -1)
        {
            if (CDplaying()) {
                fadecdaudio();
            }

            playCDtrack(edx + 2, false);
        }

        DoCinemaText(ebx);
    }

    FadeOut(kTrue);

    overwritesprite(0, 0, kMovieTile, 100, 2, kPalNormal);
    videoNextPage();

    GrabPalette();
    Clip();

    // quit the game if we've finished the last level and displayed the advert text
    if (ISDEMOVER && ((EXHUMED && nVal == 2) || nVal == 3)) {
        ExitGame();
    }
}


short nBeforeScene[] = { 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 };


void CheckBeforeScene(int nLevel)
{
    if (nLevel == kMap20)
    {
        DoLastLevelCinema();
        return;
    }

    short nScene = nBeforeScene[nLevel];

    if (nScene)
    {
        if (!nCinemaSeen[nScene])
        {
            GoToTheCinema(nScene);
            nCinemaSeen[nScene] = 1;
        }
    }
}

int showmap(short nLevel, short nLevelNew, short nLevelBest)
{
    FadeOut(0);
    EraseScreen(overscanindex);
    GrabPalette();
    BlackOut();

    if (nLevelNew != 11) {
        CheckBeforeScene(nLevelNew);
    }

    int selectedLevel = menu_DrawTheMap(nLevel, nLevelNew, nLevelBest);
    if (selectedLevel == 11) {
        CheckBeforeScene(selectedLevel);
    }

    return selectedLevel;
}

void DoAfterCinemaScene(int nLevel)
{
    short nAfterScene[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 7, 0, 0, 0, 0, 6 };

    if (nAfterScene[nLevel]) {
        GoToTheCinema(nAfterScene[nLevel]);
    }
}

void DoFailedFinalScene()
{
    videoSetViewableArea(0, 0, xdim - 1, ydim - 1);

    if (CDplaying()) {
        fadecdaudio();
    }

    playCDtrack(9, false);
    FadeToWhite();

    GoToTheCinema(4);
}

int FindGString(const char *str)
{
    int i = 0;

    while (1)
    {
        if (!strcmp(gString[i], str))
            return i + 1;

        if (!strcmp(gString[i], "EOF"))
            break;

        i++;
    }

    return -1;
}

uint8_t CheckForEscape()
{
    if (!KB_KeyWaiting() || (KB_GetCh() != 27)) {
        return kFalse;
    }

    return kTrue;
}

void DoStatic(int a, int b)
{
    RandomLong(); // nothing done with the result of this?

    tileLoad(kTileLoboLaptop);

    int v2 = 160 - a / 2;
    int v4 = 81  - b / 2;

    int var_18 = v2 + a;
    int v5 = v4 + b;

    uint8_t *pTile = (uint8_t*)(waloff[kTileLoboLaptop] + (200 * v2)) + v4;

    tileInvalidate(kTileLoboLaptop, -1, -1);

    while (v2 < var_18)
    {
        uint8_t *pStart = pTile;
        pTile += 200;

        int v7 = v4;

        while (v7 < v5)
        {
            *pStart = RandomBit() * 16;

            v7++;
            pStart++;
        }
        v2++;
    }

    overwritesprite(0, 0, kTileLoboLaptop, 0, 2, kPalNormal);
    videoNextPage();
}

void DoLastLevelCinema()
{
    FadeOut(0);
    UnMaskStatus();

    videoSetViewableArea(0, 0, xdim - 1, ydim - 1);

    EraseScreen(-1);
    RestorePalette();

    int nString = FindGString("LASTLEVEL");

    PlayLocalSound(StaticSound[kSoundScorpionZap], 0);

    tileLoad(kTileLoboLaptop);

    int var_24 = 16;
    int var_28 = 12;

    int nEndTime = (int)totalclock + 240;

    while (KB_KeyWaiting()) {
        KB_GetCh();
    }

    while (nEndTime > (int)totalclock)
    {
        HandleAsync();

        if (var_24 >= 116)
        {
            if (var_28 < 192)
                var_28 += 20;
        }
        else
        {
            var_24 += 20;
        }

        DoStatic(var_28, var_24);

        // WaitVBL();
        int time = (int)totalclock + 4;
        while ((int)totalclock < time) {
            HandleAsync();
        }
    }

//	loadtilelockmode = 1;
    tileLoad(kTileLoboLaptop);
//	loadtilelockmode = 0;

    // loc_3AD75

    do
    {  
    LABEL_11:

        HandleAsync();

        if (strlen(gString[nString]) == 0)
            break;

        int esi = nString;

        tileLoad(kTileLoboLaptop);

        while (strlen(gString[esi]) != 0)
            esi++;

        int ebp = esi;

        ebp -= nString;
        ebp <<= 2;
        ebp = 81 - ebp;

        int var_1C = esi - nString;

        // loc_3ADD7
        while (1)
        {
            HandleAsync();

            if (strlen(gString[nString]) == 0)
                break;

            int xPos = 70;

            const char *nChar = gString[nString];

            nString++;

            while (*nChar)
            {
                HandleAsync();

                if (*nChar != ' ') {
                    PlayLocalSound(StaticSound[kSoundPotPc1], 0);
                }

                xPos += CopyCharToBitmap(*nChar, kTileLoboLaptop, xPos, ebp);
                nChar++;

                overwritesprite(0, 0, kTileLoboLaptop, 0, 2, kPalNormal);
                videoNextPage();

                // WaitVBL();
                int time = (int)totalclock + 4;
                while ((int)totalclock < time) {
                    HandleAsync();
                }

                if (CheckForEscape())
                    goto LABEL_28;
            }

            ebp += 8;
        }

        nString++;

        KB_FlushKeyboardQueue();
        KB_ClearKeysDown();

        int v11 = (kTimerTicks * (var_1C + 2)) + (int)totalclock;

        do
        {
            HandleAsync();

            if (v11 <= (int)totalclock)
                goto LABEL_11;
        } while (!KB_KeyWaiting());
    }
    while (KB_GetCh() != 27);

LABEL_28:
    PlayLocalSound(StaticSound[kSoundScorpionZap], 0);

    nEndTime = (int)totalclock + 240;

    while (nEndTime > (int)totalclock)
    {
        HandleAsync();

        DoStatic(var_28, var_24);

        // WaitVBL();
        int time = (int)totalclock + 4;
        while ((int)totalclock < time) {
            HandleAsync();
        }

        if (var_28 > 20) {
            var_28 -= 20;
            continue;
        }

        if (var_24 > 20) {
            var_24 -= 20;
            continue;
        }

        break;
    }

    EraseScreen(-1);
    tileLoad(kTileLoboLaptop);
// FIXME - Revert this when fixing fades    FadeOut(0);
    MySetView(nViewLeft, nViewTop, nViewRight, nViewBottom);
    MaskStatus();
}
