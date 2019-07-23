#include "ethernetTestTask.h"
#include "ioconf.h"

#include "stm32f4x7_eth.h"
#include "stm32f4xx_exti.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_syscfg.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "debugTask.h"


#include "physics.h"
#include "ethernetif.h"

#include "webStatic.h"
#include "http_server.h"
#include "lwipopts.h"

Ethernet_t eth = {
    {192, 168, 70, 212},
    {255, 255, 255, 0},
    {192, 168, 70, 254}
};


void ethernetTask(void *p) {
    (void)p;

    lwipInit(&eth);

    http_server_init();
#if LWIP_STATS
	http_stat_init();
#endif
    vTaskDelete(NULL);
}


const uint8_t default_hw_param[6] = {0x00, 0x1f, 0x62, 0x00, 0x00, 0x33};


void setMACHwAddr(uint8_t *hw, uint8_t len) {
    for (uint8_t i = 0; i != len; i++) {
        hw[i] = default_hw_param[i];
    }
}
