#include "user.h"
#include "fs.h"
#include <string.h>
#include <stdbool.h>
#include "vga.h"

User users[MAX_USERS];
int user_count = 0;
char logged_in_user[MAX_USERNAME] = "";

// i'm too lazy to put it on the fs header
bool fs_folder_exists(const char* path) {
    FileEntry* entry = fs_get_file_entry(path);
    return entry != NULL;
}

bool read_file_safe(const char* path, char* buffer, const char* current_dir) {
    // clear buffer
    memset(buffer, 0, 64);
    fs_read_file(path, buffer, current_dir);
    if (buffer[0] == '\0') {
        return false;  // no data read
    }
    return true;
}

void create_user(const char* username, const char* password) {
    char user_folder[128];
    snprintf(user_folder, sizeof(user_folder), "users/%s", username);
    
    if (!fs_folder_exists("users")) {
        fs_make_dir("users");
    }
    
    fs_make_dirs(user_folder); // yay users/username
    
    char user_file[128];
    snprintf(user_file, sizeof(user_file), "%s/creds.txt", user_folder);
    fs_save_file(user_file, password);
}

bool user_exists(const char* username) {
    char user_dir[64];
    snprintf(user_dir, sizeof(user_dir), "users/%s", username);
    return fs_folder_exists(user_dir);
}

bool check_password(const char* username, const char* password) {
    char pass_file[256];  // Increased size
    int ret = snprintf(pass_file, sizeof(pass_file), "users/%s/creds.txt", username);
    if (ret >= sizeof(pass_file)) {
        // fuck ass path too long
        return false;
    }
    
    char stored_pass[256] = {0};  // increased size
    if (!read_file_safe(pass_file, stored_pass, "")) {
        return false;
    }
    
    size_t len = strlen(stored_pass);
    while (len > 0 && (stored_pass[len - 1] == '\n' || stored_pass[len - 1] == '\r')) {
        stored_pass[len - 1] = '\0';
        len--;
    }
    
    return strcmp(stored_pass, password) == 0;
}