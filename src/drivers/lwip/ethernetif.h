#ifndef ETHERNETIF_H_
#define ETHERNETIF_H_


#include <stdint.h>


typedef struct Ethernet_t Ethernet_t;


struct Ethernet_t {
    uint8_t ip[4];
    uint8_t mask[4];
    uint8_t gw[4];
};


void lwipInit(const Ethernet_t *eth);
void setMACHwAddr(uint8_t *hw, uint8_t len);

#endif
