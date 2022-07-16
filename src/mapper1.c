#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "cartridge.h"
#include "mapper1.h"
#include "error.h"

int init_mapper1(struct Mapper1* mapper1, uint8_t* prg_rom, size_t prg_rom_size, uint8_t* chr_rom, size_t chr_rom_size, size_t sram) {
    // Validity checking.
    if (prg_rom == NULL) return error(INVALID_INPUT, "Null PRG ROM");
    if (chr_rom == NULL) return error(INVALID_INPUT, "Null CHR ROM");

    mapper1->last_prg_bank = (prg_rom_size / 0x4000) - 1;
    mapper1->last_chr_bank = (chr_rom_size / 0x1000) - 1;

    mapper1->prg_rom = malloc(prg_rom_size);
    memcpy(mapper1->prg_rom, prg_rom, prg_rom_size);
    mapper1->chr_rom = malloc(chr_rom_size);
    memcpy(mapper1->chr_rom, chr_rom, chr_rom_size);
    mapper1->sram = sram ? malloc(0x2000) : NULL;

    mapper1->ssr = 0b10000;
    mapper1->reg0 = 0x00;
    mapper1->reg1 = 0;
    mapper1->reg2 = 0;
    mapper1->reg3 = 0;

    mapper1->prg_banks[0] = 0;
    mapper1->prg_banks[1] = 1;
    mapper1->chr_banks[0] = 0;
    mapper1->chr_banks[1] = 1;

    //
    return 0;
}

void free_mapper1(struct Mapper1* mapper1) {
    free(mapper1->prg_rom);
    free(mapper1->chr_rom);
    free(mapper1->sram);
}

uint8_t mapper1_prg_read(struct Mapper1* mapper1, uint16_t addr) {
    // Range [0x6000, 0x7FFF]: SRAM space.
    if (addr < 0x8000) {
        if (mapper1->sram == NULL) return 0;
        return mapper1->sram[addr - 0x6000];
    }

    // Range [0x8000, 0xFFFF]: PRG rom space.
    uint8_t bank = mapper1->prg_banks[addr >= 0xC000];
    assert(bank <= mapper1->last_prg_bank);
    return mapper1->prg_rom[0x4000 * bank | addr & 0x3FFF];
}

#include <stdio.h>
uint8_t mapper1_chr_read(struct Mapper1* mapper1, uint16_t addr) {
    uint8_t bank = mapper1->chr_banks[addr >= 0x1000];
    assert(bank <= mapper1->last_chr_bank);
    return mapper1->chr_rom[0x1000 * bank | addr & 0x0FFF];
}

void mapper1_prg_write(struct Mapper1* mapper1, uint16_t addr, uint8_t data) {
    #define min(a, b) ((a) < (b) ? (a) : (b));

    // Range [0x6000, 0x7FFF]: SRAM space.
    if (addr < 0x8000) {
        if (mapper1->sram == NULL) return;
        mapper1->sram[addr - 0x6000] = data;
        return;
    }

    // Range [0x8000..0xFFFF]: PRG rom space.
    // ...

    // If the 7th bit is set, reset SSR and reg0
    if (data >= 0x80) {
        mapper1->ssr = 0b10000;
        mapper1->reg0 |= 0x0C;
        return;
    }

    // If the first bit is set, queue a reset.
    uint8_t reset = mapper1->ssr & 0x01;

    // Update ssr.
    mapper1->ssr >>= 1;
    mapper1->ssr |= data << 4;

    if (reset) {
        // The last 5 bits of the address decide which register ssr gets moved to.
        switch (addr & 0xF800) {
            case 0x8000: case 0x9000: mapper1->reg0 = mapper1->ssr; break;
            case 0xA000: case 0xB000: mapper1->reg1 = mapper1->ssr; break;
            case 0xC000: case 0xD000: mapper1->reg2 = mapper1->ssr; break;
            case 0xE000: case 0xF000: mapper1->reg3 = mapper1->ssr; break;
        }

        // Update prg bank cache.
        uint8_t prg_bank_mode = (mapper1->reg0 >> 2) & 0b11;
        uint8_t reg3 = mapper1->reg3 & 0b1111; // Last bit is used elsewhere.
        switch (prg_bank_mode) {
            // 0x8000 mode:
            // First bank maps to reg3, ignoring bit 1.
            // Second bank maps to first bank + 1. 
            case 0: case 1:
            mapper1->prg_banks[0] = min(reg3 & 0xFE, mapper1->last_prg_bank - 1);
            mapper1->prg_banks[1] = mapper1->prg_banks[0] + 1;
            break;

            // First bank maps to first 0x4000 bytes.
            // Second bank maps to reg3.
            case 2:
            mapper1->prg_banks[0] = 0;
            mapper1->prg_banks[1] = min(reg3, mapper1->last_prg_bank);
            break;

            // First bank maps to reg3.
            // Second bank maps to last 0x4000 bytes.
            case 3:
            mapper1->prg_banks[0] = min(reg3, mapper1->last_prg_bank);
            mapper1->prg_banks[1] = mapper1->last_prg_bank;
            break;
        }

        // Update chr bank cache.
        uint8_t chr_bank_mode = mapper1->reg0 >> 4;
        switch (chr_bank_mode) {
            // 0x8000 mode:
            // First bank maps to reg1, ignoring bit 1.
            // Second bank maps to first bank + 1. 
            // Note: reg2 is ignored?
            case 0:
            mapper1->chr_banks[0] = min(mapper1->reg1 & 0xFE, mapper1->last_chr_bank - 1);
            mapper1->chr_banks[1] = mapper1->chr_banks[0] + 1;
            break;

            // First bank maps to reg1.
            // Second bank maps to reg2.
            case 1:
            mapper1->chr_banks[0] = min(mapper1->reg1, mapper1->last_chr_bank);
            mapper1->chr_banks[1] = min(mapper1->reg2, mapper1->last_chr_bank);
            break;
        }

        // Update mirroring.
        mapper1->base.mirroring = (~mapper1->reg0 & 0b1);
      

        // Set ssr back to its default.
        mapper1->ssr = 0b10000;
    }
}

void mapper1_chr_write(struct Mapper1* mapper1, uint16_t addr, uint8_t data) {
    uint8_t bank = mapper1->chr_banks[addr >= 0x1000];
    assert(bank <= mapper1->last_chr_bank);
    mapper1->chr_rom[0x1000 * bank | addr & 0x0FFF] = data;
}