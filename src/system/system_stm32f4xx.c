#include "stm32f4xx.h"
#include "system_stm32f4xx.h"

#include "misc.h"
#include "stm32f4xx_flash.h"
#include "stm32f4xx_pwr.h"
#include "stm32f4xx_rcc.h"


#define VECT_TAB_OFFSET  0x00000


void SystemInit(void) {
#if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
    SCB->CPACR |= ((3UL << 10*2)|(3UL << 11*2));
#endif

    RCC->APB1ENR |= (RCC_APB1ENR_PWREN);
    PWR_MainRegulatorModeConfig(PWR_Regulator_Voltage_Scale1);

    RCC_DeInit();

    RCC_HSEConfig( RCC_HSE_ON );
    while( RCC_GetFlagStatus( RCC_FLAG_HSERDY ) == RESET ) {
        continue;
    }
    //5 wait states required on the flash.
    FLASH_SetLatency(FLASH_Latency_5);
    FLASH_PrefetchBufferCmd(ENABLE);
    FLASH_InstructionCacheCmd(ENABLE);
    FLASH_DataCacheCmd(ENABLE);
    FLASH_InstructionCacheReset();
    FLASH_DataCacheReset();

    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
    /*Source - HSE                  PLLM PLLN PLLP PLLQ*/
    /*                              VCO  PLLN PLLP PLLQ*/
    RCC_PLLConfig(RCC_PLLSource_HSE, 25,  336, 2,   4);	//sysclk = 168 MHz

    RCC_PLLCmd(ENABLE);
    while (RCC_GetFlagStatus( RCC_FLAG_PLLRDY ) == RESET );
    //Wait till PLL is used as system clock source.
    while (RCC_GetSYSCLKSource() != 0x08 );

    /*AHB clock = 168 MHz*/
    RCC_HCLKConfig(RCC_SYSCLK_Div1);
    /*APB1 clock = 42 MHz, APB1 tim clock = 84 MHz*/
    RCC_PCLK1Config(RCC_HCLK_Div4);
    /*APB2 clock = 84 MHz, APB1 tim clock = 168 MHz*/
    RCC_PCLK2Config(RCC_HCLK_Div2);

    NVIC_SetVectorTable(NVIC_VectTab_FLASH, VECT_TAB_OFFSET);
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

    RCC_ClocksTypeDef  rcc_clocks;
    SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);
    RCC_GetClocksFreq(&rcc_clocks);
    SysTick_Config(rcc_clocks.HCLK_Frequency / 100);
    NVIC_SetPriority(SysTick_IRQn, 0x14);

    RCC_AHB1PeriphClockCmd(
          RCC_AHB1Periph_GPIOA
        | RCC_AHB1Periph_GPIOB
        | RCC_AHB1Periph_GPIOC
        | RCC_AHB1Periph_GPIOD
        | RCC_AHB1Periph_GPIOE
        | RCC_AHB1Periph_GPIOF
        | RCC_AHB1Periph_GPIOG, ENABLE
    );

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

    __enable_irq();
}
