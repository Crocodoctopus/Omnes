#include "nes.h"
#include "cpu.h"
#include "ppu.h"
#include "error.h"
#include <assert.h>

// Maps writes originating from the CPU to the correct component.
// https://wiki.nesdev.org/w/index.php/CPU_memory_map
uint8_t cpu_bus_read(struct Nes* nes, uint16_t addr) {
    // Range [0x0000, 0x1FFF]: CPU RAM.
    if (addr <= 0x1FFF) {
        return nes->ram[addr & 0x07FF];
    }

    // Range [0x2000, 0x3FFF]: PPU registers.
    if (addr <= 0x3FFF) {
        switch (0x2000 | (addr & 0b111)) {
            // ppustatus
            case 0x2002: {
                uint8_t out = nes->ppustatus.raw;
                nes->ppustatus.V = 0;
                nes->ppulatch = 0;
                //nes->nmi = 0; ?
                return out;
            }
            // oamdata
            case 0x2004: {
                return nes->oam[nes->oamaddr];
            }
            // ppuadata
            case 0x2007: {
                if ((nes->ppuaddr & 0x3FFF) < 0x3F00) {
                    uint8_t out = nes->read_buffer;
                    nes->read_buffer = ppu_bus_read(nes, nes->ppuaddr);
                    nes->ppuaddr += nes->ppuctrl.I ? 32 : 1;

                    // A hack to clock the MMC3
                    //if (nes->cartridge->mapper == 4)
                    //    cartridge_chr_read(nes->cartridge, nes->ppuaddr & 1 << 12);
                    
                    return out;
                } else {
                    uint8_t out = ppu_bus_read(nes, nes->ppuaddr);
                    nes->ppuaddr += nes->ppuctrl.I ? 32 : 1;

                    // A hack to clock the MMC3
                    //if (nes->cartridge->mapper == 4)
                    //    cartridge_chr_read(nes->cartridge, nes->ppuaddr & 1 << 12);

                    return out;
                }
            }
            //
            default: {
                return 0x7F;
                error(UNIMPLEMENTED, "Unimplemented bus read: 0x%04X", 0x2000 | (addr & 0b111));
                assert(0);
            }
        }
    }

    // Range [0x4000, 0x4017]: APU and IO registers.
    if (addr <= 0x4017) {
        switch (addr) {
            // Joypad 1
            case 0x4016: {
                uint8_t out = nes->joy1 & 0b1;
                if (nes->controller_strobe % 2 == 0) nes->joy1 >>= 1;
                return out;
            }
            // joypad 2
            case 0x4017: {
                uint8_t out = nes->joy2 & 0b1;
                if (!nes->controller_strobe % 2 == 0) nes->joy2 >>= 1;
                return out;
            }
            // do not abort on unhandled APU
            default: {
                return 0;
            }
        }
    }

    // Range [0x4018..0x401F]: Don't implement.
    if (addr <= 0x401F) {
        return 0;
    }

    // Range [0x4020, 0xFFFF]: Cartridge space, forwarded to cartridge.
    return cartridge_prg_read(nes->cartridge, addr);
}

