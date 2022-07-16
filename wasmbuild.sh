#! /bin/bash
mkdir -p bin_wasm
clang -D WASM32 -o bin_wasm/emu.wasm \
	--target=wasm32 -isystem "stdlib" -nostdlib -O2 -fno-builtin -fvisibility=hidden -Wl,--strip-all,--no-entry,--export-dynamic -Wl,-allow-undefined-file=wasm.syms \
	stdlib/*.c src/bus.c src/cartridge.c src/cpu.c src/debug.c src/error.c src/wasm.c src/mapper0.c src/mapper1.c src/mapper3.c src/mapper4.c src/nes.c src/ppu.c