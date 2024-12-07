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
