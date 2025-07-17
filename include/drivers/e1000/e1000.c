// holy shit wiki.osdev.org
// used chatgpt a little in ts

#include <stdint.h>
#include <stdbool.h>
#include "../PCI/pci.h"
#include "../../mmio.h"
#include "../../io.h"
#include "../../mem.h"
#include "e1000.h"
#include "../../string.h"
#include "../../vga.h"
#include "net/ethernet.h"
#include "net/ip.h"
#include "net/icmp.h"
#include "net/net.h"

e1000_device_t* global_e1000 = 0;

bool check_icmp_reply(uint8_t* pkt, size_t len);

#define INTEL_VENDOR_ID 0x8086
#define E1000_I217      0x153A

#define E1000_NUM_RX_DESC 32
#define E1000_NUM_TX_DESC 8

#define REG_CTRL       0x0000
#define REG_EEPROM     0x0014
#define REG_RCTRL      0x0100
#define REG_RXDESCLO   0x2800
#define REG_RXDESCHI   0x2804
#define REG_RXDESCLEN  0x2808
#define REG_RXDESCHEAD 0x2810
#define REG_RXDESCTAIL 0x2818
#define REG_TCTRL      0x0400
#define REG_TXDESCLO   0x3800
#define REG_TXDESCHI   0x3804
#define REG_TXDESCLEN  0x3808
#define REG_TXDESCHEAD 0x3810
#define REG_TXDESCTAIL 0x3818
#define REG_TIPG       0x0410
#define REG_IMASK      0x00D0
#define REG_STATUS     0x0008

// ---- Control bits ----
#define RCTL_EN        (1 << 1)
#define RCTL_BAM       (1 << 15)
#define RCTL_SECRC     (1 << 26)
#define RCTL_BSIZE_8192 ((2 << 16) | (1 << 25))
#define TCTL_EN        (1 << 1)
#define TCTL_PSP       (1 << 3)
#define CMD_EOP        (1 << 0)
#define CMD_IFCS       (1 << 1)
#define CMD_RS         (1 << 3)
#define TSTA_DD        (1 << 0)

static volatile uint8_t* mmio_base;
static bool eeprom_exists = false;
static struct e1000_rx_desc* rx_descs;
static struct e1000_tx_desc* tx_descs;
static uint16_t rx_cur = 0;
static uint16_t tx_cur = 0;

#define REG(offset) (*(volatile uint32_t*)(mmio_base + (offset)))

bool e1000_detect_eeprom() {
    REG(REG_EEPROM) = 1;
    for (int i = 0; i < 1000; i++) {
        uint32_t val = REG(REG_EEPROM);
        if (val & (1 << 4)) {
            eeprom_exists = true;
            return true;
        }
    }
    return false;
}

uint16_t e1000_read_eeprom(uint8_t addr) {
    REG(REG_EEPROM) = (1) | ((uint32_t)(addr) << 8);
    while (!(REG(REG_EEPROM) & (1 << 4)));
    return (uint16_t)((REG(REG_EEPROM) >> 16) & 0xFFFF);
}

void e1000_read_mac(uint8_t* mac_buf) {
    if (eeprom_exists) {
        uint32_t val;
        val = e1000_read_eeprom(0);
        mac_buf[0] = val & 0xFF;
        mac_buf[1] = (val >> 8) & 0xFF;
        val = e1000_read_eeprom(1);
        mac_buf[2] = val & 0xFF;
        mac_buf[3] = (val >> 8) & 0xFF;
        val = e1000_read_eeprom(2);
        mac_buf[4] = val & 0xFF;
        mac_buf[5] = (val >> 8) & 0xFF;
    } else {
        volatile uint8_t* mac_reg = mmio_base + 0x5400;
        for (int i = 0; i < 6; i++) mac_buf[i] = mac_reg[i];
    }
}

