#include "icmp.h"
#include "ip.h"
#include "arp.h"
#include "../mem.h"
#include "../string.h"
#include "../vga.h"

struct icmp_hdr {
    uint8_t type;
    uint8_t code;
    uint16_t checksum;
    uint16_t id;
    uint16_t seq;
    uint8_t payload[32]; // dummy data
} __attribute__((packed));

// simple shitty checksum
uint16_t checksum16(void* data, size_t len) {
    uint32_t sum = 0;
    uint16_t* ptr = data;
    for (size_t i = 0; i < len / 2; i++) sum += ptr[i];
    if (len & 1) sum += ((uint8_t*)data)[len - 1];
    while (sum >> 16) sum = (sum & 0xFFFF) + (sum >> 16);
    return ~sum;
}

void icmp_send_echo_request(e1000_device_t* dev, uint8_t target_ip[4]) {
    uint8_t* packet = kmalloc(64);
    struct icmp_hdr* icmp = (struct icmp_hdr*)packet;

    icmp->type = 8; // echo request
    icmp->code = 0;
    icmp->id = 0x1234;
    icmp->seq = 1;
    memset(icmp->payload, 'A', sizeof(icmp->payload));
    icmp->checksum = 0;
    icmp->checksum = checksum16(icmp, sizeof(struct icmp_hdr));

    ip_send(dev, target_ip, 1, packet, sizeof(struct icmp_hdr)); // proto 1
    kfree(packet);
}
