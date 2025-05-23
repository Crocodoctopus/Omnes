#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_surface.h>
#include <stdio.h>
#include <string.h>

#include "error.h"
#include "nes.h"
#include "getopt.c"
#include "settings.h"

#include <assert.h>
#include "mapper0.h"
#include "debug.h"

int main(int argc, char** argv) {
    // Slight delay, for IO reasons.
    SDL_Delay(400);

    // Parse arguments.
    char* filename = NULL;
    int opt = -1;
    while ((opt = getopt(argc, argv, "f:h")) != -1) {
        switch (opt) {
            case 'f':
                filename = optarg;
                break;
            case 'h':
                printf("  -f [arg_name]  --  Specifies rom filename as arg_name.\n");
                printf("  -h             --  Print command line arguments.\n");
                break;
        }
    }

    // Return early is no filename is set.
    if (filename == NULL) return -1;

    // Declare and load cartridge from file.
    struct Cartridge* cartridge;
    if (load_cartridge_from_file(filename, &cartridge) > 0) return -1;

    // Declare and initialize Nes struct.
    struct Nes nes;
    init_nes(&nes, cartridge);

    // Render
    uint8_t lookup[] = {
        84, 84, 84, 0, 30, 116, 8, 16, 144, 48, 0, 136, 68, 0, 100, 92, 0, 48, 84, 4, 0, 60, 24, 0, 32, 42, 0, 8, 58, 0, 0, 64, 0, 0, 60, 0, 0, 50, 60, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        152, 150, 152, 8, 76, 196, 48, 50, 236, 92, 30, 228, 136, 20, 176, 160, 20, 100, 152, 34, 32, 120, 60, 0, 84, 90, 0, 40, 114, 0, 8, 124, 0, 0, 118, 40, 0, 102, 120, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        236, 238, 236, 76, 154, 236, 120, 124, 236, 176, 98, 236, 228, 84, 236, 236, 88, 180, 236, 106, 100, 212, 136, 32, 160, 170, 0, 116, 196, 0, 76, 208, 32, 56, 204, 108, 56, 180, 204, 60, 60, 60, 0, 0, 0, 0, 0, 0, 
        236, 238, 236, 168, 204, 236, 188, 188, 236, 212, 178, 236, 236, 174, 236, 236, 174, 212, 236, 180, 176, 228, 196, 144, 204, 210, 120, 180, 222, 120, 168, 226, 144, 152, 226, 180, 160, 214, 228, 160, 162, 160, 0, 0, 0, 0, 0, 0
    };

    // Declare and load settings from file. A new settings is made if settings.conf is not found.
    struct Settings settings;
    settings_load_from_file(&settings, "settings.conf");

    // Initialize SDL vdieo.
    assert(SDL_Init(SDL_INIT_VIDEO) >= 0);

    // Create main SDL window.
    SDL_Window* game_window = SDL_CreateWindow("NES Emu", 1920/2, 1080/2, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    assert(game_window);

    // Create debug windows.
    SDL_Window* nametable_window = SDL_CreateWindow("NES Emu Nametable", 1920/2 + SCREEN_WIDTH, 1080/2, 2 * SCREEN_WIDTH, 2 * SCREEN_HEIGHT, 0);
    assert(nametable_window);
    SDL_Window* pattern_table_window = SDL_CreateWindow("NES Emu Pattern Table", 1920/2, 1080/2 + SCREEN_HEIGHT + 24, TILE_SIZE * 32, TILE_SIZE * 16, 0);
    assert(pattern_table_window);
    SDL_Window* sprite_window = SDL_CreateWindow("NES Emu PPU OAM", 1920/2, 1080/2 + SCREEN_HEIGHT + 24, 64 * 2, 64 * 2, 0);
    assert(sprite_window);

    // Declare space for PPU output, in PPU colors.
    uint8_t ppu_output[SCREEN_WIDTH * SCREEN_HEIGHT];

    int debug_scanline = 240;
    int debug_dot = 0;
    int debug_cycle = debug_scanline * 340 + debug_dot;

    // The main game loop. Runs at 60 Hz.
    while (1) {
        // For sleep timing.
        int start = SDL_GetTicks();

        // Step the NES 1 cycle.
        uint8_t vblank = step_nes(&nes, ppu_output);

        // Output debug info on the debug cycle.
        if (debug_cycle >= nes.cycle && debug_cycle <= nes.cycle + 2) {
            // Render nametable.
            uint8_t nametable_output[SCREEN_WIDTH * SCREEN_HEIGHT * 2 * 2];
            draw_ppu_nametables(&nes, nametable_output);
            SDL_Surface* s = SDL_GetWindowSurface(nametable_window);
            for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT * 2 * 2; i++) {
                ((uint8_t*)s->pixels)[i * 4 + 2] = lookup[nametable_output[i] * 3 + 0];
                ((uint8_t*)s->pixels)[i * 4 + 1] = lookup[nametable_output[i] * 3 + 1];
                ((uint8_t*)s->pixels)[i * 4 + 0] = lookup[nametable_output[i] * 3 + 2];
            }
            SDL_UpdateWindowSurface(nametable_window);

            // Render pattern table.
            uint8_t pattern_table_output[512 * TILE_SIZE * TILE_SIZE];
            draw_ppu_pattern_tables(&nes, pattern_table_output, 0);
            SDL_Surface* s2 = SDL_GetWindowSurface(pattern_table_window);
            for (int i = 0; i < 512 * TILE_SIZE * TILE_SIZE; i++) {
                ((uint8_t*)s2->pixels)[i * 4 + 2] = lookup[pattern_table_output[i] * 3 + 0];
                ((uint8_t*)s2->pixels)[i * 4 + 1] = lookup[pattern_table_output[i] * 3 + 1];
                ((uint8_t*)s2->pixels)[i * 4 + 0] = lookup[pattern_table_output[i] * 3 + 2];
            }
            SDL_UpdateWindowSurface(pattern_table_window);
            
            // Debug sprites.
            uint8_t sprite_output[64 * 64 * 4];
            memset(sprite_output, 37, 64 * 64 * 4);
            draw_oam_sprites(&nes, sprite_output);
            SDL_Surface* s3 = SDL_GetWindowSurface(sprite_window);
            for (int i = 0; i < 64 * 64 * 4; i++) {
                ((uint8_t*)s3->pixels)[i * 4 + 2] = lookup[sprite_output[i] * 3 + 0];
                ((uint8_t*)s3->pixels)[i * 4 + 1] = lookup[sprite_output[i] * 3 + 1];
                ((uint8_t*)s3->pixels)[i * 4 + 0] = lookup[sprite_output[i] * 3 + 2];
            }
            SDL_UpdateWindowSurface(sprite_window);
        }

        // Render screen info on vblank cycle.
        if (vblank) {
            // Poll window events.
            int keys = 0;
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_WINDOWEVENT) {
                    switch (event.window.event) {
                        case SDL_WINDOWEVENT_MOVED: {
                            nlog("moved");
                        } break;
                        case SDL_WINDOWEVENT_CLOSE: {
                            goto exit;
                        } break;
                    }
                }
            }

            // Check keyboard input, pack and record it in Nes object.
            const uint8_t* keyboard = SDL_GetKeyboardState(&keys);
            uint8_t state = 0;
            state |= keyboard[settings.right] << 7;
            state |= keyboard[settings.left] << 6;
            state |= keyboard[settings.down] << 5;
            state |= keyboard[settings.up] << 4;
            state |= keyboard[settings.start] << 3;
            state |= keyboard[settings.select] << 2;
            state |= keyboard[settings.b] << 1;
            state |= keyboard[settings.a] << 0;
            nes.input1 = state; 

            // Render game area.
            SDL_Surface* screen = SDL_GetWindowSurface(game_window);
            for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
                ((uint8_t*)screen->pixels)[i * 4 + 2] = lookup[ppu_output[i] * 3 + 0];
                ((uint8_t*)screen->pixels)[i * 4 + 1] = lookup[ppu_output[i] * 3 + 1];
                ((uint8_t*)screen->pixels)[i * 4 + 0] = lookup[ppu_output[i] * 3 + 2];
            }
            
            SDL_UpdateWindowSurface(game_window); 

            // Sleep for (16ms - time_taken).
            int end = SDL_GetTicks();
            uint8_t wait = 13 - end + start;
            nlog("%i", wait);
            if (wait < 0x80)
                SDL_Delay(wait);
        };
    }

exit:
    // Write settings to settings.conf.
    settings_write_to_file(&settings, "settings.conf");

    // Free SDL windows.
    SDL_DestroyWindow(sprite_window);
    SDL_DestroyWindow(game_window);
    SDL_DestroyWindow(nametable_window);
    //SDL_DestroyWindow(pattern_table_window);

    // Free Nes object.
    free_nes(&nes);
}
