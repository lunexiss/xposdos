// five little skidders jumping on the bed
// one fell off and bumped his head
// mama called the doctor and the doctor said
// "no more skidders jumping on the bed"

// cool song, right?

// this is a shell for XPOSD, a simple operating system

#include "vga.h"
#include "string.h"
#include "input.h"
#include <stdbool.h>
#include "io.h"
#include "shell.h"
#include "fs.h"
#include "disk.h"
#include "store.h"
#include "user.h"
#include "drivers/sound/beep.h"
#include "net/ip.h"
#include "drivers/e1000/e1000.h"
// #include "grub_core_img.h"
// #include "grub_mbr.h"
// #include "kernel_elf.h"

#define MAX_VARS 100
#define MAX_VAR_NAME 100
#define MAX_VAR_VALUE 128

void handle_command(char* input);

void shell() {
    char input[128];

    print("Welcome to <blue>XPOSD</blue>\n");

    while (1) {
        print("> ");

        gets(input, &(int){0});  // wait for input

        uint8_t sc;
        do {
            sc = inb(0x60);
        } while (sc != 0x9C);  // enter key release shit (idk why ts not working)

        if (strlen(input) == 0) continue;

        handle_command(input);
    }
}

char current_dir[MAX_FILENAME * 2] = "";

void handle_command(char* input) {
    if (strcmp(input, "help") == 0) {
        print("Available commands:\n");
        print("  help     - Show this help message\n");
        print("  clear    - Clear the screen\n");
        print("  sh       - Run shell script\n");
        print("  reboot   - Reboot the system\n");
        print("  echo     - Print text to the console\n");
        print("  cfdisk   - Partition management\n");
        print("  mksec    - Create a new section\n");
        print("  mkpart   - Create a new partition\n");
        print("  read     - Read from a file\n");
        print("  mount    - Mount a filesystem\n");
        print("  mkfs     - Create a new filesystem\n");
        print("  mkdir    - Create a new directory\n");
        print("  mkfile   - Create a new file\n");
        print("  whereami - Show current directory\n");
        print("  cd       - Change directory\n");
        print("  ls       - List directory contents\n");
        print("  rm       - Remove file or directory\n");
        print("  write    - Write to a file\n");
        print("  df       - Show free disk space\n");
    } else if (strcmp(input, "clear") == 0) {
        clear_screen();
    } else if (strncmp(input, "sh ", 3) == 0) {
        const char* path = input + 3;
        char content[2048] = {0};

        char full_path[64] = {0};
        if (path[0] == '/') {
            strncpy(full_path, path, sizeof(full_path) - 1);
        } else {
            snprintf(full_path, sizeof(full_path), "%s/%s", current_dir, path);
        }

        fs_read_file(full_path, content, current_dir);

        char* line = strtok(content, "\n");
        while (line) {
            handle_command(line);
            line = strtok(NULL, "\n");
        }
    } else if (strncmp(input, "cfdisk ", 3) == 0) {
        uint16_t buf[256];
        ata_read_sector(0, buf);
        uint8_t* mbr = (uint8_t*)buf;

        for (int i = 0; i < 4; i++) {
            int offset = 0x1BE + i * 16;
            uint8_t boot_flag = mbr[offset];
            uint8_t type = mbr[offset + 4];
            uint32_t lba_start = *(uint32_t*)&mbr[offset + 8];
            uint32_t sectors = *(uint32_t*)&mbr[offset + 12];

            if (type == 0 || sectors == 0) continue;

            print("Partition ");
            put_char('1' + i);
            print(": ");
            print(boot_flag == 0x80 ? "[BOOT] " : "       ");

            print("Type 0x"); print_hex(type);
            print("  Start: "); print_hex((lba_start >> 24) & 0xFF);
            print_hex((lba_start >> 16) & 0xFF);
            print_hex((lba_start >> 8) & 0xFF);
            print_hex(lba_start & 0xFF);

            print("  Size: ");
            print_hex((sectors >> 24) & 0xFF);
            print_hex((sectors >> 16) & 0xFF);
            print_hex((sectors >> 8) & 0xFF);
            print_hex(sectors & 0xFF);
            print("\n");
        }
    } else if (strncmp(input, "mount ", 6) == 0) {
        int part = atoi(input + 6);
        if (mount_partition(part)) {
            print("Mounted partition ");
            print_dec(part);
            print("\n");
        } else {
            print("Failed to mount partition ");
            print_dec(part);
            print("\n");
        }
    } else if (strncmp(input, "mkfs ", 5) == 0) {
        int part = atoi(input + 5);
        if (part < 1 || part > 4) {
            print("Invalid partition number\n");
            return;
        }

        uint16_t mbr[256];
        ata_read_sector(0, mbr);
        uint8_t* mbr_bytes = (uint8_t*)mbr;

        int offset = 0x1BE + (part - 1) * 16;
        uint32_t lba_start = *(uint32_t*)&mbr_bytes[offset + 8];
        uint32_t sectors = *(uint32_t*)&mbr_bytes[offset + 12];

        if (lba_start == 0 || sectors == 0) {
            print("Partition not found\n");
            return;
        }

        uint8_t boot[512] = {0};
        boot[0] = 'X'; boot[1] = 'F'; boot[2] = 'S';
        boot[3] = 1;
        *(uint32_t*)&boot[4] = sectors;

        boot[510] = 0x55;
        boot[511] = 0xAA;

        ata_write_sector(lba_start, (uint16_t*)boot);
        print("Formatted partition ");
        print_dec(part);
        print("\n");
    } else if (strcmp(input, "df") == 0 || strcmp(input, "free") == 0) {
        uint32_t free = fs_get_free_space();
        print("Free space: ");
        print_dec(free);
        print(" bytes (");
        print_dec(free / 1024);
        print(" KB / ");
        print_dec(free / 1024 / 1024);
        print(" MB)\n");
    } else if (strncmp(input, "mkdir ", 6) == 0) {
        const char* path = input + 6;
        char full_path[64] = {0};

        if (path[0] == '/') {
            strncpy(full_path, path, sizeof(full_path) - 1);
        } else {
            snprintf(full_path, sizeof(full_path), "%s/%s", current_dir, path);
        }
        fs_make_dir(full_path);
    } else if (strncmp(input, "set ", 4) == 0) {
        const char* eq = strchr(input + 4, '=');
        if (eq) {
            int name_len = eq - (input + 4);
            char name[MAX_VAR_NAME] = {0};
            char value[MAX_VAR_VALUE] = {0};

            strncpy(name, input + 4, name_len);
            name[name_len] = '\0';
            strcpy(value, eq + 1);

            save_to_store(name, value);
            
            return;
        }
    } else if (strncmp(input, "mkfile ", 7) == 0) {
        const char* path = input + 7;
        char full_path[64] = {0};

        if (path[0] == '/') {
            strncpy(full_path, path, sizeof(full_path) - 1);
        } else {
            snprintf(full_path, sizeof(full_path), "%s/%s", current_dir, path);
        }
        fs_save_file(full_path, "");
    } else if (strncmp(input, "ls", 2) == 0) {
        const char* path = current_dir;
        if (strlen(input) > 3 && input[2] == ' ') {
            path = input + 3;
        }

        for (int i = 0; i < MAX_FS_FILES; i++) {
            if (!file_table[i].used) continue;

            if (strcmp(file_table[i].folder, path) == 0) {
                if (file_table[i].start_sector == 0xFFFF)
                    print("[DIR] ");
                else
                    print("      ");

                print(file_table[i].name);
                print("\n");
            }
        }
    } else if (strncmp(input, "read ", 5) == 0) {
        const char* path = input + 5;
        char content[1024] = {0};
        fs_read_file(path, content, current_dir);
        print(content);
        print("\n");
    } else if (strncmp(input, "echo ", 5) == 0) {
        const char* echo_str = input + 5;
        const char* arrow = strstr(echo_str, " > ");

        if (arrow) {
            int text_len = arrow - echo_str;
            char data[512] = {0};
            strncpy(data, echo_str, text_len);
            data[text_len] = '\0';
            const char* path = arrow + 3;
            char full_path[64] = {0};
            if (path[0] == '/') {
                strncpy(full_path, path, sizeof(full_path) - 1);
            } else {
                snprintf(full_path, sizeof(full_path), "%s/%s", current_dir, path);
            }

            fs_save_file(full_path, data);
            print(data);
            print("\n");
        } else {
            print(echo_str);
            print("\n");
        }
    
    } else if (strncmp(input, "write ", 6) == 0) {
        const char* args = input + 6;
        const char* space = strchr(args, ' ');
        if (!space) {
            print("Usage: write (file) (text)\n");
            return;
        }

        int file_len = space - args;
        char filename[64] = {0};
        strncpy(filename, args, file_len);
        filename[file_len] = '\0';

        char file_path[128] = {0};
        if (filename[0] == '/') {
            strncpy(file_path, filename, sizeof(file_path) - 1);
        } else {
            snprintf(file_path, sizeof(file_path), "%s/%s", current_dir, filename);
        }

        // extract this stupid fucking text
        const char* text = space + 1;

        fs_save_file(file_path, text);
        print("Written to file: ");
        print(file_path);
        print("\n");
    } else if (strncmp(input, "rm ", 3) == 0) {
        const char* arg = input + 3;
        char full_path[64] = {0};

        fs_delete_file(full_path);
    } else if (strncmp(input, "cd ", 3) == 0) {
        const char* target = input + 3;

        if (strcmp(target, "/") == 0) {
            current_dir[0] = '\0';
            return;
        }

        if (strcmp(target, "..") == 0) {
            char* slash = (char*)strrchr(current_dir, '/');
            if (slash) *slash = '\0';
            else current_dir[0] = '\0';
        } else {
            int found = 0;
            for (int i = 0; i < MAX_FS_FILES; i++) {
                if (file_table[i].used &&
                    file_table[i].start_sector == 0xFFFF &&
                    strcmp(file_table[i].name, target) == 0 &&
                    strcmp(file_table[i].folder, current_dir) == 0) {
                    found = 1;
                    break;
                }
            }

            if (found) {
                if (strlen(current_dir) == 0) {
                    strcpy(current_dir, target);
                } else {
                    strcat(current_dir, "/");
                    strcat(current_dir, target);
                }
            } else {
                print("Directory not found\n");
            }
        }
    } else if (strcmp(input, "whereami") == 0) {
        print("/");
        print(current_dir);
        print("\n");
    } else if (strncmp(input, "reboot", 6) == 0) {
        print("Rebooting...\n");
        outb(0x64, 0xFE); // send reboot command :3
        while (1) { /* why r u watching dis */ }
    } else if (strncmp(input, "mksec ", 6) == 0) {
        const char* lba_str = input + 6;
        uint32_t lba = atoi(lba_str);

        uint16_t zero_sector[256];  // 512 bytes is 256 words, i guess...
        memset(zero_sector, 0, sizeof(zero_sector));
        ata_write_sector(lba, zero_sector);

        print("Created empty sector at LBA ");
        print_dec(lba);
        print("\n");
    } else if (strncmp(input, "mkpart ", 7) == 0) {
        const char* lba_str = input + 7;
        uint32_t lba = atoi(lba_str);  // start of partition

        uint16_t mbr[256]; // 512 bytes

        ata_read_sector(0, mbr);

        uint8_t* mbr_bytes = (uint8_t*)mbr;

        int entry = 0;
        int offset = 0x1BE + entry * 16;

        mbr_bytes[offset] = 0x80;
        mbr_bytes[offset + 4] = 0x83;

        *(uint32_t*)&mbr_bytes[offset + 8] = lba;
        *(uint32_t*)&mbr_bytes[offset + 12] = 100;

        mbr_bytes[510] = 0x55;
        mbr_bytes[511] = 0xAA;

        ata_write_sector(0, mbr);

        print("Partition created at LBA ");
        print_dec(lba);
        print("\n");
    } else if (strncmp(input, "beep", 4) == 0) {
        beep();
    } else if (strncmp(input, "stopbeep", 8) == 0) {
        nosound();
    } else if (strncmp(input, "ping", 4) == 0) {
        uint8_t google_dns[4] = {8, 8, 8, 8};
        ping(global_e1000, google_dns);
    } else if (strncmp(input, "register ", 9) == 0) {
        const char* args = input + 9;
        const char* space = strchr(args, ' ');
        if (!space) {
            print("Usage: register (username) (password)\n");
            return;
        }

        int user_len = space - args;
        char username[64] = {0};
        strncpy(username, args, user_len);
        username[user_len] = '\0';

        const char* password = space + 1;

        if (user_exists(username)) {
            print("User already exists\n");
            return;
        }

        create_user(username, password);
        print("User registered successfully\n");
        return;
    } else if (strncmp(input, "login ", 6) == 0) {
        const char* args = input + 6;
        const char* space = strchr(args, ' ');

        if (!space) {
            print("Usage: login (username) (password)\n");
            return;
        }

        int user_len = space - args;
        char username[64] = {0};
        strncpy(username, args, user_len);
        username[user_len] = '\0';

        const char* password = space + 1;

        if (!user_exists(username)) {
            print("User not found\n");
            return;
        }

        if (check_password(username, password)) {
            strcpy(logged_in_user, username);
            print("Login successful\n");
        } else {
            print("Incorrect password\n");
        }
        return;
    } else {
        print("Unknown command: ");
        print(input);
        print("\n");
    }
}