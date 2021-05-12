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
#include "exhumed.h"
#include "names.h"
#include "movie.h"
#include "light.h"
#ifndef EDUKE32
#include <stdio.h>
#include <string.h>
#else
#include <cstdio>
#include <cstring>
#endif
#include "baselayer.h"
#include "typedefs.h"
#include "keyboard.h"
#include "fx_man.h"
#include "config.h"
#include "sound.h"
#include "mutex.h"
#ifdef EDUKE32
#include "memorystream.h"
#endif

enum {
    kFramePalette = 0,
    kFrameSound,
    kFrameImage,
    kFrameDone
};

#define kSampleRate     22050
#define kSampleSize     2205

uint8_t bankbuf[kSampleRate];
uint32_t bankptr = 0;
uint32_t banktail = 0;

#ifndef EDUKE32
int32_t lSoundBytesRead = 0;
int32_t lSoundBytesUsed = 0;
#else
uint32_t lSoundBytesRead = 0;
uint32_t lSoundBytesUsed = 0;
#endif

uint8_t lh[32] = { 0 };

static uint8_t* CurFrame = NULL;

bool bServedSample = false;
palette_t moviepal[256];
static mutex_t mutex;


#ifdef EDUKE32
bool LoadGOGBookMovie(uint8_t *pMovie, int nFileSize)
{
    const uint8_t kSyncMark[] = { 0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00 };

    buildvfs_kfd hFile = kopen4load("game.gog", 0);
    if (hFile < 0)
        return false;

    int32_t fileSize = kfilelength(hFile);
    if (fileSize != 391203456) { // expected size of game.gog
        kclose(hFile);
        return false;
    }

    klseek(hFile, 13388 * 2352, SEEK_SET); // offset to start of file

    while (nFileSize)
    {
        uint8_t syncmark[12];

        kread(hFile, syncmark, sizeof(syncmark));
        klseek(hFile, 4, SEEK_CUR);

        if (memcmp(syncmark, kSyncMark, sizeof(kSyncMark)) != 0)
        {
            printf("invalid CD image file\n");
            kclose(hFile);
            return false;
        }

        int nRead = kread(hFile, pMovie, min(2048, nFileSize));
        pMovie += nRead;

        if (nFileSize < 2048)
            break;

        nFileSize -= 2048;

        // temp - check type
        klseek(hFile, 4 + 8 + 276, SEEK_CUR);
    }

    kclose(hFile);
    return true;
}
#endif

#ifndef EDUKE32
int ReadFrame(FILE *fp)
#else
int ReadFrame(MemoryReadStream &ms)
#endif
{
    uint8_t nType;
    uint8_t var_1C;
    int nSize;
    uint16_t yOffset;
    uint8_t xOffset;
    uint8_t nPixels;
    uint8_t palette[768];

    while (1)
    {
#ifndef EDUKE32
        if (fread(&nType, 1, sizeof(nType), fp) == 0)
#else
        if (!ms.GetBytesLeft())
#endif
            return 0;

#ifndef EDUKE32
        fread(&nSize, sizeof(nSize), 1, fp);
        nSize = B_LITTLE32(nSize);
#else
        nType = ms.GetByte();
        nSize = ms.GetUint32LE();
#endif

        nType--;
        if (nType > 3) {
            continue;
        }

        switch (nType)
        {
            case kFramePalette:
            {
#ifndef EDUKE32
                fread(palette, sizeof(palette[0]), sizeof(palette) / sizeof(palette[0]), fp);
                fread(&var_1C, sizeof(var_1C), 1, fp);
#else
                ms.GetBytes(palette, sizeof(palette));
                var_1C = ms.GetByte();

                for (auto &c : palette)
                    c <<= 2;
#endif

                paletteSetColorTable(ANIMPAL, palette);
                videoSetPalette(0, ANIMPAL, 2+8);

                memset(CurFrame, overscanindex, 4); //sizeof(CurFrame));
                continue;
            }
            case kFrameSound:
            {
                if (lSoundBytesRead - lSoundBytesUsed >= kSampleRate)
                {
                    DebugOut("ReadFrame() - Sound buffer full\n");
#ifndef EDUKE32
                    fseek(fp, nSize, SEEK_CUR);
#else
                    ms.SkipBytes(nSize);
#endif
                }
                else
                {
                    mutex_lock(&mutex);

#ifndef EDUKE32
                    int nRead = fread((char*)bankbuf + bankptr, 1, nSize, fp);
#else
                    int nRead = ms.GetBytes(bankbuf + bankptr, nSize);
#endif

                    lSoundBytesRead += nRead;
                    bankptr += nSize;

                    assert(nSize == nRead);
                    assert(bankptr <= kSampleRate);

                    if (bankptr >= kSampleRate) {
                        bankptr -= kSampleRate; // loop back to start
                    }

                    mutex_unlock(&mutex);
                }

                continue;
            }
            case kFrameImage:
            {
                if (nSize == 0) {
                    continue;
                }

                uint8_t *pFrame = CurFrame;

#ifndef EDUKE32
                fread(&yOffset, 1, sizeof(yOffset), fp);
                yOffset = B_LITTLE16(yOffset);
#else
                yOffset = ms.GetUint16LE();
#endif
                nSize -= 2;

                pFrame += yOffset * 200; // row position

                while (nSize > 0)
                {
#ifndef EDUKE32
                    fread(&xOffset, sizeof(xOffset), 1, fp);
                    fread(&nPixels, sizeof(nPixels), 1, fp);
#else
                    xOffset = ms.GetByte();
                    nPixels = ms.GetByte();
#endif
                    nSize -= 2;

                    pFrame += xOffset;

                    if (nPixels)
                    {
#ifndef EDUKE32
                        int nRead = fread(pFrame, 1, nPixels, fp);
#else
                        int nRead = ms.GetBytes(pFrame, nPixels);
#endif
                        pFrame += nRead;
                        nSize -= nRead;
                    }
                }

                tileInvalidate(kMovieTile, -1, -1);
                break;
            }
            case kFrameDone:
            {
                return 1;
                break;
            }
        }
    }
}

