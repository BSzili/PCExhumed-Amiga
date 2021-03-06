#ifndef PALETTE_H
#define PALETTE_H

// EDuke32 compatibility

#include "common_game.h"

void paletteSetColorTable(int32_t id, uint8_t const *table);
void videoSetPalette(char dabrightness, uint8_t dapalid, uint8_t flags);
int32_t videoUpdatePalette(int32_t start, int32_t num);
int32_t paletteSetLookupTable(int32_t palnum, const uint8_t *shtab);

extern uint8_t basepaltable[BASEPALCOUNT][768];
extern int g_lastpalettesum;
extern uint8_t curbasepal;

#endif
