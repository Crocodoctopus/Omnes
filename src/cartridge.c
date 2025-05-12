#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "mapper0.h"
#include "mapper1.h"
#include "mapper3.h"
#include "mapper4.h"
#include "error.h"

uint16_t make_u16(uint8_t hi, uint8_t lo);

#ifdef X86
#include <stdio.h>
/* load_cartridge_from_file
Loads an NES cartridge from file based on a specified filename.
Param 1 - The null terminated file name
Param 2 - Output; Cartridge pointer to push output to
*/
int load_cartridge_from_file(char* filename, struct Cartridge** cartridge) {
    int err = 0;
    uint8_t header[16] = { 0 };
    uint8_t* data = NULL;
    FILE* fp = NULL;

    // validate filename and cartridge
    if (filename == NULL) return error(INVALID_INPUT, "Null filename");
    if (cartridge == NULL) return error(INVALID_INPUT, "Null cartridge output");

    // open filename as readonly binary
    fp = fopen(filename, "rb");
    if (fp == NULL) {
        err = error(IO_ERROR, "Could not open file \"%.32s\"", filename);
        goto close;
    }

    // get file size
    fseek(fp, 0, SEEK_END);
    int size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    // alloc rom (subtract 16 bytes for header, which is not part of the rom)
    data = malloc(size - 16);
    if (data == 0) {
        err = error(ALLOC_ERROR, "Could not allocate %i bytes", size - 16);
        goto close;
    }

    // read first 16 bytes as header, and the rest as rom
    fread(header, 1, 16, fp);
    fread(data, 1, size - 16, fp);

    // log file load success
    nlog("ROM file successfully loaded\n  Filename: %.32s\n  size: %i", filename, size - 16);

    // init cartridge from data parts
    err = load_cartridge_from_data(header, data, cartridge);
    if (err > 0) goto close; // yes, I know, this line is a no-op

    // close all
close:
    free(data);
    if (fp != NULL) fclose(fp); // C is a great language
    return err;
}
#endif

/* load_cartridge_from_data
Loads an NES cartridge from a header and ROM data.
Param 1 - The 16 byte NES header
Param 2 - ROM data. PRG ROM data first, followed by CHR ROM data.
Param 3 - Output; Cartridge pointer to push output to
*/
int load_cartridge_from_data(uint8_t header[16], uint8_t* data, struct Cartridge** cartridge) {
    if (data == NULL) return error(INVALID_INPUT, "Null data");
    if (cartridge == NULL) return error(INVALID_INPUT, "Null cartridge output");
    
    // verify iNES 1.0
    uint8_t ines1 = header[0] == 'N' && header[1] == 'E' && header[2] == 'S' && header[3] == 0x1A;
    if (!ines1) return error(ROM_FORMAT_ERROR, "INES 1.0 header not found");

    // verify iNES 2.0 (note: iNES 2.0 currently unsupported, fail if detected)
    uint8_t ines2 = (header[7] & 0b1100) == 0b1000;
    if (ines2) return error(ROM_FORMAT_ERROR, "INES 2.0 header found, but not supported");

    // extract data from header, assuming ines 1.0
    uint32_t prg_rom_size = header[4] * 0x4000; // prg rom size (in units of 0x4000 bytes)
    uint32_t chr_rom_size = header[5] * 0x2000; // chr rom size (in units of 0x2000 bytes)
    if (chr_rom_size == 0) chr_rom_size = 0x2000; // chr ram hack
    uint8_t mapper = (header[7] & 0x11110000) | (header[6] >> 4); // mapper byte ID
    uint8_t sram = 1; // (header[6] & 0b10) > 0;
    uint8_t mirroring = header[6] & 0b1; // ciram mirroring mode

    // log rom interpretation success
    nlog("ROM parsed\n  Header info:\n    mapper: %i\n    prg size: 0x%X\n    chr size: 0x%X\n    mirroring: %i (0 = horizontal, 1 = vertical)\n    SRAM: %i", 
        mapper, prg_rom_size, chr_rom_size, mirroring, sram);

    // create mapper (for now, only mapper0 is supported)
    uint8_t* prg_rom = data;
    uint8_t* chr_rom = data + prg_rom_size;
    int err = 0;
    *cartridge = NULL;
    switch (mapper) {
        case 0: {
            struct Mapper0* mapper0 = malloc(sizeof(struct Mapper0));
            *cartridge = (void*)mapper0;
            (*cartridge)->mapper = 0;
            (*cartridge)->mirroring = mirroring;
            (*cartridge)->irq = 0;
            err = init_mapper0(mapper0, prg_rom, prg_rom_size, chr_rom, chr_rom_size);
        } break;

        case 1: {
            struct Mapper1* mapper1 = malloc(sizeof(struct Mapper1));
            *cartridge = (void*)mapper1;
            (*cartridge)->mapper = 1;
            (*cartridge)->mirroring = mirroring;
            (*cartridge)->irq = 0;
            err = init_mapper1(mapper1, prg_rom, prg_rom_size, chr_rom, chr_rom_size, sram);
        } break;

        case 3: {
            struct Mapper3* mapper3 = malloc(sizeof(struct Mapper3));
            *cartridge = (void*)mapper3;
            (*cartridge)->mapper = 3;
            (*cartridge)->mirroring = mirroring;
            (*cartridge)->irq = 0;
            err = init_mapper3(mapper3, prg_rom, prg_rom_size, chr_rom, chr_rom_size);
        } break;

        case 4: {
            struct Mapper4* mapper4 = malloc(sizeof(struct Mapper4));
            *cartridge = (void*)mapper4;
            (*cartridge)->mapper = 4;
            (*cartridge)->mirroring = mirroring;
            (*cartridge)->irq = 0;
            err = init_mapper4(mapper4, prg_rom, prg_rom_size, chr_rom, chr_rom_size, sram);
        } break;

        default: return error(ROM_FORMAT_ERROR, "Unsupported mapper (value was %i)", mapper);
    }

    // forward error
    if (err) {
        free(*cartridge); // clean any allocation that may have happened
        return err;
    }

    // for diagnostic purposes, read and print the interrupt vectors
    uint16_t nmi_addr = 0xFFFA;
    uint16_t reset_addr = 0xFFFC;
    uint16_t irq_addr = 0xFFFE;
    uint16_t nmi, reset, irq;
    uint8_t hi, lo;
    lo = cartridge_prg_read(*cartridge, nmi_addr);
    hi = cartridge_prg_read(*cartridge, nmi_addr + 1);
    nmi = make_u16(hi, lo);
    lo = cartridge_prg_read(*cartridge, reset_addr);
    hi = cartridge_prg_read(*cartridge, reset_addr + 1);
    reset = make_u16(hi, lo);
    lo = cartridge_prg_read(*cartridge, irq_addr);
    hi = cartridge_prg_read(*cartridge, irq_addr + 1);
    irq = make_u16(hi, lo);
    nlog("Interrupt vectors:\n  Vectors:\n    nmi: 0x%04X\n    reset: 0x%04X\n    irq: 0x%04X", nmi, reset, irq);

    // "return" cartridge
    return err;
}

