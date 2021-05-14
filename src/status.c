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

#include "engine.h"
#include "player.h"
#include "anims.h"
#include "status.h"
#include "exhumed.h"
#include "sequence.h"
#include "init.h"
#include "names.h"
#include "items.h"
#include "view.h"
#include "trigdat.h"
#include "light.h"
#include "save.h"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "typedefs.h"

//short nMaskY;
//short statusmask[MAXXDIM];

static short nAnimsFree = 0;

short message_timer = 0;
char message_text[80];
int magicperline;
int airperline;
int healthperline;
int nAirFrames;
int nCounter;
int nCounterDest;

short nStatusSeqOffset;
short nItemFrames;

int laststatusx;
int laststatusy;

int16_t nItemSeq;
short nDigit[3];

short nMagicFrames;
short nHealthLevel;
short nItemFrame;
short nMeterRange;
short nMagicLevel;
short nHealthFrame;
short nMagicFrame;

short statusx;
short statusy;
short nHealthFrames;
short airframe;

int16_t nFirstAnim;
int16_t nLastAnim;
short nItemAltSeq;

short airpages = 0;
short ammodelay = 3;
short nCounterBullet = -1;

struct statusAnim
{
    int16_t s1;
    int16_t s2;
//    int16_t nPage;
    int8_t nPrevAnim;
    int8_t nNextAnim;
};
#ifndef EDUKE32
typedef struct statusAnim statusAnim;
#endif

#define kMaxStatusAnims		50

statusAnim StatusAnim[kMaxStatusAnims];
uint8_t StatusAnimsFree[kMaxStatusAnims];
uint8_t StatusAnimFlags[kMaxStatusAnims];

short nItemSeqOffset[] = {91, 72, 76, 79, 68, 87, 83};

short word_9AD54[kMaxPlayers] = {0, 0, 0, 0, 0, 0, 0, 0};
int dword_9AD64[kMaxPlayers] = {0, 0, 0, 0, 0, 0, 0, 0};

void SetCounterDigits();
void SetItemSeq();
void SetItemSeq2(int nSeqOffset);
void DestroyStatusAnim(short nAnim);


int BuildStatusAnim(int val, int nFlags)
{
    // destroy this anim if it already exists
    for (int i = nFirstAnim; i >= 0; i = StatusAnim[i].nPrevAnim)
    {
        if (StatusAnim[i].s1 == val) {
            DestroyStatusAnim(i);
            break;
        }
    }

    if (nAnimsFree <= 0) {
        return -1;
    }

    nAnimsFree--;

    uint8_t nStatusAnim = StatusAnimsFree[nAnimsFree];

    StatusAnim[nStatusAnim].nPrevAnim = -1;
    StatusAnim[nStatusAnim].nNextAnim = nLastAnim;

    if (nLastAnim < 0) {
        nFirstAnim = nStatusAnim;
    }
    else {
        StatusAnim[nLastAnim].nPrevAnim = nStatusAnim;
    }

    nLastAnim = nStatusAnim;

    StatusAnim[nStatusAnim].s1 = val;
    StatusAnim[nStatusAnim].s2 = 0;
    StatusAnimFlags[nStatusAnim] = nFlags;
//    StatusAnim[nStatusAnim].nPage = numpages;
    return nStatusAnim;
}

#ifdef __AMIGA__
extern int statusHeightHack;
int nStatusHeight = 0;
int nStatusOpaqueHeight = 0;
int nOverlayHeight = 0;
int nOverlayWidth = 0;
int statusBgRedraw = 0;
int statusAmmoRedraw = 0;
int statusHealthRedraw = 0;
int statusMagicRedraw = 0;
#endif

void RefreshStatus()
{
    short nLives = nPlayerLives[nLocalPlayer];
    if (nLives < 0 || nLives > kMaxPlayerLives) {
        bail2dos("illegal value for nPlayerLives #%d\n", nLocalPlayer);
    }

    // draws the red dots that indicate the lives amount
    BuildStatusAnim(145 + (2 * nLives), 0);

    uint16_t nKeys = PlayerList[nLocalPlayer].keys;

    int val = 37;

    for (int i = 0; i < 4; i++)
    {
        if (nKeys & 0x1000) {
            BuildStatusAnim(val, 0);
        }

        nKeys >>= 1;
        val += 2;
    }

    SetPlayerItem(nLocalPlayer, nPlayerItem[nLocalPlayer]);
    SetHealthFrame(0);
    SetMagicFrame();
    SetAirFrame();
#ifdef __AMIGA__
	statusBgRedraw = 1;
	statusAmmoRedraw = 1;
	statusHealthRedraw = 1;
	statusMagicRedraw = 1;
#endif
}

