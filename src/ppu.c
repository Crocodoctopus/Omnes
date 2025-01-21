#include "nes.h"
#include "ppu.h"
#include "error.h"

#include <string.h>

#define incl(a, b, c) (a <= b && b <= c)

uint8_t reverse_u8(uint8_t v) {
   v = (v & 0b11110000) >> 4 | (v & 0b00001111) << 4;
   v = (v & 0b11001100) >> 2 | (v & 0b00110011) << 2;
   v = (v & 0b10101010) >> 1 | (v & 0b01010101) << 1;
   return v;
}

void ppu_evaluate_sprites(struct Nes* nes, uint16_t scanline) {
    uint8_t i1 = 0; // primary oam index
    uint8_t i2 = 0; // secondary oam index
    uint8_t offset = nes->ppuctrl.H ? 16 : 8;
    nes->spr_zero = 0xFF;
    do {
        //
        uint8_t ry = scanline - nes->oam[i1];
        if (ry < offset && nes->oam[i1] < 0xEF) {
            if (i1 == 0) nes->spr_zero = 0;
            memcpy(nes->secondary_oam + i2, nes->oam + i1, 4);
            i2 += 4;
        }
        i1 += 4;
    } while (i1 != 0x00 && i2 < 0x20);
}

void step_ppu(struct Nes* nes, uint8_t* pixels, uint8_t* vblank) {
    // Calculate scanline/dot and advance cycle counter
    if (nes->cycle == 89342) {
        nes->parity ^= 0b1; 
        nes->cycle = 0;
    }
    uint16_t scanline = nes->cycle / 341;
    uint16_t dot = nes->cycle % 341;
    nes->cycle += 1;

    uint8_t prerender_scanline = scanline == 261;
    uint8_t visible_scanline = scanline <= 239;

    // Dot 0 is idle.
    if (scanline == 339 && dot == 0 && nes->parity && (nes->ppumask.b || nes->ppumask.s)) dot += 1;
    if (dot == 0) return;

    //  Vblank begins on SL 241, D 1
    if (scanline == 241 && dot == 1) {
        *vblank = 1;
        nes->nmi = nes->ppuctrl.V;
        nes->ppustatus.V = 1; // set vblank flag
        return;
    }

    // SL [240, 260] are idle.
    if (incl(240, scanline, 260)) return;

    // Delay sprite zero hit
    if (nes->set_szh > 0) {
        nes->set_szh -= 1;
        if (nes->set_szh == 0) {
            nes->ppustatus.S = 1;
        }
    }

    // clear ppu status on SL 261 D 1
    if (prerender_scanline && dot == 1) {
        nes->ppustatus.raw = 0;
    }

    // Range: [0, 239] visible scanlines (and scanline 261*)
    if ((prerender_scanline || visible_scanline) && (nes->ppumask.b || nes->ppumask.s)) {        
        /////////////////////////
        // Background pixel output operation
        uint8_t bg_pattern = 0; // 2 bit shifter output
        uint8_t bg_attribute = 0; // 2 bit shifter output
        {
            // PPU register operations.
            if (dot == 257) ppu_restore_x_scroll(nes);
            if (prerender_scanline && incl(280, dot, 304)) ppu_restore_y_scroll(nes);
            if ((dot & 0b0111) == 0 && !incl(249, dot, 327)) ppu_inc_x_scroll(nes);
            if (dot == 256) ppu_inc_y_scroll(nes);

            // Extract tile info
            uint8_t coarse_x = (nes->ppuaddr >> 0) & 0b11111;
            uint8_t coarse_y = (nes->ppuaddr >> 5) & 0b11111;
            uint8_t nametable_select = (nes->ppuaddr >> 10) & 0b11;
            uint8_t fine_x = nes->fine_x;
            uint8_t fine_y = (nes->ppuaddr >> 12) & 0b111;

            // Pixel output
            if (dot <= 256 && !prerender_scanline) {
                bg_attribute = nes->bg_hi_at_shifter >> (14 - fine_x) & 0b10 | nes->bg_lo_at_shifter >> (15 - fine_x) & 1;
                bg_pattern = nes->ppumask.b 
                    ? nes->bg_hi_pt_shifter >> (14 - fine_x) & 0b10 | nes->bg_lo_pt_shifter >> (15 - fine_x) & 1
                    : 0b00; // if bg rendering is disabled, force pattern to transparent
            }

            // Shifter update
            if (!incl(257, dot, 320)) {
                if (dot <= 336 ) {
                    nes->bg_hi_pt_shifter <<= 1;
                    nes->bg_lo_pt_shifter <<= 1;
                    nes->bg_hi_at_shifter <<= 1;
                    nes->bg_lo_at_shifter <<= 1;
                }

                // Shifter load operations
                uint16_t nametable_base = 0x2000 | (nametable_select * 0x400);
                uint16_t attribute_base = nametable_base | 0x3C0;
                uint16_t pattern_base = nes->ppuctrl.B ? 0x1000 : 0x0000;
        
                // Behind the scenes, the PPU is fetching data to move into the shift registers
                switch (dot & 0b111) {
                    // Tile nametable byte
                    case 1: nes->nt_latch = ppu_bus_read(nes, nametable_base | (coarse_x + coarse_y * 32)); break;
                    // Tile attribute byte
                    case 3: 
                    nes->at_latch = ppu_bus_read(nes, attribute_base | (coarse_x/4 + coarse_y/4 * 32/4)); 
                    nes->at_latch >>= (coarse_x / 2 % 2) * 2 + (coarse_y / 2 % 2) * 4;
                    break;
                    // Tile pattern low byte
                    case 5: nes->pt_lo_latch = ppu_bus_read(nes, pattern_base | nes->nt_latch << 4 | fine_y); break;
                    // Tile pattern high byte
                    case 7: nes->pt_hi_latch = ppu_bus_read(nes, pattern_base | nes->nt_latch << 4 | fine_y | 8); break;
                }
    
                // Every 8 cycle, update the shifters
                if ((dot & 0b111) == 0) {
                    nes->bg_lo_at_shifter |= (nes->at_latch & 0b01) ? 0x00FF : 0x0000;
                    nes->bg_hi_at_shifter |= (nes->at_latch & 0b10) ? 0x00FF : 0x0000;
                    nes->bg_lo_pt_shifter |= nes->pt_lo_latch;
                    nes->bg_hi_pt_shifter |= nes->pt_hi_latch;
                }
            }
        }

        /////////////////////////
        // Foreground pixel output operation
        uint8_t fg_pattern = 0; // 2 bit shifter output
        uint8_t fg_attribute = 0; // 2 bit shifter output
        uint8_t fg_priority = 0; // priority bit
        {
            // Pixel output
            uint8_t dirty = 0;
            if (incl(1, dot, 256) && !prerender_scanline && scanline != 0) {
                for (uint8_t spr = 0; spr < 8; spr++) {
                    if (nes->spr_counters[spr] == 0x00) {
                        uint8_t lo = nes->spr_lo_pt_shifters[spr] >= 0x80;
                        uint8_t hi = nes->spr_hi_pt_shifters[spr] >= 0x80;
    
                        nes->spr_lo_pt_shifters[spr] <<= 1;
                        nes->spr_hi_pt_shifters[spr] <<= 1;
        
                        uint8_t attribute = nes->spr_attributes[spr] & 0b11;
                        uint8_t pattern = nes->ppumask.s 
                            ? hi << 1 | lo
                            : 0b00; // if fg rendering is disabled, force pattern to transparent
    
                        if (pattern != 0 && !dirty) {
                            dirty = 1;
                            fg_pattern = pattern;
                            fg_attribute = attribute;
                            fg_priority = (nes->spr_attributes[spr] & 0x20) > 0;
                            if (bg_pattern != 0 && nes->spr_zero == spr) nes->set_szh = 2;
                        }
                    } else {
                        nes->spr_counters[spr] -= 1;
                    }
                }
            }
            
            // By dot 65, secondary_oam should be cleared.
            if (dot == 65 && !prerender_scanline) memset(nes->secondary_oam, 0xFF, 0x20);
    
            // By dot 257, sprite evaluation should be done.
            if (dot == 257 && !prerender_scanline) ppu_evaluate_sprites(nes, scanline);

            //
            if (incl(257, dot, 320)) nes->oamaddr = 0;
    
            // Shifter operation
            if (incl(257, dot, 320) || prerender_scanline) {
                // Get sprite data
                uint8_t sprite_number = (dot - 257) / 8;
                uint8_t* s_oam = nes->secondary_oam + 4 * sprite_number;
    
                // The PPU still makes these calls, even though sprites don't use them
                uint16_t nametable_base;
                switch (nes->ppuctrl.NN) {
                    case 0: nametable_base = 0x2000; break;
                    case 1: nametable_base = 0x2400; break;
                    case 2: nametable_base = 0x2800; break;
                    case 3: nametable_base = 0x2C00; break;
                }
    
                // Calculate pattern table bank
                uint16_t pattern_base = nes->ppuctrl.H 
                    ? 0x1000 * (s_oam[1] & 0b1)
                    : 0x1000 * nes->ppuctrl.S; 
    
                switch (dot & 0b111) {
                    // Garbage nametable read
                    case 1: ppu_bus_read(nes, nametable_base); break;

                    // Garbage attribute read, sprite attribute and x value read
                    case 3: {
                        nes->spr_attributes[sprite_number] = s_oam[2];
                        nes->spr_counters[sprite_number] = s_oam[3];
                        ppu_bus_read(nes, nametable_base); 
                    } break;
    
                    // Sprite pattern low byte
                    case 5: {
                        //
                        uint8_t flip_v = (s_oam[2] & 0x80) > 0;
                        uint8_t flip_h = (s_oam[2] & 0x40) > 0;
                        uint8_t y = scanline - s_oam[0];
    
                        uint8_t pattern_index = s_oam[1];
                        if (nes->ppuctrl.H) { 
                            pattern_index &= 0xFE;
                            pattern_index |= (y >> 3 ^ flip_v) > 0;
                        }
    
                        if (flip_v) y = 7 - y;
                        y &= 0b111;
                        uint8_t lo = ppu_bus_read(nes, pattern_base | pattern_index << 4 | y);
                        if (s_oam[3] == 0xFF) lo = 0; 
                        nes->spr_lo_pt_shifters[sprite_number] = flip_h ? reverse_u8(lo) : lo;
                    } break;

                    // Sprite pattern high byte
                    case 7: {
                        //
                        uint8_t flip_v = (s_oam[2] & 0x80) > 0;
                        uint8_t flip_h = (s_oam[2] & 0x40) > 0;
                        uint8_t y = scanline - s_oam[0];   
    
                        uint8_t pattern_index = s_oam[1];
                        if (nes->ppuctrl.H) { 
                            pattern_index &= 0xFE;
                            pattern_index |= (y >> 3 ^ flip_v) > 0;
                        }
    
                        if (flip_v) y = 7 - y;
                        y &= 0b111;
                        uint8_t hi = ppu_bus_read(nes, pattern_base | pattern_index << 4 | y | 8);
                        if (s_oam[3] == 0xFF) hi = 0; 
                        nes->spr_hi_pt_shifters[sprite_number] = flip_h ? reverse_u8(hi) : hi;
                    } break;
                }
            }
        }

        /////////////////////////
        // Final pixel output
        if (incl(1, dot, 256) && !prerender_scanline) {
            uint8_t final = 0;
            if (incl(1, dot, 9) && !nes->ppumask.m) bg_pattern = 0;
            if (incl(1, dot, 9) && !nes->ppumask.M) fg_pattern = 0;
            // if (bg_pattern == 0 && fg_pattern == 0); // no op
            if (bg_pattern == 0 && fg_pattern != 0) final = 0x10 | fg_attribute << 2 | fg_pattern;
            if (bg_pattern != 0 && fg_pattern == 0) final = bg_attribute << 2 | bg_pattern;
            if (bg_pattern != 0 && fg_pattern != 0 && fg_priority == 0) final = 0x10 | fg_attribute << 2 | fg_pattern;
            if (bg_pattern != 0 && fg_pattern != 0 && fg_priority != 0) final = bg_attribute << 2 | bg_pattern;
            uint8_t grayscale_mask = nes->ppumask.g ? 0x30 : 0xFF;
            pixels[scanline * 256 + (dot - 1)] = ppu_bus_read(nes, 0x3F00 | final) & grayscale_mask;
        }
    }
}

