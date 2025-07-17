qemu-system-i386 -m 1000 -cdrom build/XPOSD.iso -hda hdd.img -boot d ^
    -netdev user,id=net0 -device e1000,netdev=net0 -vga std ^
    -audiodev dsound,id=speaker -machine pcspk-audiodev=speaker