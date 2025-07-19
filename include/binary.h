#ifndef _BINARY_H
#define _BINARY_H
#include "vga.h"

typedef int (*entry_func_t)(print_func_t);

#include "fs.h"
#include "disk.h"
#include "string.h"

int execute_binary(const char* path);
void run_program(const char* program_path);
int run_process(int pid);
int create_process(const char* path);

#endif