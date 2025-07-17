#ifndef VGA_H
#define VGA_H

#include <stdint.h>

#define VGA_WIDTH 80
#define VGA_HEIGHT 25

#define VGA_AC_INDEX      0x3C0
#define VGA_AC_WRITE      0x3C0
#define VGA_AC_READ       0x3C1
#define VGA_MISC_WRITE    0x3C2
#define VGA_SEQ_INDEX     0x3C4
#define VGA_SEQ_DATA      0x3C5
#define VGA_DAC_WRITE_INDEX 0x3C8
#define VGA_DAC_DATA      0x3C9
#define VGA_GC_INDEX      0x3CE
#define VGA_GC_DATA       0x3CF
#define VGA_CRTC_INDEX    0x3D4
#define VGA_CRTC_DATA     0x3D5
#define VGA_INPUT_STATUS1 0x3DA

extern int cursor_row;
extern int cursor_col;
extern uint16_t* vga;

void clear_screen();
void print(const char* str);
void print_char(char c);
void put_char(char c);
void move_cursor();
void clear_input_line(int length);
void print_dec(uint32_t num);
void draw_char(int x, int y, char c);
void vga_init();
void scroll();
void put_char_color(char c, uint8_t color);

#endif
