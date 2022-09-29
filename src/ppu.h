// NES component step functions:
void step_ppu(struct Nes*, uint8_t*, uint8_t*);

// Functions emulating the PPU bus (https://wiki.nesdev.org/w/index.php/PPU_memory_map).
uint8_t ppu_bus_read(struct Nes*, uint16_t);
void ppu_bus_write(struct Nes*, uint16_t, uint8_t);

// (Used internally)
void ppu_inc_x_scroll(struct Nes*);
void ppu_inc_y_scroll(struct Nes*);
void ppu_restore_x_scroll(struct Nes*);
void ppu_restore_y_scroll(struct Nes*);