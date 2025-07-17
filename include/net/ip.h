#pragma once
#include <stdint.h>
#include "../drivers/e1000/e1000.h"

typedef struct {
    uint8_t version_ihl;
    uint8_t tos;
    uint16_t total_length;
    uint16_t id;
    uint16_t frag_offset;
    uint8_t ttl;
    uint8_t protocol;
    uint16_t checksum;
    uint8_t src[4];
    uint8_t dst[4];
} __attribute__((packed)) ipv4_hdr_t;
#define IP_PROTO_ICMP 0x01

extern uint8_t local_ip[4];

void ip_send(e1000_device_t* dev, uint8_t dst_ip[4], uint8_t protocol, void* payload, uint16_t payload_len);
void ping(e1000_device_t* dev, uint8_t ip[4]);