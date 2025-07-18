#ifndef UI_H
#define UI_H

#include <stdint.h>

#define UI_BLACK       0
#define UI_BLUE        1
#define UI_GREEN       2
#define UI_CYAN        3
#define UI_RED         4
#define UI_MAGENTA     5
#define UI_BROWN       6
#define UI_LIGHT_GRAY  7
#define UI_DARK_GRAY   8
#define UI_LIGHT_BLUE  9
#define UI_LIGHT_GREEN 10
#define UI_LIGHT_CYAN  11
#define UI_LIGHT_RED   12
#define UI_LIGHT_MAGENTA 13
#define UI_YELLOW      14
#define UI_WHITE       15

typedef struct {
    int x, y;
    int width, height;
    uint8_t bg_color;
    uint8_t border_color;
    char title[64];
    int visible;
    int has_titlebar;
} window_t;

typedef struct {
    int x, y;
    int width, height;
    uint8_t color;
    uint8_t text_color;
    char text[32];
    int pressed;
} button_t;

void ui_init();
void ui_clear_screen(uint8_t color);
void ui_draw_pixel(int x, int y, uint8_t color);
void ui_draw_rect(int x, int y, int width, int height, uint8_t color);
void ui_draw_rect_outline(int x, int y, int width, int height, uint8_t color);
void ui_draw_line(int x1, int y1, int x2, int y2, uint8_t color);

void ui_create_window(window_t* win, int x, int y, int width, int height, 
                     const char* title, uint8_t bg_color, uint8_t border_color);
void ui_draw_window(window_t* win);
void ui_draw_titlebar(window_t* win);
void ui_fill_window(window_t* win, uint8_t color);

void ui_create_button(button_t* btn, int x, int y, int width, int height,
                     const char* text, uint8_t color, uint8_t text_color);
void ui_draw_button(button_t* btn, int win_x, int win_y);
void ui_draw_button_pressed(button_t* btn, int win_x, int win_y);

void ui_draw_char(int x, int y, char c, uint8_t color);
void ui_draw_text(int x, int y, const char* text, uint8_t color);
void ui_draw_text_centered(int x, int y, int width, const char* text, uint8_t color);

void ui_demo();

#endif