#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "cartridge.h"
#include "mapper0.h"
#include "error.h"

int init_mapper0(struct Mapper0* mapper0, uint8_t* prg_rom, size_t prg_rom_size, uint8_t* chr_rom, size_t chr_rom_size) {
    // validity checking.
    if (prg_rom == NULL) return error(INVALID_INPUT, "Null PRG ROM");
    if (chr_rom == NULL) return error(INVALID_INPUT, "Null CHR ROM");
    if (prg_rom_size != 0x4000 && prg_rom_size != 0x8000) return error(ROM_FORMAT_ERROR, "PRG ROM must be 0x4000 or 0x8000 bytes (value was 0x%04X)", prg_rom_size);
    if (chr_rom_size != 0x2000) return error(ROM_FORMAT_ERROR, "CHR ROM must be 0x2000 bytes (value was 0x%04X)", chr_rom_size);

    // Initialize mapper 1.
    mapper0->prg_rom_mask = prg_rom_size - 1;
    mapper0->prg_rom = malloc(prg_rom_size);
    memcpy(mapper0->prg_rom, prg_rom, prg_rom_size);
    mapper0->chr_rom = malloc(chr_rom_size);
    memcpy(mapper0->chr_rom, chr_rom, chr_rom_size);

    //
    return 0;
}

void free_mapper0(struct Mapper0* mapper0) {
    free(mapper0->prg_rom);
    free(mapper0->chr_rom);
}

uint8_t mapper0_prg_read(struct Mapper0* mapper0, uint16_t addr) {
    // While the correct formula is '(addr - 0x8000) & mapper0->prg_rom_mask', the rom mask is always 0x4000 or 0x8000.
    return mapper0->prg_rom[addr & mapper0->prg_rom_mask];
}

uint8_t mapper0_chr_read(struct Mapper0* mapper0, uint16_t addr) {
    return mapper0->chr_rom[addr];
}

void mapper0_prg_write(struct Mapper0* mapper0, uint16_t addr, uint8_t data) {
    // Intentionally blank, mapper0 has no write support.
}

void mapper0_chr_write(struct Mapper0* mapper0, uint16_t addr, uint8_t data) {
    //mapper0->chr_rom[addr] = data;
}