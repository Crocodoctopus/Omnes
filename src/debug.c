#include "debug.h"
#include "error.h"
#include <assert.h>

void draw_ppu_nametables(struct Nes* nes, uint8_t pixels[4*256*240]) {
    // palettes
    uint8_t bg_palette[4][4];
    bg_palette[0][0] = ppu_bus_read(nes, 0x3F00);
    bg_palette[0][1] = ppu_bus_read(nes, 0x3F01);
    bg_palette[0][2] = ppu_bus_read(nes, 0x3F02);
    bg_palette[0][3] = ppu_bus_read(nes, 0x3F03);
    bg_palette[1][0] = ppu_bus_read(nes, 0x3F00);
    bg_palette[1][1] = ppu_bus_read(nes, 0x3F05);
    bg_palette[1][2] = ppu_bus_read(nes, 0x3F06);
    bg_palette[1][3] = ppu_bus_read(nes, 0x3F07);
    bg_palette[2][0] = ppu_bus_read(nes, 0x3F00);
    bg_palette[2][1] = ppu_bus_read(nes, 0x3F09);
    bg_palette[2][2] = ppu_bus_read(nes, 0x3F0A);
    bg_palette[2][3] = ppu_bus_read(nes, 0x3F0B);
    bg_palette[3][0] = ppu_bus_read(nes, 0x3F00);
    bg_palette[3][1] = ppu_bus_read(nes, 0x3F0D);
    bg_palette[3][2] = ppu_bus_read(nes, 0x3F0E);
    bg_palette[3][3] = ppu_bus_read(nes, 0x3F0F);

    for (int nametable = 0; nametable < 4; nametable += 1) {
        uint16_t nametable_base = 0x2000 | (0x400 * nametable);
        uint16_t attribute_base = nametable_base | 0x3C0;

        // loop through each 8x8 tile (32 by 30 iterations)
        for (uint8_t tile8_y = 0; tile8_y < NAMETABLE_HEIGHT; tile8_y++) {
            for (uint8_t tile8_x = 0; tile8_x < NAMETABLE_WIDTH; tile8_x++) {

                uint16_t nametable_index = tile8_x + tile8_y * NAMETABLE_WIDTH;
                uint16_t attribute_index = tile8_x / 4 + tile8_y / 4 * NAMETABLE_WIDTH / 4;

                // data from nametable/attribute tables
                uint8_t nametable_data = ppu_bus_read(nes, nametable_base + nametable_index);
                uint8_t attribute_data = ppu_bus_read(nes, attribute_base + attribute_index);

                // form patterntable base
                uint16_t pattern_table_base = nametable_data << 4 | nes->ppuctrl << 8 & 0b1000000000000; 

                // form palette id
                uint8_t x_sub_tile = (tile8_x/2) % 2;
                uint8_t y_sub_tile = (tile8_y/2) % 2;
                uint8_t palette_id = (attribute_data >> (x_sub_tile * 2 + y_sub_tile * 4)) & 0b11;

                // loop through each pixel (8 by 8 iterations)
                for (uint8_t bit_plane = 0; bit_plane < TILE_SIZE; bit_plane++) {
                    // load the bit plane
                    uint8_t low_bits = ppu_bus_read(nes, pattern_table_base | bit_plane); // low bits for 8 pixels
                    uint8_t high_bits = ppu_bus_read(nes, pattern_table_base | TILE_SIZE | bit_plane); // high bits for 8 pixels

                    // for each pixel in plane
                    for (uint8_t pixel_index = 0; pixel_index < TILE_SIZE; pixel_index++) {
                        // extract rightmost pixel from the two bit planes above
                        uint8_t pixel = high_bits % 2 << 1 | low_bits % 2;
                        low_bits >>= 1;
                        high_bits >>= 1;

                        // find where this pixel is even suppose to go
                        uint16_t pixel_x = (TILE_SIZE - 1 - pixel_index) + TILE_SIZE * tile8_x + TILE_SIZE * NAMETABLE_WIDTH * (nametable % 2);
                        uint16_t pixel_y = TILE_SIZE * NAMETABLE_HEIGHT * (nametable / 2) + TILE_SIZE * tile8_y + bit_plane;
                        uint32_t index = pixel_x + pixel_y * 2 * NAMETABLE_WIDTH * TILE_SIZE;

                        // put
                        pixels[index] = bg_palette[palette_id][pixel];
                    }
                }
            }
        } 
    }
}

