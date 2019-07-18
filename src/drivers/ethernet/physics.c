#include "physics.h"
#include "stm32f4x7_eth_conf.h"
#include "stm32f4x7_eth.h"

#include "stm32f4xx_gpio.h"

#include "debugTask.h"
#include <stddef.h>

#define PHY_ADDRESS             (0x00)
#define writeRegister(reg, val) ETH_WritePHYRegister(PHY_ADDRESS, (reg), (val))
#define readRegister(reg, val)       ETH_ReadPHYRegister(PHY_ADDRESS, (reg), (val))

/* Basic Registers */
#define REG_BMCR                (0x00U)        /* Basic Mode Control Register       */
#define REG_BMSR                (0x01U)        /* Basic Mode Status Register        */
#define REG_PHYIDR1             (0x02U)        /* PHY Identifier 1                  */
#define REG_PHYIDR2             (0x03U)        /* PHY Identifier 2                  */
#define REG_ANAR                (0x04U)        /* Auto-Negotiation Advertisement    */
#define REG_ANLPAR              (0x05U)        /* Auto-Neg. Link Partner Abitily    */
#define REG_ANER                (0x06U)        /* Auto-Neg. Expansion Register      */
#define REG_ANNPTR              (0x07U)        /* Auto-Neg. Next Page TX            */
#define REG_IRQCS               (0x1bU)        /* Interrupt control/status          */
#define REG_PHYCTRL1            (0x1eU)        /* PHY control 1 Register            */
#define REG_PHYCTRL2            (0x1fU)        /* PHY control 2 Register            */

/**
 *                               MASK
 **/
/* Basic Mode Control Register */
#define BMCR_RESET              (0x8000U)      /* Software Reset                    */
#define BMCR_LOOPBACK           (0x4000U)      /* Loopback mode                     */
#define BMCR_SPEED_SEL          (0x2000U)      /* Speed Select (1=100Mb/s)          */
#define BMCR_ANEG_EN            (0x1000U)      /* Auto Negotiation Enable           */
#define BMCR_POWER_DOWN         (0x0800U)      /* Power Down                        */
#define BMCR_ISOLATE            (0x0400U)      /* Isolate Media interface           */
#define BMCR_REST_ANEG          (0x0200U)      /* Restart Auto Negotiation          */
#define BMCR_DUPLEX             (0x0100U)      /* Duplex Mode (1=Full duplex)       */
#define BMCR_COL_TEST           (0x0080U)      /* Collision Test                    */

/* Basic Mode Status Register */
#define BMSR_100B_T4            (0x8000U)      /* 100BASE-T4 Capable                */
#define BMSR_100B_TX_FD         (0x4000U)      /* 100BASE-TX Full Duplex Capable    */
#define BMSR_100B_TX_HD         (0x2000U)      /* 100BASE-TX Half Duplex Capable    */
#define BMSR_10B_T_FD           (0x1000U)      /* 10BASE-T Full Duplex Capable      */
#define BMSR_10B_T_HD           (0x0800U)      /* 10BASE-T Half Duplex Capable      */
#define BMSR_MF_PRE_SUP         (0x0040U)      /* Preamble suppression Capable      */
#define BMSR_ANEG_COMPL         (0x0020U)      /* Auto Negotiation Complete         */
#define BMSR_REM_FAULT          (0x0010U)      /* Remote Fault                      */
#define BMSR_ANEG_ABIL          (0x0008U)      /* Auto Negotiation Ability          */
#define BMSR_LINK_STAT          (0x0004U)      /* Link Status (1=established)       */
#define BMSR_JABBER_DET         (0x0002U)      /* Jaber Detect                      */
#define BMSR_EXT_CAPAB          (0x0001U)      /* Extended Capability               */

#define IRQCS_LINK_DOWN         (0x0400U)      /* Link down interrupt enable (1 en) */
#define IRQCS_LINK_UP           (0x0100U)      /* Link up interrupt enable (1 en)   */
#define IRQCS_LINK_DOWN_IRQ     (0x0004U)      /* LInk down irq                     */
#define IRQCS_LINK_UP_IRQ       (0x0001U)      /* Link up irq                       */
/* PHY control 1 Register */
#define PHYCTRL1_OPMODE_SPEED   (0x0002U)
#define PHYCTRL1_OPMODE_DUPLEX  (0x0004U)

