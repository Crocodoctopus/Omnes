#include <assert.h>

void assert(int expression) {
	if (!expression) {
		js_assert();
	}
}