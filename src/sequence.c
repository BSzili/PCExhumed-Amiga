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

#include "typedefs.h"
#include "sequence.h"
#include "engine.h"
#include "exhumed.h"
#include "sound.h"
#include "player.h"
#include "trigdat.h"
#include "move.h"
#include "view.h"
#include "init.h"
#include "light.h"
#include "baselayer.h"
#if !defined __WATCOMC__ && defined EDUKE32
#include <cstring>
#include <cstdio> // for printf
#else
#include <string.h>
#include <stdio.h>
#endif

// TEMP
#include <assert.h>

#ifdef __AMIGA__
#define kMaxSequences	1500
#define kMaxSEQFiles	78
#define kMaxSEQFrames	16300
#define kMaxSEQChunks	20000
#else
#define kMaxSequences	4096
#define kMaxSEQFiles	78
#define kMaxSEQFrames	18000
#define kMaxSEQChunks	21000
#endif

short sequences = 0;
short frames = 0;
short chunks = 0;
short nPilotLightFrame;
short nPilotLightCount;

short nPilotLightBase;
short laststatustile;

short nShadowWidth = 1;
short nFlameHeight = 1;

short SeqBase[kMaxSequences];
short SeqSize[kMaxSequences];
short SeqFlag[kMaxSequences];

short FrameSound[kMaxSEQFrames];
short FrameSize[kMaxSEQFrames];
short FrameBase[kMaxSEQFrames];
short FrameFlag[kMaxSEQFrames];

short ChunkYpos[kMaxSEQChunks];
short ChunkXpos[kMaxSEQChunks];
short ChunkPict[kMaxSEQChunks];
short ChunkFlag[kMaxSEQChunks];


const char *SeqNames[kMaxSEQFiles] =
{
  "rothands",
  "sword",
  "pistol",
  "m_60",
  "flamer", // 4
  "grenade",
  "cobra",
  "bonesaw",
  "scramble",
  "glove",
  "mummy", // 10
  "skull",
  "poof",
  "kapow",
  "fireball",
  "bubble",
  "spider", // 16
  "anubis",
  "anuball",
  "fish",
  "snakehed", // 20?
  "snakbody",
  "wasp",
  "cobrapow",
  "scorp",
  "joe", // 25
  "status",
  "dead",
  "deadex",
  "anupoof",
  "skulpoof", // 30
  "bullet",
  "shadow",
  "grenroll",
  "grenboom",
  "splash",
  "grenpow",
  "skulstrt",
  "firepoof",
  "bloodhit",
  "lion", // 40
  "items",
  "lavag", // 42
  "lsplash",
  "lavashot",
  "smokebal",
  "firepot",
  "rex",
  "set", // 48
  "queen",
  "roach", // 50
  "hawk",
  "setghost",
  "setgblow",
  "bizztail",
  "bizzpoof",
  "queenegg",
  "roacshot",
  "backgrnd",
  "screens", // 59
  "arrow",
  "fonts",
  "drips",
  "firetrap",
  "magic2",
  "creepy",
  "slider", // 66
  "ravolt",
  "eyehit",
  "font2", // 69
  "seebubbl",
  "blood",
  "drum",
  "poof2",
  "deadbrn",
  "grenbubb",
  "rochfire",
  "rat"
};

short SeqOffsets[kMaxSEQFiles];