void InitStatus()
{
    nStatusSeqOffset = SeqOffsets[kSeqStatus];
    nHealthFrames = SeqSize[nStatusSeqOffset + 1];
    int nPicNum   = seq_GetSeqPicnum(kSeqStatus, 1, 0);
    nMagicFrames  = SeqSize[nStatusSeqOffset + 129];
    nHealthFrame  = 0;
    nMagicFrame   = 0;
    nHealthLevel  = 0;
    nMagicLevel   = 0;
#ifndef EDUKE32
    nMeterRange   = tilesizy[nPicNum];
#else
    nMeterRange   = tilesiz[nPicNum].y;
#endif
    magicperline  = 1000 / nMeterRange;
    healthperline = 800 / nMeterRange;
    nAirFrames = SeqSize[nStatusSeqOffset + 133];
    airperline = 100 / nAirFrames;
    nCounter   = 0;
    nCounterDest = 0;

    memset(nDigit, 0, sizeof(nDigit));

    SetCounter(0);
    SetHealthFrame(0);
    SetMagicFrame();

    for (int i = 0; i < kMaxStatusAnims; i++) {
        StatusAnimsFree[i] = i;
    }

    nLastAnim  = -1;
    nFirstAnim = -1;
    nItemSeq   = -1;
    nAnimsFree = kMaxStatusAnims;
    statusx    = xdim - 320;
    textpages  = 0;
    message_timer = 0;
    statusy = ydim - 200;
#ifdef __AMIGA__
	int nStatusPic = seq_GetSeqPicnum(kSeqStatus, 0, 0);
	if (!waloff[nStatusPic]) tileLoad(nStatusPic);
	nStatusHeight = tilesizy[nStatusPic];
	char *statusPix = (char *)waloff[nStatusPic];
	if (nStatusOpaqueHeight == 0 && nOverlayHeight == 0) {
		nOverlayWidth = nOverlayHeight = 0;
		for (int i = 0; i < 320; i++) {
			unsigned char *statusCol = &statusPix[i*nStatusHeight];
			for (int j = 0; j < nStatusHeight; j++) {
				if (statusCol[j] != 255) {
					if (j > nOverlayHeight) {
						nOverlayHeight = j;
						nOverlayWidth = i;
					}
					break;
				}
			}
		}
		nStatusOpaqueHeight = nStatusHeight - nOverlayHeight;
		// give them a 1 pixel margin to compensate for rounding errors
		nOverlayHeight++;
		nOverlayWidth++;
		//buildprintf("%s nOverlayWidth %d nOverlayHeight %d nStatusOpaqueHeight %d\n", __FUNCTION__, nOverlayWidth, nOverlayHeight, nStatusOpaqueHeight);
	}
	//buildprintf("%s nStatusPic %d nStatusHeight %d\n", __FUNCTION__, nStatusPic, nStatusHeight);
	if (!waloff[kTile3700]) {
		tileCreate(kTile3700, nOverlayWidth, nOverlayHeight);
		char *leftPix = (char *)waloff[kTile3700];
		for (int i = 0; i < nOverlayWidth; i++) {
			memcpy(&leftPix[i * nOverlayHeight], &statusPix[i * nStatusHeight], nOverlayHeight);
		}
	}
	if (!waloff[kTile3701]) {
		tileCreate(kTile3701, nOverlayWidth, nOverlayHeight);
		char *rightPix = (char *)waloff[kTile3701];
		for (int i = 0; i < nOverlayWidth; i++) {
			memcpy(&rightPix[i * nOverlayHeight], &statusPix[i * nStatusHeight + (320 - nOverlayWidth) * nStatusHeight], nOverlayHeight);
		}
	}
#endif
}

void MoveStatusAnims()
{
    for (int i = nFirstAnim; i >= 0; i = StatusAnim[i].nPrevAnim)
    {
        seq_MoveSequence(-1, nStatusSeqOffset + StatusAnim[i].s1, StatusAnim[i].s2);

        StatusAnim[i].s2++;

        short nSize = SeqSize[nStatusSeqOffset + StatusAnim[i].s1];

        if (StatusAnim[i].s2 >= nSize)
        {
            if (StatusAnimFlags[i] & 0x10) {
                StatusAnim[i].s2 = 0;
            }
            else {
                StatusAnim[i].s2 = nSize - 1; // restart it
            }
        }
    }
}

