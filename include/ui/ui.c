#include "ui.h"
#include "vga.h"
#include "string.h"

// font by claude :3 (i suck at bitmap shit)
static uint8_t font_8x8[][8] = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // Space
    {0x18, 0x3C, 0x3C, 0x18, 0x18, 0x00, 0x18, 0x00}, // !
    {0x36, 0x36, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // "
    {0x36, 0x36, 0x7F, 0x36, 0x7F, 0x36, 0x36, 0x00}, // #
    {0x0C, 0x3E, 0x03, 0x1E, 0x30, 0x1F, 0x0C, 0x00}, // $
    {0x00, 0x63, 0x33, 0x18, 0x0C, 0x66, 0x63, 0x00}, // %
    {0x1C, 0x36, 0x1C, 0x6E, 0x3B, 0x33, 0x6E, 0x00}, // &
    {0x06, 0x06, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00}, // '
    {0x18, 0x0C, 0x06, 0x06, 0x06, 0x0C, 0x18, 0x00}, // (
    {0x06, 0x0C, 0x18, 0x18, 0x18, 0x0C, 0x06, 0x00}, // )
    {0x00, 0x66, 0x3C, 0xFF, 0x3C, 0x66, 0x00, 0x00}, // *
    {0x00, 0x0C, 0x0C, 0x3F, 0x0C, 0x0C, 0x00, 0x00}, // +
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x06, 0x00}, // ,
    {0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x00}, // -
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C, 0x00}, // .
    {0x60, 0x30, 0x18, 0x0C, 0x06, 0x03, 0x01, 0x00}, // /
    {0x3E, 0x63, 0x73, 0x7B, 0x6F, 0x67, 0x3E, 0x00}, // 0
    {0x0C, 0x0E, 0x0C, 0x0C, 0x0C, 0x0C, 0x3F, 0x00}, // 1
    {0x1E, 0x33, 0x30, 0x1C, 0x06, 0x33, 0x3F, 0x00}, // 2
    {0x1E, 0x33, 0x30, 0x1C, 0x30, 0x33, 0x1E, 0x00}, // 3
    {0x38, 0x3C, 0x36, 0x33, 0x7F, 0x30, 0x78, 0x00}, // 4
    {0x3F, 0x03, 0x1F, 0x30, 0x30, 0x33, 0x1E, 0x00}, // 5
    {0x1C, 0x06, 0x03, 0x1F, 0x33, 0x33, 0x1E, 0x00}, // 6
    {0x3F, 0x33, 0x30, 0x18, 0x0C, 0x0C, 0x0C, 0x00}, // 7
    {0x1E, 0x33, 0x33, 0x1E, 0x33, 0x33, 0x1E, 0x00}, // 8
    {0x1E, 0x33, 0x33, 0x3E, 0x30, 0x18, 0x0E, 0x00}, // 9
    {0x00, 0x0C, 0x0C, 0x00, 0x00, 0x0C, 0x0C, 0x00}, // :
    {0x00, 0x0C, 0x0C, 0x00, 0x00, 0x0C, 0x06, 0x00}, // ;
    {0x18, 0x0C, 0x06, 0x03, 0x06, 0x0C, 0x18, 0x00}, // <
    {0x00, 0x00, 0x3F, 0x00, 0x00, 0x3F, 0x00, 0x00}, // =
    {0x06, 0x0C, 0x18, 0x30, 0x18, 0x0C, 0x06, 0x00}, // >
    {0x1E, 0x33, 0x30, 0x18, 0x0C, 0x00, 0x0C, 0x00}, // ?
    {0x3E, 0x63, 0x7B, 0x7B, 0x7B, 0x03, 0x1E, 0x00}, // @
    {0x0C, 0x1E, 0x33, 0x33, 0x3F, 0x33, 0x33, 0x00}, // A
    {0x3F, 0x66, 0x66, 0x3E, 0x66, 0x66, 0x3F, 0x00}, // B
    {0x3C, 0x66, 0x03, 0x03, 0x03, 0x66, 0x3C, 0x00}, // C
    {0x1F, 0x36, 0x66, 0x66, 0x66, 0x36, 0x1F, 0x00}, // D
    {0x7F, 0x46, 0x16, 0x1E, 0x16, 0x46, 0x7F, 0x00}, // E
    {0x7F, 0x46, 0x16, 0x1E, 0x16, 0x06, 0x0F, 0x00}, // F
    {0x3C, 0x66, 0x03, 0x03, 0x73, 0x66, 0x7C, 0x00}, // G
    {0x33, 0x33, 0x33, 0x3F, 0x33, 0x33, 0x33, 0x00}, // H
    {0x1E, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00}, // I
    {0x78, 0x30, 0x30, 0x30, 0x33, 0x33, 0x1E, 0x00}, // J
    {0x67, 0x66, 0x36, 0x1E, 0x36, 0x66, 0x67, 0x00}, // K
    {0x0F, 0x06, 0x06, 0x06, 0x46, 0x66, 0x7F, 0x00}, // L
    {0x63, 0x77, 0x7F, 0x7F, 0x6B, 0x63, 0x63, 0x00}, // M
    {0x63, 0x67, 0x6F, 0x7B, 0x73, 0x63, 0x63, 0x00}, // N
    {0x1C, 0x36, 0x63, 0x63, 0x63, 0x36, 0x1C, 0x00}, // O
    {0x3F, 0x66, 0x66, 0x3E, 0x06, 0x06, 0x0F, 0x00}, // P
    {0x1E, 0x33, 0x33, 0x33, 0x3B, 0x1E, 0x38, 0x00}, // Q
    {0x3F, 0x66, 0x66, 0x3E, 0x36, 0x66, 0x67, 0x00}, // R
    {0x1E, 0x33, 0x07, 0x0E, 0x38, 0x33, 0x1E, 0x00}, // S
    {0x3F, 0x2D, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00}, // T
    {0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x3F, 0x00}, // U
    {0x33, 0x33, 0x33, 0x33, 0x33, 0x1E, 0x0C, 0x00}, // V
    {0x63, 0x63, 0x63, 0x6B, 0x7F, 0x77, 0x63, 0x00}, // W
    {0x63, 0x63, 0x36, 0x1C, 0x1C, 0x36, 0x63, 0x00}, // X
    {0x33, 0x33, 0x33, 0x1E, 0x0C, 0x0C, 0x1E, 0x00}, // Y
    {0x7F, 0x63, 0x31, 0x18, 0x4C, 0x66, 0x7F, 0x00}, // Z
};

