#include "mem.h"
#include <stdint.h>
#include <stddef.h>

typedef unsigned int uintptr_t;

extern uint8_t _end; // this shit is in link.ld
static uintptr_t heap = (uintptr_t)&_end;

void* kmalloc(size_t size) {
    void *mem = (void*)heap;
    heap += size;
    return mem;
}

void kfree(void *ptr) {
    // i'm lazy to code this
}
