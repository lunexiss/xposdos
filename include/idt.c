#include <stdint.h>
#include "idt.h"

extern void keyboard_callback();

struct idt_entry_t idt[256];
struct idt_ptr_t idt_ptr;

void idt_set_gate(unsigned char num, unsigned long base, unsigned short sel, unsigned char flags) {
    idt[num].base_lo = base & 0xFFFF;
    idt[num].base_hi = (base >> 16) & 0xFFFF;
    idt[num].sel     = sel;
    idt[num].always0 = 0;
    idt[num].flags   = flags;
}

void idt_init() {
    for (int i = 0; i < 256; i++) {
        idt_set_gate(i, 0, 0, 0);
    }

    // extern void mouse_irq_handler(); // this shit was on my old unfinished os
    idt_set_gate(0x21, (uint32_t)keyboard_callback, 0x08, 0x8E); // irq1 - keyboard

    idt_ptr.limit = sizeof(idt) - 1;
    idt_ptr.base = (uint32_t)&idt;
    idt_load();
}