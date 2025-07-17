#ifndef PCI_H
#define PCI_H

#include <stdint.h>
#include "../e1000/e1000.h"

void checkAllBuses(void);

typedef struct {
    uint32_t address;
    uint32_t size;
    uint8_t is_io;
} pci_bar_info_t;

uint16_t pciConfigReadWord(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
uint32_t pciConfigReadDword(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
uint8_t pciConfigReadByte(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);

void pciConfigWriteDword(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t value);
void pciConfigWriteWord(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint16_t value);

uint16_t getVendorID(uint8_t bus, uint8_t device, uint8_t function);
uint16_t getDeviceID(uint8_t bus, uint8_t device, uint8_t function);
uint8_t getHeaderType(uint8_t bus, uint8_t device, uint8_t function);
uint8_t getBaseClass(uint8_t bus, uint8_t device, uint8_t function);
uint8_t getSubClass(uint8_t bus, uint8_t device, uint8_t function);
uint8_t getSecondaryBus(uint8_t bus, uint8_t device, uint8_t function);

void checkBus(uint8_t bus);

pci_bar_info_t pciGetBar(uint8_t bus, uint8_t slot, uint8_t func, uint8_t bar_num);

void pci_init(void);

#endif