void ppu_inc_x_scroll(struct Nes* nes) {
    // if coarse_x == 31
    if ((nes->ppuaddr & 0b11111) == 0b11111) {
        nes->ppuaddr &= ~0b11111;
        nes->ppuaddr ^= 0b01 << 10; // flip nametable lo bit
    } else {
        nes->ppuaddr += 1;
    }
}

void ppu_inc_y_scroll(struct Nes* nes) {
    uint16_t fine_y_mask = 0b111 << 12;
    uint16_t coarse_y_mask = 0b11111 << 5;

    // if fine_y == 7
    uint16_t fine_y = (nes->ppuaddr & fine_y_mask) >> 12;
    uint16_t coarse_y = (nes->ppuaddr & coarse_y_mask) >> 5;
    if (fine_y == 7) {
        nes->ppuaddr &= ~(fine_y_mask | coarse_y_mask); // clear fine_y and coarse_y
        switch (coarse_y) {
            case 29: nes->ppuaddr ^= 0b10 << 10; // no break
            case 31: coarse_y = 0; break;
            default: coarse_y += 1; break;
        }
        nes->ppuaddr |= coarse_y << 5;
    } else {
        nes->ppuaddr += 0b1 << 12;
    }
}

void ppu_restore_x_scroll(struct Nes* nes) {
    uint16_t mask = 0b000010000011111;
    nes->ppuaddr &= ~mask;
    nes->ppuaddr |= nes->ppuscroll & mask;
}

void ppu_restore_y_scroll(struct Nes* nes) {
    uint16_t mask = 0b111101111100000;
    nes->ppuaddr &= ~mask;
    nes->ppuaddr |= nes->ppuscroll & mask;
}
