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
    uint8_t bank = mapper4->prg_banks[addr >> 13 & 0b11];
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
        // 8000
        case 0x8000: 
        mapper4->bank_select = data;
        break;

        // Bank data
        case 0x8001: 
        mapper4->regs[mapper4->bank_select & 0b111] = data;
        break;

        // A000
        case 0xA000: 
        mapper4->base.mirroring = data & 0b1;
        break;

        // A001
        case 0xA001: break;

        // C000
        case 0xC000: 
        nlog("LATCH = %i", data);
        mapper4->irq_latch = data;
        break;

        // C001
        case 0xC001: 
        nlog("IRQ RELOAD $%04X", addr);
        mapper4->irq_reload = 1;
        break;

        // E000
        case 0xE000: 
        nlog("IRQ DISABLE $%04X", addr);
        mapper4->irq_enable = 0;
        mapper4->base.irq = 0;
        break;

        // E001
        case 0xE001: 
        nlog("IRQ ENABLE $%04X", addr);
        mapper4->irq_enable = 1;
        break;
    }

    // update prg cache
    switch (mapper4->bank_select >> 6 & 0b1) {
        case 0:
        mapper4->prg_banks[0] = mapper4->regs[6];
        mapper4->prg_banks[1] = mapper4->regs[7];
        mapper4->prg_banks[2] = mapper4->last_prg_bank - 1;
        break;

        case 1:
        mapper4->prg_banks[0] = mapper4->last_prg_bank - 1;
        mapper4->prg_banks[1] = mapper4->regs[7];
        mapper4->prg_banks[2] = mapper4->regs[6];
        break;
    }

    // update chr cache
    switch (mapper4->bank_select >> 7) {
        case 0:
        mapper4->chr_banks[0] = mapper4->regs[0] & 0xFE;
        mapper4->chr_banks[1] = mapper4->chr_banks[0] + 1;
        mapper4->chr_banks[2] = mapper4->regs[1] & 0xFE;
        mapper4->chr_banks[3] = mapper4->chr_banks[2] + 1;
        mapper4->chr_banks[4] = mapper4->regs[2];
        mapper4->chr_banks[5] = mapper4->regs[3];
        mapper4->chr_banks[6] = mapper4->regs[4];
        mapper4->chr_banks[7] = mapper4->regs[5];
        break;

        case 1:
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
        // IRQ is clocked...
        nlog("IRQ clocked q=%i:  %4x", mapper4->irq_counter, addr);
        
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
        // IRQ is clocked...
        //printf("IRQ clocked q=%i:  %4x\n", mapper4->irq_counter, addr);
        
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
