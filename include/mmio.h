#ifndef MMIO_H
#define MMIO_H

#include <stdint.h>

typedef unsigned int uintptr_t;

#define MMIO8(addr)  (*(volatile uint8_t*)(addr))
#define MMIO16(addr) (*(volatile uint16_t*)(addr))
#define MMIO32(addr) (*(volatile uint32_t*)(addr))
#define MMIO64(addr) (*(volatile uint64_t*)(addr))

static inline void mmio_write32(uintptr_t addr, uint32_t val) {
    MMIO32(addr) = val;
}

static inline uint32_t mmio_read32(uintptr_t addr) {
    return MMIO32(addr);
}

#endif
