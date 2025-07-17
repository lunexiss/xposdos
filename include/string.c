#include <stdint.h>
#include <stddef.h>
#include "string.h"
#include "vga.h"
#include <stdarg.h>

int strcmp(const char* a, const char* b) {
    while (*a && (*a == *b)) {
        a++; b++;
    }
    return *(const unsigned char*)a - *(const unsigned char*)b;
}

const char* strrchr(const char* str, char ch) {
    const char* last = NULL;
    while (*str) {
        if (*str == ch) {
            last = str;
        }
        str++;
    }
    return last;
}

void strncat(char* dest, const char* src, int max_len) {
    int dlen = 0;
    while (dest[dlen] != '\0' && dlen < max_len) dlen++;

    for (int i = 0; src[i] != '\0' && dlen + i < max_len - 1; i++) {
        dest[dlen + i] = src[i];
        dest[dlen + i + 1] = '\0';
    }
}

void* memset(void* dest, int val, size_t len) {
    unsigned char* ptr = dest;
    while (len-- > 0) {
        *ptr++ = (unsigned char)val;
    }
    return dest;
}

char* strcat(char* dest, const char* src) {
    char* ptr = dest + strlen(dest);
    while (*src) {
        *ptr++ = *src++;
    }
    *ptr = '\0';
    return dest;
}

int strncmp(const char* s1, const char* s2, int n) {
    for (int i = 0; i < n; i++) {
        if (s1[i] != s2[i] || s1[i] == '\0' || s2[i] == '\0') {
            return (unsigned char)s1[i] - (unsigned char)s2[i];
        }
    }
    return 0;
}

char* strcpy(char* dest, const char* src) {
    char* ptr = dest;
    while ((*dest++ = *src++));
    return ptr;
}

char* strchr(const char* s, int c) {
    while (*s) {
        if (*s == (char)c) return (char*)s;
        s++;
    }
    return NULL;
}

void* memcpy(void* dest, const void* src, unsigned int n) {
    char* d = (char*)dest;
    const char* s = (const char*)src;
    for (unsigned int i = 0; i < n; i++)
        d[i] = s[i];
    return dest;
}

int strlen(const char* s) {
    int len = 0;
    while (*s++) len++;
    return len;
}

void print_hex(uint8_t byte) {
    char hex[] = "0123456789ABCDEF";
    put_char(hex[(byte >> 4) & 0xF]);
    put_char(hex[byte & 0xF]);
}

void itoa(int value, char* str, int base) {
    char* rc = str;
    char* ptr;
    char* low;
    // set '-' for negative decimals
    if (value < 0 && base == 10) {
        *rc++ = '-';
        value = -value;
    }

    ptr = rc;
    do {
        int mod = value % base;
        *ptr++ = (mod < 10) ? (mod + '0') : (mod - 10 + 'A');
    } while (value /= base);
    *ptr-- = '\0';

    // reverse that stupid string
    for (low = rc; low < ptr; low++, ptr--) {
        char tmp = *low;
        *low = *ptr;
        *ptr = tmp;
    }
}

char* strncpy(char* dest, const char* src, size_t n) {
    size_t i;
    for (i = 0; i < n && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }
    for (; i < n; i++) {
        dest[i] = '\0';
    }
    return dest;
}

int vsprintf(char* str, const char* fmt, va_list args) {
    char* s = str;
    while (*fmt) {
        if (*fmt == '%' && *(fmt+1)) {
            fmt++;
            if (*fmt == 's') {
                const char* arg = va_arg(args, const char*);
                while (*arg) *s++ = *arg++;
            } else if (*fmt == 'd') {
                int val = va_arg(args, int);
                char buf[16];
                itoa(val, buf, 10);
                char* b = buf;
                while (*b) *s++ = *b++;
            } else if (*fmt == 'c') {
                *s++ = va_arg(args, int);
            }
        } else {
            *s++ = *fmt;
        }
        fmt++;
    }
    *s = '\0';
    return (s - str);
}

