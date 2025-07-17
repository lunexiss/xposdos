#pragma once

#include <stdint.h>

typedef void (*isr_handler_t)(void);
void register_interrupt_handler(uint8_t n, isr_handler_t handler);
