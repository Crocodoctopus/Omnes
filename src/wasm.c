#include "nes.h"
#include "cartridge.h"
#include "defines.h"
#include "debug.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "mapper0.h"

#define WASM_EXPORT __attribute__((visibility("default"))) 

WASM_EXPORT void* wasm_malloc(size_t size) {
    return malloc(size);
}

WASM_EXPORT void wasm_free(void* ptr) {
    free(ptr);
}

WASM_EXPORT const size_t sizeof_Nes = sizeof(struct Nes);

WASM_EXPORT int wasm_init_nes(struct Nes* nes, void* rom) {
    // load and check cartridge
    struct Cartridge* cartridge;
    int err = load_cartridge_from_data(rom, rom + 16, &cartridge);
    if (err > 0) return err;

    // initialize nes
    init_nes(nes, cartridge);

    // 
    return 0;
}

WASM_EXPORT int update_nes(struct Nes* nes, uint8_t input1, uint8_t* screen_pixels) {
    // temporary buffer for outputted pixels, in indices
    uint8_t nes_pixels[SCREEN_WIDTH * SCREEN_HEIGHT];
       
    // nes -> rgb mapping
    uint8_t index_to_rgb[] = {
        84, 84, 84, 0, 30, 116, 8, 16, 144, 48, 0, 136, 68, 0, 100, 92, 0, 48, 84, 4, 0, 60, 24, 0, 32, 42, 0, 8, 58, 0, 0, 64, 0, 0, 60, 0, 0, 50, 60, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        152, 150, 152, 8, 76, 196, 48, 50, 236, 92, 30, 228, 136, 20, 176, 160, 20, 100, 152, 34, 32, 120, 60, 0, 84, 90, 0, 40, 114, 0, 8, 124, 0, 0, 118, 40, 0, 102, 120, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        236, 238, 236, 76, 154, 236, 120, 124, 236, 176, 98, 236, 228, 84, 236, 236, 88, 180, 236, 106, 100, 212, 136, 32, 160, 170, 0, 116, 196, 0, 76, 208, 32, 56, 204, 108, 56, 180, 204, 60, 60, 60, 0, 0, 0, 0, 0, 0, 
        236, 238, 236, 168, 204, 236, 188, 188, 236, 212, 178, 236, 236, 174, 236, 236, 174, 212, 236, 180, 176, 228, 196, 144, 204, 210, 120, 180, 222, 120, 168, 226, 144, 152, 226, 180, 160, 214, 228, 160, 162, 160, 0, 0, 0, 0, 0, 0
    };

    // update input
    nes->input1 = input1;

    // complete one frame
    for (int i = 0; i < 100000; i++) {
        if (nes->oam_delay == 0) step_cpu(nes);
        else nes->oam_delay -= 1;
        uint8_t vblank = 0;
        step_ppu(nes, nes_pixels, &vblank);
        step_ppu(nes, nes_pixels, &vblank);
        step_ppu(nes, nes_pixels, &vblank);

        if (vblank) break;
    }

    // convert NES PPU output to RGB
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
        screen_pixels[3 * i + 0] = index_to_rgb[3 * nes_pixels[i] + 0];
        screen_pixels[3 * i + 1] = index_to_rgb[3 * nes_pixels[i] + 1];
        screen_pixels[3 * i + 2] = index_to_rgb[3 * nes_pixels[i] + 2];
    }

    // return error codes maybe?
    return 0;
}