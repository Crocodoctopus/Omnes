#include <stddef.h>
#include <stdint.h>

void* __cdecl memcpy (void* dest, const void* src, size_t n) {
    uint8_t* dest_u8 = (uint8_t*)dest;
    uint8_t* src_u8 = (uint8_t*)src;
    for (int i = 0; i < n; i++) {
        *dest_u8 = *src_u8;
        dest_u8 += 1;
        src_u8 += 1;
    }
    return dest;
}