void draw_ppu_pattern_tables(struct Nes* nes, uint8_t pixels[512 * TILE_SIZE * TILE_SIZE], uint8_t palette_id) {
    // palettes
    uint8_t bg_palette[4][4];
    bg_palette[0][0] = ppu_bus_read(nes, 0x3F00);
    bg_palette[0][1] = ppu_bus_read(nes, 0x3F01);
    bg_palette[0][2] = ppu_bus_read(nes, 0x3F02);
    bg_palette[0][3] = ppu_bus_read(nes, 0x3F03);
    bg_palette[1][0] = ppu_bus_read(nes, 0x3F00);
    bg_palette[1][1] = ppu_bus_read(nes, 0x3F05);
    bg_palette[1][2] = ppu_bus_read(nes, 0x3F06);
    bg_palette[1][3] = ppu_bus_read(nes, 0x3F07);
    bg_palette[2][0] = ppu_bus_read(nes, 0x3F00);
    bg_palette[2][1] = ppu_bus_read(nes, 0x3F09);
    bg_palette[2][2] = ppu_bus_read(nes, 0x3F0A);
    bg_palette[2][3] = ppu_bus_read(nes, 0x3F0B);
    bg_palette[3][0] = ppu_bus_read(nes, 0x3F00);
    bg_palette[3][1] = ppu_bus_read(nes, 0x3F0D);
    bg_palette[3][2] = ppu_bus_read(nes, 0x3F0E);
    bg_palette[3][3] = ppu_bus_read(nes, 0x3F0F);

    // Left
    for (int i = 0; i < 256; i++) {
        uint16_t pattern_table_base = i << 4;
        for (uint8_t bit_plane = 0; bit_plane < TILE_SIZE; bit_plane++) {
            uint8_t lo = ppu_bus_read(nes, pattern_table_base | bit_plane); // low bits for 8 pixels
            uint8_t hi = ppu_bus_read(nes, pattern_table_base | TILE_SIZE | bit_plane); // high bits for 8 pixels

            for (uint8_t pixel_index = 0; pixel_index < TILE_SIZE; pixel_index++) {
                uint8_t pixel = (hi & 1) << 1 | lo & 1;
                lo >>= 1;
                hi >>= 1;

                uint16_t pixel_x = 8 * (i % 16) + (TILE_SIZE - 1 - pixel_index);
                uint16_t pixel_y = 8 * (i / 16) + bit_plane;
                uint32_t index = pixel_x + pixel_y * 32 * TILE_SIZE;
                pixels[index] = bg_palette[palette_id & 0b11][pixel];
            }
        }
    }

    // Right
    for (int i = 0; i < 256; i++) {
        uint16_t pattern_table_base = 0x1000 | i << 4;
        for (uint8_t bit_plane = 0; bit_plane < TILE_SIZE; bit_plane++) {
            uint8_t lo = ppu_bus_read(nes, pattern_table_base | bit_plane); // low bits for 8 pixels
            uint8_t hi = ppu_bus_read(nes, pattern_table_base | TILE_SIZE | bit_plane); // high bits for 8 pixels

            for (uint8_t pixel_index = 0; pixel_index < TILE_SIZE; pixel_index++) {
                uint8_t pixel = (hi & 1) << 1 | lo & 1;
                lo >>= 1;
                hi >>= 1;

                uint16_t pixel_x = 128 + 8 * (i % 16) + (7 - pixel_index);
                uint16_t pixel_y = 8 * (i / 16) + bit_plane;
                uint32_t index = pixel_x + pixel_y * 32 * TILE_SIZE;
                pixels[index] = bg_palette[palette_id & 0b11][pixel];
            }
        }
    }
}

/*uint8_t reverse_u8(uint8_t v) {
   v = (v & 0b11110000) >> 4 | (v & 0b00001111) << 4;
   v = (v & 0b11001100) >> 2 | (v & 0b00110011) << 2;
   v = (v & 0b10101010) >> 1 | (v & 0b01010101) << 1;
   return v;
}*/

