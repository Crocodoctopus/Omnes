#pragma once

#include "cartridge.h"

struct Mapper3 {
    struct Cartridge base;

    size_t prg_rom_mask;
    uint8_t* prg_rom;
    uint8_t* chr_rom;

    uint8_t chr_bank;
};

int init_mapper3(struct Mapper3*, uint8_t*, size_t, uint8_t*, size_t);
void free_mapper3(struct Mapper3*);

uint8_t mapper3_prg_read(struct Mapper3*, uint16_t);
uint8_t mapper3_chr_read(struct Mapper3*, uint16_t);
void mapper3_prg_write(struct Mapper3*, uint16_t, uint8_t);
void mapper3_chr_write(struct Mapper3*, uint16_t, uint8_t);