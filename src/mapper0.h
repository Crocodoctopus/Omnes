#pragma once

#include "cartridge.h"

// Mapper 0
// Does no real mapping. IO routes to fixed banks. 
// PRG ROM sizes can be 0x4000 or 0x8000 bytes, cpu address 0x8000 routes to 0x0000. 
// CHR ROM sized is always 0x2000.
struct Mapper0 {
    struct Cartridge base;
    size_t prg_rom_mask;
    uint8_t* prg_rom;
    uint8_t* chr_rom;
};

int init_mapper0(struct Mapper0*, uint8_t*, size_t, uint8_t*, size_t);
void free_mapper0(struct Mapper0*);

uint8_t mapper0_prg_read(struct Mapper0*, uint16_t);
uint8_t mapper0_chr_read(struct Mapper0*, uint16_t);
void mapper0_prg_write(struct Mapper0*, uint16_t, uint8_t);
void mapper0_chr_write(struct Mapper0*, uint16_t, uint8_t);