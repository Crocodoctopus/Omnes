#pragma once

#include <stdint.h>
#include "cartridge.h"
#include "defines.h"
#include "ppu.h"

#define STATUS_FLAG_CARRY       0
#define STATUS_FLAG_ZERO        1
#define STATUS_FLAG_INTERRUPT   2
#define STATUS_FLAG_DECIMAL     3
#define STATUS_FLAG_BREAK       4
#define STATUS_FLAG_ALWAYS      5
#define STATUS_FLAG_OVERFLOW    6
#define STATUS_FLAG_NEGATIVE    7

struct Nes {
    // Cartridge
    struct Cartridge* cartridge;

    // IO
    uint8_t input1;
    uint8_t input2;
    uint8_t joy1;
    uint8_t joy2;
    uint8_t controller_strobe;
    uint16_t oam_delay;

    // CPU
    uint8_t ram[0x0800];
    uint8_t acc;
    uint8_t x, y;
    uint16_t pc;
    uint8_t sp;
    uint8_t status;
    // interrupt 
    uint8_t reset;
    uint8_t nmi;
    uint8_t irq;
    // CPU micro
    void (**instr_ptr)(struct Nes* nes);
    uint8_t ir;
    uint8_t dph, dpl;
    uint8_t b, t;
    uint8_t ic;
    
    // PPU
    uint8_t ciram[0x0800];
    uint8_t oam[0x100];
    uint8_t palette[0x20];
    uint32_t cycle;
    // Mapped PPU registers (https://www.nesdev.org/wiki/PPU_registers)
    union PpuCtrl ppuctrl; // $2000
    union PpuMask ppumask; // $2001
    union PpuStatus ppustatus; // $2002
    uint8_t oamaddr; // $2003
    uint16_t ppuscroll; // $2005
    uint16_t ppuaddr; // $2006
    // Interal PPU registers
    uint8_t parity;
    uint8_t read_buffer;
    uint8_t set_szh;
    uint8_t fine_x;
    uint8_t ppulatch;
    uint8_t nt_latch;
    uint8_t at_latch;
    uint8_t pt_lo_latch;
    uint8_t pt_hi_latch;
    // PPU background
    uint16_t bg_lo_at_shifter;
    uint16_t bg_hi_at_shifter;
    uint16_t bg_lo_pt_shifter;
    uint16_t bg_hi_pt_shifter;
    // PPU foreground
    uint8_t secondary_oam[0x20];
    uint8_t spr_lo_pt_shifters[8]; // sprites pattern low bits
    uint8_t spr_hi_pt_shifters[8]; // sprite pattern high bits
    uint8_t spr_attributes[8]; // sprite attributes
    uint8_t spr_counters[8]; // sprite x positions
    uint8_t spr_zero;
};

uint16_t make_u16(uint8_t hi, uint8_t lo);

// Initializes an NES struct with a cartridge, taking ownership of said cartridge.
void init_nes(struct Nes*, struct Cartridge*);

// Frees an NES struct and all its resources.
void free_nes(struct Nes*);

// Sets reset interrupt, resetting an NES.
void reset(struct Nes*);

// Steps an NES by 1 cycle.
uint8_t step_nes(struct Nes*, uint8_t*);
