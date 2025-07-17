// old fs.c because, i broke the new fs.c

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

void fs_make_dir(const char* full_path) {
    char folder[MAX_FILENAME] = {0};
    char name[MAX_FILENAME] = {0};

    // use last slash to split path into folder and name
    const char* slash = strrchr(full_path, '/');
    if (slash) {
        int folder_len = slash - full_path;
        strncpy(folder, full_path, folder_len);
        folder[folder_len] = '\0';
        strcpy(name, slash + 1);
    } else {
        strcpy(name, full_path);
        strcpy(folder, "");  // root
    }

    // check if folder already exists
    for (int i = 0; i < MAX_FS_FILES; i++) {
        if (file_table[i].used &&
            file_table[i].start_sector == 0xFFFF &&
            strcmp(file_table[i].name, name) == 0 &&
            strcmp(file_table[i].folder, folder) == 0) {
            print("Folder already exists!\n");
            return;
        }
    }

    // create new folder
    for (int i = 0; i < MAX_FS_FILES; i++) {
        if (!file_table[i].used) {
            file_table[i].used = 1;
            strncpy(file_table[i].name, name, MAX_FILENAME);
            strncpy(file_table[i].folder, folder, MAX_FILENAME);
            file_table[i].size = 0;
            file_table[i].start_sector = 0xFFFF;  // mark as folder

            ata_write_sector(FS_TABLE_SECTOR, (uint16_t*)file_table);
            print("Folder created\n");
            return;
        }
    }

    print("No space in file table!\n");
}

void fs_save_file(const char* full_path, const char* data) {
    char folder[MAX_FILENAME * 2] = {0};
    char name[MAX_FILENAME] = {0};
    uint32_t size = strlen(data);

    const char* slash = strrchr(full_path, '/');
    if (slash) {
        int folder_len = slash - full_path;
        strncpy(folder, full_path, folder_len);
        folder[folder_len] = '\0';
        strcpy(name, slash + 1);
    } else {
        strcpy(name, full_path);
        strcpy(folder, "");
    }

    // h
    if (strlen(folder) > 0) {
        char path[MAX_FILENAME * 2] = {0};
        int len = strlen(folder);
        for (int i = 0; i <= len; i++) {
            if (folder[i] == '/' || folder[i] == '\0') {
                char temp[MAX_FILENAME] = {0};
                strncpy(temp, folder, i);
                temp[i] = '\0';

                const char* slash = strrchr(temp, '/');
                char parent[MAX_FILENAME] = {0};
                char fname[MAX_FILENAME] = {0};

                if (slash) {
                    int parent_len = slash - temp;
                    strncpy(parent, temp, parent_len);
                    parent[parent_len] = '\0';
                    strcpy(fname, slash + 1);
                } else {
                    strcpy(fname, temp);
                    strcpy(parent, "");
                }

                int exists = 0;
                for (int j = 0; j < MAX_FS_FILES; j++) {
                    if (file_table[j].used &&
                        file_table[j].start_sector == 0xFFFF &&
                        strcmp(file_table[j].name, fname) == 0 &&
                        strcmp(file_table[j].folder, parent) == 0) {
                        exists = 1;
                        break;
                    }
                }

                if (!exists && strlen(fname) > 0) {
                    for (int k = 0; k < MAX_FS_FILES; k++) {
                        if (!file_table[k].used) {
                            file_table[k].used = 1;
                            strncpy(file_table[k].name, fname, MAX_FILENAME);
                            strncpy(file_table[k].folder, parent, MAX_FILENAME);
                            file_table[k].size = 0;
                            file_table[k].start_sector = 0xFFFF;
                            break;
                        }
                    }
                }
            }
        }
    }

    // check if file already exist
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

    for (int i = 0; i < MAX_FS_FILES; i++) {
        if (!file_table[i].used) {
            file_table[i].used = 1;
            strncpy(file_table[i].name, name, MAX_FILENAME);
            strncpy(file_table[i].folder, folder, MAX_FILENAME);
            file_table[i].size = size;
            file_table[i].start_sector = start_sector;

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

void fs_delete_file(const char* pattern) {
    print("Deleting: ");
    print(pattern);
    print("\n");

    int deleted = 0;

    int is_folder = 0;
    char folder[MAX_FILENAME] = {0};
    char name[MAX_FILENAME] = {0};

    const char* slash = strchr(pattern, '/');
    if (!slash) {
        // check if it's a folder
        strncpy(folder, pattern, MAX_FILENAME);
        is_folder = 1;
    } else {
        int folder_len = slash - pattern;
        strncpy(folder, pattern, folder_len);
        folder[folder_len] = '\0';
        strcpy(name, slash + 1);
    }

    for (int i = 0; i < MAX_FS_FILES; i++) {
        if (!file_table[i].used) continue;

        if (is_folder) {
            // FUCK EVERYTHING IN THAT SHIT ASS FOLDER
            if (strcmp(file_table[i].folder, folder) == 0) {
                file_table[i].used = 0;
                deleted++;
            }
        } else {
            // exact match
            if (strcmp(file_table[i].folder, folder) == 0 &&
                strcmp(file_table[i].name, name) == 0) {
                file_table[i].used = 0;
                deleted++;
            }
        }
    }

    ata_write_sector(FS_TABLE_SECTOR, (uint16_t*)file_table);

    if (deleted == 0)
        print("No files matched\n");
    else {
        print_dec(deleted);
        print(" file(s)/folder(s) deleted\n");
    }
}

void fs_read_file(const char* full_path, char* out, const char* current_dir) {
    char folder[MAX_FILENAME] = {0};
    char name[MAX_FILENAME] = {0};

    const char* slash = strrchr(full_path, '/');
    if (slash) {
        int folder_len = slash - full_path;
        if (folder_len >= MAX_FILENAME) folder_len = MAX_FILENAME - 1;
        strncpy(folder, full_path, folder_len);
        folder[folder_len] = '\0';
        strcpy(name, slash + 1);
    } else {
        strcpy(name, full_path);
        strcpy(folder, current_dir); // use current_dir when no slash
    }

    for (int i = 0; i < MAX_FS_FILES; i++) {
        if (file_table[i].used &&
            strcmp(file_table[i].name, name) == 0 &&
            strcmp(file_table[i].folder, folder) == 0) {

            uint32_t size = file_table[i].size;
            uint32_t start = file_table[i].start_sector;
            uint32_t sectors = (size + 511) / 512;

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
