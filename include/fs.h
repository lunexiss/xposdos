#ifndef FS_H
#define FS_H

#include <stdint.h>
#include <stdbool.h>

extern uint32_t table_sector;

#define MAX_FS_FILES 64
#define MAX_FILENAME 32

#define FS_START_SECTOR 100
#define FS_TABLE_SECTOR 2

typedef struct {
    int used;
    char name[MAX_FILENAME];
    char folder[MAX_FILENAME];
    uint32_t size;
    uint16_t start_sector;
    uint8_t _padding[27];
} FileEntry;

extern FileEntry file_table[MAX_FS_FILES];

void fs_init();
void fs_load_file_table();
void fs_write_table(uint32_t sector, FileEntry* table);
void fs_save_file(const char* name, const char* data);
void fs_read_file(const char* full_path, char* out, const char* current_dir);
void fs_list();
void fs_delete_file(const char* pattern);
uint32_t fs_get_free_space();
void fs_make_dir(const char* full_path);
bool mount_partition(int part);
void fs_save_file_len(const char* full_path, const char* data, uint32_t size);
uint32_t fs_get_total_space();
uint32_t fs_get_max_sectors();
void fs_mount(const char* name, uint16_t start, uint32_t size);
void fs_set_table_sector(uint32_t sector);
void fs_format();
uint32_t fs_get_table_sector(void);
void fs_read_table(uint32_t sector, FileEntry* table);
void fs_save_table(uint32_t sector, FileEntry *table);

#endif
