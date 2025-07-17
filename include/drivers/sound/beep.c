#include <stdint.h>
#include <stddef.h>
#include "../../io.h"
#include "beep.h"
#include "../../isr.h"

volatile uint32_t ticks = 0;

void timer_callback() {
    ticks++;
}

void init_timer(uint32_t freq) {
    uint32_t divisor = 1193180 / freq;
    outb(0x43, 0x36);
    outb(0x40, divisor & 0xFF);
    outb(0x40, (divisor >> 8) & 0xFF);

    register_interrupt_handler(32, timer_callback);
}

static void play_sound(uint32_t nFrequence) {
	uint32_t Div;
	uint8_t tmp;

 	Div = 1193180 / nFrequence;
 	outb(0x43, 0xb6);
 	outb(0x42, (uint8_t) (Div) );
 	outb(0x42, (uint8_t) (Div >> 8));
 
 	tmp = inb(0x61);
  	if (tmp != (tmp | 3)) {
 		outb(0x61, tmp | 3);
 	}
}
 
// make it shut up
void nosound() {
	uint8_t tmp = inb(0x61) & 0xFC;
    
    outb(0x61, tmp);
}

// make a beep
void beep() {
	play_sound(1000);
	//init_timer(100);
	//nosound();
}