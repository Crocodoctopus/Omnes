#pragma once

#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

#define NAMETABLE_WIDTH 32 // In tiles.
#define NAMETABLE_HEIGHT 30 // In tiles.
#define TILE_SIZE 8 // In pixels.
#define SPRITE_SIZE 8 // In pixels.
#define SCREEN_WIDTH (NAMETABLE_WIDTH * TILE_SIZE) // In pixels.
#define SCREEN_HEIGHT (NAMETABLE_HEIGHT * TILE_SIZE) // In pixels.
