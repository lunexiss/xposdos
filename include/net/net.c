#include "net.h"
#include "string.h"

uint16_t ntohs(uint16_t x) {
    return (x >> 8) | (x << 8);
}

uint16_t htons(uint16_t x) {
    return (x >> 8) | (x << 8);
}