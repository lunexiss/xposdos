#include <stdint.h>
#include "string.h"
#include "vga.h"
#include "io.h"

int cursor_x = 0;
int cursor_y = 0;

uint16_t* vga = (uint16_t*)VGA_ADDRESS;
uint8_t current_color = WHITE_ON_BLACK;

// color name to vga color code :3
uint8_t color_from_name(const char* name) {
    if (strcmp(name, "black") == 0) return 0x00;
    if (strcmp(name, "blue") == 0) return 0x01;
    if (strcmp(name, "green") == 0) return 0x02;
    if (strcmp(name, "cyan") == 0) return 0x03;
    if (strcmp(name, "red") == 0) return 0x04;
    if (strcmp(name, "magenta") == 0) return 0x05;
    if (strcmp(name, "yellow") == 0) return 0x0E;
    if (strcmp(name, "white") == 0) return 0x0F;
    return current_color;
}

void move_cursor() {
    uint16_t pos = cursor_y * VGA_WIDTH + cursor_x;
    outb(0x3D4, 14);
    outb(0x3D5, pos >> 8);
    outb(0x3D4, 15);
    outb(0x3D5, pos);
}

void scroll() {
    for (int y = 1; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            vga[(y - 1) * VGA_WIDTH + x] = vga[y * VGA_WIDTH + x];
        }
    }
    for (int x = 0; x < VGA_WIDTH; x++) {
        vga[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = ' ' | (current_color << 8);
    }
    cursor_y = VGA_HEIGHT - 1;
}

void clear_screen() {
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++)
        vga[i] = ' ' | (current_color << 8);

    cursor_x = 0;
    cursor_y = 0;
    move_cursor();
}

void put_char(char c) {
    volatile char* video = (volatile char*) VGA_ADDRESS;

    if (cursor_y >= VGA_HEIGHT)
        scroll();

    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else if (c == '\b') {
        if (cursor_x > 0) {
            cursor_x--;
        } else if (cursor_y > 0) {
            cursor_y--;
            cursor_x = VGA_WIDTH - 1;
        }
        int pos = cursor_y * VGA_WIDTH + cursor_x;
        video[2 * pos] = ' ';
        video[2 * pos + 1] = WHITE_ON_BLACK;
    } else {
        int pos = cursor_y * VGA_WIDTH + cursor_x;
        video[2 * pos] = c;
        video[2 * pos + 1] = WHITE_ON_BLACK;
        cursor_x++;
        if (cursor_x >= VGA_WIDTH) {
            cursor_x = 0;
            cursor_y++;
        }
    }

    if (cursor_y >= VGA_HEIGHT)
        scroll();

    move_cursor();
}

void put_char_color(char c, uint8_t color) {
    if (cursor_y >= VGA_HEIGHT) scroll();

    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else if (c == '\b') {
        if (cursor_x > 0) {
            cursor_x--;
        } else if (cursor_y > 0) {
            cursor_y--;
            cursor_x = VGA_WIDTH - 1;
        }
        int pos = cursor_y * VGA_WIDTH + cursor_x;
        vga[pos] = ' ' | (color << 8);
    } else {
        int pos = cursor_y * VGA_WIDTH + cursor_x;
        vga[pos] = c | (color << 8);
        cursor_x++;
        if (cursor_x >= VGA_WIDTH) {
            cursor_x = 0;
            cursor_y++;
        }
    }

    if (cursor_y >= VGA_HEIGHT) scroll();

    move_cursor();
}

void print(const char* str) {
    while (*str) {
        if (*str == '<') {
            str++;
            int closing = 0;
            if (*str == '/') {
                closing = 1;
                str++;
            }

            char tag[16] = {0};
            int i = 0;
            while (*str && *str != '>' && i < 15) {
                tag[i++] = *str++;
            }

            if (*str == '>') str++;

            if (closing) {
                current_color = WHITE_ON_BLACK;
            } else {
                current_color = color_from_name(tag);
            }
        } else {
            put_char_color(*str++, current_color);
        }
    }
}

void print_char(char c) {
    put_char_color(c, current_color);
}

void print_dec(uint32_t num) {
    char buf[11];
    int i = 0;

    if (num == 0) {
        print("0");
        return;
    }

    while (num > 0 && i < 10) {
        buf[i++] = '0' + (num % 10);
        num /= 10;
    }

    for (int j = i - 1; j >= 0; j--) {
        print_char(buf[j]);
    }
}

void vga_init() {
    clear_screen();
}

void write_regs(uint8_t *regs) {
    // misc
    outb(VGA_MISC_WRITE, *regs++);
    
    // sequencer
    for (uint8_t i = 0; i < 5; i++) {
        outb(VGA_SEQ_INDEX, i);
        outb(VGA_SEQ_DATA, *regs++);
    }

    // unlock shit
    outb(VGA_CRTC_INDEX, 0x03);
    outb(VGA_CRTC_DATA, inb(VGA_CRTC_DATA) | 0x80);
    outb(VGA_CRTC_INDEX, 0x11);
    outb(VGA_CRTC_DATA, inb(VGA_CRTC_DATA) & ~0x80);
    
    for (uint8_t i = 0; i < 25; i++) {
        outb(VGA_CRTC_INDEX, i);
        outb(VGA_CRTC_DATA, *regs++);
    }

    // graphics controller
    for (uint8_t i = 0; i < 9; i++) {
        outb(VGA_GC_INDEX, i);
        outb(VGA_GC_DATA, *regs++);
    }

    // attribute controller
    for (uint8_t i = 0; i < 21; i++) {
        (void)inb(VGA_INPUT_STATUS1); // reset flipflop
        outb(VGA_AC_INDEX, i);
        outb(VGA_AC_WRITE, *regs++);
    }

    // enable video output
    (void)inb(VGA_INPUT_STATUS1);
    outb(VGA_AC_INDEX, 0x20);
}

void set_graphics_mode() {
    uint8_t graphics_mode_regs[] = {
        0x63,
        0x03, 0x01, 0x0F, 0x00, 0x0E,
        0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0xBF, 0x1F,
        0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x9C, 0x0E, 0x8F, 0x28, 0x40, 0x96, 0xB9, 0xA3,
        0xFF,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x0F,
        0xFF,
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
        0x41, 0x00, 0x0F, 0x00, 0x00
    };
    
    write_regs(graphics_mode_regs);
}

void put_pixel(int x, int y, uint8_t color) {
    if (x >= 0 && x < 320 && y >= 0 && y < 200) {
        uint8_t* vga_graphics = (uint8_t*)VGA_GRAPHICS_ADDRESS;
        vga_graphics[y * 320 + x] = color;
    }
}

void clear_graphics_screen(uint8_t color) {
    uint8_t* vga_graphics = (uint8_t*)VGA_GRAPHICS_ADDRESS;
    for (int i = 0; i < 320 * 200; i++) {
        vga_graphics[i] = color;
    }
}

void switch_to_text_mode() {
    uint8_t text_mode_regs[] = {
        0x67,
        0x03, 0x00, 0x03, 0x00, 0x02,
        0x5F, 0x4F, 0x50, 0x82, 0x55, 0x81, 0xBF, 0x1F,
        0x00, 0x4F, 0x0D, 0x0E, 0x00, 0x00, 0x00, 0x50,
        0x9C, 0x0E, 0x8F, 0x28, 0x1F, 0x96, 0xB9, 0xA3,
        0xFF,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x0E, 0x00,
        0xFF,
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07,
        0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
        0x0C, 0x00, 0x0F, 0x08, 0x00
    };

    write_regs(text_mode_regs);
}