void ui_init() {
    set_graphics_mode();
    ui_clear_screen(UI_BLUE);
}

void ui_clear_screen(uint8_t color) {
    uint8_t* vga_graphics = (uint8_t*)VGA_GRAPHICS_ADDRESS;
    for (int i = 0; i < 320 * 200; i++) {
        vga_graphics[i] = color;
    }
}

void ui_draw_pixel(int x, int y, uint8_t color) {
    if (x >= 0 && x < 320 && y >= 0 && y < 200) {
        uint8_t* vga_graphics = (uint8_t*)VGA_GRAPHICS_ADDRESS;
        vga_graphics[y * 320 + x] = color;
    }
}

void ui_draw_rect(int x, int y, int width, int height, uint8_t color) {
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            ui_draw_pixel(x + j, y + i, color);
        }
    }
}

void ui_draw_rect_outline(int x, int y, int width, int height, uint8_t color) {
    for (int i = 0; i < width; i++) {
        ui_draw_pixel(x + i, y, color);
        ui_draw_pixel(x + i, y + height - 1, color);
    }

    for (int i = 0; i < height; i++) {
        ui_draw_pixel(x, y + i, color);
        ui_draw_pixel(x + width - 1, y + i, color);
    }
}

void ui_draw_line(int x1, int y1, int x2, int y2, uint8_t color) {
    int dx = x2 - x1;
    int dy = y2 - y1;
    int steps = (dx > dy) ? dx : dy;
    if (steps < 0) steps = -steps;
    
    float x_inc = (float)dx / steps;
    float y_inc = (float)dy / steps;
    
    float x = x1;
    float y = y1;
    
    for (int i = 0; i <= steps; i++) {
        ui_draw_pixel((int)x, (int)y, color);
        x += x_inc;
        y += y_inc;
    }
}

// yay window
void ui_create_window(window_t* win, int x, int y, int width, int height, 
                      const char* title, uint8_t bg_color, uint8_t border_color) {
    win->x = x;
    win->y = y;
    win->width = width;
    win->height = height;
    win->bg_color = bg_color;
    win->border_color = border_color;
    win->visible = 1;
    win->has_titlebar = 1;
    
    int i = 0;
    while (title[i] && i < 63) {
        win->title[i] = title[i];
        i++;
    }
    win->title[i] = '\0';
}

