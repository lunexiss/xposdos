// some of the code was fixed by claude :3

#include "fs.h"
#include "string.h"
#include "vga.h"
#include "disk.h"
#include <stddef.h>
#include <stdint.h>
#include "string.h"

FileEntry file_table[MAX_FS_FILES];

uint16_t start_sector = FS_START_SECTOR;
#define FS_MAX_SECTORS (FS_TOTAL_SPACE / 512)

void fs_init() {
    fs_load_file_table();
}

void fs_load_file_table() {
    ata_read_sector(FS_TABLE_SECTOR, (uint16_t*)file_table);
}

fs_find_result_t fs_find(const char* full_path) {
    fs_find_result_t result = {NULL, FS_TYPE_NOT_FOUND};
    
    if (!full_path) {
        print("Path is null!\n");
        return result;
    }
    
    char folder[MAX_FILENAME] = {0};
    char name[MAX_FILENAME] = {0};
    const char* slash = strrchr(full_path, '/');
    
    if (slash) {
        // parse the fucking path again
        int folder_len = slash - full_path;
        if (folder_len >= MAX_FILENAME) folder_len = MAX_FILENAME - 1;
        strncpy(folder, full_path, folder_len);
        folder[folder_len] = '\0';
        
        strncpy(name, slash + 1, MAX_FILENAME - 1);
        name[MAX_FILENAME - 1] = '\0';
    } else {
        strncpy(name, full_path, MAX_FILENAME - 1);
        name[MAX_FILENAME - 1] = '\0';
        folder[0] = '\0';
    }
    
    // search through the file table like a detective
    for (int i = 0; i < MAX_FS_FILES; i++) {
        if (file_table[i].used &&
            strcmp(file_table[i].name, name) == 0 &&
            strcmp(file_table[i].folder, folder) == 0) {
            
            result.entry = &file_table[i];
            
            // check if it's a folder (start_sector = 0xFFFF) or file
            if (file_table[i].start_sector == 0xFFFF) {
                result.type = FS_TYPE_FOLDER;
            } else {
                result.type = FS_TYPE_FILE;
            }
            
            return result;
        }
    }
    
    // didn't find shit
    return result;
}

int fs_exists(const char* full_path) {
    fs_find_result_t result = fs_find(full_path);
    return (result.type != FS_TYPE_NOT_FOUND);
}

int fs_is_folder(const char* full_path) {
    fs_find_result_t result = fs_find(full_path);
    return (result.type == FS_TYPE_FOLDER);
}

int fs_is_file(const char* full_path) {
    fs_find_result_t result = fs_find(full_path);
    return (result.type == FS_TYPE_FILE);
}

FileEntry* fs_get_file_entry(const char* full_path) {
    if (!full_path) {
        print("Path not found!");
        return NULL;
    }
    
    char folder[MAX_FILENAME] = {0};
    char name[MAX_FILENAME] = {0};
    const char* slash = strrchr(full_path, '/');
    
    if (slash) {
        int folder_len = slash - full_path;
        if (folder_len >= MAX_FILENAME) folder_len = MAX_FILENAME - 1;
        strncpy(folder, full_path, folder_len);
        folder[folder_len] = '\0';
        
        strncpy(name, slash + 1, MAX_FILENAME - 1);
        name[MAX_FILENAME - 1] = '\0';
    } else {
        strncpy(name, full_path, MAX_FILENAME - 1);
        name[MAX_FILENAME - 1] = '\0';
        folder[0] = '\0';
    }
    
    // search for the file in the file table like finding waldo
    for (int i = 0; i < MAX_FS_FILES; i++) {
        if (file_table[i].used &&
            strcmp(file_table[i].name, name) == 0 &&
            strcmp(file_table[i].folder, folder) == 0) {
            return &file_table[i];
        }
    }
    
    // didn't find jack shit
    return NULL;
}

