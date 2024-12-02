#pragma once

#include "cartridge.h"

struct Mapper4 {
    struct Cartridge base;

    uint8_t last_prg_bank;
    uint8_t last_chr_bank;
    uint8_t* prg_rom;
    uint8_t* chr_rom;
    uint8_t* sram;

    //
    uint8_t bank_select;
    uint8_t irq_latch;
    uint8_t irq_enable;
    uint8_t irq_reload;
    uint8_t regs[8];

    //
    uint8_t last_a12;
    uint8_t irq_counter;
    
    //
    uint8_t prg_banks[4]; // 0x2000 bytes each
    uint8_t chr_banks[8]; // 0x400 bytes each
};

int init_mapper4(struct Mapper4*, uint8_t*, size_t, uint8_t*, size_t, size_t);
void free_mapper4(struct Mapper4*);

uint8_t mapper4_prg_read(struct Mapper4*, uint16_t);
uint8_t mapper4_chr_read(struct Mapper4*, uint16_t);
void mapper4_prg_write(struct Mapper4*, uint16_t, uint8_t);
void mapper4_chr_write(struct Mapper4*, uint16_t, uint8_t);
