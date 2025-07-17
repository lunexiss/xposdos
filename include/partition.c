#include "partition.h"
#include "disk.h"
#include "vga.h"
#include "string.h"

void list_partitions() {
    uint8_t* mbr = (uint8_t*) (uint16_t[256]){0};
    ata_read_sector(0, (uint16_t*)mbr);

    for (int i = 0; i < 4; i++) {
        int offset = 0x1BE + i * 16;
        uint8_t type = mbr[offset + 4];
        uint32_t start = *(uint32_t*)&mbr[offset + 8];
        uint32_t size = *(uint32_t*)&mbr[offset + 12];

        if (type == 0 || size == 0) continue;

        print("Partition ");
        put_char('1' + i);
        print(": Type 0x"); print_hex(type);
        print(" Start "); print_hex((start >> 24) & 0xFF);
        print_hex((start >> 16) & 0xFF);
        print_hex((start >> 8) & 0xFF);
        print_hex(start & 0xFF);
        print(" Size "); print_hex((size >> 24) & 0xFF);
        print_hex((size >> 16) & 0xFF);
        print_hex((size >> 8) & 0xFF);
        print_hex(size & 0xFF);
        print("\n");
    }
}

void add_partition(int index, uint32_t start, uint32_t size) {
    uint8_t* mbr = (uint8_t*) (uint16_t[256]){0};
    ata_read_sector(0, (uint16_t*)mbr);

    if (index < 0 || index >= 4) {
        print("Invalid partition index\n");
        return;
    }

    int offset = 0x1BE + index * 16;
    mbr[offset + 0] = 0x80; // bootable
    mbr[offset + 4] = 0x83; // linux type
    *(uint32_t*)&mbr[offset + 8] = start;
    *(uint32_t*)&mbr[offset + 12] = size;

    mbr[510] = 0x55;
    mbr[511] = 0xAA;
    ata_write_sector(0, (uint16_t*)mbr);

    print("Partition added and written\n");
}

void write_partition_table() {
    uint8_t mbr[512] = {0};
    mbr[510] = 0x55;
    mbr[511] = 0xAA;
    ata_write_sector(0, (uint16_t*)mbr);
    print("Wrote empty MBR with signature\n");
}

void create_partition(uint32_t lba_start, uint32_t num_sectors) {
    uint8_t mbr[512] = {0};
    int offset = 0x1BE;

    mbr[offset + 0] = 0x80;
    mbr[offset + 4] = 0x83;
    *(uint32_t*)&mbr[offset + 8] = lba_start;
    *(uint32_t*)&mbr[offset + 12] = num_sectors;

    mbr[510] = 0x55;
    mbr[511] = 0xAA;

    ata_write_sector(0, (uint16_t*)mbr);
    print("MBR with single partition created\n");
}
