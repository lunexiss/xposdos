#pragma once

#define BINARY_MAGIC 0xDEADBEEF

typedef struct {
    uint32_t magic;
    uint32_t entryPoint;
    uint32_t reserved1;
    uint32_t reserved2;
} __attribute__((packed)) BinaryHeader;
