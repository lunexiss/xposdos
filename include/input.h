#ifndef INPUT_H
#define INPUT_H

#include <stdint.h>
#include <stdbool.h>

void keyboard_handler_c(void);
bool input_has_char(void);
char input_get_char(void);
void gets(char* input, int* char_hidden);

#endif
