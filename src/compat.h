#ifndef EDUKE32_COMPAT_H
#define EDUKE32_COMPAT_H

//
// EDuke32 compatibility
//

#include_next "compat.h" // include the actual build/include/compat.h
//#include "baselayer.h"
#include "build.h"
#include "cache1d.h"
#include "pragmas.h"
#include "osd.h"
#include "vfs.h"
#include "types.h"
#include "keyboard.h"
#include "control.h"
#include "assert.h"
#include "fx_man.h"
#include "scriptfile.h"
#ifdef RENDERTYPEWIN
#include "winlayer.h"
#define Paths_ReadRegistryValue(a,b,c,d) (NULL)
#endif
#include "palette.h"

#include <math.h>
#include <stdlib.h>

//#include "mutex.h"

#if !USE_OPENGL
#undef USE_OPENGL
#endif
#ifdef _WIN32
#define MIXERTYPEWIN
#endif

#define initprintf buildprintf
#define ERRprintf buildprintf

typedef struct
{
	int32_t x, y;
} vec2_t; // TODO mapwinxy1, mapwinxy2

typedef struct
{
	int32_t x, y, z;
} vec3_t; // TODO remove opos

#define UNREFERENCED_PARAMETER(...)
#define UNREFERENCED_CONST_PARAMETER(...)
#ifdef __GNUC__
#define fallthrough__ __attribute__ ((fallthrough))
#else
#define fallthrough__
#endif
#define EDUKE32_FUNCTION __FUNCTION__
#define ARRAY_SIZE(arr) (sizeof((arr)) / sizeof((arr)[0]))
#define ARRAY_SSIZE(arr) (int)ARRAY_SIZE(arr)
#define EDUKE32_STATIC_ASSERT(...)


typedef int32_t fix16_t;
#define fix16_from_int(a) ((a))
#define fix16_to_int(a) ((a))
#define fix16_to_float(a) ((float)(a))
#define fix16_clamp(x, lo, hi) clamp((x), (lo), (hi))
#define F16(x) ((x))


#define Xaligned_alloc(x,y) malloc((y))
#define Xaligned_free free
#define ALIGNED_FREE_AND_NULL(x) do { Xaligned_free((x)); (x) = NULL; } while(0)
#define Xcalloc calloc
#define Xfree free
#define Xstrdup strdup
#define Xmalloc malloc

#define Bfstat fstat
#define Bexit exit
#define Bfflush fflush
//#define Blrintf(x) ((int32_t)lrintf((x)))


void gltexinvalidate (long dapicnum, long dapalnum, long dameth);
#if USE_POLYMOST && USE_OPENGL
#define tileInvalidate invalidatetile
#else
#define tileInvalidate(x,y,z)
#endif



#define ClockTicks long
//#define timerInit inittimer
//#define timerSetCallback installusertimercallback
#define timerInit(tickspersecond) { int tps = (tickspersecond);
#define timerSetCallback(callback) inittimer(tps, (callback)); }
#define timerGetTicks getticks
#define timerGetPerformanceCounter getusecticks
#define timerGetPerformanceFrequency gettimerfreq
static inline int32_t BGetTime(void) { return (int32_t)totalclock; }

enum rendmode_t
{
	REND_CLASSIC,
	REND_POLYMOST = 3,
};

#if USE_POLYMOST
#define videoGetRenderMode() ((enum rendmode_t)getrendermode())
#else
#define videoGetRenderMode() REND_CLASSIC
#endif
#define videoNextPage nextpage
#define renderDrawRoomsQ16(daposx, daposy, daposz, daang, dahoriz, dacursectnum) drawrooms((daposx), (daposy), (daposz), fix16_to_int((daang)), fix16_to_int((dahoriz)), (dacursectnum))
#define renderDrawMasks drawmasks
#define videoCaptureScreen(x,y) screencapture((char *)(x),(y))
#define videoCaptureScreenTGA(x,y) screencapture((char *)(x),(y))
#define engineFPSLimit() 1
#define enginePreInit preinitengine
#define PrintBuildInfo() // already part of preinitengine
#define windowsCheckAlreadyRunning() 1
#define enginePostInit() 0 // TODO
//#define engineLoadBoard loadboard
//#define engineLoadBoardV5V6 loadoldboard
#define artLoadFiles loadpics
#define engineUnInit uninitengine
#define engineInit initengine
#define tileCreate allocatepermanenttile
//#define tileLoad(tilenume) do { if (!waloff[(tilenume)]) loadtile((tilenume)); } while(0)
#define tileLoad loadtile

// common.h
void G_AddGroup(const char *buffer);
void G_AddPath(const char *buffer);
void G_AddDef(const char *buffer);
int32_t G_CheckCmdSwitch(int32_t argc, char const * const * argv, const char *str);


// palette

#if USE_POLYMOST && USE_OPENGL
static inline void hicsetpalettetint_eduke32(int32_t palnum, char r, char g, char b, char sr, char sg, char sb, long effect)
{
	hicsetpalettetint(palnum, r, g, b, effect);
}
#define hicsetpalettetint hicsetpalettetint_eduke32
extern palette_t palookupfog[MAXPALOOKUPS];
extern float palookupfogfactor[MAXPALOOKUPS];
#define videoTintBlood(r,g,b) // TODO
//extern char nofog;
// polymost_setFogEnabled
#define fullscreen_tint_gl(a,b,c,d) // TODO
#define forcegl (0)
extern int32_t polymostcenterhoriz; // TODO !!!
#endif
#define renderEnableFog() // TODO
#define renderDisableFog() // TODO
#define numshades numpalookups

