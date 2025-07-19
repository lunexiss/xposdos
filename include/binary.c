#include "binary.h"
#include "fs.h"
#include "disk.h"
#include "string.h"
#include "vga.h"
#include "binary_format.h"

static char memory_pool[1024 * 1024];
static unsigned int pool_used = 0;

void* simple_alloc(unsigned int size) {
    if (pool_used + size > sizeof(memory_pool)) {
        print("simple_alloc failed");
        char buf[16];
        utoa(size, buf, 10);
        print(" requested ");
        print(buf);
        print(" bytes\n");
        return 0;
    }
    void* ptr = &memory_pool[pool_used];
    pool_used += size;
    return ptr;
}

int load_binary(const char* path, void** entry_point) {
    for (int i = 0; i < MAX_FS_FILES; i++) {
        if (!file_table[i].used || file_table[i].start_sector == 0xFFFF)
            continue;
            
        char full_path[MAX_FILENAME * 2 + 2] = {0};
        if (strlen(file_table[i].folder) > 0) {
            strcpy(full_path, file_table[i].folder);
            strcat(full_path, "/");
        }
        strncat(full_path, file_table[i].name, MAX_FILENAME);
        
        if (strcmp(full_path, path) != 0)
            continue;
            
        print("Match found!\n");
        
        uint32_t sectors = (file_table[i].size + SECTOR_SIZE - 1) / SECTOR_SIZE;
        char buf[16];
        
        uint8_t* binary = simple_alloc(sectors * SECTOR_SIZE);
        if (!binary) {
            print("Allocation failed!\n");
            return 0;
        }
        
        for (uint32_t s = 0; s < sectors; s++) {
            ata_read_sector(file_table[i].start_sector + s,
                            (uint16_t*)(binary + s * SECTOR_SIZE));
        }
        
        if (file_table[i].size < 4) {
            print("File too small (<4 bytes)\n");
            return 0;
        }
        
        BinaryHeader* header = (BinaryHeader*)binary;
        void* entry_address = (void*)((uint8_t*)binary + header->entryPoint);
        //if (header->magic != BINARY_MAGIC) {
        //    print("Invalid magic!\n");
        //    return 0;
        //}
        
        uint32_t offset = header->entryPoint;
        
        if (offset >= file_table[i].size) {
            print("Offset too big\n");
            return 0;
        }
        
        *entry_point = binary + offset;
        print("Entry at 0x");
        utoa((unsigned int)*entry_point, buf, 16);
        print(buf);
        print("\n");
        
        return 1;
    }
    
    return 0;
}

int execute_binary(const char* path) {
    void* entry_point = 0;

    if (!load_binary(path, &entry_point)) {
        print("Failed to load binary ");
        print(path);
        print("\n");
        return 0;
    }

    print("Executing binary at 0x");
    char buf[16];
    utoa((unsigned int)entry_point, buf, 16);
    print(buf);
    print("\n");

    entry_func_t func = (entry_func_t)entry_point;
    int result = func(print);

    print("App returned ");
    itoa(result, buf, 10);
    print(buf);
    print("\n");

    return result;
}

void run_program(const char* program_path) {
    int exit_code = execute_binary(program_path);
    
    print("Program finished with exit code ");
    char buf[16];
    utoa(exit_code, buf, 10);
    print(buf);
    print("\n");
}

typedef struct {
    void* entry_point;
    void* memory_base;
    uint32_t memory_size;
    int pid;
    char name[64];
} Process;

static Process processes[16];
static int next_pid = 1;

int create_process(const char* path) {
    // find free process slot
    int proc_slot = -1;
    for (int i = 0; i < 16; i++) {
        if (processes[i].pid == 0) {
            proc_slot = i;
            break;
        }
    }
    
    if (proc_slot == -1) {
        print("No free process slots\n");
        return -1;
    }
    
    // Load binary
    void* entry_point = 0;
    if (!load_binary(path, &entry_point)) {
        return -1;
    }
    
    // Set up process
    processes[proc_slot].entry_point = entry_point;
    processes[proc_slot].pid = next_pid++;
    strncpy(processes[proc_slot].name, path, 63);
    processes[proc_slot].name[63] = '\0';
    
    print("Created process ");
    char buf[16];
    utoa(processes[proc_slot].pid, buf, 10);
    print(buf);
    print(" for ");
    print(path);
    print("\n");
    
    return processes[proc_slot].pid;
}

int run_process(int pid) {
    // Find process
    Process* proc = 0;
    for (int i = 0; i < 16; i++) {
        if (processes[i].pid == pid) {
            proc = &processes[i];
            break;
        }
    }
    
    if (!proc) {
        print("Process not found\n");
        return -1;
    }
    
    print("Running process ");
    char buf[16];
    utoa(pid, buf, 10);
    print(buf);
    print(": ");
    print(proc->name);
    print("\n");
    
    // Execute
    typedef int (*binary_func_t)(void);
    binary_func_t func = (binary_func_t)proc->entry_point;
    
    int result = func();
    
    // Clean up process
    proc->pid = 0;
    proc->entry_point = 0;
    
    return result;
}