#ifndef EDUKE32
static void ServeSample(char** ptr, unsigned int* length)
#else
static void ServeSample(const char** ptr, uint32_t* length, void* userdata)
#endif
{
    mutex_lock(&mutex);

    *ptr = (char*)bankbuf + banktail;
    *length = kSampleSize;

    banktail += kSampleSize;
    if (banktail >= kSampleRate) {
        banktail -= kSampleRate; // rotate back to start
    }

    lSoundBytesUsed += kSampleSize;
    bServedSample = true;

    mutex_unlock(&mutex);
}

void PlayMovie(const char* fileName)
{
    int bDoFade = kTrue;
    int hFx = -1;
    uint8_t* pMovieFile = NULL;
    int fileSize = 0;

    if (!waloff[kMovieTile]) tileLoad(kMovieTile);
    CurFrame = (uint8_t*)waloff[kMovieTile];

#ifndef EDUKE32
    FILE* fp = fopen(fileName, "rb");
    setvbuf(fp, NULL, _IOFBF, 68000);
    if (fp == NULL)
#else
    // try for a loose BOOK.MOV first
    buildvfs_kfd hFile = kopen4loadfrommod(fileName, 0);
    if (hFile >= 0)
    {
        fileSize = kfilelength(hFile);
        pMovieFile = new uint8_t[fileSize];
        if (pMovieFile)
            kread(hFile, pMovieFile, fileSize);

        kclose(hFile);
    }
    else
    {
        // try read from the GOG versions game.gog image file
        fileSize = 9938599; // BOOK.MOV file size
        pMovieFile = new uint8_t[fileSize];
        if (!pMovieFile)
        {
            DebugOut("Can't open movie file %s\n", fileName);
            return;
        }

        if (!LoadGOGBookMovie(pMovieFile, fileSize))
        {
            DebugOut("Can't open movie file %s\n", fileName);
            if (!pMovieFile)
                delete[] pMovieFile;
            return;
        }
    }

    if (pMovieFile == NULL)
#endif
    {
        DebugOut("Can't open movie file %s\n", fileName);
        return;
    }

#ifndef EDUKE32
    fread(lh, sizeof(lh), 1, fp);
#else
    MemoryReadStream ms(pMovieFile, fileSize);

    ms.GetBytes(lh, sizeof(lh));
#endif

    // sound stuff
    mutex_init(&mutex);
    bankptr = 0;
    banktail = 0;

    // clear keys
    KB_FlushKeyboardQueue();
    KB_ClearKeysDown();

    if (bDoFade) {
        StartFadeIn();
    }

    int angle = 1536;
    int z = 0;
    int f = 255;

    videoSetPalette(0, ANIMPAL, 2 + 8);

    // Read a frame in first
#ifndef EDUKE32
    if (ReadFrame(fp))
#else
    if (ReadFrame(ms))
#endif
    {
        // start audio playback
#ifndef EDUKE32
        hFx = FX_StartDemandFeedPlayback(ServeSample, kSampleRate, 0, 255, 255, 255, FX_MUSIC_PRIORITY, -1);
#else
        hFx = FX_StartDemandFeedPlayback(ServeSample, 8, 1, kSampleRate, 0, MusicVolume, MusicVolume, MusicVolume, FX_MUSIC_PRIORITY, fix16_one, -1, nullptr);
#endif

        while (!KB_KeyWaiting())
        {
            HandleAsync();

            // audio is king for sync - if the backend doesn't need any more samples yet, 
            // don't process any more movie file data.
            if (!bServedSample) {
                continue;
            }

            bServedSample = false;

            if (z < 65536) { // Zoom - normal zoom is 65536.
                z += 2048;
            }
            if (angle != 0) {
                angle += 16;
                if (angle == 2048) {
                    angle = 0;
                }
            }

#ifdef __AMIGA__
            if (angle != 0) // don't clear the background after the rotate finished
#endif
            videoClearViewableArea(blackcol);
            rotatesprite(160 << 16, 100 << 16, z, angle, kMovieTile, 0, 1, 2, 0, 0, xdim - 1, ydim - 1);

            if (videoGetRenderMode() == REND_CLASSIC)
            {
                if (bDoFade) {
                    bDoFade = DoFadeIn();
                }
            }
#ifdef USE_OPENGL
            else
            {
                if (f >= 0)
                {
                    fullscreen_tint_gl(0, 0, 0, f);
                    f -= 8;
                }
            }
#endif

            videoNextPage();

#ifndef EDUKE32
            if (ReadFrame(fp) == 0) {
#else
            if (ReadFrame(ms) == 0) {
#endif
                break;
            }
        }
    }

    if (hFx > 0) {
        FX_StopSound(hFx);
    }

    if (KB_KeyWaiting()) {
        KB_GetCh();
    }

    mutex_destroy(&mutex);

#ifndef EDUKE32
    fclose(fp);
#else
    if (pMovieFile)
        delete[] pMovieFile;
#endif

#ifdef USE_OPENGL
    // need to do OpenGL fade out here
    f = 0;

    while (f <= 255)
    {
        HandleAsync();

        rotatesprite(160 << 16, 100 << 16, z, angle, kMovieTile, 0, 1, 2, 0, 0, xdim - 1, ydim - 1);

        fullscreen_tint_gl(0, 0, 0, f);
        f += 4;

        WaitTicks(2);

        videoNextPage();
    }
#endif
}
