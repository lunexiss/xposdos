#include "user.h"
#include "fs.h"
#include <string.h>
#include <stdbool.h>
#include "vga.h"

// Globals
User users[MAX_USERS];
int user_count = 0;
char logged_in_user[MAX_USERNAME] = "";

// i'm too lazy to put it on the fs header
bool fs_dir_exists(const char* path) {
    char dummy[512] = {0};
    fs_read_file(path, dummy, "");
    return dummy[0] != 0;
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
    char user_dir[64];
    snprintf(user_dir, sizeof(user_dir), "/users/%s", username);
    
    if (username == NULL) {
        print("username is NULL\n");
        return;
    }
    if (username[0] == '\0') {
        print("username is empty\n");
        return;
    }

    if (!fs_dir_exists("/users")) {
        fs_make_dir("/users");
    }

    fs_make_dir(user_dir);

    char pass_file[80];
    snprintf(pass_file, sizeof(pass_file), "%s/pass", user_dir);
    fs_save_file(pass_file, password);
}

bool user_exists(const char* username) {
    char user_dir[64];
    snprintf(user_dir, sizeof(user_dir), "/users/%s", username);

    return fs_dir_exists(user_dir);
}

bool check_password(const char* username, const char* password) {
    char pass_file[80];
    snprintf(pass_file, sizeof(pass_file), "/users/%s/pass", username);
    char stored_pass[64] = {0};

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