int seq_ReadSequence(const char *seqName)
{
    int i;
    char buffer[200];
    buffer[0] = '\0';

    strcat(buffer, seqName);
    strcat(buffer, ".seq");

    int hFile = kopen4loadfrommod(buffer, 0);
    if (hFile == -1)
    {
        initprintf("Unable to open '%s'!\n", buffer);
        kclose(hFile);
        return 0;
    }

    short tag;
    kread(hFile, &tag, sizeof(tag));
#if B_BIG_ENDIAN == 1
    tag = B_LITTLE16(tag);
#endif
    if (tag < 'HI' || tag > 'HI' && tag != 'SD')
    {
        initprintf("Unsupported sequence version!\n");
        kclose(hFile);
        return 0;
    }

    short centerx, centery; // TODO - are global vars?
    short nSeqs;
    kread(hFile, &centerx, sizeof(centerx));
    kread(hFile, &centery, sizeof(centery));
    kread(hFile, &nSeqs, sizeof(nSeqs));
#if B_BIG_ENDIAN == 1
    centerx = B_LITTLE16(centerx);
    centery = B_LITTLE16(centery);
    nSeqs = B_LITTLE16(nSeqs);
#endif

    if (nSeqs <= 0 || sequences + nSeqs >= kMaxSequences)
    {
        if (nSeqs < 0)
        {
            initprintf("Invalid sequence count!\n");
            kclose(hFile);
            return 0;
        }
        else {
            bail2dos("Not enough sequences available!  Increase array!\n");
        }
    }

    kread(hFile, &SeqBase[sequences], nSeqs * sizeof(SeqBase[0]));
    kread(hFile, &SeqSize[sequences], nSeqs * sizeof(SeqSize[0]));
    kread(hFile, &SeqFlag[sequences], nSeqs * sizeof(SeqFlag[0]));

    for (i = 0; i < nSeqs; i++)
    {
#if B_BIG_ENDIAN == 1
        SeqBase[sequences + i] = B_LITTLE16(SeqBase[sequences + i]);
        SeqSize[sequences + i] = B_LITTLE16(SeqSize[sequences + i]);
        SeqFlag[sequences + i] = B_LITTLE16(SeqFlag[sequences + i]);
#endif
        SeqBase[sequences + i] += frames;
    }

    short vdi = frames;

    int16_t nFrames;
    kread(hFile, &nFrames, sizeof(nFrames));
#if B_BIG_ENDIAN == 1
    nFrames = B_LITTLE16(nFrames);
#endif

    if (nFrames <= 0 || frames + nFrames >= kMaxSEQFrames)
    {
        if (nFrames < 0 )
        {
            initprintf("Invalid frame count!\n");
            kclose(hFile);
            return 0;
        }
        else {
            bail2dos("Not enough frames available!  Increase FRAMEMAX!\n");
        }
    }

    kread(hFile, &FrameBase[frames], nFrames * sizeof(FrameBase[0]));
    kread(hFile, &FrameSize[frames], nFrames * sizeof(FrameSize[0]));
    kread(hFile, &FrameFlag[frames], nFrames * sizeof(FrameFlag[0]));
    memset(&FrameSound[frames], -1,  nFrames * sizeof(FrameSound[0]));

    for (i = 0; i < nFrames; i++)
    {
#if B_BIG_ENDIAN == 1
        FrameBase[frames + i] = B_LITTLE16(FrameBase[frames + i]);
        FrameSize[frames + i] = B_LITTLE16(FrameSize[frames + i]);
        FrameFlag[frames + i] = B_LITTLE16(FrameFlag[frames + i]);
#endif
        FrameBase[frames + i] += chunks;
    }

    int16_t nChunks;
    kread(hFile, &nChunks, sizeof(nChunks));
#if B_BIG_ENDIAN == 1
    nChunks = B_LITTLE16(nChunks);
#endif

    if (nChunks < 0 || chunks + nChunks >= kMaxSEQChunks)
    {
        if (nChunks < 0 )
        {
            initprintf("Invalid chunk count!\n");
            kclose(hFile);
            return 0;
        }
        else {
            bail2dos("Not enough chunks available!  Increase CHUNKMAX!\n");
        }
    }

    kread(hFile, &ChunkXpos[chunks], nChunks * sizeof(ChunkXpos[0]));
    kread(hFile, &ChunkYpos[chunks], nChunks * sizeof(ChunkYpos[0]));
    kread(hFile, &ChunkPict[chunks], nChunks * sizeof(ChunkPict[0]));
    kread(hFile, &ChunkFlag[chunks], nChunks * sizeof(ChunkFlag[0]));

    for (i = 0; i < nChunks; i++)
    {
#if B_BIG_ENDIAN == 1
        ChunkXpos[chunks + i] = B_LITTLE16(ChunkXpos[chunks + i]);
        ChunkYpos[chunks + i] = B_LITTLE16(ChunkYpos[chunks + i]);
        ChunkPict[chunks + i] = B_LITTLE16(ChunkPict[chunks + i]);
        ChunkFlag[chunks + i] = B_LITTLE16(ChunkFlag[chunks + i]);
#endif
        ChunkXpos[chunks + i] -= centerx;
        ChunkYpos[chunks + i] -= centery;

        short nTile = ChunkPict[chunks + i];
        if (waloff[nTile] == 0)
        {
            tileLoad(nTile);
#ifdef __AMIGA__
            // precache the sequence pictures at the lowest lock
            walock[nTile] = 1;
            //if (tilesizx[nTile]*tilesizy[nTile]) buildprintf("%s precached tile: %d\n", __FUNCTION__, nTile);
#endif
        }
    }

    sequences += nSeqs;
    FrameBase[frames + nFrames] = chunks + nChunks;
    frames += nFrames;
    SeqBase[sequences] = frames;
    chunks += nChunks;

    if (tag == 'SD')
    {
        short var_20;
        kread(hFile, &var_20, sizeof(var_20));
#if B_BIG_ENDIAN == 1
        var_20 = B_LITTLE16(var_20);
#endif

        for (i = 0; i < var_20; i++)
        {
            kread(hFile, &buffer[i * 10], 8);
        }

        short var_24;
        kread(hFile, &var_24, sizeof(var_24));
#if B_BIG_ENDIAN == 1
        var_24 = B_LITTLE16(var_24);
#endif

        for (i = 0; i < var_24; i++)
        {
            short var_28, var_2C;
            kread(hFile, &var_28, sizeof(var_28));
            kread(hFile, &var_2C, sizeof(var_2C));
#if B_BIG_ENDIAN == 1
            var_28 = B_LITTLE16(var_28);
            var_2C = B_LITTLE16(var_2C);
#endif

            int hSound = LoadSound(&buffer[(var_2C&0x1FF)*10]);

            FrameSound[vdi + var_28] = hSound | (var_2C & 0xFE00);
        }
    }

    kclose(hFile);
    return nSeqs;
}

