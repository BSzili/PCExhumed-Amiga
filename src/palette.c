//
// EDuke32 compatibility
//

#include "palette.h"
#include "baselayer.h"

uint8_t basepaltable[BASEPALCOUNT][768];
int g_lastpalettesum; // TODO lastpalettesum
uint8_t curbasepal;
static int basepaletteset = 0;

void paletteSetColorTable(int32_t id, uint8_t const *table)
{
	Bmemcpy(basepaltable[id], table, 768);
}

void videoSetPalette(char dabrightness, uint8_t dapalid, uint8_t flags)
{
	if (!basepaletteset && dapalid == BASEPAL)
	{
		paletteSetColorTable(BASEPAL, palette);
		basepaletteset = 1;
	}
	curbasepal = dapalid;
	setbrightness(dabrightness, basepaltable[dapalid], flags);
}

int32_t videoUpdatePalette(int32_t start, int32_t num)
{
	setpalette(start, num, NULL);
	return 0;
}

int32_t paletteSetLookupTable(int32_t palnum, const uint8_t *shtab)
{
	makepalookup(palnum, NULL, 0, 0, 0, 0); // just allocate it
	if (shtab != NULL)
		Bmemcpy(palookup[palnum], shtab, numpalookups<<8);

	return 0;
}
