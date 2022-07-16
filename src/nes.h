#pragma once

#include <stdint.h>
#include "cartridge.h"
#include "defines.h"

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
    uint8_t ppuctrl; // ppu register @ 0x2000
    uint8_t ppumask; // ppu register @ 0x2001
    uint8_t ppustatus; // ppu register @ 0x2002
    uint8_t oamaddr; // ppu register @ 0x2003
    uint16_t ppuscroll; // ppu register @ 0x2005
    uint16_t ppuaddr; // ppu register @ 0x2006
    uint8_t read_buffer; // used by ppuaddr
    uint8_t set_szh;
    // ppu registers
    uint8_t fine_x; // fine x
    uint8_t ppulatch; // latch
    uint8_t nt_latch;
    uint8_t at_latch;
    uint8_t pt_lo_latch;
    uint8_t pt_hi_latch;
    // ppu background
    uint16_t bg_lo_at_shifter;
    uint16_t bg_hi_at_shifter;
    uint16_t bg_lo_pt_shifter;
    uint16_t bg_hi_pt_shifter;
    // ppu sprite
    uint8_t secondary_oam[0x20];
    uint8_t spr_lo_pt_shifters[8]; // sprites pattern low bits
    uint8_t spr_hi_pt_shifters[8]; // sprite pattern high bits
    uint8_t spr_attributes[8]; // sprite attributes
    uint8_t spr_counters[8]; // sprite x positions
    uint8_t spr_zero;
};

uint16_t make_u16(uint8_t hi, uint8_t lo);

// init/free
void init_nes(struct Nes*, struct Cartridge*);
void free_nes(struct Nes*);

// exec
void step_cpu(struct Nes*);
void step_ppu(struct Nes*, uint8_t*, uint8_t*);
void reset(struct Nes*);

// misc
void set_flag(struct Nes*, uint8_t, uint8_t);
uint8_t get_flag(struct Nes*, uint8_t);

// bus
// https://wiki.nesdev.org/w/index.php/CPU_memory_map
uint8_t cpu_bus_read(struct Nes*, uint16_t);
void cpu_bus_write(struct Nes*, uint16_t, uint8_t);
// https://wiki.nesdev.org/w/index.php/PPU_memory_map
uint8_t ppu_bus_read(struct Nes*, uint16_t);
void ppu_bus_write(struct Nes*, uint16_t, uint8_t);

// ppu
void ppu_inc_x_scroll(struct Nes*);
void ppu_inc_y_scroll(struct Nes*);
void ppu_restore_x_scroll(struct Nes*);
void ppu_restore_y_scroll(struct Nes*);

