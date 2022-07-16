#pragma once

#include <stdint.h>
#include <stddef.h>

/* Cartridge
    Models an NES cartridge. Since cartridges can vary in size and layout, polymorphism is required, via vtables.
*/
struct Cartridge {
    uint8_t mapper;
    uint8_t mirroring : 1; // bit 0 == mirroring; 0 = horizontal, 1 = vertical
    uint8_t irq : 1;
    uint8_t data[0];
};

// Cartridge initialization/free functions.
int load_cartridge_from_file(char*, struct Cartridge**);
int load_cartridge_from_data(uint8_t*, uint8_t*, struct Cartridge**);
void free_cartridge(struct Cartridge*);

// Cartridge IO. prg_read and prg_write are used by the CPU, chr_read is used by the PPU.
uint8_t cartridge_prg_read(struct Cartridge*, uint16_t);
uint8_t cartridge_chr_read(struct Cartridge*, uint16_t);
void cartridge_prg_write(struct Cartridge*, uint16_t, uint8_t);
void cartridge_chr_write(struct Cartridge*, uint16_t, uint8_t);

// Other
uint8_t cartridge_get_mirroring(struct Cartridge*);
