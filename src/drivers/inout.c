#include <stddef.h>
#include "inout.h"


struct Gpio_t {
    GPIO_TypeDef *port;
    uint32_t pin;
};


static struct Gpio_t output[] = {
    #define XX(name, port, pin, mode, type, pull, speed, state)               \
        {port, pin},
    OUT_MAP(XX)
    {NULL, 0}
    #undef XX
};

static struct Gpio_t input[] = {
    #define XX(name, port, pin, mode, pupd) \
        {port, pin},
    IN_MAP(XX)
    {NULL, 0}
    #undef XX
};


void configOutput(void) {
    GPIO_InitTypeDef init = {0};

    #define XX(name, port, pin, mode, type, pull, speed, state) \
        init.GPIO_Pin = pin;                                    \
        init.GPIO_Mode = mode;                                  \
        init.GPIO_OType = type;                                 \
        init.GPIO_PuPd = pull;                                  \
        init.GPIO_Speed = speed;                                \
        GPIO_Init(port, &init);                                 \
        state == IO_SET ? GPIO_SetBits(port, pin)               \
                        : GPIO_ResetBits(port, pin);
    OUT_MAP(XX)
    #undef XX
    (void)init;
}


void configInput(void) {
    GPIO_InitTypeDef init = {0};

    #define XX(name, port, pin, mode, pupd)                     \
        init.GPIO_Pin = pin;                                    \
        init.GPIO_Mode = mode;                                  \
        init.GPIO_PuPd = pupd;                                  \
        GPIO_Init(port, &init);                                 \
    IN_MAP(XX)
    #undef XX
    (void)init;
}


uint8_t getInputState(InputMap_t name) {
    return GPIO_ReadInputDataBit(input[name].port, input[name].pin);
}


void setOutputState(OutMap_t name, IO_State_t state) {
    if (state == IO_SET) {
        GPIO_SetBits(output[name].port, output[name].pin);
    } else {
        GPIO_ResetBits(output[name].port, output[name].pin);
    }
}


uint8_t getOutputState(OutMap_t name) {
    return GPIO_ReadOutputDataBit(output[name].port, output[name].pin);
}


void toggleOutputState(OutMap_t name) {
    if (getOutputState(name)) {
        setOutputState(name, IO_RESET);
    } else {
        setOutputState(name, IO_SET);
    }
}
