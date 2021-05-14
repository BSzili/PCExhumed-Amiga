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

#include "mummy.h"
#include "sequence.h"
#include "move.h"
#include "map.h"
#include "sound.h"
#include "exhumed.h"
#include "random.h"
#include "trigdat.h"
#include "bullet.h"
#include "items.h"
#include "save.h"
#include <assert.h>
#include "engine.h"

struct Mummy
{
    short nHealth;
    short B;
    short nAction;
    short nSprite;
    short nTarget;
    short F;
    short G;
    short H;
};
#ifndef EDUKE32
typedef struct Mummy Mummy;
#endif

static actionSeq ActionSeq[] = {
    {8,  0},
    {0,  0},
    {16, 0},
    {24, 0},
    {32, 1},
    {40, 1},
    {48, 1},
    {50, 0}
};

short nMummies = -1;
Mummy MummyList[kMaxMummies];


void InitMummy()
{
    nMummies = 0;
}

int BuildMummy(int nSprite, int x, int y, int z, int nSector, int nAngle)
{
    if (nMummies >= kMaxMummies) {
        return -1;
    }

    short nMummy = nMummies++;

    if (nSprite == -1)
    {
        nSprite = insertsprite(nSector, 102);
    }
    else
    {
        x = sprite[nSprite].x;
        y = sprite[nSprite].y;
        z = sprite[nSprite].z;
        nAngle = sprite[nSprite].ang;

        changespritestat(nSprite, 102);
    }

    assert(nSprite >= 0 && nSprite < kMaxSprites);

    sprite[nSprite].x = x;
    sprite[nSprite].y = y;
    sprite[nSprite].z = z;
    sprite[nSprite].cstat = 0x101;
    sprite[nSprite].shade = -12;
    sprite[nSprite].clipdist = 32;
    sprite[nSprite].xvel = 0;
    sprite[nSprite].yvel = 0;
    sprite[nSprite].zvel = 0;
    sprite[nSprite].xrepeat = 42;
    sprite[nSprite].yrepeat = 42;
    sprite[nSprite].pal = sector[sprite[nSprite].sectnum].ceilingpal;
    sprite[nSprite].xoffset = 0;
    sprite[nSprite].yoffset = 0;
    sprite[nSprite].ang = nAngle;
    sprite[nSprite].picnum = 1;
    sprite[nSprite].hitag = 0;
    sprite[nSprite].lotag = runlist_HeadRun() + 1;
    sprite[nSprite].extra = -1;

//  GrabTimeSlot(3);

    MummyList[nMummy].nAction = 0;
    MummyList[nMummy].nHealth = 640;
    MummyList[nMummy].B = 0;
    MummyList[nMummy].nSprite = nSprite;
    MummyList[nMummy].nTarget = -1;
    MummyList[nMummy].F = nMummy;
    MummyList[nMummy].G = 0;

    sprite[nSprite].owner = runlist_AddRunRec(sprite[nSprite].lotag - 1, nMummy | 0xE0000);

    MummyList[nMummy].H = runlist_AddRunRec(NewRun, nMummy | 0xE0000);

    nCreaturesLeft++;

    return (nMummy | 0xE0000);
}

void CheckMummyRevive(short nMummy)
{
    short nSprite = MummyList[nMummy].nSprite;

    for (int i = 0; i < nMummies; i++)
    {
        if (i != nMummy)
        {
            short nSprite2 = MummyList[i].nSprite;
            if (sprite[nSprite2].statnum != 102) {
                continue;
            }

            if (MummyList[i].nAction != 5) {
                continue;
            }

            int x = klabs(sprite[nSprite2].x - sprite[nSprite].x) >> 8;
            int y = klabs(sprite[nSprite2].y - sprite[nSprite].y) >> 8;

            if (x <= 20 && y <= 20)
            {
                if (cansee(sprite[nSprite].x, sprite[nSprite].y, sprite[nSprite].z - 8192, sprite[nSprite].sectnum,
                          sprite[nSprite2].x, sprite[nSprite2].y, sprite[nSprite2].z - 8192, sprite[nSprite2].sectnum))
                {
                    sprite[nSprite2].cstat = 0;
                    MummyList[i].nAction = 6;
                    MummyList[i].B = 0;
                }
            }
        }
    }
}