// VTABLE stuff
#define PRG_READ 0
#define CHR_READ 1
#define PRG_WRITE 2
#define CHR_WRITE 3
#define FREE 4
#define VTABLE_SIZE 5

typedef uint8_t(prg_read_func)(void*, uint16_t);
typedef uint8_t(chr_read_func)(void*, uint16_t);
typedef void(prg_write_func)(void*, uint16_t, uint8_t);
typedef void(chr_write_func)(void*, uint16_t, uint8_t);
typedef void(free_func)(void*);

void* vtable[] = {
    mapper0_prg_read,
    mapper0_chr_read,
    mapper0_prg_write,
    mapper0_chr_write,
    free_mapper0,

    mapper1_prg_read,
    mapper1_chr_read,
    mapper1_prg_write,
    mapper1_chr_write,
    free_mapper1,

    NULL,
    NULL,
    NULL,
    NULL,
    NULL,

    mapper3_prg_read,
    mapper3_chr_read,
    mapper3_prg_write,
    mapper3_chr_write,
    free_mapper3,

    mapper4_prg_read,
    mapper4_chr_read,
    mapper4_prg_write,
    mapper4_chr_write,
    free_mapper4,
};

void free_cartridge(struct Cartridge* cartridge) {
    free_func* freef = vtable[FREE + cartridge->mapper * VTABLE_SIZE];
    freef(cartridge);
    free(cartridge);
}

uint8_t cartridge_prg_read(struct Cartridge* cartridge, uint16_t addr) {
    // Should be impossible to call prg_read outside [0x6000..0xFFFF].
    assert(addr > 0x5FFF);

    // Lookup and call appropriate function.
    prg_read_func* prg_read = vtable[PRG_READ + cartridge->mapper * VTABLE_SIZE];
    return prg_read(cartridge, addr);
}

uint8_t cartridge_chr_read(struct Cartridge* cartridge, uint16_t addr) {
    // Should be impossible to call chr_read outside [0x0000..0x2000].
    assert(addr < 0x2000);

    // Lookup and call appropriate function.
    chr_read_func* chr_read = vtable[CHR_READ + cartridge->mapper * VTABLE_SIZE];
    return chr_read(cartridge, addr);
}

void cartridge_prg_write(struct Cartridge* cartridge, uint16_t addr, uint8_t val) {
    // Should be impossible to call prg_write outside [0x6000..0xFFFF].
    assert(addr > 0x5FFF);

    // Lookup and call appropriate function.
    prg_write_func* prg_write = vtable[PRG_WRITE + cartridge->mapper * VTABLE_SIZE];
    prg_write(cartridge, addr, val);
}

void cartridge_chr_write(struct Cartridge* cartridge, uint16_t addr, uint8_t val) {
    // Should be impossible to call chr_write outside [0x0000..0x2000].
    assert(addr < 0x2000);

    // Lookup and call appropriate function.
    chr_write_func* chr_write = vtable[CHR_WRITE + cartridge->mapper * VTABLE_SIZE];
    chr_write(cartridge, addr, val);
}

uint8_t cartridge_get_mirroring(struct Cartridge* cartridge) {
    return cartridge->mirroring;
}
