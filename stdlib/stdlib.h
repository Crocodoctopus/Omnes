#include <stddef.h>

extern const unsigned char __heap_base;
extern void* js_grow(void*);

//void* __cdecl calloc(size_t, size_t);
void* __cdecl malloc(size_t);
//void* __cdecl realloc(void*, size_t);
void __cdecl free(void*);