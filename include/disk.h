#ifndef DISK_H
#define DISK_H

#include <stdint.h>

void ata_read_sector(uint32_t lba, uint16_t* buffer);
void ata_write_sector(uint32_t lba, const uint16_t* buffer);
int atoi(const char* str);

#endif
