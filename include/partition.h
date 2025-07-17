#ifndef PARTITION_H
#define PARTITION_H

#include <stdint.h>

void list_partitions();
void create_partition(uint32_t lba_start, uint32_t num_sectors);
void write_partition_table();
void add_partition(int index, uint32_t start, uint32_t size);

#endif
