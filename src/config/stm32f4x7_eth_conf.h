#ifndef STM32F4X7_ETH_CONF_H_
#define STM32F4X7_ETH_CONF_H_


#include "FreeRTOS.h"
#include "task.h"


#define eth_delay(time) vTaskDelay(time)

#define USE_ENHANCED_DMA_DESCRIPTORS

#include "stm32f4x7_eth.h"
#define ETH_AUTONEGOTIATION ETH_AutoNegotiation_Enable
#define ETH_SPEED ETH_Speed_100M
#define ETH_DUPLEX_MODE ETH_Mode_FullDuplex

/* Delay when writing to Ethernet registers*/
#define ETH_REG_WRITE_DELAY ((uint32_t)0x0000000F)

#endif