void ui_draw_window(window_t* win) {
    if (!win->visible) return;
    
    ui_draw_rect(win->x, win->y, win->width, win->height, win->bg_color);
    
    ui_draw_rect_outline(win->x, win->y, win->width, win->height, win->border_color);
    
    if (win->has_titlebar) {
        ui_draw_titlebar(win);
    }
}

void ui_draw_titlebar(window_t* win) {
    ui_draw_rect(win->x + 1, win->y + 1, win->width - 2, 16, UI_DARK_GRAY);
    ui_draw_rect_outline(win->x + 1, win->y + 1, win->width - 2, 16, UI_WHITE);
    ui_draw_text(win->x + 5, win->y + 5, win->title, UI_WHITE);
    ui_draw_rect(win->x + win->width - 15, win->y + 3, 10, 10, UI_RED);
    ui_draw_text(win->x + win->width - 13, win->y + 5, "X", UI_WHITE);
}

void ui_fill_window(window_t* win, uint8_t color) {
    int content_y = win->y + (win->has_titlebar ? 17 : 1);
    int content_height = win->height - (win->has_titlebar ? 18 : 2);
    
    ui_draw_rect(win->x + 1, content_y, win->width - 2, content_height, color);
}

// button functions
void ui_create_button(button_t* btn, int x, int y, int width, int height,
                      const char* text, uint8_t color, uint8_t text_color) {
    btn->x = x;
    btn->y = y;
    btn->width = width;
    btn->height = height;
    btn->color = color;
    btn->text_color = text_color;
    btn->pressed = 0;
    
    // Copy text
    int i = 0;
    while (text[i] && i < 31) {
        btn->text[i] = text[i];
        i++;
    }
    btn->text[i] = '\0';
}

void ui_draw_button(button_t* btn, int win_x, int win_y) {
    int abs_x = win_x + btn->x;
    int abs_y = win_y + btn->y;
    
    ui_draw_rect(abs_x, abs_y, btn->width, btn->height, btn->color);
    
    ui_draw_rect_outline(abs_x, abs_y, btn->width, btn->height, UI_WHITE);
    ui_draw_rect_outline(abs_x + 1, abs_y + 1, btn->width - 2, btn->height - 2, UI_DARK_GRAY);
    
    int text_x = abs_x + (btn->width - strlen(btn->text) * 8) / 2;
    int text_y = abs_y + (btn->height - 8) / 2;
    ui_draw_text(text_x, text_y, btn->text, btn->text_color);
}

void ui_draw_button_pressed(button_t* btn, int win_x, int win_y) {
    int abs_x = win_x + btn->x;
    int abs_y = win_y + btn->y;
    
    ui_draw_rect(abs_x, abs_y, btn->width, btn->height, btn->color - 1);
    
    ui_draw_rect_outline(abs_x, abs_y, btn->width, btn->height, UI_DARK_GRAY);
    ui_draw_rect_outline(abs_x + 1, abs_y + 1, btn->width - 2, btn->height - 2, UI_WHITE);
    
    int text_x = abs_x + (btn->width - strlen(btn->text) * 8) / 2 + 1;
    int text_y = abs_y + (btn->height - 8) / 2 + 1;
    ui_draw_text(text_x, text_y, btn->text, btn->text_color);
}

// text
void ui_draw_char(int x, int y, char c, uint8_t color) {
    if (c < 32 || c > 90) c = 32;
    
    int char_index = c - 32;
    if (char_index < 0 || char_index >= 59) return;
    
    for (int row = 0; row < 8; row++) {
        uint8_t font_row = font_8x8[char_index][row];
        for (int col = 0; col < 8; col++) {
            if (font_row & (1 << (7 - col))) {
                ui_draw_pixel(x + (7 - col), y + row, color);
            }
        }
    }
}

void ui_draw_text(int x, int y, const char* text, uint8_t color) {
    int pos_x = x;
    while (*text) {
        ui_draw_char(pos_x, y, *text, color);
        pos_x += 8;
        text++;
    }
}

void ui_draw_text_centered(int x, int y, int width, const char* text, uint8_t color) {
    int text_width = strlen(text) * 8;
    int start_x = x + (width - text_width) / 2;
    ui_draw_text(start_x, y, text, color);
}

// demo
void ui_demo() {
    ui_clear_screen(UI_BLUE);
    
    window_t win1;
    ui_create_window(&win1, 50, 30, 200, 120, "TEST", UI_LIGHT_GRAY, UI_BLACK);
    ui_draw_window(&win1);
}