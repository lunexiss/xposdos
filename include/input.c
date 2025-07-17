#include "input.h"
#include "vga.h"
#include "string.h"
#include "io.h"

// the most painful shit i literally coded

uint8_t shift_down = 0;
uint8_t caps_lock = 0;
uint8_t last_scancode = 0;

static const char unshifted[128] = {
    0, 27, '1','2','3','4','5','6','7','8','9','0','-','=','\b',
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n', 0,
    'a','s','d','f','g','h','j','k','l',';','\'','`', 0,'\\',
    'z','x','c','v','b','n','m',',','.','/', 0,'*', 0,' ', 0,
};

static const char shifted[128] = {
    0, 27, '!','@','#','$','%','^','&','*','(',')','_','+','\b',
    '\t','Q','W','E','R','T','Y','U','I','O','P','{','}','\n', 0,
    'A','S','D','F','G','H','J','K','L',':','"','~', 0,'|',
    'Z','X','C','V','B','N','M','<','>','?', 0,'*', 0,' ', 0,
};

#define INPUT_BUFFER_SIZE 128
static char input_buffer[INPUT_BUFFER_SIZE];
static int input_head = 0;
static int input_tail = 0;

bool input_has_char(void) {
    return input_head != input_tail;
}

char input_get_char(void) {
    if (!input_has_char()) return 0;
    char c = input_buffer[input_tail];
    input_tail = (input_tail + 1) % INPUT_BUFFER_SIZE;
    return c;
}

void push_char(char c) {
    int next = (input_head + 1) % INPUT_BUFFER_SIZE;
    if (next != input_tail) {
        input_buffer[input_head] = c;
        input_head = next;
    }
}

int is_alpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

char get_char_from_scancode(uint8_t sc) {
    last_scancode = sc;

    if (sc == 0x2A || sc == 0x36) { shift_down = 1; return 0; }
    if (sc == 0xAA || sc == 0xB6) { shift_down = 0; return 0; }

    if (sc == 0x3A) { caps_lock ^= 1; return 0; }

    // ignore key releases
    if (sc & 0x80) return 0;

    char c = shift_down ? shifted[sc] : unshifted[sc];
    if (c == 0) return 0;

    if (c >= 'a' && c <= 'z') {
        if (shift_down ^ caps_lock) {
            c -= 32;  // to uppercase
        }
    } else if (c >= 'A' && c <= 'Z') {
        if (!(shift_down ^ caps_lock)) {
            c += 32;  // to lowercase
        }
    }

    return c;
}

void keyboard_handler_c(void) {
    //uint8_t sc = inb(0x60);
    //char c = get_char_from_scancode(sc);
    //if (c) push_char(c);
    outb(0x20, 0x20);
}

void gets(char* buffer, int* char_hidden) {
    int pos = 0;
    uint8_t last_scancode = 0xFF;

    while (1) {
        uint8_t sc = inb(0x60);

        if (sc & 0x80) {
            uint8_t released = sc & 0x7F;
            if (released == last_scancode) {
                last_scancode = 0xFF;  // clear after release
            }
            continue;
        }

        if (sc == last_scancode) continue;

        last_scancode = sc;

        char c = get_char_from_scancode(sc);
        if (!c) continue;

        if (c == '\n') {
            buffer[pos] = '\0';
            put_char('\n');
            break;
        }

        if (c == '\b') {
            if (pos > 0) {
                pos--;
                put_char('\b');
                put_char(' ');
                put_char('\b');
            }
            continue;
        }

        if (pos < 127) {
            buffer[pos++] = c;
            if (*char_hidden) put_char('*');
            else put_char(c);
        }
    }

    buffer[pos] = '\0';
}
