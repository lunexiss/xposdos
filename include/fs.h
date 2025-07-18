#ifndef FS_H
#define FS_H

#include <stdint.h>

#define MAX_FS_FILES 64
#define MAX_FILENAME 32

#define FS_START_SECTOR 100
#define FS_TABLE_SECTOR 2
#define FS_TOTAL_SPACE 1024 * 512 

typedef struct {
    int used;
    char name[MAX_FILENAME];
    char folder[MAX_FILENAME];
    uint32_t size;
    uint16_t start_sector;
} FileEntry;

extern FileEntry file_table[MAX_FS_FILES];

typedef enum {
    FS_TYPE_FILE = 0,
    FS_TYPE_FOLDER = 1,
    FS_TYPE_NOT_FOUND = 2
} fs_entry_type_t;

typedef struct {
    FileEntry* entry;
    fs_entry_type_t type;
} fs_find_result_t;

void fs_init();
void fs_load_file_table();
void fs_save_file(const char* name, const char* data);
void fs_read_file(const char* full_path, char* out, const char* current_dir);
void fs_list();
void fs_delete_file(const char* pattern);
uint32_t fs_get_free_space();
void fs_make_dir(const char* full_path);
void fs_make_dirs(const char* full_path);
FileEntry* fs_get_file_entry(const char* full_path);
fs_find_result_t fs_find(const char* full_path);
int fs_exists(const char* full_path);
int fs_is_folder(const char* full_path);
int fs_is_file(const char* full_path);

#endif
