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

static xSemaphoreHandle phyIRQSemaphore = NULL;
uint32_t linkStatus = 0;


void ethernetTask(void *p) {
    (void)p;


    for (;;) {

    }
}