// Maps writes originating from the CPU to the correct component.
// https://wiki.nesdev.org/w/index.php/CPU_memory_map
void cpu_bus_write(struct Nes* nes, uint16_t addr, uint8_t byte) { 
    // Range [0x0000, 0x1FFF]: CPU RAM.
    if (addr <= 0x1FFF) {
        nes->ram[addr & 0x07FF] = byte;
        return;
    }

    // Range [0x2000, 0x3FFF]: PPU registers.
    if (addr <= 0x3FFF) {
        switch (addr & 0b0010000000000111) {
            // ppuctrl
            case 0x2000: {
                //if (nes->ppuctrl < 0x80 && byte >= 0x80 && nes->ppustatus >= 0x80) {
                //    nes->nmi = 1;
                //}
                nes->ppuctrl.raw = byte;
                // micro
                nes->ppuscroll &= 0b111001111111111;
                nes->ppuscroll |= (byte & 0b11) << 10;
                return;
            }
            // ppumask
            case 0x2001: {
                nes->ppumask.raw = byte;
                return;
            }
            // ppustatus
            case 0x2002: {
                return;
            }
            // oamaddr
            case 0x2003: {
                nes->oamaddr = byte;
                return;
            }
            // oamdata
            case 0x2004: {
                nes->oam[nes->oamaddr] = byte;
                nes->oamaddr += 1;
                return;
            };
            // ppuscroll
            case 0x2005: {
                if (nes->ppulatch == 0) {
                    uint8_t fine_x = byte & 0b111;
                    uint8_t coarse_x = byte >> 3;

                    nes->ppuscroll &= 0b1111111111100000;
                    nes->ppuscroll|= coarse_x;
                    nes->fine_x = fine_x;
                } else {
                    uint8_t fine_y = byte & 0b111;
                    uint8_t coarse_y = byte >> 3;

                    nes->ppuscroll &= 0b000110000011111;
                    nes->ppuscroll |= coarse_y << 5;
                    nes->ppuscroll |= fine_y << 12;
                }
                nes->ppulatch = !nes->ppulatch;
                return;
            }
            // ppuaddr
            case 0x2006: {
                if (nes->ppulatch == 0) {
                    nes->ppuscroll &= 0b0000000011111111;
                    nes->ppuscroll |= byte << 8;
                } else {
                    nes->ppuscroll &= 0b1111111100000000;
                    nes->ppuscroll |= byte;
                    nes->ppuaddr = nes->ppuscroll;

                    // A hack to clock the MMC3
                    //if (nes->cartridge->mapper == 4)
                    //    cartridge_chr_read(nes->cartridge, nes->ppuaddr & 1 << 12);
                }
                nes->ppulatch = !nes->ppulatch;
                return;
            }
            // ppudata
            case 0x2007: {
                ppu_bus_write(nes, nes->ppuaddr, byte); 
                nes->ppuaddr += nes->ppuctrl.I ? 32 : 1;

                // A hack to clock the MMC3
                //if (nes->cartridge->mapper == 4)
                //    cartridge_chr_read(nes->cartridge, nes->ppuaddr & 1 << 12);
                
                return;
            }
            //
            default: {
                error(UNIMPLEMENTED, "Unimplemented bus write: 0x%04X <- 0x%02X", addr, byte);
                assert(0);
                return;
            }
        }
    }

    // Range [0x4000, 0x4017]: APU and IO registers.
    if (addr <= 0x4017) {
        // TODO: implement
        switch (addr) {
            // input polling
            case 0x4016: {
                nes->controller_strobe = byte;
                if (nes->controller_strobe) {
                    nes->joy1 = nes->input1;
                    nes->joy2 = nes->input2;
                }    
                return;
            };
            // OAM
            case 0x4014: {
                nes->oam_delay = 513;
                uint8_t i = 0;
                do cpu_bus_write(nes, 0x2004, cpu_bus_read(nes, byte << 8 | i));
                while (++i != 0);
                return;
            };
            // do not abort on unhandled APU
            default: {
                return;
            }
        }
    }

    // Range [0x4018..0x401F]: Don't implement.
    if (addr <= 0x401F) {
        return;
    }

    // Range [0x4020, 0xFFFF]: Cartridge space, forwarded to cartridge.
    cartridge_prg_write(nes->cartridge, addr, byte);
    return;
}

// Maps reads originating from the PPU to the correct component.
// https://wiki.nesdev.org/w/index.php/PPU_memory_map
uint8_t ppu_bus_read(struct Nes* nes, uint16_t addr) {
    // ppu addresses above 0x3FFF are mirrored
    addr &= 0x3FFF;

    // Range [0x0000, 0x1FFF]: CHR ROM.
    if (addr <= 0x1FFF) {
        return cartridge_chr_read(nes->cartridge, addr);
    }

    // Range [0x2000, 0x3EFF]: CIRAM.
    if (addr <= 0x3EFF) {
        uint8_t mirroring = !cartridge_get_mirroring(nes->cartridge);
        uint16_t page = (addr >> mirroring) & 0b010000000000;
        uint16_t base = addr & 0b001111111111;
        return nes->ciram[base + page];
    }

    // Range [0x3F00, 0x3FFF]: Palette.
    if (addr <= 0x3FFF) {
        if (addr % 4 == 0) return nes->palette[addr & 0x0F];
        else return nes->palette[addr & 0x1F];
    }

    // Unreachable.
    error(UNREACHABLE, "This line should not be reachable");
    assert(0);
    return 0;
}

// Maps writes originating from the PPU to the correct component.
// https://wiki.nesdev.org/w/index.php/PPU_memory_map
void ppu_bus_write(struct Nes* nes, uint16_t addr, uint8_t byte) {
    // ppu addresses above 0x3FFF are mirrored
    addr &= 0x3FFF;

    // Range [0x0000, 0x1FFF]: CHR ROM.
    if (addr <= 0x1FFF) {
        cartridge_chr_write(nes->cartridge, addr, byte);
        return;
    }

    // Range [0x2000, 0x3EFF]: CIRAM.
    if (addr <= 0x3EFF) {
        uint8_t mirroring = !cartridge_get_mirroring(nes->cartridge);
        uint16_t page = (addr >> mirroring) & 0b010000000000;
        uint16_t base = addr & 0b001111111111;
        nes->ciram[base + page] = byte;
        return;
    }

    // Range [0x3F00, 0x3FFF]: Palette.
    if (addr <= 0x3FFF) {
        if (addr % 4 == 0) nes->palette[addr & 0x0F] = byte;
        else nes->palette[addr & 0x1F] = byte;
        return;
    }

    // Unreachable.
    error(UNREACHABLE, "This line should not be reachable");
    assert(0);
}