void fs_make_dir(const char* full_path) {
    if (!full_path || full_path[0] == '\0') return; // ignore null or empty shit

    char folder[MAX_FILENAME] = {0};
    char name[MAX_FILENAME] = {0};

    const char* slash = strrchr(full_path, '/');
    if (slash) {
        // fuck this stupid fucking folder parsing
        int folder_len = slash - full_path;
        if (folder_len >= MAX_FILENAME) folder_len = MAX_FILENAME - 1;
        strncpy(folder, full_path, folder_len);
        folder[folder_len] = '\0';
        strncpy(name, slash + 1, MAX_FILENAME - 1);
        name[MAX_FILENAME - 1] = '\0';
    } else {
        strncpy(name, full_path, MAX_FILENAME - 1);
        name[MAX_FILENAME - 1] = '\0';
        folder[0] = '\0';
    }

    if (strlen(name) == 0) return;

    for (int i = 0; i < MAX_FS_FILES; i++) {
        if (file_table[i].used &&
            file_table[i].start_sector == 0xFFFF &&
            strcmp(file_table[i].name, name) == 0 &&
            strcmp(file_table[i].folder, folder) == 0) {
            return;
        }
    }

    for (int i = 0; i < MAX_FS_FILES; i++) {
        if (!file_table[i].used) {
            file_table[i].used = 1;
            strncpy(file_table[i].name, name, MAX_FILENAME - 1);
            file_table[i].name[MAX_FILENAME - 1] = '\0';
            strncpy(file_table[i].folder, folder, MAX_FILENAME - 1);
            file_table[i].folder[MAX_FILENAME - 1] = '\0';
            file_table[i].size = 0;
            file_table[i].start_sector = 0xFFFF;

            ata_write_sector(FS_TABLE_SECTOR, (uint16_t*)file_table);
            return;
        }
    }
}

void fs_make_dirs(const char* full_path) {
    if (!full_path) return;
    
    char path[MAX_FILENAME * 2] = {0};
    int len = strlen(full_path);

    // create directories like a fucking tree
    for (int i = 0; i < len; i++) {
        if (full_path[i] == '/' || i == len - 1) {
            int segment_len = (full_path[i] == '/') ? i : i + 1;
            if (segment_len >= sizeof(path)) segment_len = sizeof(path) - 1;
            strncpy(path, full_path, segment_len);
            path[segment_len] = '\0';
            fs_make_dir(path);
        }
    }
}

void fs_save_file(const char* full_path, const char* data) {
    if (!full_path || !data) return;
    
    char folder[MAX_FILENAME * 2] = {0};
    char name[MAX_FILENAME] = {0};
    uint32_t size = strlen(data);

    const char* slash = strrchr(full_path, '/');
    if (slash) {
        // more goddamn path parsing
        int folder_len = slash - full_path;
        if (folder_len >= sizeof(folder)) folder_len = sizeof(folder) - 1;
        strncpy(folder, full_path, folder_len);
        folder[folder_len] = '\0';
        strncpy(name, slash + 1, MAX_FILENAME - 1);
        name[MAX_FILENAME - 1] = '\0';
    } else {
        strncpy(name, full_path, MAX_FILENAME - 1);
        name[MAX_FILENAME - 1] = '\0';
        folder[0] = '\0';
    }

    char path[MAX_FILENAME * 2] = {0};
    int folder_len = strlen(folder);
    for (int i = 0; i < folder_len; i++) {
        if (folder[i] == '/' || i == folder_len - 1) {
            int len = (folder[i] == '/') ? i : i + 1;
            if (len >= sizeof(path)) len = sizeof(path) - 1;
            strncpy(path, folder, len);
            path[len] = '\0';
            fs_make_dir(path);
        }
    }

    // check if file already exists and no overwrites bitch
    for (int i = 0; i < MAX_FS_FILES; i++) {
        if (file_table[i].used &&
            strcmp(file_table[i].name, name) == 0 &&
            strcmp(file_table[i].folder, folder) == 0) {
            print("File already exists!\n");
            return;
        }
    }

    int sectors_needed = (size + 511) / 512;
    if (start_sector + sectors_needed > FS_MAX_SECTORS) {
        print("Not enough space to save file\n");
        return;
    }

    // find empty slot in file table and claim it
    for (int i = 0; i < MAX_FS_FILES; i++) {
        if (!file_table[i].used) {
            file_table[i].used = 1;
            strncpy(file_table[i].name, name, MAX_FILENAME - 1);
            file_table[i].name[MAX_FILENAME - 1] = '\0';
            strncpy(file_table[i].folder, folder, MAX_FILENAME - 1);
            file_table[i].folder[MAX_FILENAME - 1] = '\0';
            file_table[i].size = size;
            file_table[i].start_sector = start_sector;

            // write file data to disk like a fucking typewriter
            for (int s = 0; s < sectors_needed; s++) {
                ata_write_sector(start_sector + s, (uint16_t*)(data + s * 512));
            }

            start_sector += sectors_needed;
            ata_write_sector(FS_TABLE_SECTOR, (uint16_t*)file_table);

            print("File saved\n");
            return;
        }
    }

    print("No space in file table!\n");
}