void e1000_init_rx() {
    rx_descs = (struct e1000_rx_desc*)kmalloc(E1000_NUM_RX_DESC * sizeof(struct e1000_rx_desc));
    for (int i = 0; i < E1000_NUM_RX_DESC; i++) {
        void* buf = kmalloc(8192);
        rx_descs[i].addr = (uint32_t)buf;
        rx_descs[i].status = 0;
    }
    REG(REG_RXDESCLO) = (uint32_t)rx_descs;
    REG(REG_RXDESCHI) = 0;
    REG(REG_RXDESCLEN) = E1000_NUM_RX_DESC * sizeof(struct e1000_rx_desc);
    REG(REG_RXDESCHEAD) = 0;
    REG(REG_RXDESCTAIL) = E1000_NUM_RX_DESC - 1;
    REG(REG_RCTRL) = RCTL_EN | RCTL_BAM | RCTL_SECRC | RCTL_BSIZE_8192;
}

void e1000_init_tx() {
    tx_descs = (struct e1000_tx_desc*)kmalloc(E1000_NUM_TX_DESC * sizeof(struct e1000_tx_desc));
    for (int i = 0; i < E1000_NUM_TX_DESC; i++) {
        tx_descs[i].addr = 0;
        tx_descs[i].cmd = 0;
        tx_descs[i].status = TSTA_DD;
    }
    REG(REG_TXDESCLO) = (uint32_t)tx_descs;
    REG(REG_TXDESCHI) = 0;
    REG(REG_TXDESCLEN) = E1000_NUM_TX_DESC * sizeof(struct e1000_tx_desc);
    REG(REG_TXDESCHEAD) = 0;
    REG(REG_TXDESCTAIL) = 0;
    REG(REG_TCTRL) = 0b0110000000000111111000011111010;
    REG(REG_TIPG) = 0x0060200A;
}

int e1000_send(void* data, uint16_t len) {
    tx_descs[tx_cur].addr = (uint32_t)data;
    tx_descs[tx_cur].length = len;
    tx_descs[tx_cur].cmd = CMD_EOP | CMD_IFCS | CMD_RS;
    tx_descs[tx_cur].status = 0;

    uint8_t old_cur = tx_cur;
    tx_cur = (tx_cur + 1) % E1000_NUM_TX_DESC;
    REG(REG_TXDESCTAIL) = tx_cur;

    while (!(tx_descs[old_cur].status & TSTA_DD));
    return 0;
}

int e1000_init(e1000_device_t* dev, uint8_t bus, uint8_t slot, uint8_t func) {
    mmio_base = (uint8_t*)dev->mmio_base;
    eeprom_exists = false;
    e1000_detect_eeprom();

    uint8_t mac[6];
    e1000_read_mac(mac);
    memcpy(dev->mac, mac, 6);

    e1000_init_rx();
    e1000_init_tx();
    return 0;
}

void e1000_poll() {
    struct e1000_rx_desc* desc = &rx_descs[rx_cur];

    if (!(desc->status & 0x01)) return; // not ready

    uint8_t* pkt = (uint8_t*)(uintptr_t)desc->addr;
    size_t len = desc->length;

    if (check_icmp_reply(pkt, len)) {
        print("ICMP reply received\n");
    }

    // reset
    desc->status = 0;
    rx_cur = (rx_cur + 1) % E1000_NUM_RX_DESC;
    REG(REG_RXDESCTAIL) = rx_cur;
}

bool check_icmp_reply(uint8_t* pkt, size_t len) {
    eth_hdr_t* eth = (eth_hdr_t*)pkt;
    if (ntohs(eth->ethertype) != ETH_TYPE_IPV4) return false;

    ipv4_hdr_t* ip = (ipv4_hdr_t*)(pkt + sizeof(eth_hdr_t));
    if (ip->protocol != IP_PROTO_ICMP) return false;

    icmp_hdr_t* icmp = (icmp_hdr_t*)(pkt + sizeof(eth_hdr_t) + sizeof(ipv4_hdr_t));
    if (icmp->type == ICMP_ECHO_REPLY) {
        print("Got ICMP Echo Reply!\n");
        return true;
    }
    return false;
}

void e1000_print_mac(e1000_device_t* dev) {
    uint8_t* mac = dev->mac;
    char buf[8];

    print("MAC: ");

    for (int i = 0; i < 6; i++) {
        itoa(mac[i], buf, 16);
        if (mac[i] < 0x10) print("0");  // add leading zero
        print(buf);
        if (i != 5) print(":");
    }

    print("\n");
}