/* PHY Identifier Registers */
#define PHY_ID1                 (0x0022U)      /* KSZ8031 Device Identifier MSB     */
#define PHY_ID2                 (0x1560U)      /* KSZ8031 Device Identifier LSB     */
/************************************************************************************/

#define RESET_PORT GPIOE
#define RESET_PIN  GPIO_Pin_0

#define OUT_MAP(XX) \
    XX(RST_KSZ,     RESET_PORT,  RESET_PIN,   GPIO_Mode_OUT,  GPIO_OType_PP,  GPIO_PuPd_UP,    GPIO_High_Speed)


void PHY_configResetOutput(void) {
    GPIO_InitTypeDef init = {0};

    #define XX(name, port, pin, mode, type, pull, speed)        \
        init.GPIO_Pin = pin;                                    \
        init.GPIO_Mode = mode;                                  \
        init.GPIO_OType = type;                                 \
        init.GPIO_PuPd = pull;                                  \
        init.GPIO_Speed = speed;                                \
        GPIO_Init(port, &init);                                 \
        GPIO_ResetBits(port, pin);
    OUT_MAP(XX)
    #undef XX
    (void)init;
}


void PHY_powerDown(void) {
    GPIO_SetBits(RESET_PORT, RESET_PIN);
	eth_delay(10);
}


void PHY_powerUp(void) {
    GPIO_ResetBits(RESET_PORT, RESET_PIN);
	eth_delay(10);
}

void PHY_hardReset(void) {
	PHY_powerDown();
	PHY_powerUp();
}


uint32_t PHY_softReset(void) {
    if (writeRegister(REG_BMCR, BMCR_RESET)) {
        return PHY_OK;
    }
    return PHY_ERR;
}


uint32_t PHY_startAutoNegotiation(void) {
    uint16_t reg = 0;
    if (readRegister(REG_BMCR, &reg) == ETH_ERROR) {
        return PHY_ERR;
    }

    if (writeRegister(REG_BMCR, (reg | BMCR_ANEG_EN))) {
        return PHY_OK;
    }
    return PHY_ERR;
}


uint32_t PHY_stopAutoNegotiation(void) {
    uint16_t reg = 0;
    if (readRegister(REG_BMCR, &reg) == ETH_ERROR) {
        return PHY_ERR;
    }

    if (writeRegister(REG_BMCR, (reg & ~BMCR_ANEG_EN))) {
        return PHY_OK;
    }
    return PHY_ERR;
}


uint32_t PHY_getLinkStatus(void) {
    uint16_t reg = 0;
    if (readRegister(REG_BMSR, &reg) == ETH_ERROR) {
        return PHY_LINK_DOWN;
    }
    return reg & BMSR_LINK_STAT ? PHY_LINK_UP : PHY_LINK_DOWN;
}


uint32_t PHY_isAutoNegotiationComplete(void) {
    uint16_t reg = 0;
    if (readRegister(REG_BMSR, &reg) == ETH_ERROR) {
        return PHY_ERR;
    }
    return reg & BMSR_ANEG_COMPL ? PHY_OK : PHY_ERR;
}


int32_t PHY_getPhysicsID(void) {
    uint16_t reg = 0;
    if (readRegister(REG_PHYIDR1, &reg)) {
        return reg;
    }
    return -1;
}


uint32_t PHY_setDuplexSpeed(struct DuplexSpeed *duplexSpeed) {
    uint16_t reg = 0;
    if (readRegister(REG_BMCR, &reg) == ETH_ERROR) {
        return PHY_ERR;
    }

    reg &= ~(BMCR_SPEED_SEL | BMCR_DUPLEX);
    if (duplexSpeed->speed) {
        reg |= BMCR_SPEED_SEL;
    }
    if (duplexSpeed->duplex) {
        reg |= BMCR_DUPLEX;
    }

    if (writeRegister(REG_BMCR, reg)) {
        return PHY_OK;
    }
    return PHY_ERR;
}


