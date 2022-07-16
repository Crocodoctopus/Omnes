#pragma once

#include <SDL2/SDL.h>

struct Settings {
	// Keybinds.
	SDL_Scancode a;
	SDL_Scancode b;
	SDL_Scancode start;
	SDL_Scancode select;
	SDL_Scancode up;
	SDL_Scancode down;
	SDL_Scancode left;
	SDL_Scancode right;
};

int settings_load_from_file(struct Settings*, const char*);
void settings_create_from_default(struct Settings*);
int settings_write_to_file(struct Settings*, const char*);