#include "ip.h"
#include "arp.h"
#include "../string.h"
#include "../mem.h"
#include "net.h"
#include "../vga.h"
#include "ethernet.h"
#include "icmp.h"

#define IP_VERSION 4
#define IP_HEADER_LEN 20

uint8_t local_ip[4] = {192, 168, 0, 2};

struct ip_header {
    uint8_t ver_ihl;
    uint8_t tos;
    uint16_t total_len;
    uint16_t id;
    uint16_t flags_frag;
    uint8_t ttl;
    uint8_t protocol;
    uint16_t checksum;
    uint8_t src_ip[4];
    uint8_t dst_ip[4];
} __attribute__((packed));

uint16_t checksum16(void* data, size_t len); // forward

void ip_send(e1000_device_t* dev, uint8_t dst_ip[4], uint8_t protocol, void* payload, uint16_t payload_len) {
    uint8_t* pkt = kmalloc(IP_HEADER_LEN + payload_len);
    struct ip_header* ip = (struct ip_header*)pkt;

    ip->ver_ihl = (IP_VERSION << 4) | (IP_HEADER_LEN / 4);
    ip->tos = 0;
    ip->total_len = htons(IP_HEADER_LEN + payload_len);
    ip->id = 0;
    ip->flags_frag = 0;
    ip->ttl = 64;
    ip->protocol = protocol;
    ip->checksum = 0;
    memcpy(ip->src_ip, local_ip, 4);
    memcpy(ip->dst_ip, dst_ip, 4);
    ip->checksum = checksum16(ip, IP_HEADER_LEN);

    memcpy(pkt + IP_HEADER_LEN, payload, payload_len);

    uint8_t dst_mac[6];
    if (!arp_resolve(dev, dst_ip, dst_mac)) {
        print("ARP failed\n");
        kfree(pkt);
        return;
    }

    eth_send(dev, dst_mac, 0x0800, pkt, IP_HEADER_LEN + payload_len); // 0x0800 = ipv4
    kfree(pkt);
}

void ping(e1000_device_t* dev, uint8_t ip[4]) {
    print("Pinging: ");
    for (int i = 0; i < 4; i++) {
        print_dec(ip[i]);
        if (i < 3) print(".");
    }
    print("\n");

    icmp_send_echo_request(dev, ip);

    // wait for a reply
    int timeout = 1000000;
    while (timeout-- > 0) {
        e1000_poll();
    }

    print("Ping finished\n");
}