void FuncMummy(int a, int nDamage, int nRun)
{
    short nMummy = RunData[nRun].nVal;
    assert(nMummy >= 0 && nMummy < kMaxMummies);

    short nTarget = UpdateEnemy(&MummyList[nMummy].nTarget);

    short nSprite = MummyList[nMummy].nSprite;
    short nAction = MummyList[nMummy].nAction;

    int nMessage = a & kMessageMask;

    switch (nMessage)
    {
        default:
        {
            DebugOut("unknown msg %d for Mummy\n", nMessage);
            break;
        }

        case 0x20000:
        {
            Gravity(nSprite);

            int nSeq = SeqOffsets[kSeqMummy] + ActionSeq[nAction].a;

            sprite[nSprite].picnum = seq_GetSeqPicnum2(nSeq, MummyList[nMummy].B);

            short nFrame = SeqBase[nSeq] + MummyList[nMummy].B;
#ifdef __AMIGA__
            volatile // compiler bug workaround
#endif
            short nFrameFlag = FrameFlag[nFrame];

            seq_MoveSequence(nSprite, nSeq, MummyList[nMummy].B);

            bool bVal = false;

            MummyList[nMummy].B++;
            if (MummyList[nMummy].B >= SeqSize[nSeq])
            {
                MummyList[nMummy].B = 0;

                bVal = true;
            }

            if (nTarget != -1 && nAction < 4)
            {
                if ((!sprite[nTarget].cstat) && nAction)
                {
                    MummyList[nMummy].nAction = 0;
                    MummyList[nMummy].B = 0;
                    sprite[nSprite].xvel = 0;
                    sprite[nSprite].yvel = 0;
                }
            }

            int nMov = MoveCreatureWithCaution(nSprite);

            if (nAction > 7)
                return;

            switch (nAction)
            {
                case 0:
                {
                    if ((MummyList[nMummy].F & 0x1F) == (totalmoves & 0x1F))
                    {
                        sprite[nSprite].cstat = 0x101;

                        if (nTarget < 0)
                        {
                            int nTarget = FindPlayer(nSprite, 100);
                            if (nTarget >= 0)
                            {
                                D3PlayFX(StaticSound[kSoundMummyChant0], nSprite);
                                MummyList[nMummy].B = 0;
                                MummyList[nMummy].nTarget = nTarget;
                                MummyList[nMummy].nAction = 1;
                                MummyList[nMummy].G = 90;

                                sprite[nSprite].xvel = Cos(sprite[nSprite].ang) >> 2;
                                sprite[nSprite].yvel = sintable[sprite[nSprite].ang] >> 2; // NOTE no angle masking in original code
                            }
                        }
                    }
                    return;
                }

                case 1:
                {
                    if (MummyList[nMummy].G > 0)
                    {
                        MummyList[nMummy].G--;
                    }

                    if ((MummyList[nMummy].F & 0x1F) == (totalmoves & 0x1F))
                    {
                        sprite[nSprite].cstat = 0x101;

                        PlotCourseToSprite(nSprite, nTarget);

                        if (MummyList[nMummy].nAction == 1)
                        {
                            if (RandomBit())
                            {
                                if (cansee(sprite[nSprite].x, sprite[nSprite].y, sprite[nSprite].z - GetSpriteHeight(nSprite), sprite[nSprite].sectnum,
                                    sprite[nTarget].x, sprite[nTarget].y, sprite[nTarget].z - GetSpriteHeight(nTarget), sprite[nTarget].sectnum))
                                {
                                    MummyList[nMummy].nAction = 3;
                                    MummyList[nMummy].B = 0;

                                    sprite[nSprite].xvel = 0;
                                    sprite[nSprite].yvel = 0;
                                    return;
                                }
                            }
                        }
                    }

                    // loc_2B5A8
                    if (!MummyList[nMummy].B)
                    {
                        sprite[nSprite].xvel = Cos(sprite[nSprite].ang) >> 1;
                        sprite[nSprite].yvel = Sin(sprite[nSprite].ang) >> 1;
                    }

                    if (sprite[nSprite].xvel || sprite[nSprite].yvel)
                    {
                        if (sprite[nSprite].xvel > 0)
                        {
                            sprite[nSprite].xvel -= 1024;
                            if (sprite[nSprite].xvel < 0) {
                                sprite[nSprite].xvel = 0;
                            }
                        }
                        else if (sprite[nSprite].xvel < 0)
                        {
                            sprite[nSprite].xvel += 1024;
                            if (sprite[nSprite].xvel > 0) {
                                sprite[nSprite].xvel = 0;
                            }
                        }

                        if (sprite[nSprite].yvel > 0)
                        {
                            sprite[nSprite].yvel -= 1024;
                            if (sprite[nSprite].yvel < 0) {
                                sprite[nSprite].yvel = 0;
                            }
                        }
                        else if (sprite[nSprite].yvel < 0)
                        {
                            sprite[nSprite].yvel += 1024;
                            if (sprite[nSprite].yvel > 0) {
                                sprite[nSprite].yvel = 0;
                            }
                        }
                    }

                    if (nMov)
                    {
                        switch (nMov & 0xC000)
                        {
                            case 0x8000:
                            {
                                sprite[nSprite].ang = (sprite[nSprite].ang + ((RandomWord() & 0x3FF) + 1024)) & kAngleMask;
                                sprite[nSprite].xvel = Cos(sprite[nSprite].ang) >> 2;
                                sprite[nSprite].yvel = Sin(sprite[nSprite].ang) >> 2;
                                return;
                            }

                            case 0xC000:
                            {
                                if ((nMov & 0x3FFF) == nTarget)
                                {
                                    int nAngle = getangle(sprite[nTarget].x - sprite[nSprite].x, sprite[nTarget].y - sprite[nSprite].y);
                                    if (AngleDiff(sprite[nSprite].ang, nAngle) < 64)
                                    {
                                        MummyList[nMummy].nAction = 2;
                                        MummyList[nMummy].B = 0;

                                        sprite[nSprite].xvel = 0;
                                        sprite[nSprite].yvel = 0;
                                    }
                                }
                                return;
                            }
                        }
                    }

                    break;
                }

                case 2:
                {
                    if (nTarget == -1)
                    {
                        MummyList[nMummy].nAction = 0;
                        MummyList[nMummy].B = 0;
                    }
                    else
                    {
                        if (PlotCourseToSprite(nSprite, nTarget) >= 1024)
                        {
                            MummyList[nMummy].nAction = 1;
                            MummyList[nMummy].B = 0;
                        }
                        else if (nFrameFlag & 0x80)
                        {
                            runlist_DamageEnemy(nTarget, nSprite, 5);
                        }
                    }
                    return;
                }

                case 3:
                {
                    if (bVal)
                    {
                        MummyList[nMummy].B = 0;
                        MummyList[nMummy].nAction = 0;
                        MummyList[nMummy].G = 100;
                        MummyList[nMummy].nTarget = -1;
                        return;
                    }
                    else if (nFrameFlag & 0x80)
                    {
                        SetQuake(nSprite, 100);

                        // low 16 bits of returned var contains the sprite index, the high 16 the bullet number
                        int nBullet = BuildBullet(nSprite, 9, 0, 0, -15360, sprite[nSprite].ang, nTarget + 10000, 1);
                        CheckMummyRevive(nMummy);

                        if (nBullet > -1)
                        {
                            if (!RandomSize(3))
                            {
                                // FIXME CHECKME - nBullet & 0xFFFF can be -1. Original code doesn't handle this??

                                SetBulletEnemy(nBullet >> 16, nTarget); // isolate the bullet number (shift off the sprite index)
                                sprite[nBullet & 0xFFFF].pal = 5;
                            }
                        }
                    }
                    return;
                }

                case 4:
                {
                    if (bVal)
                    {
                        MummyList[nMummy].B = 0;
                        MummyList[nMummy].nAction = 5;
                    }
                    return;
                }

                case 5:
                {
                    MummyList[nMummy].B = 0;
                    return;
                }

                case 6:
                {
                    if (bVal)
                    {
                        sprite[nSprite].cstat = 0x101;

                        MummyList[nMummy].nAction = 0;
                        MummyList[nMummy].nHealth = 300;
                        MummyList[nMummy].nTarget = -1;

                        nCreaturesLeft++;
                    }
                    return;
                }

                case 7:
                {
                    if (nMov & 0x20000)
                    {
                        sprite[nSprite].xvel >>= 1;
                        sprite[nSprite].yvel >>= 1;
                    }

                    if (bVal)
                    {
                        sprite[nSprite].xvel = 0;
                        sprite[nSprite].yvel = 0;
                        sprite[nSprite].cstat = 0x101;

                        MummyList[nMummy].nAction = 0;
                        MummyList[nMummy].B = 0;
                        MummyList[nMummy].nTarget = -1;
                    }

                    return;
                }
            }

            return;
        }

        case 0x90000:
        {
            seq_PlotSequence(a & 0xFFFF, SeqOffsets[kSeqMummy] + ActionSeq[nAction].a, MummyList[nMummy].B, ActionSeq[nAction].b);
            return;
        }

        case 0xA0000:
        {
            if (MummyList[nMummy].nHealth <= 0)
                return;

            nDamage = runlist_CheckRadialDamage(nSprite);
            // fall through to 0x80000
            fallthrough__;
        }
        case 0x80000:
        {
            if (nDamage <= 0)
                return;

            if (MummyList[nMummy].nHealth <= 0) {
                return;
            }

            MummyList[nMummy].nHealth -= nDamage;

            if (MummyList[nMummy].nHealth <= 0)
            {
                MummyList[nMummy].nHealth = 0;
                sprite[nSprite].cstat &= 0xFEFE;
                nCreaturesLeft--;

                DropMagic(nSprite);

                MummyList[nMummy].B = 0;
                MummyList[nMummy].nAction = 4;

                sprite[nSprite].xvel = 0;
                sprite[nSprite].yvel = 0;
                sprite[nSprite].zvel = 0;
                sprite[nSprite].z = sector[sprite[nSprite].sectnum].floorz;
            }
            else
            {
                if (!RandomSize(2))
                {
                    MummyList[nMummy].nAction = 7;
                    MummyList[nMummy].B = 0;

                    sprite[nSprite].xvel = 0;
                    sprite[nSprite].yvel = 0;
                }
            }

            return;
        }
    }
}

#ifdef EDUKE32
class MummyLoadSave : public LoadSave
{
public:
    virtual void Load();
    virtual void Save();
};

void MummyLoadSave::Load()
{
    Read(&nMummies, sizeof(nMummies));
    Read(&MummyList, sizeof(MummyList[0]) * nMummies);
}

void MummyLoadSave::Save()
{
    Write(&nMummies, sizeof(nMummies));
    Write(&MummyList, sizeof(MummyList[0]) * nMummies);
}

static MummyLoadSave* myLoadSave;

void MummyLoadSaveConstruct()
{
    myLoadSave = new MummyLoadSave();
}
#endif