void DestroyStatusAnim(short nAnim)
{
    int8_t nPrev = StatusAnim[nAnim].nPrevAnim;
    int8_t nNext = StatusAnim[nAnim].nNextAnim;

    if (nNext >= 0) {
        StatusAnim[nNext].nPrevAnim = nPrev;
    }

    if (nPrev >= 0) {
        StatusAnim[nPrev].nNextAnim = nNext;
    }

    if (nAnim == nFirstAnim) {
        nFirstAnim = nPrev;
    }

    if (nAnim == nLastAnim) {
        nLastAnim = nNext;
    }

    StatusAnimsFree[nAnimsFree] = (uint8_t)nAnim;
    nAnimsFree++;
}

void DrawStatusAnims()
{
    for (int i = nFirstAnim; i >= 0; i = StatusAnim[i].nPrevAnim)
    {
        int nSequence = nStatusSeqOffset + StatusAnim[i].s1;

        seq_DrawStatusSequence(nSequence, StatusAnim[i].s2, 0);

/*
        if (StatusAnim[nAnim].s2 >= (SeqSize[nSequence] - 1))
        {
            if (!(StatusAnimFlags[nAnim] & 0x10))
            {
                StatusAnim[nAnim].nPage--;
                if (StatusAnim[nAnim].nPage <= 0) {
                    DestroyStatusAnim(nAnim);
                }
            }
        }
*/
    }
}

void SetMagicFrame()
{
    nMagicLevel = (1000 - PlayerList[nLocalPlayer].nMagic) / magicperline;

    if (nMagicLevel >= nMeterRange) {
        nMagicLevel = nMeterRange - 1;
    }

    if (nMagicLevel < 0) {
        nMagicLevel = 0;
    }

    SetItemSeq();
#ifdef __AMIGA__
	statusMagicRedraw = 1;
#endif
}

void SetHealthFrame(short nVal)
{
    nHealthLevel = (800 - PlayerList[nLocalPlayer].nHealth) / healthperline;

    if (nHealthLevel >= nMeterRange ) {
        nHealthLevel = nMeterRange - 1;
    }

    if (nHealthLevel < 0) {
        nHealthLevel = 0;
    }

    if (nVal < 0) {
        BuildStatusAnim(4, 0);
    }
#ifdef __AMIGA__
	statusHealthRedraw = 1;
#endif
}

void SetAirFrame()
{
    airframe = PlayerList[nLocalPlayer].nAir / airperline;

    if (airframe >= nAirFrames)
    {
        airframe = nAirFrames - 1;
    }
    else if (airframe < 0)
    {
        airframe = 0;
    }
}

void SetCounter(short nVal)
{
    if (nVal <= 999)
    {
        if (nVal < 0) {
            nVal = 0;
        }
    }
    else {
        nVal = 999;
    }

    nCounterDest = nVal;
}

void SetCounterImmediate(short nVal)
{
    SetCounter(nVal);
    nCounter = nCounterDest;

    SetCounterDigits();
#ifdef __AMIGA__
	statusAmmoRedraw = 1;
#endif
}

void SetCounterDigits()
{
    nDigit[2] = 3 * (nCounter / 100 % 10);
    nDigit[1] = 3 * (nCounter / 10 % 10);
    nDigit[0] = 3 * (nCounter % 10);
}

void SetItemSeq()
{
    short nItem = nPlayerItem[nLocalPlayer];
    if (nItem < 0)
    {
        nItemSeq = -1;
        return;
    }

    short nOffset = nItemSeqOffset[nItem];

    SetItemSeq2(nOffset);
}

void SetItemSeq2(int nSeqOffset)
{
    short nItem = nPlayerItem[nLocalPlayer];

    if (nItemMagic[nItem] <= PlayerList[nLocalPlayer].nMagic) {
        nItemAltSeq = 0;
    }
    else {
        nItemAltSeq = 2;
    }

    nItemFrame = 0;
    nItemSeq = nSeqOffset + nItemAltSeq;
    nItemFrames = SeqSize[nItemSeq + nStatusSeqOffset];
}

