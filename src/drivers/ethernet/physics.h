#ifndef PHYSICS_H_
#define PHYSICS_H_

#include <stdint.h>

#define PHY_LINK_UP_IRQ                 (1)
#define PHY_LINK_DOWN_IRQ               (2)
#define PHY_LINK_UP                     (1)
#define PHY_LINK_DOWN                   (0)
#define PHY_OK                          (1)
#define PHY_ERR                         (0)

inline void PHY_powerDown(void);
inline void PHY_powerUp(void);
void PHY_hardReset(void);

uint32_t PHY_softReset(void);
uint32_t PHY_startAutoNegotiation(void);
uint32_t PHY_getLinkStatus(void);
uint32_t PHY_isAutoNegotiationComplete(void);

int32_t PHY_getPhysicsID(void);


struct DuplexSpeed {
    uint32_t speed;
    uint32_t duplex;
};


struct DuplexSpeed PHY_getLinkSpeed(void);
uint32_t ETH_PHYInit(uint8_t isAutonegotiation, struct DuplexSpeed duplexSpeed);
uint32_t PHY_configIRQ_linkDownUp(void);
uint32_t PHY_getCauseIRQ(void);

#endif
