//
// EDuke32 compatibility
//

#include "compat.h"

int32_t G_CheckCmdSwitch(int32_t argc, char const * const * argv, const char *str)
{
    int32_t i;
    for (i=0; i<argc; i++)
    {
        if (str && !Bstrcasecmp(argv[i], str))
            return 1;
    }

    return 0;
}

void fnlist_clearnames(fnlist_t *fnl)
{
    klistfree(fnl->finddirs);
    klistfree(fnl->findfiles);

    fnl->finddirs = fnl->findfiles = NULL;
    fnl->numfiles = fnl->numdirs = 0;
}

// dirflags, fileflags:
//  -1 means "don't get dirs/files",
//  otherwise ORed to flags for respective klistpath
int32_t fnlist_getnames(fnlist_t *fnl, const char *dirname, const char *pattern,
                        int32_t dirflags, int32_t fileflags)
{
    BUILDVFS_FIND_REC *r;

    fnlist_clearnames(fnl);

    if (dirflags != -1)
        fnl->finddirs = klistpath(dirname, "*", BUILDVFS_FIND_DIR|dirflags);
    if (fileflags != -1)
        fnl->findfiles = klistpath(dirname, pattern, BUILDVFS_FIND_FILE|fileflags);

    for (r=fnl->finddirs; r; r=r->next)
        fnl->numdirs++;
    for (r=fnl->findfiles; r; r=r->next)
        fnl->numfiles++;

    return 0;
}

char* g_defNamePtr = NULL;
int CONTROL_BindsEnabled; // TODO remove
//const char* s_buildRev = ""; // TODO remove

#ifndef __AMIGA__
// JFBuild
int nextvoxid = 0;
#endif

#ifdef USE_OPENGL
// TODO
float palookupfogfactor[MAXPALOOKUPS];
int32_t polymostcenterhoriz = 100;
#endif