void SetPlayerItem(short nPlayer, short nItem)
{
    nPlayerItem[nPlayer] = nItem;

    if (nPlayer == nLocalPlayer)
    {
        SetItemSeq();
        if (nItem >= 0) {
            BuildStatusAnim(156 + (2 * PlayerList[nLocalPlayer].items[nItem]), 0);
        }
    }
}

void SetNextItem(int nPlayer)
{
    short nItem = nPlayerItem[nPlayer];

    int i;

    for (i = 6; i > 0; i--)
    {
        nItem++;
        if (nItem == 6)
            nItem = 0;

        if (PlayerList[nPlayer].items[nItem] != 0)
            break;
    }

    if (i > 0) {
        SetPlayerItem(nPlayer, nItem);
    }
}

void SetPrevItem(int nPlayer)
{
    if (nPlayerItem[nPlayer] == -1)
        return;

    int nItem = nPlayerItem[nPlayer];

    int i;

    for (i = 6; i > 0; i--)
    {
        nItem--;
        if (nItem < 0)
            nItem = 5;

        if (PlayerList[nPlayer].items[nItem] != 0)
            break;
    }

    if (i > 0) {
        SetPlayerItem(nPlayer, nItem);
    }
}

void MoveStatus()
{
    if (nItemSeq >= 0)
    {
        nItemFrame++;

        if (nItemFrame >= nItemFrames)
        {
            if (nItemSeq == 67) {
                SetItemSeq();
            }
            else
            {
                nItemSeq -= nItemAltSeq;

                if (nItemAltSeq || totalmoves & 0x1F)
                {
                    if (nItemSeq < 2) {
                        nItemAltSeq = 0;
                    }
                }
                else
                {
                    nItemAltSeq = 1;
                }

                nItemFrame = 0;
                nItemSeq += nItemAltSeq;
                nItemFrames = SeqSize[nStatusSeqOffset + nItemSeq];
            }
        }
    }

    if (message_timer)
    {
        message_timer -= 4;
        if (message_timer <= 0)
        {
            if (screensize > 0) {
                textpages = numpages;
            }

            message_timer = 0;
        }
    }

    nHealthFrame++;
    if (nHealthFrame >= nHealthFrames) {
        nHealthFrame = 0;
    }

    nMagicFrame++;
    if (nMagicFrame >= nMagicFrames) {
        nMagicFrame = 0;
    }

    MoveStatusAnims();

    if (nCounter == nCounterDest)
    {
        nCounter = nCounterDest;
        ammodelay = 3;
        return;
    }
    else
    {
        ammodelay--;
        if (ammodelay > 0) {
            return;
        }
    }

    int eax = nCounterDest - nCounter;

#ifdef __AMIGA__
	statusAmmoRedraw = 1;
#endif

    if (eax <= 0)
    {
        if (eax >= -30)
        {
            for (int i = 0; i < 3; i++)
            {
                nDigit[i]--;

                if (nDigit[i] < 0)
                {
                    nDigit[i] += 30;
                }

                if (nDigit[i] < 27) {
                    break;
                }
            }
        }
        else
        {
            nCounter += (nCounterDest - nCounter) >> 1;
            SetCounterDigits();
            return;
        }
    }
    else
    {
        if (eax <= 30)
        {
            for (int i = 0; i < 3; i++)
            {
                nDigit[i]++;

                if (nDigit[i] <= 27) {
                    break;
                }

                if (nDigit[i] >= 30) {
                    nDigit[i] -= 30;
                }
            }
        }
        else
        {
            nCounter += (nCounterDest - nCounter) >> 1;
            SetCounterDigits();
            return;
        }
    }

    if (!(nDigit[0] % 3)) {
        nCounter = nDigit[0] / 3 + 100 * (nDigit[2] / 3) + 10 * (nDigit[1] / 3);
    }

    eax = nCounterDest - nCounter;
    if (eax < 0) {
        eax = -eax;
    }

    ammodelay = 4 - (eax >> 1);
    if (ammodelay < 1) {
        ammodelay = 1;
    }
}

void UnMaskStatus()
{
#if 0
    for (int i = 0; i < xdim; i++) {
        startdmost[i] = ydim;
    }
#endif
}

void MaskStatus()
{
#if 0
    for (int i = 0; i < xdim; i++)
    {
        short bx = startdmost[i];
        short cx = statusmask[i];

        if (bx > cx) {
            startdmost[i] = cx;
        }
    }
#endif
}

