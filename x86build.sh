#! /bin/bash
mkdir -p bin_x86
gcc -D X86 -o bin_x86/emu \
	-O2 -lSDL2 -lSDL2main -Wno-unused-result \
	src/main.c src/bus.c src/cartridge.c src/cpu.c src/debug.c src/error.c src/mapper0.c src/mapper1.c src/mapper3.c src/mapper4.c src/nes.c src/ppu.c src/settings.c
