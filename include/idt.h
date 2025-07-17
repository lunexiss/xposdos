#ifndef IDT_H
#define IDT_H

#include <stdint.h>

struct idt_entry_t {
    uint16_t base_lo;   // lower 16 bits of the handler function address
    uint16_t sel;       // kernel segment selector
    uint8_t  always0;   // this must always be zero
    uint8_t  flags;     // cool flags: type and attributes
    uint16_t base_hi;   // upper 16 bits of the handler function address
} __attribute__((packed));

struct idt_ptr_t {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

extern struct idt_entry_t idt[256];
extern struct idt_ptr_t idt_ptr;

void idt_set_gate(unsigned char num, unsigned long base, unsigned short sel, unsigned char flags);
void idt_init();
extern void idt_load();

#endif