void LoadStatus()
{
#if 0
    int i;
    short nSize;
    short tmp;
    short buffer[1024];
//	memset(buffer, 0, sizeof(buffer)); // bjd - added by me

    for (i = 0; i < xdim; i++) {
        statusmask[i] = ydim;
    }

    nMaskY = ydim;

    int hStatus = kopen4load("status.msk", 1);
    if (!hStatus) {
        return;
    }

    kread(hStatus, &nSize, sizeof(nSize));

    int nCount = nSize >> 1;

    kread(hStatus, &tmp, sizeof(tmp));
    kread(hStatus, buffer, nSize);

    kclose(hStatus);

    short *pStatusMask = statusmask;

    for (i = 0; i < nCount; i++)
    {
        int v8 = ydim - ((ydim * buffer[i]) / 200);
        *pStatusMask++ = ydim - v8;

        if (bHiRes) {
            *pStatusMask++ = ydim - v8;
        }

        if (ydim - v8 < nMaskY) {
            nMaskY = ydim - v8;
        }
    }
#endif
}

void ClearStatusMessage()
{
    message_timer = 0;
    message_text[0] = '\0';
}

void StatusMessage(int messageTime, const char *fmt, ...)
{
    message_timer = messageTime;

    va_list args;
    va_start(args, fmt);

    vsprintf(message_text, fmt, args);

    if (screensize > 0) {
        textpages = numpages;
    }
}

void DrawSnakeCamStatus()
{
    printext(0, 0, "S E R P E N T   C A M", kTileFont);
}