void draw_oam_sprites(struct Nes* nes, uint8_t* pixels) {
    // constants
    uint16_t nametable_width = 32; // in tiles
    uint16_t nametable_height = 30; // in tiles
    uint16_t tile_size = 8; // in pixels
    uint16_t sprite_size = 8; // in pixels

    uint8_t spr_palette[4][4];
    spr_palette[0][0] = ppu_bus_read(nes, 0x3F10);
    spr_palette[0][1] = ppu_bus_read(nes, 0x3F11);
    spr_palette[0][2] = ppu_bus_read(nes, 0x3F12);
    spr_palette[0][3] = ppu_bus_read(nes, 0x3F13);
    spr_palette[1][0] = ppu_bus_read(nes, 0x3F14);
    spr_palette[1][1] = ppu_bus_read(nes, 0x3F15);
    spr_palette[1][2] = ppu_bus_read(nes, 0x3F16);
    spr_palette[1][3] = ppu_bus_read(nes, 0x3F17);
    spr_palette[2][0] = ppu_bus_read(nes, 0x3F18);
    spr_palette[2][1] = ppu_bus_read(nes, 0x3F19);
    spr_palette[2][2] = ppu_bus_read(nes, 0x3F1A);
    spr_palette[2][3] = ppu_bus_read(nes, 0x3F1B);
    spr_palette[3][0] = ppu_bus_read(nes, 0x3F1C);
    spr_palette[3][1] = ppu_bus_read(nes, 0x3F1D);
    spr_palette[3][2] = ppu_bus_read(nes, 0x3F1E);
    spr_palette[3][3] = ppu_bus_read(nes, 0x3F1F);

    // sprites
    for (uint8_t sprite = 0; sprite < 64; sprite++) {
        // Sprite y position.
        uint8_t byte0 = nes->oam[sprite * 4 + 0];
        // Sprite index information.
        uint8_t byte1 = nes->oam[sprite * 4 + 1];
        // Sprite attribute information.
        uint8_t byte2 = nes->oam[sprite * 4 + 2];
        // Sprite x position.
        uint8_t byte3 = nes->oam[sprite * 4 + 3];

        if (byte0 >= 0xEF || byte3 >= 0xF9) {
            continue;
        }

        //
        uint8_t palette_id = byte2 & 0b11;
        uint8_t flip_v = (byte2 & 0x80) > 0;
        uint8_t flip_h = (byte2 & 0x40) > 0;

        // Calculate pattern table base.
        uint8_t mode = (nes->ppuctrl & 0b00100000) > 0;
        uint16_t pattern_table_index = mode 
            ? (byte1 & 0xFE) << 4 // 8x16 mode
            : byte1 << 4; // 8x8 mode
        uint16_t pattern_table_page = mode 
            ? (byte1 & 0x01) << 12 // 8x16 mode
            : (nes->ppuctrl & 0b1000) << 9; // 8x8 mode
        uint16_t pattern_table_base = pattern_table_page | pattern_table_index;

        for (uint8_t spr = mode; spr != 255; spr--) {
            for (uint8_t bit_plane = 0; bit_plane < sprite_size; bit_plane++) {
                //
                uint8_t true_bit_plane = flip_v ? 7 - bit_plane : bit_plane;
                uint8_t low_bits = ppu_bus_read(nes, pattern_table_base | spr << 4 | true_bit_plane); // low bits for 8 pixels
                uint8_t high_bits = ppu_bus_read(nes, pattern_table_base | spr << 4 | sprite_size | true_bit_plane); // high bits for 8 pixels

                /*if (flip_h) {
                    low_bits = reverse_u8(low_bits);
                    high_bits = reverse_u8(high_bits);
                }*/

                //
                for (uint8_t pixel_index = 0; pixel_index < sprite_size; pixel_index++) {
                    //
                    uint8_t pixel = high_bits % 2 << 1 | low_bits % 2;
                    low_bits >>= 1;
                    high_bits >>= 1;
                    if (pixel == 0) continue;
    
                    //
                    uint16_t pixel_x = byte3 + sprite_size - 1 - pixel_index;
                    uint16_t pixel_y = 1 + byte0 + (flip_v ? -spr + 1 : spr) * 8 + bit_plane;
                    if (pixel_x >= 256 || pixel_y >= 240) continue;
                    uint32_t index = pixel_x + pixel_y * nametable_width * tile_size;
    
                    //

                    pixels[index] = spr_palette[palette_id][pixel];
                }
            }
        }
    }
}

