#include "ethernetTestTask.h"
#include "ioconf.h"

#include "stm32f4x7_eth.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_syscfg.h"

#include "FreeRTOS.h"
#include "task.h"

#include "debugTask.h"


static ETH_InitTypeDef ETH_InitStructure;
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

    ETH_MACInit(&ETH_InitStructure);

    for (;;) {
        vTaskDelay(1000);
    }
}
