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

#include <string.h>
#include "lwipopts.h"
#include "lwip/stats.h"


static void debugSerialRxTask(void *p);

#define DEBUG_UART_RTS_PIN  GPIO_Pin_11
#define DEBUG_UART_RTS_PORT GPIOA
#define rtsOff()            (DEBUG_UART_RTS_PORT->BSRRH = DEBUG_UART_RTS_PIN)
#define rtsOn()             (DEBUG_UART_RTS_PORT->BSRRL = DEBUG_UART_RTS_PIN)


enum {
	DEBUG_QUEUE_SIZE = 4196,
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

    xTaskCreate(debugSerialRxTask, "dbg", configMINIMAL_STACK_SIZE * 3, NULL, MAIN_TASK_PRIO, NULL);
}


static char pagehits[100] = {0};
#define BSZ	2000
static char PG_BODY[BSZ];
#define FREERTOS_STATS_BUFLEN 	BSZ
static char freertos_stats[FREERTOS_STATS_BUFLEN];


#if LWIP_STATS
static char * memp_names[] = 
	{
	#define LWIP_MEMPOOL(name,num,size,desc) desc,
	#include "lwip/priv/memp_std.h"
	};
#endif

static void debugSerialRxTask(void *p) {
    uint8_t ch;

    for (;;) {
        if (xQueueReceive(SerialRx, &ch, portMAX_DELAY) == pdPASS) {
            switch(ch) {
            case 'R':
                ch = 0x00;
                // SYS_DEBUGF(SYS_MAIN_DBG, ("application is restart!\n"));
                __disable_irq();
                NVIC_SystemReset();
                break;
            case 'S':
            {
                uint32_t size=xPortGetFreeHeapSize();
                printf("FreeHeapSize=%d\r\n",size);
                memset(PG_BODY, 0,BSZ);
                strcat((char *)PG_BODY, "--------------------Rtos---------------------\r\n");
                strcat((char *)PG_BODY, "Name          State  Priority  Stack   Num\r\n" );
                vTaskList((char *)(PG_BODY + strlen(PG_BODY)));
                strcat((char *)PG_BODY, "B : Blocked, R : Ready, D : Deleted, S : Suspended\r\n");
                printf("%s",PG_BODY);
                printf("---------------------------------------------\r\n");
                memset(PG_BODY, 0,BSZ);
                strcat((char *)PG_BODY, "Name          Abs Time\t\tTime\r\n" );
                vTaskGetRunTimeStats(freertos_stats);		//for this modify FreeRTOSConf.h and def timer functions
                strcat(PG_BODY, (const char *)freertos_stats);
                printf("%s",PG_BODY);
                //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                memset(PG_BODY, 0,BSZ);
                strcat((char *)PG_BODY, "\r\n-------------------LwIP----------------------\r\n");
                for(uint32_t i=0;i<100;i++){pagehits[i]=0x00;}
                sprintf(pagehits,"%-15s %-5s %-5s %-5s %-5s %s\n","Name","Cur","Size","Max","Err","Usage");
                strcat(PG_BODY, pagehits);
                for(uint32_t i=0;i<100;i++){pagehits[i]=0x00;}

    #if LWIP_STATS
                sprintf(pagehits,"%-15s %-5d %-5d %-5d %-5u %u%%\n","Heap",lwip_stats.mem.used,lwip_stats.mem.avail,lwip_stats.mem.max,lwip_stats.mem.err,lwip_stats.mem.used*100/lwip_stats.mem.avail);
                strcat(PG_BODY, pagehits);
								uint32_t z, i;
                for (z = 0; z < 8; z++)
                    {
                    for(i=0;i<100;i++){pagehits[i]=0x00;}
                    sprintf(pagehits,"%-15s %-5d %-5d %-5d %-5u %u%%\n",memp_names[z],lwip_stats.memp[z]->used,lwip_stats.memp[z]->avail,lwip_stats.memp[z]->max,lwip_stats.memp[z]->err,lwip_stats.memp[z]->used*100/lwip_stats.memp[z]->avail);
                    strcat(PG_BODY, pagehits);
                    }
                printf("%s",PG_BODY);

                memset(PG_BODY, 0,BSZ);
                for (z = 8; z < 13; z++)
                    {
                    for(i=0;i<100;i++){pagehits[i]=0x00;}
                    sprintf(pagehits,"%-15s %-5d %-5d %-5d %-5u %u%%\n",memp_names[z],lwip_stats.memp[z]->used,lwip_stats.memp[z]->avail,lwip_stats.memp[z]->max,lwip_stats.memp[z]->err,lwip_stats.memp[z]->used*100/lwip_stats.memp[z]->avail);
                    strcat(PG_BODY, pagehits);
                    }
                printf("%s",PG_BODY);
                memset(PG_BODY, 0,BSZ);
                for(i=0;i<100;i++){pagehits[i]=0x00;}
                printf("\r\n-----------------------------------------------\r\n");
                sprintf(pagehits,"%-15s %-5s %-5s %-5s %-5s %-5s\n","Name","Recv","Trsmt","Drop","Mem","Err");
                strcat(PG_BODY, pagehits);
                for(i=0;i<100;i++){pagehits[i]=0x00;}
                sprintf(pagehits, "%-15s %-5d %-5d %-5d %-5d %-5d\n", "Icmp", lwip_stats.icmp.recv, lwip_stats.icmp.xmit ,lwip_stats.icmp.drop, lwip_stats.icmp.memerr, lwip_stats.icmp.err);
                strcat(PG_BODY, pagehits);
                for(i=0;i<100;i++){pagehits[i]=0x00;}
                sprintf(pagehits, "%-15s %-5d %-5d %-5d %-5d %-5d\n", "Udp", lwip_stats.udp.recv, lwip_stats.udp.xmit ,lwip_stats.udp.drop, lwip_stats.udp.memerr, lwip_stats.udp.err);
                strcat(PG_BODY, pagehits);
                for(i=0;i<100;i++){pagehits[i]=0x00;}
                sprintf(pagehits, "%-15s %-5d %-5d %-5d %-5d %-5d\n", "Link", lwip_stats.link.recv, lwip_stats.link.xmit ,lwip_stats.link.drop, lwip_stats.link.memerr, lwip_stats.link.err);
                strcat(PG_BODY, pagehits);
                for(i=0;i<100;i++){pagehits[i]=0x00;}
                sprintf(pagehits, "%-15s %-5d %-5d %-5d %-5d %-5d\n", "Tcp", lwip_stats.tcp.recv, lwip_stats.tcp.xmit ,lwip_stats.tcp.drop, lwip_stats.tcp.memerr, lwip_stats.tcp.err);
                strcat(PG_BODY, pagehits);
                for(i=0;i<100;i++){pagehits[i]=0x00;}
                sprintf(pagehits, "%-15s %-5d %-5d %-5d %-5d %-5d\n", "Ip", lwip_stats.ip.recv, lwip_stats.ip.xmit ,lwip_stats.ip.drop, lwip_stats.ip.memerr, lwip_stats.ip.err);
                strcat(PG_BODY, pagehits);
                for(i=0;i<100;i++){pagehits[i]=0x00;}
                printf("%s",PG_BODY);
                printf("\r\n-----------------------------------------------\r\n");
                memset(PG_BODY, 0,BSZ);
                sprintf(pagehits,"%-15s %-5s %-5s %-5s\n","Name","Used","Max","Err");
                strcat(PG_BODY, pagehits);
                for(i=0;i<100;i++){pagehits[i]=0x00;}
                sprintf(pagehits, "%-15s %-5d %-5d %-5d\n", "Mbox", lwip_stats.sys.mbox.used, lwip_stats.sys.mbox.max, lwip_stats.sys.mbox.err);
                strcat(PG_BODY, pagehits);
                for(i=0;i<100;i++){pagehits[i]=0x00;}
                sprintf(pagehits, "%-15s %-5d %-5d %-5d\n", "Mutex", lwip_stats.sys.mutex.used, lwip_stats.sys.mutex.max, lwip_stats.sys.mutex.err);
                strcat(PG_BODY, pagehits);
                for(i=0;i<100;i++){pagehits[i]=0x00;}
                sprintf(pagehits, "%-15s %-5d %-5d %-5d\n", "Semph", lwip_stats.sys.sem.used , lwip_stats.sys.sem.max, lwip_stats.sys.sem.err);
                strcat(PG_BODY, pagehits);
                for(i=0;i<100;i++){pagehits[i]=0x00;}
                printf("%s",PG_BODY);
    #endif
                printf("\r\n-----------------------------------------------\r\n");
                }
                break;
            default:
                ch = 0x00;
            }
        }
    }
}

void USART1_IRQHandler(void) {
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
        uint8_t data = (uint8_t)USART_ReceiveData(USART1);

        portBASE_TYPE xHigherPriorityTaskWoken = pdTRUE;
        xQueueSendToBackFromISR(SerialRx, &data, &xHigherPriorityTaskWoken);
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
