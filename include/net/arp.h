#pragma once

#include <stdint.h>
#include "../drivers/e1000/e1000.h"

#define bool	_Bool
#define true	1
#define false	0

bool arp_resolve(e1000_device_t* dev, uint8_t* ip, uint8_t* out_mac);
void arp_handle_packet(e1000_device_t* dev, uint8_t* pkt, size_t len);