// microcode
typedef void(micro_instr)(struct Nes* nes);
micro_instr** get_micro(uint8_t);
void _A0_8_AD_24_0_0(struct Nes*);
void _A0_8_AD_44_0_0(struct Nes*);
void _A0_8_AD_2C_0_0(struct Nes*);
void _5B_32_CD_28_0_0_ml6502(struct Nes*);
void _A0_8_AD_54_0_0(struct Nes*);
void _5B_32_CD_90_0_0_ml6502(struct Nes*);
void _50_32_C_88_0_0_ml6502(struct Nes*);
void _50_32_FC_80_0_0_ml6502(struct Nes*);
void _A0_8_CE_55_0_0(struct Nes*);
void _A0_8_3E_55_0_0(struct Nes*);
void _33_8_C_4C_0_0(struct Nes*);
void _33_8_FC_44_0_0(struct Nes*);
void _30_42_0_0_10_0(struct Nes*);
void _A3_8_C_44_0_0(struct Nes*);
void _23_8_8_44_0_0(struct Nes*);
void _32_8_C_44_0_0(struct Nes*);
void _44_8_C_4C_0_0(struct Nes*);
void _44_8_FC_44_0_0(struct Nes*);
void _40_8_C_44_0_0(struct Nes*);
void _A4_8_C_44_0_0(struct Nes*);
void _A0_8_6C_4_0_0(struct Nes*);
void _A0_8_EC_4_0_0(struct Nes*);
void _B0_8_3D_4C_0_0(struct Nes*);
void _B3_8_3D_4C_0_0(struct Nes*);
void _B4_8_3D_4C_0_0(struct Nes*);
void _A0_8_8C_4_0_0(struct Nes*);
void _B0_8_8F_34_0_0(struct Nes*);
void _A0_8_CC_4_0_0(struct Nes*);
void _30_8_CC_4_0_0(struct Nes*);
void _40_8_CC_4_0_0(struct Nes*);
void _C0_42_0_0_10_0(struct Nes*);
void _5B_32_CD_20_0_0_ml6502(struct Nes*);
void _5B_32_CD_80_0_0_ml6502(struct Nes*);
void _F5_42_0_0_10_0(struct Nes*);
void _B0_56_0_0_0_0(struct Nes*);
void _70_56_0_0_10_0(struct Nes*);
void _80_56_0_0_10_0(struct Nes*);
void _90_52_0_0_10_0(struct Nes*);
void _F7_57_0_0_10_0(struct Nes*);
void _F3_42_0_0_10_0(struct Nes*);
void _F4_42_0_0_10_0(struct Nes*);
void _F0_42_0_0_10_0(struct Nes*);
void _3_42_C8_40_0_0(struct Nes*);
void _4_42_C8_40_0_0(struct Nes*);
void _B0_42_0_0_10_0(struct Nes*);
void _10_0_0_0_0_0(struct Nes*);
void _C0_12_0_0_10_0(struct Nes*);
void _3_80_C8_40_0_0(struct Nes*);
void _F5_12_0_0_10_0(struct Nes*);
void _F0_12_0_0_10_0(struct Nes*);
void _F3_12_0_0_10_0(struct Nes*);
void _F4_12_0_0_10_0(struct Nes*);
void _3_0_C8_40_0_0(struct Nes*);
void _4_0_C8_40_0_0(struct Nes*);
void _10_12_C8_18_0_0(struct Nes*);
void _B0_12_0_0_10_0(struct Nes*);
void _B0_18_0_4_0_0(struct Nes*);
void _50_44_0_0_10_0(struct Nes*);
void _10_42_0_0_10_0(struct Nes*);
void _F0_22_0_0_10_0(struct Nes*);
void _B0_22_0_0_10_0(struct Nes*);
void _3_80_C8_42_0_0(struct Nes*);
void _4_80_C8_42_0_0(struct Nes*);
void _4_C2_C8_42_10_0(struct Nes*);
void _80_14_0_0_0_0(struct Nes*);
void _90_12_0_0_0_0(struct Nes*);
void _F0_57_0_0_10_0(struct Nes*);
void _B0_44_0_0_10_0(struct Nes*);
void _4_C2_C8_40_10_0(struct Nes*);
void _11_12_8_58_0_0(struct Nes*);
void _0_8_0_4_0_0(struct Nes*);
void _B0_52_0_0_10_0(struct Nes*);
void _F9_57_0_0_10_0(struct Nes*);
void _F8_57_0_0_10_0(struct Nes*);
void _F7_57_0_0_10_10(struct Nes*);
void _70_52_0_0_10_0(struct Nes*);
void _0_8_0_6_0_0(struct Nes*);
void _88_2_C8_41_0_0(struct Nes*);
void _99_2_C8_78_0_0(struct Nes*);
void _F9_57_0_0_10_0_brk(struct Nes*);
void _F8_57_0_0_10_0_brk(struct Nes*);
void _F7_57_0_0_10_0_brk(struct Nes*);
void _80_74_0_0_0_0(struct Nes*);
void _90_62_2_0_0_0(struct Nes*);
void _0_0_0_0_0_0(struct Nes*);
void _F7_57_0_0_10_0_bflag(struct Nes*);
void _0_0_0_0_0_0_end(struct Nes*);
void _B0_8_9_4_0_0_carry(struct Nes*);
void _B0_8_F9_4_0_0_carry(struct Nes*);
void _B0_8_9_4_0_0_interrupt(struct Nes*);
void _B0_8_F9_4_0_0_interrupt(struct Nes*);
void _B0_8_9_4_0_0_overflow(struct Nes*);
void _B0_8_9_4_0_0_decimal(struct Nes*);
void _B0_8_F9_4_0_0_decimal(struct Nes*);
void _INCDPH(struct Nes*);
void _bpl(struct Nes*); // awful hack
void _bmi(struct Nes*); // awful hack
void _bvc(struct Nes*); // awful hack
void _bvs(struct Nes*); // awful hack
void _bcc(struct Nes*); // awful hack
void _bcs(struct Nes*); // awful hack
void _bne(struct Nes*); // awful hack
void _beq(struct Nes*); // awful hack