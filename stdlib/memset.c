#include <stddef.h>
#include <stdint.h>

void* __cdecl memset(void* str, int c, size_t n) {
	char* ptr = str;
	for (int i = 0; i < n; i++) {
		ptr[i] = c;
	}
	return str;
}