// opcode ID to name lookup table
char* lookup_opcode(uint8_t op) {
    switch (op) {
        case 0x69: case 0x65: case 0x75: case 0x6D: case 0x7D: case 0x79: case 0x61: case 0x71: return "adc";
        case 0x29: case 0x25: case 0x35: case 0x2D: case 0x3D: case 0x39: case 0x21: case 0x31: return "and";
        case 0x0A: case 0x06: case 0x16: case 0x0E: case 0x1E: return "asl";
        case 0x24: case 0x2C: return "bit";
        case 0x10: return "bpl";
        case 0x30: return "bmi";
        case 0x50: return "bvc";
        case 0x70: return "bvs";
        case 0x90: return "bcc";
        case 0xB0: return "bcs";
        case 0xD0: return "bne";
        case 0xF0: return "beq";
        case 0x00: return "brk";
        case 0xC9: case 0xC5:  case 0xD5:  case 0xCD:  case 0xDD:  case 0xD9:  case 0xC1:  case 0xD1: return "cmp";
        case 0xE0: case 0xE4: case 0xEC: return "cpx";
        case 0xC0: case 0xC4: case 0xCC: return "cpy";
        case 0xC6: case 0xD6: case 0xCE: case 0xDE: return "dec";
        case 0x49: case 0x45: case 0x55: case 0x4D: case 0x5D: case 0x59: case 0x41: case 0x51: return "eor";
        case 0x18: return "clc";
        case 0x38: return "sec";
        case 0x58: return "cli";
        case 0x78: return "sei";
        case 0xB8: return "clv";
        case 0xD8: return "cld";
        case 0xF8: return "sed";
        case 0xE6: case 0xF6: case 0xEE: case 0xFE: return "inc";
        case 0x4C: case 0x6C: return "jmp";
        case 0x20: return "jsr";
        case 0xA9: case 0xA5: case 0xB5: case 0xAD: case 0xBD: case 0xB9: case 0xA1: case 0xB1: return "lda";
        case 0xA2: case 0xA6: case 0xB6: case 0xAE: case 0xBE: return "ldx";
        case 0xA0: case 0xA4: case 0xB4: case 0xAC: case 0xBC: return "ldy";
        case 0x4A: case 0x46: case 0x56: case 0x4E: case 0x5E: return "lsr";
        case 0xEA: case 0x1A: case 0x3A: case 0x5A: case 0x7A: case 0xDA: case 0xFA: case 0x04: case 0x44: case 0x64: case 0x0C: case 0x14: case 0x34: case 0x54: case 0x74: case 0xD4: case 0xF4: case 0x80: case 0x1C: case 0x3C: case 0x5C: case 0x7C: case 0xDC: case 0xFC: return "nop";
        case 0x09: case 0x05: case 0x15: case 0x0D: case 0x1D: case 0x19: case 0x01: case 0x11: return "ora";
        case 0xAA: return "tax";
        case 0x8A: return "txa";
        case 0xCA: return "dex";
        case 0xE8: return "inx";
        case 0xA8: return "tay";
        case 0x98: return "tya";
        case 0x88: return "dey";
        case 0xC8: return "iny";
        case 0x2A: case 0x26: case 0x36: case 0x2E: case 0x3E: return "rol";
        case 0x6A: case 0x66: case 0x76: case 0x6E: case 0x7E: return "ror";
        case 0x40: return "rti";
        case 0x60: return "rts";
        case 0xE9: case 0xE5: case 0xF5: case 0xED: case 0xFD: case 0xF9: case 0xE1: case 0xF1: return "sbc";
        case 0x85: case 0x95: case 0x8D: case 0x9D: case 0x99: case 0x81: case 0x91: return "sta";
        case 0x9A: return "txs";
        case 0xBA: return "tsx";
        case 0x48: return "pha";
        case 0x68: return "pla";
        case 0x08: return "php";
        case 0x28: return "plp";
        case 0x86: case 0x96: case 0x8E: return "stx"; 
        case 0x84: case 0x94: case 0x8C: return "sty";
        default: return "inv";
    }
}