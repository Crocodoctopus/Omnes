#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "cartridge.h"
#include "mapper4.h"
#include "error.h"

#include <stdio.h>

int init_mapper4(struct Mapper4* mapper4, uint8_t* prg_rom, size_t prg_rom_size, uint8_t* chr_rom, size_t chr_rom_size, size_t sram) {
    if (prg_rom == NULL) return error(INVALID_INPUT, "Null PRG ROM");
    if (chr_rom == NULL) return error(INVALID_INPUT, "Null CHR ROM");

    // allocate
    mapper4->prg_rom = malloc(prg_rom_size);
    mapper4->chr_rom = malloc(chr_rom_size);
    mapper4->sram = sram ? malloc(0x2000) : NULL;

    // 
    memcpy(mapper4->prg_rom, prg_rom, prg_rom_size);
    memcpy(mapper4->chr_rom, chr_rom, chr_rom_size);
    if (mapper4->sram != NULL) memset(mapper4->sram, 0, 0x2000);

    //
    mapper4->last_prg_bank = (prg_rom_size / 0x2000) - 1;
    mapper4->last_chr_bank = (chr_rom_size / 0x400) - 1;

    //
    mapper4->bank_select = 0;
    mapper4->irq_latch = 0;
    mapper4->irq_enable = 0;
    mapper4->irq_reload = 0;
    memset(mapper4->regs, 0, 8);

    //
    mapper4->last_a12 = 0;
    mapper4->irq_counter = 0;   

    //
    memset(mapper4->prg_banks, 0, 4);
    memset(mapper4->chr_banks, 0, 8);
    mapper4->prg_banks[3] = mapper4->last_prg_bank;

    return 0;
}

void free_mapper4(struct Mapper4* mapper4) {
    free(mapper4->prg_rom);
    free(mapper4->chr_rom);
    free(mapper4->sram);
}

uint8_t mapper4_prg_read(struct Mapper4* mapper4, uint16_t addr) {
    // Range [0x6000, 0x7FFF]: SRAM space.
    if (addr < 0x8000) {
        if (mapper4->sram == NULL) return 0;
        return mapper4->sram[addr - 0x6000];
    }

    // Range [0x8000, 0xFFFF]: PRG rom space.
    //uint8_t bank = mapper4->prg_banks[addr >> 13 & 0b11];
    uint8_t bank = mapper4->prg_banks[addr / 0x2000 & 0b11];
    assert(bank <= mapper4->last_prg_bank);
    return mapper4->prg_rom[0x2000 * bank | addr & 0x1FFF];
}

