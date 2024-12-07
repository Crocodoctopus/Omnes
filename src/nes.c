#include "nes.h"
#include "cpu.h"
#include "ppu.h"
#include <string.h>

uint16_t make_u16(uint8_t hi, uint8_t lo) {
    return ((uint16_t)hi << 8) | (uint16_t)lo;
}

void init_nes(struct Nes* nes, struct Cartridge* cartridge) {
    // cartridge
    nes->cartridge = cartridge;

    //
    nes->input1 = 0;
    nes->input2 = 0;
    nes->joy1 = 0;
    nes->joy2 = 0;
    nes->controller_strobe = 0;
    nes->oam_delay = 0;

    // https://wiki.nesdev.org/w/index.php/CPU_power_up_state
    memset(nes->ram, 0, 0x800); 
    nes->acc = 0;
    nes->x = nes->y = 0;
    nes->pc = 0x0000;
    nes->sp = 0x00;
    nes->status = 0x34;
    //
    nes->reset = 1; // the nes will perform a reset interrupt upon boot
    nes->nmi = 0;
    nes->irq = 0;
    //
    nes->instr_ptr = get_micro(0);
    nes->ir = 0;
    nes->dph = nes->dpl = 0;
    nes->b = nes->t = 0;
    nes->ic = 0;

    // https://wiki.nesdev.org/w/index.php/PPU_power_up_state
    memset(nes->ciram, 0, 0x800);
    memset(nes->oam, 0, 0x100);
    memset(nes->palette, 0, 0x20);
    nes->cycle = 0;
    nes->ppuctrl.raw = 0;
    nes->ppumask.raw = 0;
    nes->ppustatus.raw = 0;
    nes->oamaddr = 0;
    nes->ppuscroll = 0;
    nes->ppuaddr = 0;
    nes->read_buffer = 0;
    nes->set_szh = 0;
    // micro
    nes->fine_x = 0;
    nes->ppulatch = 0;
    nes->nt_latch = 0;
    nes->at_latch = 0;
    nes->pt_lo_latch = 0;
    nes->pt_hi_latch = 0;
    // background
    nes->bg_lo_at_shifter = 0;
    nes->bg_hi_at_shifter = 0;
    nes->bg_lo_pt_shifter = 0;
    nes->bg_hi_pt_shifter = 0;
}

void free_nes(struct Nes* nes) {
    free_cartridge(nes->cartridge);
}

void reset(struct Nes* nes) {
    nes->reset = 1;
}

uint8_t step_nes(struct Nes* nes, uint8_t* nes_pixels) {
    // Step CPU if there is no OAM delay
    if (nes->oam_delay == 0) step_cpu(nes);
    else nes->oam_delay -= 1;

    uint8_t vblank = 0;
    step_ppu(nes, nes_pixels, &vblank);
    step_ppu(nes, nes_pixels, &vblank);
    step_ppu(nes, nes_pixels, &vblank);

    return vblank;
}