int seq_GetFirstSeqPicnum(int nSeq)
{
    int i = SeqOffsets[nSeq];
    i = SeqBase[i];
    i = FrameBase[i];
    i = ChunkPict[i];

    return i;
}

void seq_LoadSequences()
{
    int i;

    for (i = 0; i < kMaxSEQFiles; i++)
    {
        SeqOffsets[i] = sequences;

        if (seq_ReadSequence(SeqNames[i]) == 0) {
            initprintf("Error loading '%s'\n", SeqNames[i]);
        }
    }

    nShadowPic = seq_GetFirstSeqPicnum(kSeqShadow);
#ifndef EDUKE32
    nShadowWidth = tilesizx[nShadowPic];
    nFlameHeight = tilesizy[seq_GetFirstSeqPicnum(kSeqFirePoof)];
#else
    nShadowWidth = tilesiz[nShadowPic].x;

    nFlameHeight = tilesiz[seq_GetFirstSeqPicnum(kSeqFirePoof)].y;
#endif

    nBackgroundPic = seq_GetFirstSeqPicnum(kSeqBackgrnd);

    nPilotLightBase  = SeqBase[SeqOffsets[kSeqFlamer] + 3];
    nPilotLightCount = SeqSize[SeqOffsets[kSeqFlamer] + 3];
    nPilotLightFrame = 0;

    nFontFirstChar = seq_GetFirstSeqPicnum(kSeqFont2);

    short nSize = SeqSize[SeqOffsets[kSeqFont2]];

    for (i = 0; i < nSize; i++)
    {
#ifndef EDUKE32
        picanm[nFontFirstChar + i] &= 0xFF0000FF;
#else
        picanm[nFontFirstChar + i].xofs = 0;
        picanm[nFontFirstChar + i].yofs = 0;
#endif
    }

	//buildprintf("sequences %d frames %d chunks %d\n", sequences, frames, chunks);
}

void seq_DrawStatusSequence(short nSequence, uint16_t edx, short ebx)
{
    edx += SeqBase[nSequence];

    short nFrameBase = FrameBase[edx];
    int16_t nFrameSize = FrameSize[edx];

#ifdef __AMIGA__
    int const nPal = kPalNormal;
#else
    int const nPal = RemapPLU(kPalNormal);
#endif

    while (1)
    {
        nFrameSize--;
        if (nFrameSize < 0)
            break;

        uint8_t nStat = 1; // (thex, they) is middle

        laststatusx = ChunkXpos[nFrameBase] + 160;
        laststatusy = ChunkYpos[nFrameBase] + 100 + ebx;

        short chunkFlag = ChunkFlag[nFrameBase];

        if (chunkFlag & 1) {
            nStat = 0x9; // (thex, they) is middle, and x-flipped
        }

        if (chunkFlag & 2) {
            nStat |= 0x10; // y-flipped
        }

        laststatustile = ChunkPict[nFrameBase];

        if (bHiRes) {
            nStat |= 0x2; // scale and clip to viewing window
        }

        overwritesprite(laststatusx, laststatusy, laststatustile, 0, nStat, nPal);
        nFrameBase++;
    }
}

