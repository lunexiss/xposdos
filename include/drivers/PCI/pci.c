// no way wiki.osdev.org

#include <stdint.h>
#include "io.h"
#include "drivers/PCI/pci.h"
#include "drivers/e1000/e1000.h"
#include "../../string.h"
#include "../../vga.h"

#define CONFIG_ADDRESS 0xCF8
#define CONFIG_DATA    0xCFC

uint32_t pciConfigReadDword(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address = (uint32_t)((bus << 16) | (slot << 11) |
                     (func << 8) | (offset & 0xFC) | 0x80000000);

    outl(CONFIG_ADDRESS, address);
    return inl(CONFIG_DATA);
}

uint16_t pciConfigReadWord(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    return (uint16_t)((pciConfigReadDword(bus, slot, func, offset) >> ((offset & 2) * 8)) & 0xFFFF);
}

uint8_t pciConfigReadByte(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    return (uint8_t)((pciConfigReadDword(bus, slot, func, offset) >> ((offset & 3) * 8)) & 0xFF);
}

void pciConfigWriteDword(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t value) {
    uint32_t address = (uint32_t)((bus << 16) | (slot << 11) |
                     (func << 8) | (offset & 0xFC) | 0x80000000);
    outl(CONFIG_ADDRESS, address);
    outl(CONFIG_DATA, value);
}

void pciConfigWriteWord(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint16_t value) {
    uint32_t aligned = pciConfigReadDword(bus, slot, func, offset & 0xFC);
    uint32_t shift = (offset & 2) * 8;
    aligned = (aligned & ~(0xFFFF << shift)) | ((uint32_t)value << shift);
    pciConfigWriteDword(bus, slot, func, offset & 0xFC, aligned);
}

uint16_t getVendorID(uint8_t bus, uint8_t device, uint8_t function) {
    return pciConfigReadWord(bus, device, function, 0x00);
}

uint16_t getDeviceID(uint8_t bus, uint8_t device, uint8_t function) {
    return pciConfigReadWord(bus, device, function, 0x02);
}

uint8_t getHeaderType(uint8_t bus, uint8_t device, uint8_t function) {
    return pciConfigReadByte(bus, device, function, 0x0E);
}

uint8_t getBaseClass(uint8_t bus, uint8_t device, uint8_t function) {
    return pciConfigReadByte(bus, device, function, 0x0B);
}

uint8_t getSubClass(uint8_t bus, uint8_t device, uint8_t function) {
    return pciConfigReadByte(bus, device, function, 0x0A);
}

uint8_t getSecondaryBus(uint8_t bus, uint8_t device, uint8_t function) {
    return pciConfigReadByte(bus, device, function, 0x19);
}

void checkFunction(uint8_t bus, uint8_t device, uint8_t function) {
    uint16_t vendorID = getVendorID(bus, device, function);
    if (vendorID == 0xFFFF) return;

    uint16_t deviceID = getDeviceID(bus, device, function);
    uint8_t baseClass = getBaseClass(bus, device, function);
    uint8_t subClass = getSubClass(bus, device, function);

    char buf[16];

    print("PCI Device [");

    itoa(bus, buf, 16);
    print(buf);
    print(":");

    itoa(device, buf, 16);
    print(buf);
    print(":");

    itoa(function, buf, 16);
    print(buf);
    print("] -> Vendor: ");

    itoa(vendorID, buf, 16);
    print(buf);
    print(", Device: ");

    itoa(deviceID, buf, 16);
    print(buf);
    print(", Class: ");

    itoa(baseClass, buf, 16);
    print(buf);
    print(", Subclass: ");

    itoa(subClass, buf, 16);
    print(buf);
    print("\n");

    if (baseClass == 0x06 && subClass == 0x04) {
        uint8_t secondaryBus = getSecondaryBus(bus, device, function);
        checkBus(secondaryBus);
    }
}

void checkDevice(uint8_t bus, uint8_t device) {
    uint16_t vendorID = getVendorID(bus, device, 0);
    if (vendorID == 0xFFFF) return;

    checkFunction(bus, device, 0);
    uint8_t headerType = getHeaderType(bus, device, 0);

    if (headerType & 0x80) {
        for (uint8_t function = 1; function < 8; function++) {
            if (getVendorID(bus, device, function) != 0xFFFF) {
                checkFunction(bus, device, function);
            }
        }
    }
}

void checkBus(uint8_t bus) {
    for (uint8_t device = 0; device < 32; device++) {
        checkDevice(bus, device);
    }
}

void checkAllBuses(void) {
    uint8_t headerType = getHeaderType(0, 0, 0);
    if ((headerType & 0x80) == 0) {
        checkBus(0);
    } else {
        for (uint8_t function = 0; function < 8; function++) {
            if (getVendorID(0, 0, function) != 0xFFFF) {
                checkBus(function);
            }
        }
    }
}

pci_bar_info_t pciGetBar(uint8_t bus, uint8_t slot, uint8_t func, uint8_t bar_num) {
    pci_bar_info_t bar_info = {0};

    uint8_t offset = 0x10 + bar_num * 4;

    // read original bar value
    uint32_t original = pciConfigReadDword(bus, slot, func, offset);

    // fuck the io access
    uint16_t cmd = pciConfigReadWord(bus, slot, func, 0x04);
    pciConfigWriteWord(bus, slot, func, 0x04, cmd & ~(1 << 0 | 1 << 1));

    pciConfigWriteDword(bus, slot, func, offset, 0xFFFFFFFF);
    uint32_t size_mask = pciConfigReadDword(bus, slot, func, offset);

    pciConfigWriteDword(bus, slot, func, offset, original);
    pciConfigWriteWord(bus, slot, func, 0x04, cmd);

    if (original & 0x1) {
        // io bar
        bar_info.is_io = 1;
        bar_info.address = original & ~0x3;
        bar_info.size = ~(size_mask & ~0x3) + 1;
    } else {
        // mem bar
        bar_info.is_io = 0;
        bar_info.address = original & ~0xF;
        bar_info.size = ~(size_mask & ~0xF) + 1;
    }

    return bar_info;
}

void pci_init() {
    for (uint8_t bus = 0; bus < 256; bus++) {
        for (uint8_t slot = 0; slot < 32; slot++) {
            for (uint8_t func = 0; func < 8; func++) {
                uint16_t vendor = getVendorID(bus, slot, func);
                if (vendor == 0xFFFF) continue;

                uint16_t device = getDeviceID(bus, slot, func);
                if (vendor == 0x8086 && device == 0x100E) {
                    print("Intel e1000 found\n");

                    pci_bar_info_t bar = pciGetBar(bus, slot, func, 0);
                    global_e1000->mmio_base = bar.address;

                    e1000_init(global_e1000, bus, slot, func);
                    return;
                }
            }
        }
    }

    print("e1000 not found\n");
}