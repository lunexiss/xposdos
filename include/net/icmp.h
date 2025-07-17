#pragma once

#include <stdint.h>
#include "../drivers/e1000/e1000.h"

typedef struct {
    uint8_t type;
    uint8_t code;
    uint16_t checksum;
    uint16_t identifier;
    uint16_t sequence_number;
} __attribute__((packed)) icmp_hdr_t;
#define ICMP_ECHO_REPLY 0x00

void icmp_send_echo_request(e1000_device_t* dev, uint8_t target_ip[4]);