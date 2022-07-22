#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>

struct Node {
	struct Node* next;
	size_t size;
	uint8_t data[0];
};

void* get_heap_base() {
	return (void*)&__heap_base;
}

size_t align8(size_t v) {
	return v + (8 - v & 0b111);
}

__cdecl void* malloc(size_t size) {
    if (size == 0) return NULL;
    
    // round size up to nearest 64 bit
    size = align8(size);

    // find space
    struct Node* node = get_heap_base();
    while (1) {
        size_t node_size = sizeof(struct Node) + node->size;
        
        // if next node is null, insert there
        if (node->next == NULL) {
            // TODO: check if space exists
            
            node->next = node + node_size;
            node->next->next = NULL;
            node->next->size = size;
            
            return &node->next->data;
        }
        
        // if space exists between current and next node
        if (node->next - node - node_size >= size) {
            struct Node* old_next = node->next;
            
            node->next = node + node_size;
            node->next->next = old_next;
            node->next->size = size;
            
            return &node->next->data;
        }
        
        node = node->next;
    }
}

void print_node_chain() {
	return;

    struct Node* node = get_heap_base();
    do {
        printf("%lX -> struct Node { struct Node* next = %p; size_t size = %li; uint8_t data[%li] = .. }\n", (void*)node - get_heap_base(), node->next, node->size, node->size);
        node = node->next;
    } while(node != NULL);
}