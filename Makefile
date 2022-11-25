# Exhumed Makefile for GNU Make

# Create Makefile.user yourself to provide your own overrides
# for configurable values
-include Makefile.user

##
##
## CONFIGURABLE OPTIONS
##
##

# Debugging options
RELEASE ?= 1

# Engine source code path
EROOT ?= jfbuild

# JMACT library source path
MACTROOT ?= jfmact

# JFAudioLib source path
AUDIOLIBROOT ?= jfaudiolib

# Engine options
#  USE_POLYMOST   - enables Polymost renderer
#  USE_OPENGL     - enables OpenGL support in Polymost
#     Define as 0 to disable OpenGL
#     Define as USE_GL2 (or 1 or 2) for GL 2.0/2.1 profile
#     Define as USE_GLES2 (or 12) for GLES 2.0 profile
#  USE_ASM        - enables the use of assembly code
USE_POLYMOST ?= 0
USE_OPENGL ?= 0
USE_ASM ?= 0


##
##
## HERE BE DRAGONS
##
##

# build locations
SRC=src
RSRC=rsrc
EINC=$(EROOT)/include
ELIB=$(EROOT)
INC=$(SRC)
o=o
res=o

ifneq (0,$(RELEASE))
  # debugging disabled
  debug=-fomit-frame-pointer -O2 -DNDEBUG
else
  # debugging enabled
  debug=-ggdb -Og -D_DEBUG #-Werror
endif

include $(AUDIOLIBROOT)/Makefile.shared

CC?=gcc
CXX?=g++
NASM?=nasm
RC?=windres
OURCFLAGS=$(debug) -W -Wall -Wimplicit -Wno-unused \
	-fno-strict-aliasing -DNO_GCC_BUILTINS -std=gnu99 \
	-I$(INC) -I$(EINC) -I$(MACTROOT) -I$(AUDIOLIBROOT)/include
OURCXXFLAGS=-fno-exceptions -fno-rtti
LIBS=-lm
GAMELIBS=
NASMFLAGS=-s #-g
EXESUFFIX=

JMACTOBJ=$(MACTROOT)/util_lib.$o \
	$(MACTROOT)/file_lib.$o \
	$(MACTROOT)/control.$o \
	$(MACTROOT)/keyboard.$o \
	$(MACTROOT)/mouse.$o \
	$(MACTROOT)/scriplib.$o

GAMEOBJS= \
	$(SRC)/anims.o \
	$(SRC)/anubis.o \
	$(SRC)/bubbles.o \
	$(SRC)/bullet.o \
	$(SRC)/cd.o \
	$(SRC)/common.o \
	$(SRC)/compat.o \
	$(SRC)/config.o \
	$(SRC)/enginesubs.o \
	$(SRC)/exhumed.o \
	$(SRC)/fish.o \
	$(SRC)/grenade.o \
	$(SRC)/gun.o \
	$(SRC)/grpscan.o \
	$(SRC)/init.o \
	$(SRC)/input.o \
	$(SRC)/items.o \
	$(SRC)/lavadude.o \
	$(SRC)/light.o \
	$(SRC)/lighting.o \
	$(SRC)/lion.o \
	$(SRC)/map.o \
	$(SRC)/menu.o \
	$(SRC)/move.o \
	$(SRC)/movie.o \
	$(SRC)/mummy.o \
	$(SRC)/network.o \
	$(SRC)/object.o \
	$(SRC)/osdcmds.o \
	$(SRC)/player.o \
	$(SRC)/queen.o \
	$(SRC)/ra.o \
	$(SRC)/ramses.o \
	$(SRC)/random.o \
	$(SRC)/rat.o \
	$(SRC)/rex.o \
	$(SRC)/roach.o \
	$(SRC)/runlist.o \
	$(SRC)/scorp.o \
	$(SRC)/sequence.o \
	$(SRC)/serial.o \
	$(SRC)/set.o \
	$(SRC)/sound.o \
	$(SRC)/snake.o \
	$(SRC)/spider.o \
	$(SRC)/status.o \
	$(SRC)/switch.o \
	$(SRC)/timer.o \
	$(SRC)/trigdat.o \
	$(SRC)/version.o \
	$(SRC)/version-auto.o \
	$(SRC)/view.o \
	$(SRC)/wasp.o \
	$(SRC)/palette.o \
	$(JMACTOBJ)

include $(EROOT)/Makefile.shared

ifeq ($(PLATFORM),LINUX)
	NASMFLAGS+= -f elf
	GAMELIBS+= $(JFAUDIOLIB_LDFLAGS)
endif
ifeq ($(PLATFORM),BSD)
	NASMFLAGS+= -f elf
	GAMELIBS+= $(JFAUDIOLIB_LDFLAGS) -pthread
