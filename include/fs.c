#include "fs.h"
#include "string.h"
#include "vga.h"
#include "disk.h"
#include <stddef.h>
#include <stdint.h>
#include "string.h"
#include "stdbool.h"
FileEntry file_table[MAX_FS_FILES];

uint16_t start_sector = FS_START_SECTOR;

uint32_t mounted_lba = 0;
uint32_t mounted_size = 0;
bool fs_mounted = false;

FileEntry file_table[MAX_FS_FILES];

char mounted_fs_name[64] = {0};
uint32_t mounted_start_sector = 0;
uint32_t mounted_total_sectors = 0;

uint32_t table_sector = FS_TABLE_SECTOR;

uint32_t fs_get_total_space() {
    return mounted_size * 512; // total bytes
}

uint32_t fs_get_max_sectors() {
    return mounted_size;
}

bool mount_partition(int part) {
    if (part < 1 || part > 4) return false;

    uint16_t mbr[256];
    ata_read_sector(0, mbr);
    uint8_t* mbr_bytes = (uint8_t*)mbr;
    int offset = 0x1BE + (part - 1) * 16;

    uint32_t lba_start = *(uint32_t*)&mbr_bytes[offset + 8];
    uint32_t sectors = *(uint32_t*)&mbr_bytes[offset + 12];

    if (lba_start == 0 || sectors == 0) return false;

    fs_mount("XPOSD_FS", lba_start, sectors);
    return true;
}

void fs_format() {
    uint32_t table_sector = 20; // just an example sector
    fs_write_table(table_sector, file_table);

    ata_write_sector(1, (uint8_t*)&table_sector);
}

void fs_load_file_table() {
    uint32_t table_sector = fs_get_table_sector();
    ata_read_sector(table_sector, (uint16_t*)file_table);
}

void fs_write_table(uint32_t sector, FileEntry* table) {
    ata_write_sector(sector, (uint16_t*)table);
}

void fs_init() {
    if (!fs_mounted) {
        print("FS not mounted\n");
        return;
    }

    fs_load_file_table();

    start_sector = mounted_start_sector;
    for (int i = 0; i < MAX_FS_FILES; i++) {
        if (file_table[i].used && file_table[i].start_sector != 0xFFFF) {
            uint32_t end_sector = file_table[i].start_sector + (file_table[i].size + 511) / 512;
            if (end_sector > start_sector) {
                start_sector = end_sector;
            }
        }
    }
}

uint32_t fs_get_table_sector(void) {
    uint16_t buffer[256];
    ata_read_sector(1, buffer); 
    return buffer[0];
}

void fs_set_table_sector(uint32_t sector) {
    uint16_t buffer[256] = {0};
    buffer[0] = (uint16_t)sector;
    ata_write_sector(1, buffer);
}

void fs_make_dir(const char* full_path) {
    char folder[MAX_FILENAME] = {0};
    char name[MAX_FILENAME] = {0};

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

    for (int i = 0; i < MAX_FS_FILES; i++) {
        if (file_table[i].used &&
            file_table[i].start_sector == 0xFFFF &&
            strcmp(file_table[i].name, name) == 0 &&
            strcmp(file_table[i].folder, folder) == 0) {
            print("Folder already exists!\n");
            return;
        }
    }

    // i like egg
    for (int i = 0; i < MAX_FS_FILES; i++) {
        if (!file_table[i].used) {
            file_table[i].used = 1;
            strncpy(file_table[i].name, name, MAX_FILENAME);
            strncpy(file_table[i].folder, folder, MAX_FILENAME);
            file_table[i].size = 0;
            file_table[i].start_sector = 0xFFFF;  // mark as folder

            uint32_t table_sector = fs_get_table_sector();
            ata_write_sector(table_sector, (uint16_t*)file_table);
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
                            file_table[k].start_sector = 0xFFFF; // mark as folder
                            break;
                        }
                    }
                }
            }
        }
    }

    for (int i = 0; i < MAX_FS_FILES; i++) {
        if (file_table[i].used &&
            strcmp(file_table[i].name, name) == 0 &&
            strcmp(file_table[i].folder, folder) == 0) {
            print("File already exists!\n");
            return;
        }
    }

    // me when save file :3
    for (int i = 0; i < MAX_FS_FILES; i++) {
        if (!file_table[i].used) {
            file_table[i].used = 1;
            strncpy(file_table[i].name, name, MAX_FILENAME);
            strncpy(file_table[i].folder, folder, MAX_FILENAME);
            file_table[i].size = size;
            file_table[i].start_sector = start_sector;

            int sectors_needed = (size + 511) / 512;
            for (int s = 0; s < sectors_needed; s++) {
                if (start_sector + sectors_needed > fs_get_max_sectors()) {
                    print("Disk full! Cannot save file\n");
                    return;
                }
                ata_write_sector(start_sector + s, (uint16_t*)(data + s * 512));
            }

            start_sector += sectors_needed;
            ata_write_sector(FS_TABLE_SECTOR, (uint16_t*)file_table);

            print("File saved\n");
            fs_save_table(table_sector, file_table);
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
        strncpy(folder, pattern, MAX_FILENAME);
        is_folder = 1;
    } else {
        int folder_len = slash - pattern;
        strncpy(folder, pattern, folder_len);
        folder[folder_len] = '\0';
        strcpy(name, slash + 1);
    }

    uint8_t empty_sector[512] = {0};

    for (int i = 0; i < MAX_FS_FILES; i++) {
        if (!file_table[i].used) continue;

        int match = 0;
        if (is_folder) {
            match = strcmp(file_table[i].folder, folder) == 0;
        } else {
            match = strcmp(file_table[i].folder, folder) == 0 &&
                    strcmp(file_table[i].name, name) == 0;
        }

        if (match) {
            if (file_table[i].start_sector != 0xFFFF) { // don't wipe folders
                uint32_t size = file_table[i].size;
                uint32_t start = file_table[i].start_sector;
                uint32_t sectors = (size + 511) / 512;

                for (uint32_t j = 0; j < sectors; j++) {
                    ata_write_sector(start + j, (uint16_t*)empty_sector);
                }
            }

            file_table[i].used = 0;
            deleted++;
        }
    }

    ata_write_sector(FS_TABLE_SECTOR, (uint16_t*)file_table);

    if (deleted == 0)
        print("No files matched\n");
    else {
        print_dec(deleted);
        print(" file(s)/folder(s) deleted and wiped\n");
    }
}

