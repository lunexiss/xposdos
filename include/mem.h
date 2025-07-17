#ifndef MEM_H
#define MEM_H

#include <stddef.h>
#include <stdint.h>

void *kmalloc(size_t size);
void kfree(void *ptr);

#endif