void fs_delete_file(const char* full_path) {
    if (!full_path) {
        print("Path is null!\n");
        return;
    }
    
    char folder[MAX_FILENAME] = {0};
    char name[MAX_FILENAME] = {0};
    int is_folder = 0;

    const char* slash = strrchr(full_path, '/');
    if (slash) {
        // yet another round of path parsing hell
        int folder_len = slash - full_path;
        if (folder_len >= MAX_FILENAME) folder_len = MAX_FILENAME - 1;
        strncpy(folder, full_path, folder_len);
        folder[folder_len] = '\0';
        strncpy(name, slash + 1, MAX_FILENAME - 1);
        name[MAX_FILENAME - 1] = '\0';
    } else {
        // simple name no path bullshit :3
        strncpy(name, full_path, MAX_FILENAME - 1);
        name[MAX_FILENAME - 1] = '\0';
        folder[0] = '\0';
    }

    // check if this is a folder
    for (int i = 0; i < MAX_FS_FILES; i++) {
        if (file_table[i].used &&
            file_table[i].start_sector == 0xFFFF &&
            strcmp(file_table[i].name, name) == 0 &&
            strcmp(file_table[i].folder, folder) == 0) {
            is_folder = 1;
            break;
        }
    }

    int deleted = 0;

    // time to delete some shit
    for (int i = 0; i < MAX_FS_FILES; i++) {
        if (!file_table[i].used)
            continue;

        // if folder :3
        if (is_folder) {
            char target_folder_path[256];
            if (strlen(folder) > 0) {
                int ret = snprintf(target_folder_path, sizeof(target_folder_path), "%s/%s", folder, name);
                if (ret >= sizeof(target_folder_path)) continue;
            } else {
                strncpy(target_folder_path, name, sizeof(target_folder_path) - 1);
                target_folder_path[sizeof(target_folder_path) - 1] = '\0';
            }

            // delete the folder entry itself
            if (strcmp(file_table[i].folder, folder) == 0 &&
                strcmp(file_table[i].name, name) == 0) {
                print("Deleting folder entry\n");
                file_table[i].used = 0;
                deleted++;
            }
            // delete any files or folders inside this folder
            else if (strncmp(file_table[i].folder, target_folder_path, strlen(target_folder_path)) == 0) {
                int target_len = strlen(target_folder_path);
                if (file_table[i].folder[target_len] == '/' || file_table[i].folder[target_len] == '\0') {
                    print("Deleting file inside folder\n");
                    file_table[i].used = 0;
                    deleted++;
                }
            }
        }
        // if file, exact match or gtfo
        else if (strcmp(file_table[i].folder, folder) == 0 &&
                 strcmp(file_table[i].name, name) == 0) {
            print("Deleting file\n");
            file_table[i].used = 0;
            deleted++;
        }
    }

    ata_write_sector(FS_TABLE_SECTOR, (uint16_t*)file_table);

    if (deleted > 0) {
        print_dec(deleted);
        print(" file(s)/folder(s) deleted\n");
    } else {
        print("No files matched\n");
    }
}

void fs_read_file(const char* full_path, char* out, const char* current_dir) {
    if (!full_path || !out) return;
    
    char folder[MAX_FILENAME] = {0};
    char name[MAX_FILENAME] = {0};

    const char* slash = strrchr(full_path, '/');
    if (slash) {
        // wowwww, more path parsing fun
        int folder_len = slash - full_path;
        if (folder_len >= MAX_FILENAME) folder_len = MAX_FILENAME - 1;
        strncpy(folder, full_path, folder_len);
        folder[folder_len] = '\0';
        strncpy(name, slash + 1, MAX_FILENAME - 1);
        name[MAX_FILENAME - 1] = '\0';
    } else {
        strncpy(name, full_path, MAX_FILENAME - 1);
        name[MAX_FILENAME - 1] = '\0';
        if (current_dir) {
            strncpy(folder, current_dir, MAX_FILENAME - 1);
            folder[MAX_FILENAME - 1] = '\0';
        } else {
            folder[0] = '\0';
        }
    }

    // find the damn file and read it
    for (int i = 0; i < MAX_FS_FILES; i++) {
        if (file_table[i].used &&
            strcmp(file_table[i].name, name) == 0 &&
            strcmp(file_table[i].folder, folder) == 0) {

            uint32_t size = file_table[i].size;
            uint32_t start = file_table[i].start_sector;
            uint32_t sectors = (size + 511) / 512;

            // read all the sectors like a hungry bitch (not like neco because, i'm a picky eater)
            for (uint32_t s = 0; s < sectors; s++) {
                ata_read_sector(start + s, (uint16_t*)(out + s * 512));
            }

            out[size] = '\0';
            return;
        }
    }

    print("File not found\n");
    out[0] = '\0';
}

uint32_t fs_get_free_space() {
    uint32_t used_bytes = 0;
    for (int i = 0; i < MAX_FS_FILES; i++) {
        if (file_table[i].used) {
            used_bytes += file_table[i].size;
        }
    }
    return FS_TOTAL_SPACE - used_bytes;
}