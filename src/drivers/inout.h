#ifndef INOUT_H__
#define INOUT_H__


#include <stdint.h>
#include "stm32f4xx_gpio.h"


typedef enum IO_State_t IO_State_t;
typedef enum InputMap_t InputMap_t;
typedef enum OutMap_t OutMap_t;


enum IO_State_t {
    IO_RESET = 0,
    IO_SET,
};


    /*  name         port     pin             mode          pu_pd     */
#define IN_MAP(XX)                                                            \
    XX(JP1,         GPIOF,  GPIO_Pin_2,     GPIO_Mode_IN, GPIO_PuPd_UP)       \
    XX(JP2,         GPIOF,  GPIO_Pin_3,     GPIO_Mode_IN, GPIO_PuPd_UP)       \
    XX(JP3,         GPIOF,  GPIO_Pin_4,     GPIO_Mode_IN, GPIO_PuPd_UP)       \
    XX(MDC,         GPIOC,  GPIO_Pin_0,     GPIO_Mode_IN, GPIO_PuPd_NOPULL)   \
    XX(TAMPER,      GPIOB,  GPIO_Pin_14,    GPIO_Mode_IN, GPIO_PuPd_UP)


/*    name         port       pin          mode           o_type              pu_pd             speed         state  */
#define OUT_MAP(XX)                                                                                                    \
    XX(PA12_LIMIT,  GPIOA,  GPIO_Pin_12,  GPIO_Mode_OUT,  GPIO_OType_PP,  GPIO_PuPd_DOWN,  GPIO_High_Speed,  IO_RESET) \
    XX(LED_R,       GPIOC,  GPIO_Pin_10,  GPIO_Mode_OUT,  GPIO_OType_PP,  GPIO_PuPd_UP,    GPIO_High_Speed,  IO_RESET) \
    XX(LED_G,       GPIOC,  GPIO_Pin_11,  GPIO_Mode_OUT,  GPIO_OType_PP,  GPIO_PuPd_UP,    GPIO_High_Speed,  IO_RESET) \
    XX(LED_G_ETH,   GPIOA,  GPIO_Pin_6,   GPIO_Mode_OUT,  GPIO_OType_PP,  GPIO_PuPd_UP,    GPIO_High_Speed,  IO_RESET)


enum InputMap_t {
    #define XX(name, port, pin, mode, pupd) IN_##name,
        IN_MAP(XX)
    #undef XX
    IN_NULL,
};

enum OutMap_t {
    #define XX(name, port, pin, mode, type, pupd, speed, state) OUT_##name,
        OUT_MAP(XX)
    #undef XX
    OUT_NULL,
};


void configInput(void);
uint8_t getInputState(InputMap_t name);

void configOutput(void);
void setOutputState(OutMap_t name, IO_State_t state);
uint8_t getOutputState(OutMap_t name);
void toggleOutputState(OutMap_t name);

#endif
