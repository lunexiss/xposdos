#include "arp.h"
#include "ethernet.h"
#include "ip.h"
#include "../string.h"
#include "../store.h"
#include "net.h"

#define ARP_HTYPE_ETH 0x0001
#define ARP_PTYPE_IPV4 0x0800
#define ARP_HLEN 6
#define ARP_PLEN 4
#define ARP_REQUEST 1
#define ARP_REPLY   2

struct __attribute__((packed)) arp_packet {
    uint16_t htype;
    uint16_t ptype;
    uint8_t hlen;
    uint8_t plen;
    uint16_t op;
    uint8_t sender_mac[6];
    uint8_t sender_ip[4];
    uint8_t target_mac[6];
    uint8_t target_ip[4];
};

static uint8_t arp_cache_ip[4];
static uint8_t arp_cache_mac[6];
static bool cache_valid = false;

void arp_handle_packet(e1000_device_t* dev, uint8_t* pkt, size_t len) {
    struct arp_packet* arp = (struct arp_packet*)pkt;

    if (ntohs(arp->op) == ARP_REPLY) {
        // it
        memcpy(arp_cache_ip, arp->sender_ip, 4);
        memcpy(arp_cache_mac, arp->sender_mac, 6);
        cache_valid = true;
    }
}

bool arp_resolve(e1000_device_t* dev, uint8_t* ip, uint8_t* out_mac) {
    if (cache_valid && memcmp(arp_cache_ip, ip, 4) == 0) {
        memcpy(out_mac, arp_cache_mac, 6);
        return true;
    }

    struct arp_packet pkt;
    pkt.htype = htons(ARP_HTYPE_ETH);
    pkt.ptype = htons(ARP_PTYPE_IPV4);
    pkt.hlen = ARP_HLEN;
    pkt.plen = ARP_PLEN;
    pkt.op = htons(ARP_REQUEST);
    memcpy(pkt.sender_mac, dev->mac, 6);
    memcpy(pkt.sender_ip, local_ip, 4);
    memset(pkt.target_mac, 0, 6);
    memcpy(pkt.target_ip, ip, 4);

    eth_send(dev, (uint8_t[]){0xFF,0xFF,0xFF,0xFF,0xFF,0xFF}, 0x0806, &pkt, sizeof(pkt)); // fuckass arp

    // wait for the reply
    for (int i = 0; i < 100000; i++) {
        __asm__ __volatile__("pause");
        if (cache_valid && memcmp(arp_cache_ip, ip, 4) == 0) {
            memcpy(out_mac, arp_cache_mac, 6);
            return true;
        }
    }

    return false; // timeout
}
