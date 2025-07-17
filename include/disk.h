#ifndef DISK_H
#define DISK_H

#include <stdint.h>

void ata_read_sector(uint32_t lba, void* buffer);
void ata_write_sector(uint32_t lba, const void* buffer);
int atoi(const char* str);

#endif
