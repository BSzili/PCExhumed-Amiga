#!/bin/sh
make USE_POLYMOST=0 USE_OPENGL=0 USE_ASM=0 CC="m68k-amigaos-gcc -noixemul -m68060 -m68881" CXX="m68k-amigaos-g++ -noixemul -m68060 -m68881" AS="vasm -Faout -quiet -phxass -m68060 -DM68060" AR=m68k-amigaos-ar RANLIB=m68k-amigaos-ranlib PLATFORM=AMIGA AMISUFFIX=".060" $*