int sprintf(char* str, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int n = vsprintf(str, fmt, args);
    va_end(args);
    return n;
}

int snprintf(char* out, size_t size, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    size_t written = 0;

    while (*fmt && written < size - 1) {
        if (*fmt == '%') {
            fmt++;
            if (*fmt == 's') {
                const char* s = va_arg(args, const char*);
                while (*s && written < size - 1) {
                    out[written++] = *s++;
                }
            } else if (*fmt == 'd') {
                int val = va_arg(args, int);
                if (val < 0) {
                    if (written < size - 1) out[written++] = '-';
                    val = -val;
                }
                char buf[10];
                int len = 0;
                do {
                    buf[len++] = '0' + (val % 10);
                    val /= 10;
                } while (val > 0);
                while (len-- > 0 && written < size - 1)
                    out[written++] = buf[len];
            } else if (*fmt == 'u') {
                unsigned int val = va_arg(args, unsigned int);
                char buf[10];
                int len = 0;
                do {
                    buf[len++] = '0' + (val % 10);
                    val /= 10;
                } while (val > 0);
                while (len-- > 0 && written < size - 1)
                    out[written++] = buf[len];
            } else if (*fmt == 'x') {
                unsigned int val = va_arg(args, unsigned int);
                char buf[8];
                int len = 0;
                do {
                    int digit = val % 16;
                    buf[len++] = (digit < 10) ? ('0' + digit) : ('a' + digit - 10);
                    val /= 16;
                } while (val > 0);
                while (len-- > 0 && written < size - 1)
                    out[written++] = buf[len];
            } else {
                if (written < size - 1)
                    out[written++] = '%';
                if (written < size - 1)
                    out[written++] = *fmt;
            }
        } else {
            out[written++] = *fmt;
        }
        fmt++;
    }

    out[written] = '\0';
    va_end(args);
    return written;
}

char* strstr(const char* haystack, const char* needle) {
    if (!*needle) return (char*)haystack;

    for (; *haystack; haystack++) {
        const char* h = haystack;
        const char* n = needle;
        while (*h && *n && *h == *n) {
            h++;
            n++;
        }
        if (!*n) return (char*)haystack;
    }
    return 0;
}

char* strtok(char* str, const char* delim) {
    static char* last = 0;
    if (str) last = str;
    if (!last) return 0;

    // skip leading delimiters shi
    while (*last && strchr(delim, *last)) last++;

    if (!*last) return 0;

    char* token = last;

    // find token's end
    while (*last && !strchr(delim, *last)) last++;

    if (*last) {
        *last = '\0';
        last++;
    } else {
        last = 0;
    }

    return token;
}

int memcmp(const void* s1, const void* s2, size_t n) {
    const unsigned char* p1 = s1;
    const unsigned char* p2 = s2;
    for (size_t i = 0; i < n; ++i) {
        if (p1[i] != p2[i]) return p1[i] - p2[i];
    }
    return 0;
}

int sscanf(const char *input, const char *format, ...) {
    va_list args;
    va_start(args, format);

    int count = 0;

    while (*format && *input) {
        if (*format == '%') {
            format++;

            if (*format == 'd') {
                int *out_int = va_arg(args, int*);
                *out_int = 0;

                bool negative = false;
                if (*input == '-') {
                    negative = true;
                    input++;
                }

                while (*input >= '0' && *input <= '9') {
                    *out_int = (*out_int * 10) + (*input - '0');
                    input++;
                }

                if (negative) *out_int *= -1;
                count++;
            } else if (*format == 's') {
                char *out_str = va_arg(args, char*);
                int i = 0;
                while (*input && *input != ' ' && *input != '\n') {
                    out_str[i++] = *input++;
                }
                out_str[i] = '\0';
                count++;
            }
        }

        // skip spaces and move format forward
        while (*input == ' ') input++;
        while (*format && *format != '%' && *format != ' ') format++;
        if (*format == ' ') format++;
    }

    va_end(args);
    return count;
}