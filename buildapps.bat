@echo off

i686-elf-gcc -m32 -ffreestanding -nostdlib -nostartfiles ^
    -fno-builtin -fno-stack-protector ^
    -Iinclude ^
    -T apptest/app.ld ^
    -o test.elf ^
    apptest/main.c include/vga.c include/string.c

python tools/create_binary.py test.elf test.bin