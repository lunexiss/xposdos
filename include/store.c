#include "store.h"
#include "string.h"
#include "fs.h"
#include "vga.h"
#include <stddef.h>

typedef struct {
    char key[16];
    char value[64];
} Storage;

#define MAX_KEYS 16
static Storage store[MAX_KEYS];
static int storage_count = 0;

void save_to_store(const char* key, const char* value) {
    for (int i = 0; i < storage_count; i++) {
        if (strcmp(store[i].key, key) == 0) {
            strncpy(store[i].value, value, 64);
            goto write;
        }
    }
    if (storage_count < MAX_KEYS) {
        strncpy(store[storage_count].key, key, 16);
        strncpy(store[storage_count].value, value, 64);
        storage_count++;
    } else {
        print("Storage full!\n");
        return;
    }

write:
    fs_save_file("store.db", (char*)store);
}

void load_store_from_disk() {
    char buf[MAX_KEYS * sizeof(Storage)] = {0};
    fs_read_file("store.db", buf, "");
    memcpy(store, buf, sizeof(store));
    storage_count = MAX_KEYS;
}

const char* load_from_store(const char* key) {
    for (int i = 0; i < storage_count; i++) {
        if (strcmp(store[i].key, key) == 0) {
            return store[i].value;
        }
    }
    return NULL;
}
