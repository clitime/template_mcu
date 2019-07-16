#ifdef DEBUG_PORT

#include <stdint.h>
#include <stdio.h>

#include "debugTask.h"

#include "stm32f4xx_gpio.h"
#include "stm32f4xx_usart.h"
#include "misc.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"


#define DEBUG_UART_RTS_PIN  GPIO_Pin_11
#define DEBUG_UART_RTS_PORT GPIOA
#define rtsOff()            (DEBUG_UART_RTS_PORT->BSRRH = DEBUG_UART_RTS_PIN)
#define rtsOn()             (DEBUG_UART_RTS_PORT->BSRRL = DEBUG_UART_RTS_PIN)


enum {
	DEBUG_QUEUE_SIZE = 512,
	DEBUG_ITEM_SIZE = sizeof(uint8_t),
};


static uint8_t tx_buf[DEBUG_QUEUE_SIZE * DEBUG_ITEM_SIZE];
static uint8_t rx_buf[64];

static StaticQueue_t xStaticQueueTx;
static StaticQueue_t xStaticQueueRx;
static QueueHandle_t SerialTx = NULL;
static QueueHandle_t SerialRx = NULL;


#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)

PUTCHAR_PROTOTYPE {
    xQueueSendToBack(SerialTx, &ch, 1);
    rtsOn();
    USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
    return ch;
}


static void initQueue(void) {
    SerialTx = xQueueCreateStatic(DEBUG_QUEUE_SIZE,
                                  DEBUG_ITEM_SIZE,
                                  tx_buf,
                                  &xStaticQueueTx);

    while(!SerialTx) {
        continue;
    }

    SerialRx = xQueueCreateStatic(64,
                                  sizeof(char),
                                  rx_buf,
                                  &xStaticQueueRx);

    while(!SerialRx) {
        continue;
    }
}


static void debugUartInit(void) {
    USART_InitTypeDef   uart;
    GPIO_InitTypeDef    GPIO_InitStruct;
    NVIC_InitTypeDef    NVIC_InitStructure;

    RCC->APB2ENR |= (RCC_APB2ENR_USART1EN);

    GPIO_PinAFConfig(GPIOA, GPIO_PinSource9,  GPIO_AF_USART1);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStruct.GPIO_Speed = GPIO_High_Speed;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStruct.GPIO_Speed = GPIO_High_Speed;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin = DEBUG_UART_RTS_PIN;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStruct.GPIO_Speed = GPIO_High_Speed;
	GPIO_Init(DEBUG_UART_RTS_PORT, &GPIO_InitStruct);

    uart.USART_BaudRate = 115200;
    uart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    uart.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    uart.USART_Parity = USART_Parity_No;
    uart.USART_StopBits = USART_StopBits_1;
    uart.USART_WordLength = USART_WordLength_8b;
    USART_Init(USART1, &uart);

    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 14;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    USART_ITConfig(USART1, USART_IT_TC, ENABLE);
    USART_Cmd(USART1, ENABLE);

    NVIC_EnableIRQ(USART1_IRQn);
}


void initializeDebug(void) {
    initQueue();
    debugUartInit();
}


void USART1_IRQHandler(void) {
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
        uint8_t data = (uint8_t)USART_ReceiveData(USART1);
    }

    if(USART_GetITStatus(USART1, USART_IT_TXE) != RESET) {
        uint8_t ch;
        portBASE_TYPE xHigherPriorityTaskWoken = pdTRUE;
        if (xQueueReceiveFromISR(SerialTx, &ch, &xHigherPriorityTaskWoken) == pdPASS) {
            USART_SendData(USART1, ch);
        } else {
            USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
        }
    }

    if (USART_GetITStatus(USART1, USART_IT_TC) != RESET) {
        rtsOff();
        USART_ClearITPendingBit(USART1, USART_IT_TC);
    }
}

#endif