struct DuplexSpeed PHY_getLinkSpeed(void) {
    uint16_t reg = 0;
    if (readRegister(REG_PHYCTRL1, &reg) == ETH_ERROR) {
        return (struct DuplexSpeed) {
            .speed = 0xffffffff,
            .duplex = 0xffffffff
        };
    }

    return (struct DuplexSpeed) {
            .speed = (reg & PHYCTRL1_OPMODE_SPEED) ? ETH_Speed_100M : ETH_Speed_10M,
            .duplex = (reg & PHYCTRL1_OPMODE_DUPLEX) ? ETH_Mode_FullDuplex : ETH_Mode_HalfDuplex
        };
}

#define PHY_RESET_DELAY    ((uint32_t)0x0000000F)
/* PHY Configuration delay */
#define PHY_CONFIG_DELAY   ((uint32_t)0x0000000F)	//((uint32_t)0x00000FFF)
/* Delay when writing to Ethernet registers*/
#define ETH_REG_WRITE_DELAY ((uint32_t)0x0000000F)
//Autonegotation delay (full timeout not less 5,3 second for 10/100/1000 Mbit/s)
#define ETH_ANEG_DELAY			(3000)
#define ETH_LINK_DELAY			(5500)

static inline uint32_t waitLinkStatus(uint32_t timeout) {
    while (PHY_getLinkStatus() == PHY_LINK_DOWN && timeout) {
        eth_delay(100);
        timeout -= 100;
        dPrintf(("wait link, left ms: %d\r\n", timeout));
    }
    if (timeout == 0) {
        dPrintf(("timeout link\r\n"));
        return PHY_ERR;
    }
    return PHY_OK;
}


static inline uint32_t waitAutonegotiationComplete(uint32_t timeout) {
    while (!PHY_isAutoNegotiationComplete() && timeout) {
        eth_delay(10);
        timeout -= 10;
        dPrintf(("wait autonegotiation, left ms: %d\r\n", timeout));
    }

    if (timeout == 0) {
        dPrintf(("timeout autonegotiation\r\n"));
        return PHY_ERR;
    }
    return PHY_OK;
}

uint32_t ETH_PHYInit(uint8_t isAutonegotiation, struct DuplexSpeed duplexSpeed) {
    PHY_softReset();
    eth_delay(PHY_RESET_DELAY);

    if (isAutonegotiation) {
        if (waitLinkStatus(ETH_LINK_DELAY) == PHY_ERR) {
            return PHY_ERR;
        }
        dPrintf(("link status OK\r\n"));

        if (PHY_startAutoNegotiation() == PHY_ERR) {
            return PHY_ERR;
        }
        dPrintf(("start autonegotiation\r\n"));

        if (waitAutonegotiationComplete(ETH_ANEG_DELAY) == PHY_ERR) {
            return PHY_ERR;
        }
        dPrintf(("autonegotiation complete\r\n"));
    } else {
        if (PHY_stopAutoNegotiation() == PHY_ERR) {
            return PHY_ERR;
        }
        dPrintf(("autonegotiation DISABLE\r\n"));
        if (PHY_setDuplexSpeed(&duplexSpeed) == PHY_ERR) {
            return PHY_ERR;
        }
        dPrintf(("set speed and duplex mode OK\r\n"));
    }

    return PHY_OK;
}


uint32_t PHY_configIRQ_linkDownUp(void) {
    uint16_t reg = 0;
    if (readRegister(REG_IRQCS, &reg) == ETH_ERROR) {
        return PHY_ERR;
    }

    reg |= IRQCS_LINK_DOWN | IRQCS_LINK_UP;
    if (writeRegister(REG_IRQCS, reg)) {
        return PHY_OK;
    }
    return PHY_ERR;
}


uint32_t PHY_getCauseIRQ(void) {
    uint16_t reg = 0;
    if (readRegister(REG_IRQCS, &reg) == ETH_ERROR) {
        return PHY_ERR;
    }

    if (reg & IRQCS_LINK_UP_IRQ) {
        return PHY_LINK_UP_IRQ;
    }
    if (reg & IRQCS_LINK_DOWN_IRQ) {
        return PHY_LINK_DOWN_IRQ;
    }

    return PHY_ERR;
}
