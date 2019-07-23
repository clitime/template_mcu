#include "ioconf.h"

#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_exti.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_syscfg.h"
#include "misc.h"


/*    name         port       pin          mode           o_type              pu_pd             speed         af_func       pin_source*/
#define ETHER_NAME_MAP(XX)                                                                                                                    \
    XX(RX0_P,        GPIOC,  GPIO_Pin_4,    GPIO_Mode_AF,   GPIO_OType_PP,  GPIO_PuPd_NOPULL, GPIO_High_Speed,  GPIO_AF_ETH,  GPIO_PinSource4)  \
    XX(RX0_N,        GPIOC,  GPIO_Pin_5,    GPIO_Mode_AF,   GPIO_OType_PP,  GPIO_PuPd_NOPULL, GPIO_High_Speed,  GPIO_AF_ETH,  GPIO_PinSource5)  \
    XX(MDC,          GPIOC,  GPIO_Pin_1,    GPIO_Mode_AF,   GPIO_OType_PP,  GPIO_PuPd_NOPULL, GPIO_High_Speed,  GPIO_AF_ETH,  GPIO_PinSource1)  \
    XX(TX0_EN,       GPIOB,  GPIO_Pin_11,   GPIO_Mode_AF,   GPIO_OType_PP,  GPIO_PuPd_NOPULL, GPIO_High_Speed,  GPIO_AF_ETH,  GPIO_PinSource11) \
    XX(TX0_P,        GPIOB,  GPIO_Pin_12,   GPIO_Mode_AF,   GPIO_OType_PP,  GPIO_PuPd_NOPULL, GPIO_High_Speed,  GPIO_AF_ETH,  GPIO_PinSource12) \
    XX(TX0_N,        GPIOB,  GPIO_Pin_13,   GPIO_Mode_AF,   GPIO_OType_PP,  GPIO_PuPd_NOPULL, GPIO_High_Speed,  GPIO_AF_ETH,  GPIO_PinSource13) \
    XX(MDIO,         GPIOA,  GPIO_Pin_2,    GPIO_Mode_AF,   GPIO_OType_PP,  GPIO_PuPd_NOPULL, GPIO_High_Speed,  GPIO_AF_ETH,  GPIO_PinSource2)  \
    XX(REF_CLK,      GPIOA,  GPIO_Pin_1,    GPIO_Mode_AF,   GPIO_OType_PP,  GPIO_PuPd_NOPULL, GPIO_High_Speed,  GPIO_AF_ETH,  GPIO_PinSource1)  \
    XX(CRS_DV,       GPIOA,  GPIO_Pin_7,    GPIO_Mode_AF,   GPIO_OType_PP,  GPIO_PuPd_NOPULL, GPIO_High_Speed,  GPIO_AF_ETH,  GPIO_PinSource7)  \
    XX(PA_50MHZ,     GPIOA,  GPIO_Pin_8,    GPIO_Mode_AF,   GPIO_OType_PP,  GPIO_PuPd_UP,     GPIO_High_Speed,  GPIO_AF_MCO,  GPIO_PinSource8)


void ETH_ConfigIO(void) {
    GPIO_InitTypeDef   init;
    EXTI_InitTypeDef   EXTI_InitStructure;
    NVIC_InitTypeDef   NVIC_InitStructure;

    GPIO_StructInit(&init);

    init.GPIO_Mode = GPIO_Mode_IN;
    init.GPIO_PuPd = GPIO_PuPd_NOPULL;
    init.GPIO_Pin = GPIO_Pin_4;
    GPIO_Init(GPIOA, &init);

    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource4);

    EXTI_InitStructure.EXTI_Line = EXTI_Line4;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0D;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    #define XX(name, port, pin, mode, type, pull, speed, alternate, source) \
        init.GPIO_Pin = pin;                                                \
        init.GPIO_Mode = mode;                                              \
        init.GPIO_OType = type;                                             \
        init.GPIO_PuPd = pull;                                              \
        init.GPIO_Speed = speed;                                            \
        GPIO_Init(port, &init);                                             \
        GPIO_PinAFConfig(port, source, alternate);
    ETHER_NAME_MAP(XX)
    #undef XX

    RCC_MCO1Config(RCC_MCO1Source_HSE, RCC_MCO1Div_1);
}


void ETH_NVIC_Config(void) {
    NVIC_InitTypeDef NVIC_InitStructure;

    /* Enable the Ethernet global Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = ETH_IRQn;

    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 7;	//priority more then 6 (5 highest)

    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}


void ETH_NVIC_Disable(void) {
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
    NVIC_Init(&NVIC_InitStructure);
}
