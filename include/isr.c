#include "isr.h"
#include <stdint.h>

isr_handler_t interrupt_handlers[256] = { 0 };

void register_interrupt_handler(uint8_t n, isr_handler_t handler) {
    interrupt_handlers[n] = handler;
}
