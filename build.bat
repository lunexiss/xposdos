@echo off
setlocal EnableDelayedExpansion

if not exist build mkdir build
mkdir build\iso\boot\grub 2>nul

set OBJ_FILES=

for %%F in (asm\*.s asm\*.asm) do (
    echo Assembling %%F...
    nasm -f elf32 %%F -o build\%%~nF.o
    set "OBJ_FILES=!OBJ_FILES! build\%%~nF.o"
)

for %%F in (kernel\*.c) do (
    echo Compiling %%F...
    set "filename=%%~nF"
    i686-elf-gcc -fno-builtin -Iinclude -Iinclude/drivers/PCI -Iinclude/drivers/e1000 -c %%F -o build\!filename!.o
    set "OBJ_FILES=!OBJ_FILES! build\!filename!.o"
)

for %%F in (include\*.c) do (
    echo Compiling %%F...
    set "filename=%%~nF"
    i686-elf-gcc -fno-builtin -Iinclude -Iinclude/drivers/PCI -Iinclude/drivers/e1000 -c %%F -o build\!filename!.o
    set "OBJ_FILES=!OBJ_FILES! build\!filename!.o"
)

for %%F in (include\drivers\PCI\*.c) do (
    echo Compiling %%F...
    set "filename=%%~nF"
    i686-elf-gcc -fno-builtin -Iinclude -Iinclude/drivers/e1000 -c %%F -o build\!filename!_pci.o
    set "OBJ_FILES=!OBJ_FILES! build\!filename!_pci.o"
)

for %%F in (include\drivers\e1000\*.c) do (
    echo Compiling %%F...
    set "filename=%%~nF"
    i686-elf-gcc -fno-builtin -Iinclude -Iinclude/drivers/e1000 -Iinclude/drivers/PCI -c %%F -o build\!filename!_e1000.o
    set "OBJ_FILES=!OBJ_FILES! build\!filename!_e1000.o"
)

for %%F in (include\drivers\sound\*.c) do (
    echo Compiling %%F...
    set "filename=%%~nF"
    i686-elf-gcc -fno-builtin -Iinclude -Iinclude/drivers/e1000 -Iinclude/drivers/PCI -c %%F -o build\!filename!_sound.o
    set "OBJ_FILES=!OBJ_FILES! build\!filename!_sound.o"
)

for %%F in (include\net\*.c) do (
    echo Compiling %%F...
    set "filename=%%~nF"
    i686-elf-gcc -fno-builtin -Iinclude -Iinclude/net -Iinclude/drivers/PCI -Iinclude/drivers/e1000 -c %%F -o build\!filename!.o
    set "OBJ_FILES=!OBJ_FILES! build\!filename!.o"
)

echo Linking...
echo %OBJ_FILES%
i686-elf-ld -T link.ld -o build\kernel.elf %OBJ_FILES%

REM wsl xxd -i build/iso/boot/kernel.elf > include/kernel_elf.h
REM wsl xxd -i grub_mbr.img > include/grub_mbr.h
REM wsl xxd -i core.img     > include/grub_core_img.h

copy build\kernel.elf build\iso\boot\kernel.elf >nul
copy boot\grub\grub.cfg build\iso\boot\grub\grub.cfg >nul

if not exist iso mkdir iso
xcopy /s /e /y build\iso iso >nul

REM grub, why THE FUCK YOU KEEP MAKING 30 MB KIND OF ISO
wsl grub-mkrescue -o build/XPOSD.iso iso

echo.

run