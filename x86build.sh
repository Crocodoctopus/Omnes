#! /bin/bash
mkdir -p bin_x86
gcc -D X86 -o bin_x86/emu \
	-O3 -Wno-unused-result \
	src/main.c src/bus.c src/cartridge.c src/cpu.c src/debug.c src/error.c src/mapper0.c src/mapper1.c src/mapper3.c src/mapper4.c src/nes.c src/ppu.c src/settings.c \
	-lSDL2 -lSDL2main 
