#ifndef STM32F4xx_IT_H_
#define STM32F4xx_IT_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32f4xx.h"

void NMI_Handler(void);
void HardFault_Handler(void) __attribute__((noreturn));
void MemManage_Handler(void) __attribute__((noreturn));
void BusFault_Handler(void) __attribute__((noreturn));
void UsageFault_Handler(void) __attribute__((noreturn));
void SysTick_Handler(void);

#ifdef __cplusplus
}
#endif

#endif
