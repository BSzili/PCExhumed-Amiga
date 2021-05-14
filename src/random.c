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

#include "random.h"
#include "save.h"

static int randseed = 0x1010101;


void InitRandom()
{
    randseed = 0x1010101;
}

int RandomBit()
{
    randseed = (randseed >> 1) | ((((randseed >> 1) ^ (randseed >> 28)) & 1) << 28);
    return randseed & 1;
}

char RandomByte()
{
    char randByte = RandomBit() << 7;
    randByte |= RandomBit() << 6;
    randByte |= RandomBit() << 5;
    randByte |= RandomBit() << 4;
    randByte |= RandomBit() << 3;
    randByte |= RandomBit() << 2;
    randByte |= RandomBit() << 1;
    randByte |= RandomBit();
    return randByte;
}

uint16_t RandomWord()
{
#ifndef EDUKE32
    // this workaround is needed on some 32-bit systems
    uint8_t byte1 = (uint8_t)RandomByte();
    uint8_t byte2 = (uint8_t)RandomByte();
    uint16_t randWord = byte1 << 8 | byte2;
#else
    short randWord = RandomByte() << 8;
    randWord |= RandomByte();
#endif
    return randWord;
}

int RandomLong()
{
    int randLong = RandomWord() << 16;
    randLong |= RandomWord();
    return randLong;
}

int RandomSize(int nSize)
{
    int randSize = 0;

    while (nSize > 0)
    {
        randSize = randSize * 2 | RandomBit();
        nSize--;
    }

    return randSize;
}

#ifdef EDUKE32
class RandomLoadSave : public LoadSave
{
public:
    virtual void Load();
    virtual void Save();
};

void RandomLoadSave::Load()
{
    Read(&randseed, sizeof(randseed));
}

void RandomLoadSave::Save()
{
    Write(&randseed, sizeof(randseed));
}

static RandomLoadSave* myLoadSave;

void RandomLoadSaveConstruct()
{
    myLoadSave = new RandomLoadSave();
}
#endif
