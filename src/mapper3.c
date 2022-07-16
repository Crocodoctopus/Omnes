#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "cartridge.h"
#include "mapper3.h"
#include "error.h"

int init_mapper3(struct Mapper3* mapper3, uint8_t* prg_rom, size_t prg_rom_size, uint8_t* chr_rom, size_t chr_rom_size) {
    // validity checking.
    if (prg_rom == NULL) return error(INVALID_INPUT, "Null PRG ROM");
    if (chr_rom == NULL) return error(INVALID_INPUT, "Null CHR ROM");
    if (prg_rom_size != 0x4000 && prg_rom_size != 0x8000) return error(ROM_FORMAT_ERROR, "PRG ROM must be 0x4000 or 0x8000 bytes (value was 0x%04X)", prg_rom_size);
    if (chr_rom_size != 0x2000) return error(ROM_FORMAT_ERROR, "CHR ROM must be 0x2000 bytes (value was 0x%04X)", chr_rom_size);

    // Initialize mapper 1.
    mapper3->prg_rom_mask = prg_rom_size - 1;
    mapper3->prg_rom = malloc(prg_rom_size);
    memcpy(mapper3->prg_rom, prg_rom, prg_rom_size);
    mapper3->chr_rom = malloc(chr_rom_size);
    memcpy(mapper3->chr_rom, chr_rom, chr_rom_size);

    mapper3->chr_bank = 0;

    //
    return 0;
}

void free_mapper3(struct Mapper3* mapper3) {
    free(mapper3->prg_rom);
    free(mapper3->chr_rom);
}

uint8_t mapper3_prg_read(struct Mapper3* mapper3, uint16_t addr) {
    // While the correct formula is '(addr - 0x8000) & mapper3->prg_rom_mask', the rom mask is always 0x4000 or 0x8000.
    return mapper3->prg_rom[addr & mapper3->prg_rom_mask];
}

uint8_t mapper3_chr_read(struct Mapper3* mapper3, uint16_t addr) {
    uint8_t bank = mapper3->chr_bank;
    return mapper3->chr_rom[0x2000 * bank | addr];
}

void mapper3_prg_write(struct Mapper3* mapper3, uint16_t addr, uint8_t data) {
    mapper3->chr_bank = data & 0b11;
    // Intentionally blank, mapper3 has no write support.
}

void mapper3_chr_write(struct Mapper3* mapper3, uint16_t addr, uint8_t data) {
    uint8_t bank = mapper3->chr_bank;
    mapper3->chr_rom[0x2000 * bank | addr] = data;
}