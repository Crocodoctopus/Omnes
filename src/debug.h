#pragma once

#include <stdint.h>
#include "nes.h"

void draw_ppu_nametables(struct Nes* nes, uint8_t pixels[4 * SCREEN_WIDTH * SCREEN_HEIGHT]);
void draw_ppu_pattern_tables(struct Nes* nes, uint8_t pixels[512 * TILE_SIZE * TILE_SIZE], uint8_t);
void draw_oam_sprites(struct Nes* nes, uint8_t pixels[4 * SCREEN_WIDTH * SCREEN_HEIGHT]);
char* lookup_opcode(uint8_t);