void DrawStatus()
{
    char numberBuf[10] = {0};
    char stringBuf[20] = {0};
    char coordBuf[50] = {0}; // not sure of the size for this?

    if (!bFullScreen && nNetTime)
    {
        // bjd - commenting out this check seems to fix the black status bar at 320x200 resolution
//		if (bHiRes) {
            NoClip();
//		}

        // draw the main bar itself
#ifdef __AMIGA__
		if (statusBgRedraw) {
			seq_DrawStatusSequence(nStatusSeqOffset, 0, 0);
		} else if (screensize <= 7) {
			int y = 200 + nOverlayHeight/2 - nStatusHeight;
			overwritesprite(0 + nOverlayWidth/2, y, kTile3700, 0, 3, kPalNormal);
			overwritesprite(320 - nOverlayWidth/2, y, kTile3701, 0, 3, kPalNormal);
		}
		if ((statusHealthRedraw && !statusBgRedraw)) {
			seq_DrawStatusSequence(nStatusSeqOffset + 128, 0, 0);
			statusHealthRedraw = 0;
		}
		if ((statusMagicRedraw && !statusBgRedraw)) {
			seq_DrawStatusSequence(nStatusSeqOffset + 127, 0, 0);
			statusMagicRedraw = 0;
		}
#else
        seq_DrawStatusSequence(nStatusSeqOffset, 0, 0);
        seq_DrawStatusSequence(nStatusSeqOffset + 128, 0, 0);
        seq_DrawStatusSequence(nStatusSeqOffset + 127, 0, 0);
#endif
        seq_DrawStatusSequence(nStatusSeqOffset + 1, nHealthFrame, nHealthLevel);
        seq_DrawStatusSequence(nStatusSeqOffset + 129, nMagicFrame, nMagicLevel);
        seq_DrawStatusSequence(nStatusSeqOffset + 125, 0, 0); // draw ankh on health pool
        seq_DrawStatusSequence(nStatusSeqOffset + 130, 0, 0); // draw health pool frame (top)
        seq_DrawStatusSequence(nStatusSeqOffset + 131, 0, 0); // magic pool frame (bottom)

        if (nItemSeq >= 0) {
            seq_DrawStatusSequence(nItemSeq + nStatusSeqOffset, nItemFrame, 0);
        }

        // draws health level dots, animates breathing lungs and other things
        DrawStatusAnims();

        // draw the blue air level meter when underwater (but not responsible for animating the breathing lungs otherwise)
        if (airpages)
        {
            seq_DrawStatusSequence(nStatusSeqOffset + 133, airframe, 0);
            // airpages--;
        }

        // draw compass
#ifdef __AMIGA__
		static short oldAngle = -1;
		short angle = ((inita + 128) & kAngleMask) >> 8;
		if (statusBgRedraw || oldAngle != angle) {
			seq_DrawStatusSequence(nStatusSeqOffset + 35, angle, 0);
			oldAngle = angle;
		}
#else
        seq_DrawStatusSequence(nStatusSeqOffset + 35, ((inita + 128) & kAngleMask) >> 8, 0);
#endif

        /*
        if (bCoordinates)
        {
            sprintf(numberBuf, "%i", lastfps);
            // char *cFPS = itoa(lastfps, numberBuf, 10);
            printext(xdim - 20, nViewTop, numberBuf, kTile159, -1);
        }
        */

        // draw ammo count
#ifdef __AMIGA__
        if (statusAmmoRedraw) {
#endif
        seq_DrawStatusSequence(nStatusSeqOffset + 44, nDigit[2], 0);
        seq_DrawStatusSequence(nStatusSeqOffset + 45, nDigit[1], 0);
        seq_DrawStatusSequence(nStatusSeqOffset + 46, nDigit[0], 0);
#ifdef __AMIGA__
        statusAmmoRedraw = 0;
		}
#endif

#ifdef __AMIGA__
		statusBgRedraw = 0;
#endif

        // bjd - commenting out this check seems to fix the black status bar at 320x200 resolution
//		if (bHiRes) {
            Clip();
//		}
    }

    if (nNetPlayerCount)
    {
        NoClip();

        int shade;

        if ((int)totalclock / kTimerTicks & 1) {
            shade = -100;
        }
        else {
            shade = 127;
        }

        int nTile = kTile3593;

        int x = 320 / (nTotalPlayers + 1);

        for (int i = 0; i < nTotalPlayers; i++)
        {
            int nScore = nPlayerScore[i];
            if (word_9AD54[i] == nScore)
            {
                int v9 = dword_9AD64[i];
                if (v9 && v9 <= (int)totalclock) {
                    dword_9AD64[i] = 0;
                }
            }
            else
            {
                word_9AD54[i] = nScore;
                dword_9AD64[i] = (int)totalclock + 30;
            }

            overwritesprite(x, 7, nTile, 0, 3, kPalNormal);

            if (i != nLocalPlayer) {
                shade = -100;
            }

            sprintf(stringBuf, "%d", nPlayerScore[i]);
            int nStringLen = MyGetStringWidth(stringBuf);

            myprintext(x - (nStringLen / 2), 4, stringBuf, shade);

            x *= 2;
            nTile++;
        }

        if (nNetTime >= 0)
        {
            int y = nViewTop;

            if (nNetTime)
            {
                int v12 = (nNetTime + 29) / 30 % 60;
                int v13 = (nNetTime + 29) / 1800;
                nNetTime += 29;

                sprintf(stringBuf, "%d.%02d", v13, v12);

                if (bHiRes) {
                    y = nViewTop / 2;
                }

                if (nViewTop <= 0) {
                    y += 20;
                }
                else {
                    y += 15;
                }

                nNetTime -= 29;
            }
            else
            {
                y = 100;
                strcpy(stringBuf, "GAME OVER");
            }

            int nLenString = MyGetStringWidth(stringBuf);
            myprintext((320 - nLenString) / 2, y, stringBuf, 0);
        }

        Clip();
    }

    if (bCoordinates)
    {
        int nSprite = PlayerList[nLocalPlayer].nSprite;

        int x = (nViewLeft + nViewRight) / 2;

        sprintf(coordBuf, "X %d", (int)sprite[nSprite].x);
        printext(x, nViewTop + 1, coordBuf, kTileFont);

        sprintf(coordBuf, "Y %d", (int)sprite[nSprite].y);
        printext(x, nViewTop + 10, coordBuf, kTileFont);
    }

    if (bHolly)
    {
        sprintf(message_text, "HOLLY: %s", sHollyStr);
        printext(0, 0, message_text, kTileFont);
    }
    else if (nSnakeCam < 0)
    {
        if (message_timer) {
            printext(0, 0, message_text, kTileFont);
        }
    }
}

#ifdef EDUKE32
class StatusLoadSave : public LoadSave
{
public:
    virtual void Load();
    virtual void Save();
};

