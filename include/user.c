#include "user.h"
#include "fs.h"
#include <string.h>
#include <stdbool.h>
#include "vga.h"

User users[MAX_USERS];
int user_count = 0;
char logged_in_user[MAX_USERNAME] = "";

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
    
    if (!fs_exists("users")) {
        fs_make_dir("users");
    }
    
    fs_make_dirs(user_folder); // yay users/username
    
    char user_file[128];
    snprintf(user_file, sizeof(user_file), "%s/extra/creds.txt", user_folder);
    fs_save_file(user_file, password);
}

int user_exists(const char* username) {
    char path[256];
    snprintf(path, sizeof(path), "users/%s", username);
    return fs_is_folder(path);
}

bool check_password(const char* username, const char* password) {
    char pass_file[256];
    int ret = snprintf(pass_file, sizeof(pass_file), "users/%s/extra/creds.txt", username);

    if (!user_exists(username) == 1) {
        return false;
    }

    if (username[0] == '\0') {
        return false;
    }
   
    char stored_pass[256] = {0};
    read_file_safe(pass_file, stored_pass, "");
   
    size_t len = strlen(stored_pass);
    while (len > 0 && (stored_pass[len - 1] == '\n' || stored_pass[len - 1] == '\r' || stored_pass[len - 1] == ' ' || stored_pass[len - 1] == '\t')) {
        stored_pass[len - 1] = '\0';
        len--;
    }
   
    char trimmed_input[256];
    strcpy(trimmed_input, password);
    len = strlen(trimmed_input);
    while (len > 0 && (trimmed_input[len - 1] == '\n' || trimmed_input[len - 1] == '\r' || trimmed_input[len - 1] == ' ' || trimmed_input[len - 1] == '\t')) {
        trimmed_input[len - 1] = '\0';
        len--;
    }

    bool result = strcmp(stored_pass, trimmed_input) == 0;
   
    return result;
}

void init_user() {
    if (fs_is_folder("users") == 2) {
        create_user("root", "");
        strcpy(logged_in_user, "root"); // log in as root
        return;
    }
    
    bool has_users = false;
    
    // try to find the root user
    if (user_exists("root") == 1 ) {
        has_users = true;
    } else {
        for (int i = 0; i < MAX_FS_FILES; i++) {
            if (file_table[i].used && 
                strncmp(file_table[i].folder, "users/", 6) == 0) {
                has_users = true;
                break;
            }
        }
    }
    
    if (!has_users) {
        create_user("root", "");
    }
}