void fs_read_table(uint32_t table_sector, FileEntry* out_table) {
    ata_read_sector(table_sector, (uint8_t*)out_table);
    //print("Loaded table from disk:\n");
    //for (int i = 0; i < MAX_FS_FILES; i++) {
    //    if (out_table[i].used) {
    //        print("File: ");
    //        print(out_table[i].name);
    //        print("\n");
    //    }
    //}
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
        strcpy(folder, current_dir);
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
    uint32_t used_sectors = 0;

    for (int i = 0; i < MAX_FS_FILES; i++) {
        if (file_table[i].used && file_table[i].start_sector != 0xFFFF) {
            uint32_t sectors = (file_table[i].size + 511) / 512;
            used_sectors += sectors;
        }
    }

    uint32_t reserved_sectors = FS_TABLE_SECTOR;

    // make sure we don’t underflow
    if (mounted_size < used_sectors + reserved_sectors) {
        return 0;
    }

    uint32_t free_sectors = mounted_size - used_sectors - reserved_sectors;
    return free_sectors * 512; // return in bytes
}

void fs_save_file_table() {
    uint32_t table_sector = fs_get_table_sector();
    ata_write_sector(table_sector, (uint16_t*)file_table);
}

void fs_save_table(uint32_t sector, FileEntry* table) {
    ata_write_sector(sector, (uint8_t*)table);
    // print("Saved table to disk\n");
}


void fs_save_file_len(const char* full_path, const char* data, uint32_t size) {
    char folder[MAX_FILENAME * 2] = {0};
    char name[MAX_FILENAME] = {0};

    // split folder/name
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

    // me when auto-create folders
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
                            file_table[k].start_sector = 0xFFFF; // mark as folder
                            break;
                        }
                    }
                }
            }
        }
    }

    for (int i = 0; i < MAX_FS_FILES; i++) {
        if (file_table[i].used &&
            strcmp(file_table[i].name, name) == 0 &&
            strcmp(file_table[i].folder, folder) == 0) {
            print("File already exists!\n");
            return;
        }
    }

    for (int i = 0; i < MAX_FS_FILES; i++) {
        if (!file_table[i].used) {
            file_table[i].used = 1;
            strncpy(file_table[i].name, name, MAX_FILENAME);
            strncpy(file_table[i].folder, folder, MAX_FILENAME);
            file_table[i].size = size;
            file_table[i].start_sector = start_sector;

            int sectors_needed = (size + 511) / 512;
            if (start_sector + sectors_needed > fs_get_max_sectors()) {
                print("Disk full! Cannot save file\n");
                return;
            }

            for (int s = 0; s < sectors_needed; s++) {
                uint8_t sector_buf[512] = {0};
                int chunk_size = (size - s * 512 > 512) ? 512 : (size - s * 512);
                memcpy(sector_buf, data + s * 512, chunk_size);
                ata_write_sector(start_sector + s, (uint16_t*)sector_buf);
            }

            start_sector += sectors_needed;
            ata_write_sector(FS_TABLE_SECTOR, (uint16_t*)file_table);

            print("File saved\n");
            return;
        }
    }

    print("No space in file table!\n");
}

void fs_mount(const char* name, uint16_t start, uint32_t size) {
    strcpy(mounted_fs_name, name);
    mounted_start_sector = start;
    mounted_total_sectors = size;

    mounted_lba = start;
    mounted_size = size;
    fs_mounted = true;

    fs_load_file_table();  // after setting mount info!

    // find last used sector
    start_sector = mounted_start_sector;
    for (int i = 0; i < MAX_FS_FILES; i++) {
        if (file_table[i].used && file_table[i].start_sector != 0xFFFF) {
            uint32_t end = file_table[i].start_sector + (file_table[i].size + 511) / 512;
            if (end > start_sector) {
                start_sector = end;
            }
        }
    }

    char temp[32];
    print("Filesystem '");
    print(name);
    print("' mounted at sector ");
    itoa(start, temp, 10);
    print(temp);
    print(" (size: ");
    itoa(size, temp, 10);
    print(temp);
    print(" sectors)\n");
}