#pragma once

#include "defines.h"

union PpuCtrl {
    struct __attribute__((packed)) {
        u8 NN: 2; // Base nametable address (0 = $2000; 1 = $2400; 2 = $2800; 3 = $2C00)
        u8 I: 1; // Ppuaddr increment mode (0 = add 1; 1 = add 32)
        u8 S: 1; // Base FG pattern table for 8x8 sprite mode (0 = $0000; 1 = $1000)
        u8 B: 1; // Base BG pattern table (0 = $1000; 1 = $1000)
        u8 H: 1; // Sprite mode (0 = 8x8; 1 = 8x16)
        u8 P_unimp: 1; // UNIMPLEMENTED
        u8 V: 1; // Enable vblank NMI
    };
    u8 raw;
};

union PpuMask {
    struct __attribute__((packed)) {
        u8 g: 1; // Greyscale mode (0 = color; 1 = greyscale)
        u8 m: 1; // Show background in left most 8 screen pixels
        u8 M: 1; // Show sprites in left most 8 screen pixels
        u8 b: 1; // Enable background rendering
        u8 s: 1; // Enable sprite rendering
        u8 R_unimpl: 1;
        u8 G_unimpl: 1;
        u8 B_unimpl: 1;
    };
    u8 raw;
};

union PpuStatus {
    struct __attribute__((packed)) {
        u8 unused: 5;
        u8 O: 1; // Sprite overflow flag
        u8 S: 1; // Sprite 0 hit flag
        u8 V: 1; // Vblank flag; set on vblank, cleared on read.
    };
    u8 raw;
};

/*
union PpuVramAddress {
    struct __attribute__((packed)) {
        u8 XXXX: 4;
        u8 YYYYY: 5;
        u8 NN: 2;
        u8 yyy: 3;
        u8 unused: 1;
        u8 pad: 1;
    } scroll;
    struct __attribute__((packed)) {
        u16 raw: 14;
        u8 unused: 1;
        u8 pad: 1;
    } address;
};
*/

struct Nes;

// NES component step functions:
void step_ppu(struct Nes*, uint8_t*, uint8_t*);

// Functions emulating the PPU bus (https://wiki.nesdev.org/w/index.php/PPU_memory_map).
uint8_t ppu_bus_read(struct Nes*, uint16_t);
void ppu_bus_write(struct Nes*, uint16_t, uint8_t);

// (Used internally)
void ppu_inc_x_scroll(struct Nes*);
void ppu_inc_y_scroll(struct Nes*);
void ppu_restore_x_scroll(struct Nes*);
void ppu_restore_y_scroll(struct Nes*);