short seq_GetFrameFlag(short val, short nFrame)
{
    return FrameFlag[SeqBase[val] + nFrame];
}

void seq_DrawPilotLightSeq(int xOffset, int yOffset)
{
    short nSect = nPlayerViewSect[nLocalPlayer];

    if (!(SectFlag[nSect] & kSectUnderwater))
    {
        short nFrame = nPilotLightBase + nPilotLightFrame;
        short nFrameBase = FrameBase[nFrame];
        short nFrameSize = FrameSize[nFrame];

        while (1)
        {
            nFrameSize--;
            if (nFrameSize < 0)
                return;

            short nTile = ChunkPict[nFrameBase];
            int x = ChunkXpos[nFrameBase] + (160 + xOffset);
            int y = ChunkYpos[nFrameBase] + (100 + yOffset);

#ifndef EDUKE32
            rotatesprite(x << 16, y << 16, 0x10000, (-2 * fix16_to_int(nPlayerDAng)) & kAngleMask, nTile, -127, 1, 2, windowx1, windowy1, windowx2, windowy2);
#else
            rotatesprite(x << 16, y << 16, 0x10000, (-2 * fix16_to_int(nPlayerDAng)) & kAngleMask, nTile, -127, 1, 2, windowxy1.x, windowxy1.y, windowxy2.x, windowxy2.y);
#endif
            nFrameBase++;
        }
    }
}

/*
    6 parameters

    arg0 - shade?

*/

int seq_DrawGunSequence(int nSeqOffset, short dx, int xOffs, int yOffs, int nShade, int nPal)
{
    short nFrame = SeqBase[nSeqOffset] + dx;
    short nFrameBase = FrameBase[nFrame];
    short nFrameSize = FrameSize[nFrame];
    short frameFlag = FrameFlag[nFrame];

    while (1)
    {
        nFrameSize--;
        if (nFrameSize < 0)
            break;

        uint8_t nStat = 3;
        int x = ChunkXpos[nFrameBase] + 160;
        int y = ChunkYpos[nFrameBase] + 100;

        if (ChunkFlag[nFrameBase] & 1) {
            nStat = 11;
        }

        if (ChunkFlag[nFrameBase] & 2) {
            nStat |= 0x10;
        }

        short nTile = ChunkPict[nFrameBase];

        if (frameFlag & 4) {
            nShade = -100;
        }

        if (nPlayerInvisible[nLocalPlayer]) {
            nStat |= 0x4;
        }

        overwritesprite(x + xOffs, y + yOffs, nTile, nShade, nStat, nPal);
        nFrameBase++;
    }

    return frameFlag;
}

int seq_GetFrameSound(int val, int edx)
{
    return FrameSound[SeqBase[val] + edx];
}

void seq_MoveSequence(short nSprite, short nSeq, short bx)
{
    assert(nSeq >= 0); // TEMP

    int nSound = FrameSound[SeqBase[nSeq] + bx];
    if (nSound == -1) {
        return;
    }

    if (nSprite > -1) {
        D3PlayFX(nSound, nSprite);
    }
    else {
        PlayLocalSound(nSound, 0);
    }
}

int seq_GetSeqPicnum2(short nSeq, short nFrame)
{
    short nBase = FrameBase[SeqBase[nSeq] + nFrame];
    return ChunkPict[nBase];
}

int seq_GetSeqPicnum(short nSeq, short edx, short ebx)
{
    edx += SeqOffsets[nSeq];
    ebx += SeqBase[edx];
    short c = FrameBase[ebx];

    return ChunkPict[c];
}