endif
ifeq ($(PLATFORM),WINDOWS)
	OURCFLAGS+= -I$(DXROOT)/include
	NASMFLAGS+= -f win32 --prefix _
	GAMEOBJS+= $(RSRC)/gameres.$(res) $(SRC)/winbits.$o $(SRC)/startwin.game.$o
	GAMELIBS+= -ldsound \
	       $(AUDIOLIBROOT)/third-party/mingw32/lib/libvorbisfile.a \
	       $(AUDIOLIBROOT)/third-party/mingw32/lib/libvorbis.a \
	       $(AUDIOLIBROOT)/third-party/mingw32/lib/libogg.a
endif

ifeq ($(RENDERTYPE),SDL)
	OURCFLAGS+= $(SDLCONFIG_CFLAGS)
	LIBS+= $(SDLCONFIG_LIBS)

	ifeq (1,$(HAVE_GTK))
		OURCFLAGS+= $(GTKCONFIG_CFLAGS)
		LIBS+= $(GTKCONFIG_LIBS)
		GAMEOBJS+= $(SRC)/startgtk.game.$o $(RSRC)/startgtk_game_gresource.$o
	endif

	GAMEOBJS+= $(RSRC)/sdlappicon_game.$o
endif

# Source-control version stamping
GAMEOBJS+= $(SRC)/version-auto.$o
GAMEOBJS+= $(SRC)/version.$o

OURCFLAGS+= $(BUILDCFLAGS)
LIBS+= $(BUILDLIBS)

.PHONY: clean all engine $(ELIB)/$(ENGINELIB) $(AUDIOLIBROOT)/$(JFAUDIOLIB)

# TARGETS

all: pcexhumed$(EXESUFFIX)

pcexhumed$(EXESUFFIX): $(GAMEOBJS) $(ELIB)/$(ENGINELIB) $(AUDIOLIBROOT)/$(JFAUDIOLIB)
	$(CXX) $(CXXFLAGS) $(OURCXXFLAGS) $(OURCFLAGS) -o $@ $^ $(LIBS) $(GAMELIBS) -Wl,-Map=$@.map

#include Makefile.deps

.PHONY: enginelib
enginelib:
	$(MAKE) -C $(EROOT) \
		USE_POLYMOST=$(USE_POLYMOST) \
		USE_OPENGL=$(USE_OPENGL) \
		USE_ASM=$(USE_ASM) \
		RELEASE=$(RELEASE) AMISUFFIX=$(AMISUFFIX) CFLAGS="-DENGINE_19950829" $@
$(EROOT)/generatesdlappicon$(EXESUFFIX):
	$(MAKE) -C $(EROOT) generatesdlappicon$(EXESUFFIX)

$(ELIB)/$(ENGINELIB): enginelib
$(AUDIOLIBROOT)/$(JFAUDIOLIB):
	$(MAKE) -C $(AUDIOLIBROOT) RELEASE=$(RELEASE)

# RULES
$(SRC)/%.$o: $(SRC)/%.nasm
	$(NASM) $(NASMFLAGS) $< -o $@

$(SRC)/%.$o: $(SRC)/%.c
	$(CC) $(CFLAGS) $(OURCFLAGS) -c $< -o $@
$(SRC)/%.$o: $(SRC)/%.cpp
	$(CXX) $(CXXFLAGS) $(OURCXXFLAGS) $(OURCFLAGS) -c $< -o $@
$(MACTROOT)/%.$o: $(MACTROOT)/%.c
	$(CC) $(CFLAGS) $(OURCFLAGS) -c $< -o $@

$(RSRC)/%.$(res): $(RSRC)/%.rc
	$(RC) -i $< -o $@ --include-dir=$(EINC) --include-dir=$(SRC)

$(RSRC)/%.$o: $(RSRC)/%.c
	$(CC) $(CFLAGS) $(OURCFLAGS) -c $< -o $@

$(RSRC)/%_gresource.c: $(RSRC)/%.gresource.xml
	glib-compile-resources --generate --manual-register --c-name=startgtk --target=$@ --sourcedir=$(RSRC) $<
$(RSRC)/%_gresource.h: $(RSRC)/%.gresource.xml
	glib-compile-resources --generate --manual-register --c-name=startgtk --target=$@ --sourcedir=$(RSRC) $<
$(RSRC)/sdlappicon_%.c: $(RSRC)/%.png $(EROOT)/generatesdlappicon$(EXESUFFIX)
	$(EROOT)/generatesdlappicon$(EXESUFFIX) $< > $@

# PHONIES
clean:
	-rm -f $(GAMEOBJS)
	$(MAKE) -C $(EROOT) clean
	$(MAKE) -C $(AUDIOLIBROOT) clean

veryclean: clean
	-rm -f pcexhumed$(EXESUFFIX) core*
	$(MAKE) -C $(EROOT) veryclean

.PHONY: $(SRC)/version-auto.c
$(SRC)/version-auto.c:
	printf "const char *s_buildRev = \"%s\";\n" "$(shell git describe --always || echo git error)" > $@
	echo "const char *s_buildTimestamp = __DATE__ \" \" __TIME__;" >> $@
