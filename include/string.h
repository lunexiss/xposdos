#ifndef STRING_H
#define STRING_H

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

#define bool	_Bool
#define true	1
#define false	0

int strcmp(const char* a, const char* b);
int strncmp(const char* s1, const char* s2, int n);
int strlen(const char* s);
void print_hex(uint8_t byte);
void itoa(int value, char* str, int base);
char* strcpy(char* dest, const char* src);
char* strchr(const char* s, int c);
void* memcpy(void* dest, const void* src, unsigned int n);
char* strncpy(char* dest, const char* src, size_t n);
const char* strrchr(const char* str, char ch);
char* strcat(char* dest, const char* src);
void strncat(char* dest, const char* src, int max_len);
void* memset(void* dest, int val, size_t len);
int sprintf(char* str, const char* fmt, ...);
int vsprintf(char* str, const char* fmt, va_list args);
char* strstr(const char* haystack, const char* needle);
int snprintf(char* out, size_t size, const char* fmt, ...);
char* strtok(char* str, const char* delim);
int memcmp(const void* s1, const void* s2, size_t n);
int sscanf(const char *input, const char *format, ...);
void* memmove(void* dest, const void* src, size_t n);

#endif