void StatusLoadSave::Load()
{
    Read(&nAnimsFree, sizeof(nAnimsFree));
    Read(&message_timer, sizeof(message_timer));
    Read(&message_text, sizeof(message_text));
    Read(&magicperline, sizeof(magicperline));
    Read(&airperline, sizeof(airperline));
    Read(&healthperline, sizeof(healthperline));
    Read(&nAirFrames, sizeof(nAirFrames));
    Read(&nCounter, sizeof(nCounter));
    Read(&nCounterDest, sizeof(nCounterDest));
    Read(&nStatusSeqOffset, sizeof(nStatusSeqOffset));
    Read(&nItemFrames, sizeof(nItemFrames));
    Read(&laststatusx, sizeof(laststatusx));
    Read(&laststatusy, sizeof(laststatusy));
    Read(&nItemSeq, sizeof(nItemSeq));
    Read(nDigit, sizeof(nDigit));
    Read(&nMagicFrames, sizeof(nMagicFrames));
    Read(&nHealthLevel, sizeof(nHealthLevel));
    Read(&nItemFrame, sizeof(nItemFrame));
    Read(&nMeterRange, sizeof(nMeterRange));
    Read(&nMagicLevel, sizeof(nMagicLevel));
    Read(&nHealthFrame, sizeof(nHealthFrame));
    Read(&nMagicFrame, sizeof(nMagicFrame));
    Read(&statusx, sizeof(statusx));
    Read(&statusy, sizeof(statusy));
    Read(&nHealthFrames, sizeof(nHealthFrames));
    Read(&airframe, sizeof(airframe));
    Read(&nFirstAnim, sizeof(nFirstAnim));
    Read(&nLastAnim, sizeof(nLastAnim));
    Read(&nItemAltSeq, sizeof(nItemAltSeq));
    Read(&airpages, sizeof(airpages));
    Read(&ammodelay, sizeof(ammodelay));
    Read(&nCounterBullet, sizeof(nCounterBullet));

    Read(StatusAnim, sizeof(StatusAnim));
    Read(StatusAnimsFree, sizeof(StatusAnimsFree));
    Read(StatusAnimFlags, sizeof(StatusAnimFlags));
}

void StatusLoadSave::Save()
{
    Write(&nAnimsFree, sizeof(nAnimsFree));
    Write(&message_timer, sizeof(message_timer));
    Write(&message_text, sizeof(message_text));
    Write(&magicperline, sizeof(magicperline));
    Write(&airperline, sizeof(airperline));
    Write(&healthperline, sizeof(healthperline));
    Write(&nAirFrames, sizeof(nAirFrames));
    Write(&nCounter, sizeof(nCounter));
    Write(&nCounterDest, sizeof(nCounterDest));
    Write(&nStatusSeqOffset, sizeof(nStatusSeqOffset));
    Write(&nItemFrames, sizeof(nItemFrames));
    Write(&laststatusx, sizeof(laststatusx));
    Write(&laststatusy, sizeof(laststatusy));
    Write(&nItemSeq, sizeof(nItemSeq));
    Write(nDigit, sizeof(nDigit));
    Write(&nMagicFrames, sizeof(nMagicFrames));
    Write(&nHealthLevel, sizeof(nHealthLevel));
    Write(&nItemFrame, sizeof(nItemFrame));
    Write(&nMeterRange, sizeof(nMeterRange));
    Write(&nMagicLevel, sizeof(nMagicLevel));
    Write(&nHealthFrame, sizeof(nHealthFrame));
    Write(&nMagicFrame, sizeof(nMagicFrame));
    Write(&statusx, sizeof(statusx));
    Write(&statusy, sizeof(statusy));
    Write(&nHealthFrames, sizeof(nHealthFrames));
    Write(&airframe, sizeof(airframe));
    Write(&nFirstAnim, sizeof(nFirstAnim));
    Write(&nLastAnim, sizeof(nLastAnim));
    Write(&nItemAltSeq, sizeof(nItemAltSeq));
    Write(&airpages, sizeof(airpages));
    Write(&ammodelay, sizeof(ammodelay));
    Write(&nCounterBullet, sizeof(nCounterBullet));

    Write(StatusAnim, sizeof(StatusAnim));
    Write(StatusAnimsFree, sizeof(StatusAnimsFree));
    Write(StatusAnimFlags, sizeof(StatusAnimFlags));
}

static StatusLoadSave* myLoadSave;

void StatusLoadSaveConstruct()
{
    myLoadSave = new StatusLoadSave();
}
#endif
