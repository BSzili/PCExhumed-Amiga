#!/bin/sh
make USE_POLYMOST=0 USE_OPENGL=0 USE_ASM=0 CC="m68k-amigaos-gcc -noixemul -m68040 -m68881 -fbbb=- -fno-peephole2 -ffast-math" CXX="m68k-amigaos-g++ -noixemul -m68040 -m68881 -fbbb=- -fno-peephole2 -ffast-math" AS="vasm -Fhunk -quiet -phxass -m68040" AR=m68k-amigaos-ar RANLIB=m68k-amigaos-ranlib PLATFORM=AMIGA AMISUFFIX=".040" $*
