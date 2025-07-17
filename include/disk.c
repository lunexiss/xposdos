#include "disk.h"
#include "io.h"

static inline void io_wait() {
    for (int i = 0; i < 1000; i++) __asm__ __volatile__("nop");
}

#define ATA_PRIMARY_IO 0x1F0
#define ATA_PRIMARY_CTRL 0x3F6

int atoi(const char* str) {
    int res = 0;
    while (*str >= '0' && *str <= '9') {
        res = res * 10 + (*str - '0');
        str++;
    }
    return res;
}

void ata_write_sector(uint32_t lba, const void* vbuf) {
    const uint16_t* buffer = (const uint16_t*)vbuf;
    outb(ATA_PRIMARY_CTRL, 0x00); // disable these stupid interrupts

    outb(ATA_PRIMARY_IO + 2, 1);              // sector count
    outb(ATA_PRIMARY_IO + 3, (uint8_t)(lba));         // lba low
    outb(ATA_PRIMARY_IO + 4, (uint8_t)(lba >> 8));     // lba mid
    outb(ATA_PRIMARY_IO + 5, (uint8_t)(lba >> 16));    // lba high
    outb(ATA_PRIMARY_IO + 6, 0xE0 | ((lba >> 24) & 0x0F)); // drive and lba mode

    outb(ATA_PRIMARY_IO + 7, 0x30); // write command

    while (!(inb(ATA_PRIMARY_IO + 7) & 0x08));

    // Send 256 words (512 bytes i guess)
    for (int i = 0; i < 256; i++) {
        outw(ATA_PRIMARY_IO, buffer[i]);
    }

    outb(ATA_PRIMARY_IO + 7, 0xE7);

    while (inb(ATA_PRIMARY_IO + 7) & 0x80);

    while (inb(ATA_PRIMARY_IO + 7) & 0x08); 
}

void ata_read_sector(uint32_t lba, void* vbuf) {
    uint16_t* buf = (uint16_t*)vbuf;
    outb(0x1F6, 0xE0 | ((lba >> 24) & 0x0F)); // drive/head
    outb(0x1F2, 1);                          // sector count
    outb(0x1F3, (uint8_t)(lba & 0xFF));      // lba low
    outb(0x1F4, (uint8_t)(lba >> 8));        // lba mid
    outb(0x1F5, (uint8_t)(lba >> 16));       // lba high
    outb(0x1F7, 0x20);                       // read sectors

    while (inb(0x1F7) & 0x80); // bsy
    while (!(inb(0x1F7) & 0x08)); // drq

    for (int i = 0; i < 256; i++) {
        buf[i] = inw(0x1F0);
    }

    io_wait();
}