int seq_PlotArrowSequence(short nSprite, short nSeq, int nVal)
{
    tspriteptr_t pTSprite = &tsprite[nSprite];
    int nAngle = GetMyAngle(nCamerax - pTSprite->x, nCameray - pTSprite->y);

    int nSeqOffset = ((((pTSprite->ang + 512) - nAngle) + 128) & kAngleMask) >> 8;

    short nFrame = SeqBase[nSeqOffset + nSeq] + nVal;

    short nFrameBase = FrameBase[nFrame];
    short nFrameSize = FrameSize[nFrame];

    uint8_t nShade = pTSprite->shade;
    short nStat = pTSprite->cstat;

    nStat |= 0x80;

    if (nSeqOffset & 3) {
        nStat |= 0x18;
    }
    else {
        nStat &= 0x0E7;
    }

    if (FrameFlag[nFrame] & 4) {
        nShade -= 100;
    }

    pTSprite->cstat = nStat;
    pTSprite->shade = nShade;
    pTSprite->statnum = nFrameSize;

    if (ChunkFlag[nFrameBase] & 1)
    {
        pTSprite->xoffset = ChunkXpos[nFrameBase];
        pTSprite->cstat |= 4;
    }
    else
    {
        pTSprite->xoffset = -ChunkXpos[nFrameBase];
    }

    pTSprite->yoffset = -ChunkYpos[nFrameBase];
    pTSprite->picnum = ChunkPict[nFrameBase];

    return ChunkPict[nFrameBase];
}

int seq_PlotSequence(short nSprite, short edx, short nFrame, short ecx)
{
    tspriteptr_t pTSprite = &tsprite[nSprite];
    int nAngle = GetMyAngle(nCamerax - pTSprite->x, nCameray - pTSprite->y);

    int val;

    if (ecx & 1)
    {
        val = 0;
    }
    else
    {
        val = (((pTSprite->ang - nAngle) + 128) & kAngleMask) >> 8;
    }

    int eax = SeqBase[edx] + nFrame;
    int edi = SeqBase[edx + val] + nFrame;

    short nBase = FrameBase[edi];
    short nSize = FrameSize[edi];

    int8_t shade = pTSprite->shade;

    if (FrameFlag[eax] & 4)
    {
        shade -= 100;
    }

    short nPict = ChunkPict[nBase];

    if (ecx & 0x100)
    {
        edx = -3;
    }
    else
    {
        edx = 100;
    }

    int esi = nSize + 1;
    esi += edx;

    int var_14 = edx + 1;
    short nOwner = pTSprite->owner;

    while (1)
    {
        esi--;
        nSize--;

        if (esi < var_14) {
            break;
        }

        tspriteptr_t tsp = &tsprite[spritesortcnt++];
        tsp->x       = pTSprite->x;
        tsp->y       = pTSprite->y;
        tsp->z       = pTSprite->z;
        tsp->shade   = shade;
        tsp->pal     = pTSprite->pal;
        tsp->xrepeat = pTSprite->xrepeat;
        tsp->yrepeat = pTSprite->yrepeat;
        tsp->ang     = pTSprite->ang;
        tsp->owner   = pTSprite->owner;
        tsp->sectnum = pTSprite->sectnum;
        tsp->cstat   = pTSprite->cstat |= 0x80;
        tsp->statnum = esi;

        if (ChunkFlag[nBase] & 1)
        {
            tsp->xoffset = ChunkXpos[nBase];
            tsp->cstat |= 4; // x-flipped
        }
        else
        {
            tsp->xoffset = -ChunkXpos[nBase];
        }

        tsp->yoffset = -ChunkYpos[nBase];
        tsp->picnum = ChunkPict[nBase];

        nBase++;
    }

    if (!(pTSprite->cstat & 0x101) || (sprite[nOwner].statnum == 100 && nNetPlayerCount))
    {
        pTSprite->owner = -1;
    }
    else
    {
        short nSector = pTSprite->sectnum;
        int nFloorZ = sector[nSector].floorz;

        if (nFloorZ <= eyelevel[nLocalPlayer] + initz) {
            pTSprite->owner = -1;
        }
        else
        {
            pTSprite->picnum = nShadowPic;

#ifndef EDUKE32
            int edx = ((tilesizx[nPict] << 5) / nShadowWidth) - ((nFloorZ - pTSprite->z) >> 10);
#else
            int edx = ((tilesiz[nPict].x << 5) / nShadowWidth) - ((nFloorZ - pTSprite->z) >> 10);
#endif
            if (edx < 1) {
                edx = 1;
            }

            pTSprite->cstat = 0x22; // transluscence, floor sprite
            pTSprite->z = videoGetRenderMode() >= REND_POLYMOST ? nFloorZ : nFloorZ + 1;
            pTSprite->yrepeat = (uint8_t)edx;
            pTSprite->xrepeat = (uint8_t)edx;
            pTSprite->statnum = -3;
            pTSprite->pal = 0;
        }
    }

    return nPict;
}
