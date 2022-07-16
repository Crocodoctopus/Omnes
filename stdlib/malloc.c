#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>

__attribute__((aligned(8))) uint8_t __data[1048576]; // 10 MB
uint8_t* __head = __data + 1048576;

void* __cdecl malloc(size_t size) {
	__head -= size;
	return __head;
}

/*struct Node {
	Node* next;
	size_t size;
}

Node start = Node { 0, &__heap_base - &start };
size_t __heap_end = &__heap_base;

size_t align8(size_t v) {
	return v / 8 + 8;
}

void* __cdecl malloc(size_t size) {
	// calculate the size of the block
	size_t block_size = align8(sizeof(Node) + size);

	// find open block
	Node* node = Node;
	while (node->next != 0) {
		if (node + size + block_size < node->next) break;
		node = node->next;
	}

	if (node->next)


	uint32_t true_size = sizeof(node) + (size)
	if (__heap_end - &__heap_base < size) {
		__heap_end = grow(size);
	}

	grow(5);
	return 0;
}*/