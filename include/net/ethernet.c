#include "ethernet.h"
#include "../drivers/e1000/e1000.h"
#include "net.h"
#include "../string.h"

void eth_send(e1000_device_t* dev, uint8_t* dst_mac, uint16_t eth_type, void* payload, size_t payload_len) {
    struct eth_hdr {
        uint8_t dst[6];
        uint8_t src[6];
        uint16_t type;
    } __attribute__((packed));

    uint8_t frame[1500];
    struct eth_hdr* hdr = (struct eth_hdr*)frame;

    memcpy(hdr->dst, dst_mac, 6);
    memcpy(hdr->src, dev->mac, 6);
    hdr->type = htons(eth_type);

    memcpy(frame + sizeof(struct eth_hdr), payload, payload_len);

    e1000_send(frame, sizeof(struct eth_hdr) + payload_len);
}
