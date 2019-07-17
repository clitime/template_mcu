#include "ethernetTestTask.h"
#include "ioconf.h"

#include "stm32f4x7_eth.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_syscfg.h"

#include "FreeRTOS.h"
#include "task.h"

#include "debugTask.h"


#include "physics.h"

static ETH_InitTypeDef ETH_InitStructure;
uint32_t linkStatus = 0;
uint16_t phyID = 0;
void ethernetTask(void *p) {
    (void)p;
    ETH_ConfigIO();

    SYSCFG_ETH_MediaInterfaceConfig(SYSCFG_ETH_MediaInterface_RMII);

    RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_ETH_MAC
                        | RCC_AHB1Periph_ETH_MAC_Tx
                        | RCC_AHB1Periph_ETH_MAC_Rx, ENABLE);

    ETH_DeInit();
    ETH_SoftwareReset();
    while (ETH_GetSoftwareResetStatus() == SET) {
        continue;
    };
    ETH_StructInit(&ETH_InitStructure);

    PHY_hardReset();
    uint32_t phyInitResult = ETH_PHYInit(ETH_InitStructure.ETH_AutoNegotiation,
                (struct DuplexSpeed){
                        .speed = ETH_InitStructure.ETH_Speed,
                        .duplex = ETH_InitStructure.ETH_Mode
                    }
                );
    if (phyInitResult == PHY_ERR) {
        phyInitResult = ETH_PHYInit(ETH_AutoNegotiation_Disable,
                (struct DuplexSpeed){
                        .speed = ETH_InitStructure.ETH_Speed,
                        .duplex = ETH_InitStructure.ETH_Mode
                    }
                );
    }

    struct DuplexSpeed ds = PHY_getLinkSpeed();
    dPrintf(("speed: %d\r\n", ds.speed));
    dPrintf(("duplex: %d\r\n", ds.duplex));
    ETH_InitStructure.ETH_Speed = ds.speed;
    ETH_InitStructure.ETH_Mode = ds.duplex;

    ETH_MACInit(&ETH_InitStructure);

    for (;;) {
        vTaskDelay(1000);
        linkStatus = PHY_getLinkStatus();
    }
}