void mapper4_prg_write(struct Mapper4* mapper4, uint16_t addr, uint8_t data) {
    // Range [0x6000, 0x7FFF]: SRAM space.
    if (addr < 0x8000) {
        if (mapper4->sram == NULL) return;
        mapper4->sram[addr - 0x6000] = data;
        return;
    }

    switch (addr & 0b1110000000000001) {
        // bank select
        case 0x8000: 
        //nlog("BANK SELECT = %02X", data);
        mapper4->bank_select = data;
        break;

        // bank data
        case 0x8001: 
        //nlog("BANK DATA = %i", data);
        mapper4->regs[mapper4->bank_select & 0b0111] = data;
        break;

        // mirroring
        case 0xA000: 
        nlog("MIRROR = %i", data);
        mapper4->base.mirroring = !data;
        break;

        // sram protection
        case 0xA001: break;

        // irq latch
        case 0xC000: 
        nlog("LATCH = %i", data);
        mapper4->irq_latch = data;
        break;

        // irq reload
        case 0xC001: 
        nlog("IRQ RELOAD $%04X", addr);
        mapper4->irq_reload = 1;
        break;

        // irq disable
        case 0xE000: 
        nlog("IRQ DISABLE $%04X", addr);
        mapper4->irq_enable = 0;
        mapper4->base.irq = 0;
        break;

        // irq enable
        case 0xE001: 
        nlog("IRQ ENABLE $%04X", addr);
        mapper4->irq_enable = 1;
        break;
    }

    // update prg cache
    switch (mapper4->bank_select & 0b0100000) {
        case 0b00000000:
        mapper4->prg_banks[0] = mapper4->regs[6];
        mapper4->prg_banks[1] = mapper4->regs[7]; // always
        mapper4->prg_banks[2] = mapper4->last_prg_bank - 1;
        //mapper4->prg_banks[3] = mapper4->last_prg_bank;
        break;

        case 0b01000000:
        mapper4->prg_banks[0] = mapper4->last_prg_bank - 1;
        mapper4->prg_banks[1] = mapper4->regs[7]; // always
        mapper4->prg_banks[2] = mapper4->regs[6];
        //mapper4->prg_banks[3] = mapper4->last_prg_bank;
        break;
    }

    // update chr cache
    switch (mapper4->bank_select & 0b10000000) {
        case 0b00000000:
        mapper4->chr_banks[0] = mapper4->regs[0] & 0xFE;
        mapper4->chr_banks[1] = mapper4->chr_banks[0] + 1;
        mapper4->chr_banks[2] = mapper4->regs[1] & 0xFE;
        mapper4->chr_banks[3] = mapper4->chr_banks[2] + 1;
        mapper4->chr_banks[4] = mapper4->regs[2];
        mapper4->chr_banks[5] = mapper4->regs[3];
        mapper4->chr_banks[6] = mapper4->regs[4];
        mapper4->chr_banks[7] = mapper4->regs[5];
        break;

        case 0b10000000:
        mapper4->chr_banks[0] = mapper4->regs[2];
        mapper4->chr_banks[1] = mapper4->regs[3];
        mapper4->chr_banks[2] = mapper4->regs[4];
        mapper4->chr_banks[3] = mapper4->regs[5];
        mapper4->chr_banks[4] = mapper4->regs[0] & 0xFE;
        mapper4->chr_banks[5] = mapper4->chr_banks[4] + 1;
        mapper4->chr_banks[6] = mapper4->regs[1] & 0xFE;
        mapper4->chr_banks[7] = mapper4->chr_banks[6] + 1;
        break;
    }
} 

uint8_t mapper4_chr_read(struct Mapper4* mapper4, uint16_t addr) {
    uint8_t current_a12 = (addr & 0b1 << 12) > 0;
    if  (mapper4->last_a12 == 0 && current_a12 == 1) {
        //nlog("MMC3 IRQ Clocked %i", mapper4->irq_counter);
        if (mapper4->irq_reload || mapper4->irq_counter == 0) {
            mapper4->irq_reload = 0;
            mapper4->irq_counter = mapper4->irq_latch;
        } else {
            mapper4->irq_counter -= 1;
        }
        
        if (mapper4->irq_counter == 0 && mapper4->irq_enable) {
            mapper4->base.irq = 1;
            nlog("IRQ");
        }
    }
    mapper4->last_a12 = current_a12;

    uint8_t bank = mapper4->chr_banks[addr >> 10 & 0b111];
    assert(bank <= mapper4->last_chr_bank);
    return mapper4->chr_rom[0x400 * bank | addr & 0x3FF];
}

void mapper4_chr_write(struct Mapper4* mapper4, uint16_t addr, uint8_t data) {
    uint8_t current_a12 = (addr & 0b1 << 12) > 0;
    if  (mapper4->last_a12 == 0 && current_a12 == 1) {
        //nlog("MMC3 IRQ Clocked %i", mapper4->irq_counter);
        if (mapper4->irq_reload || mapper4->irq_counter == 0) {
            mapper4->irq_reload = 0;
            mapper4->irq_counter = mapper4->irq_latch;
        } else {
            mapper4->irq_counter -= 1;
        }
        
        if (mapper4->irq_counter == 0 && mapper4->irq_enable) {
            mapper4->base.irq = 1;
            nlog("IRQ");
        }
    }
    mapper4->last_a12 = current_a12;

    uint8_t bank = mapper4->chr_banks[addr >> 10 & 0b111];
    assert(bank <= mapper4->last_chr_bank);
    mapper4->chr_rom[0x400 * bank | addr & 0x3FF] = data;
}
