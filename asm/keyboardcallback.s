global keyboard_callback
extern keyboard_handler_c

section .text
keyboard_callback:
    pusha
    call keyboard_handler_c
    popa
    iretd
