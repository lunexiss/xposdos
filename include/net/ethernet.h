#pragma once

#include "../string.h"

typedef struct {
    uint8_t dest[6];
    uint8_t src[6];
    uint16_t ethertype;
} __attribute__((packed)) eth_hdr_t;
#define ETH_TYPE_IPV4 0x0800

#include "../drivers/e1000/e1000.h"

void eth_send(e1000_device_t* dev, uint8_t* dst_mac, uint16_t eth_type, void* payload, size_t payload_len);
