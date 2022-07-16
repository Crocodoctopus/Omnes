#pragma once

#include "cartridge.h"

// Mapper 1
struct Mapper1 {
    struct Cartridge base;

    uint8_t last_prg_bank;
    uint8_t last_chr_bank;
    uint8_t* prg_rom; // CPU: [0x8000..0xFFFF]
    uint8_t* chr_rom; // PPU: [0x0000..0x1FFF]
    uint8_t* sram; // CPU: [0x6000..0x7FFF]

    uint8_t ssr; 
    uint8_t reg0;
    uint8_t reg1;
    uint8_t reg2;
    uint8_t reg3;

    // Cached bank locations.
    uint8_t prg_banks[2];
    uint8_t chr_banks[2];
};

int init_mapper1(struct Mapper1*, uint8_t*, size_t, uint8_t*, size_t, size_t);
void free_mapper1(struct Mapper1*);

uint8_t mapper1_prg_read(struct Mapper1*, uint16_t);
uint8_t mapper1_chr_read(struct Mapper1*, uint16_t);
void mapper1_prg_write(struct Mapper1*, uint16_t, uint8_t);
void mapper1_chr_write(struct Mapper1*, uint16_t, uint8_t);