extern char pow2char[8];
#define renderDrawLine drawline256
#define renderSetAspect setaspect
typedef walltype *uwallptr_t;
#define tspritetype spritetype
typedef spritetype *tspriteptr_t;
#define videoSetCorrectedAspect() // newaspect not needed
#define renderDrawMapView drawmapview
#define videoClearViewableArea clearview
#define videoSetGameMode(davidoption, daupscaledxdim, daupscaledydim, dabpp, daupscalefactor) setgamemode(davidoption, daupscaledxdim, daupscaledydim, dabpp)
#define videoSetViewableArea setview
#define videoClearScreen clearallviews
#define mouseLockToWindow(a) grabmouse((a)-2)
#define videoResetMode resetvideomode
//extern int whitecol, blackcol; // TODO don't hardcode these
#define whitecol 4
#define blackcol 96
#define kopen4loadfrommod kopen4load
#define g_visibility visibility

#define CSTAT_SPRITE_BLOCK (1)
#define CSTAT_SPRITE_TRANSLUCENT (2)
#define CSTAT_SPRITE_INVISIBLE (0x8000)

#define clipmove_old clipmove
#define getzrange_old getzrange
#define pushmove_old pushmove


// controls

#ifndef MAXMOUSEBUTTONS // TODO
#define MAXMOUSEBUTTONS 6
#endif
#define CONTROL_ProcessBinds()
#define CONTROL_ClearAllBinds()
extern int CONTROL_BindsEnabled; // TODO
#define KB_UnBoundKeyPressed KB_KeyPressed


// math

//#define fPI 3.14159265358979323846f

static inline int32_t clamp(int32_t in, int32_t min, int32_t max) { return in <= min ? min : (in >= max ? max : in); }
#ifndef min
# define min(a, b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef max
# define max(a, b) (((a) > (b)) ? (a) : (b))
#endif

//#define tabledivide32_noinline(n, d) ((n)/(d))
//#define tabledivide64(n, d) ((n)/(d))


// defs.c
extern const char *G_DefaultDefFile(void);
extern const char *G_DefFile(void);
extern char *g_defNamePtr;
enum {
	T_EOF = -2,
	T_ERROR = -1,
};
typedef struct { char *text; int tokenid; } tokenlist;
static int getatoken(scriptfile *sf, tokenlist *tl, int ntokens)
{
	char *tok;
	int i;

	if (!sf) return T_ERROR;
	tok = scriptfile_gettoken(sf);
	if (!tok) return T_EOF;

	for(i=0;i<ntokens;i++) {
		if (!Bstrcasecmp(tok, tl[i].text))
			return tl[i].tokenid;
	}

	return T_ERROR;
}


#ifndef __cplusplus
#define nullptr NULL
#define constexpr const
#endif

static inline char *dup_filename(const char *fn)
{
    char * const buf = (char *) Xmalloc(BMAX_PATH);
    return Bstrncpy(buf, fn, BMAX_PATH);
}

// ODS stuff

typedef const osdfuncparm_t *osdcmdptr_t;
#define system_getcvars() // TODO 
#define OSD_SetLogFile buildsetlogfile

// fnlist

struct strllist
{
    struct strllist *next;
    char *str;
};

typedef struct
{
    BUILDVFS_FIND_REC *finddirs, *findfiles;
    int32_t numdirs, numfiles;
}
fnlist_t;

#define FNLIST_INITIALIZER { NULL, NULL, 0, 0 }

void fnlist_clearnames(fnlist_t *fnl);
int32_t fnlist_getnames(fnlist_t *fnl, const char *dirname, const char *pattern, int32_t dirflags, int32_t fileflags);

extern const char *s_buildRev;

#define FX_Play(ptr, ptrlength, loopstart, loopend, pitchoffset, vol, left, right, priority, volume, callbackval) \
	FX_PlayLoopedAuto(ptr, ptrlength, loopstart, loopend, pitchoffset, vol, left, right, priority, callbackval)

static inline int FX_SoundValidAndActive(int handle) { return handle > 0 && FX_SoundActive(handle); } 

//#define rotatesprite_(sx, sy, z, a, picnum, dashade, dapalnum, dastat, daalpha, dablend, cx1, cy1, cx2, cy2) rotatesprite((sx), (sy), (z), (a), (picnum), (dashade), (dapalnum), (dastat)&0xFF, (cx1), (cy1), (cx2), (cy2))
#define rotatesprite_win(sx, sy, z, a, picnum, dashade, dapalnum, dastat) rotatesprite((sx), (sy), (z), (a), (picnum), (dashade), (dapalnum), (dastat), windowx1, windowy1, windowx2, windowy2)

#define wrand rand

#ifdef __GNUC__
# define UNUSED(x) UNUSED_ ## x __attribute__((unused))
#else
# define UNUSED(x) x
#endif

#endif
