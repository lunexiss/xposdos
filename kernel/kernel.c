// hi

// Ия, мен қазақпын, менің атым Нурислам.

#include <stdint.h>
#include "vga.h"
#include "input.h"
#include "idt.h"
#include <shell.h>
#include "fs.h"
#include "drivers/PCI/pci.h"
#include "drivers/e1000/e1000.h"

void kernel_main() {
    vga_init();
    idt_init();
    pci_init();
    fs_init();

    if (global_e1000->mmio_base) {
        print("E1000 initialized successfully\n");
        e1000_print_mac(global_e1000);
    } else {
        print("E1000 not initialized\n");
    }

    shell();
}
