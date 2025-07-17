#ifndef E1000_H
#define E1000_H

#include <stdint.h>
#include <stddef.h>

#define E1000_VENDOR_ID     0x8086
#define E1000_DEVICE_ID     0x100E
#define E1000_DEVICE_I217   0x153A
#define E1000_DEVICE_82577  0x10EA

#define PCI_BAR_IO      0
#define PCI_BAR_MEM     1

#define REG_CTRL        0x0000
#define REG_STATUS      0x0008
#define REG_EEPROM      0x0014
#define REG_CTRL_EXT    0x0018
#define REG_IMASK       0x00D0

#define REG_RCTRL       0x0100
#define REG_RXDESCLO    0x2800
#define REG_RXDESCHI    0x2804
#define REG_RXDESCLEN   0x2808
#define REG_RXDESCHEAD  0x2810
#define REG_RXDESCTAIL  0x2818
#define REG_RDTR        0x2820
#define REG_RXDCTL      0x2828
#define REG_RADV        0x282C
#define REG_RSRPD       0x2C00

#define REG_TCTRL       0x0400
#define REG_TIPG        0x0410
#define REG_TXDESCLO    0x3800
#define REG_TXDESCHI    0x3804
#define REG_TXDESCLEN   0x3808
#define REG_TXDESCHEAD  0x3810
#define REG_TXDESCTAIL  0x3818

#define RCTL_EN         (1 << 1)
#define RCTL_SBP        (1 << 2)
#define RCTL_UPE        (1 << 3)
#define RCTL_MPE        (1 << 4)
#define RCTL_LPE        (1 << 5)
#define RCTL_LBM_NONE   (0 << 6)
#define RCTL_RDMTS_HALF (0 << 8)
#define RCTL_BAM        (1 << 15)
#define RCTL_SECRC      (1 << 26)

#define RCTL_BSIZE_2048  (0 << 16)
#define RCTL_BSIZE_4096  ((3 << 16) | (1 << 25))
#define RCTL_BSIZE_8192  ((2 << 16) | (1 << 25))

#define TCTL_EN         (1 << 1)
#define TCTL_PSP        (1 << 3)
#define TCTL_CT_SHIFT   4
#define TCTL_COLD_SHIFT 12
#define TCTL_RTLC       (1 << 24)

#define CMD_EOP         (1 << 0)
#define CMD_IFCS        (1 << 1)
#define CMD_RS          (1 << 3)

#define TSTA_DD         (1 << 0)

#define E1000_NUM_RX_DESC  32
#define E1000_NUM_TX_DESC  8

typedef struct {
    uint32_t mmio_base;
    uint8_t mac[6];

    uint8_t  bar_type;
    uint16_t io_base;
    uint64_t mem_base;
    int eeprom_exists;

    struct e1000_rx_desc* rx_descs[E1000_NUM_RX_DESC];
    struct e1000_tx_desc* tx_descs[E1000_NUM_TX_DESC];
    uint16_t rx_cur;
    uint16_t tx_cur;
} e1000_device_t;

extern e1000_device_t* global_e1000;

struct e1000_rx_desc {
    volatile uint64_t addr;
    volatile uint16_t length;
    volatile uint16_t checksum;
    volatile uint8_t  status;
    volatile uint8_t  errors;
    volatile uint16_t special;
} __attribute__((packed));

struct e1000_tx_desc {
    volatile uint64_t addr;
    volatile uint16_t length;
    volatile uint8_t  cso;
    volatile uint8_t  cmd;
    volatile uint8_t  status;
    volatile uint8_t  css;
    volatile uint16_t special;
} __attribute__((packed));

int  e1000_init(e1000_device_t* dev, uint8_t bus, uint8_t slot, uint8_t func);
void e1000_handle_rx(e1000_device_t* dev);
int  e1000_send_packet(e1000_device_t* dev, const void* data, uint16_t len);
void e1000_print_mac(e1000_device_t* dev);
void e1000_poll();
int e1000_send(void* data, uint16_t len);

#endif
