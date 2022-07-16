#include "error.h"
#include "settings.h"
#include <stdio.h>

int settings_load_from_file(struct Settings* settings, const char* filename) {
	FILE* fp = NULL;

	// parameter validation
	if (filename == NULL) return error(INVALID_INPUT, "Null filename");
	if (settings == NULL) return error(INVALID_INPUT, "Null settings");

	//
	settings_create_from_default(settings);

	// open file
	fp = fopen(filename, "rb");
	if (fp == NULL) return error(IO_ERROR, "Could not open \"%.32s\"", filename);

	// read
	fread(settings, 1, sizeof(struct Settings), fp);

	return 0;
}

void settings_create_from_default(struct Settings* settings) {
	settings->a = SDL_SCANCODE_Z;
	settings->b = SDL_SCANCODE_X;
	settings->start = SDL_SCANCODE_RETURN;
	settings->select = SDL_SCANCODE_RSHIFT;
	settings->up = SDL_SCANCODE_UP;
	settings->down = SDL_SCANCODE_DOWN;
	settings->left = SDL_SCANCODE_LEFT;
	settings->right = SDL_SCANCODE_RIGHT;
}

int settings_write_to_file(struct Settings* settings, const char* filename) {
	FILE* fp = NULL;

	// parameter validation
	if (filename == NULL) return error(INVALID_INPUT, "Null filename");
	if (settings == NULL) return error(INVALID_INPUT, "Null settings");

	// open file
	fp = fopen(filename, "wb");
	if (fp == NULL) return error(IO_ERROR, "Could not create \"%.32s\"", filename); 

	// write to file (endianess might cause problems)
	fwrite(settings, 1, sizeof(struct Settings), fp);

	// close
	fclose(fp);

	return 0;
}