/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */


#include "lwip/opt.h"


#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/stats.h"
#include "lwip/snmp.h"
#include "lwip/ethip6.h"
#include "lwip/etharp.h"
#include "netif/ppp/pppoe.h"

#include "stm32f4x7_eth.h"
#include "stm32f4xx_exti.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_syscfg.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "debugTask.h"
#include "physics.h"

/* Define those to better describe your network interface. */
#define IFNAME0 'e'
#define IFNAME1 'n'


static xSemaphoreHandle phyIRQSemaphore = NULL;

#define MAC_ADDR0 (0x00U)
#define MAC_ADDR1 (0x1fU)
#define MAC_ADDR2 (0x62U)
#define MAC_ADDR3 (0x00U)
#define MAC_ADDR4 (0x00U)
#define MAC_ADDR5 (0x00U)

struct netif xnetif = {
    .hwaddr_len = (u8_t)ETHARP_HWADDR_LEN,
    .hwaddr =  {(u8_t)MAC_ADDR0, (u8_t)MAC_ADDR1,
                (u8_t)MAC_ADDR2, (u8_t)MAC_ADDR3,
                (u8_t)MAC_ADDR4, (u8_t)MAC_ADDR5},
    .mtu = 1500U,
    .name = {IFNAME0, IFNAME1},
};


static void ethernetif_input(struct netif *netif);
static void link_status_callback(struct netif *netif);
static void link_status(void *args);
static void ethernetif_input(void *arg);


static void link_status_callback(struct netif *netif) {
    if (netif_is_link_up(netif)) {
        uint32_t phyInitResult = ETH_PHYInit(ETH_InitStructure.ETH_AutoNegotiation,
                    (struct DuplexSpeed){
                            .speed = ETH_InitStructure.ETH_Speed,
                            .duplex = ETH_InitStructure.ETH_Mode
                        }
                    );
        if (phyInitResult == PHY_ERR) {
            phyInitResult = ETH_PHYInit(ETH_AutoNegotiation_Disable,
                    (struct DuplexSpeed){
                            .speed = ETH_InitStructure.ETH_Speed,
                            .duplex = ETH_InitStructure.ETH_Mode
                        }
                    );
            assertLog(phyInitResult == PHY_OK);
        }

        struct DuplexSpeed ds = PHY_getLinkSpeed();
        dPrintf(("speed: %d\r\n", ds.speed));
        dPrintf(("duplex: %d\r\n", ds.duplex));
        ETH_InitStructure.ETH_Speed = ds.speed;
        ETH_InitStructure.ETH_Mode = ds.duplex;

        ETH_MACInit(&ETH_InitStructure);

        PHY_configIRQ_linkDownUp();
        ETH_Start();
        tcpip_callback((tcpip_callback_fn)netif_set_up, &xnetif);
    } else {
        ETH_Stop();
        tcpip_callback((tcpip_callback_fn)netif_set_down, &xnetif);
    }
}


static void link_status(void *args) {
    (void)p;

    if (phyIRQSemaphore == NULL) {
        vSemaphoreCreateBinary(phyIRQSemaphore);
        xSemaphoreTake(phyIRQSemaphore, 0);
    }

    if (PHY_getLinkStatus() == PHY_LINK_UP) {
        netif_set_link_up(&xnetif);
    }

    for (;;) {
        if (xSemaphoreTake(phyIRQSemaphore, 0) == pdTRUE) {
            switch(PHY_getCauseIRQ()) {
            case PHY_LINK_DOWN_IRQ:
                if (netif_is_link_up(&xnetif)) {
                    netif_set_link_down(&xnetif);
                }
                dPrintf(("Link Down IRQ\r\n"));
                break;
            case PHY_LINK_UP_IRQ:
                if (!netif_is_link_up(&xnetif)) {
                    netif_set_link_up(&xnetif);
                }
                dPrintf(("Link Up IRQ\r\n"));
                break;
            default:
                dPrintf(("Unknown Link IRQ\r\n"));
                break;
            }
        }
        vTaskDelay(10);
    }
}

/**
 * In this function, the hardware should be initialized.
 * Called from ethernetif_init().
 *
 * @param netif the already initialized lwip network interface structure
 *        for this ethernetif
 */
static ETH_InitTypeDef ETH_InitStructure;
static void low_level_init(struct netif *netif) {
    /* set MAC hardware address length */
    netif->hwaddr_len = ETHARP_HWADDR_LEN;
    setMACHwAddr(netif->hwaddr, netif->hwaddr_len);
    /* maximum transfer unit */
    netif->mtu = 1500;

    ETH_ConfigIO();
    SYSCFG_ETH_MediaInterfaceConfig(SYSCFG_ETH_MediaInterface_RMII);

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_ETH_MAC
                         | RCC_AHB1Periph_ETH_MAC_Tx
                         | RCC_AHB1Periph_ETH_MAC_Rx, ENABLE);

    ETH_DeInit();
    ETH_SoftwareReset();
    while (ETH_GetSoftwareResetStatus() == SET) {
        continue;
    };
    ETH_StructInit(&ETH_InitStructure);

    PHY_hardReset();
    uint32_t phyInitResult = ETH_PHYInit(ETH_InitStructure.ETH_AutoNegotiation,
                (struct DuplexSpeed){
                        .speed = ETH_InitStructure.ETH_Speed,
                        .duplex = ETH_InitStructure.ETH_Mode
                    }
                );
    if (phyInitResult == PHY_ERR) {
        phyInitResult = ETH_PHYInit(ETH_AutoNegotiation_Disable,
                (struct DuplexSpeed){
                        .speed = ETH_InitStructure.ETH_Speed,
                        .duplex = ETH_InitStructure.ETH_Mode
                    }
                );
        assertLog(phyInitResult == PHY_OK);
    }

    struct DuplexSpeed ds = PHY_getLinkSpeed();
    dPrintf(("speed: %d\r\n", ds.speed));
    dPrintf(("duplex: %d\r\n", ds.duplex));
    ETH_InitStructure.ETH_Speed = ds.speed;
    ETH_InitStructure.ETH_Mode = ds.duplex;

    ETH_MACInit(&ETH_InitStructure);

    PHY_configIRQ_linkDownUp();
}

/**
 * This function should do the actual transmission of the packet. The packet is
 * contained in the pbuf that is passed to the function. This pbuf
 * might be chained.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @param p the MAC packet to send (e.g. IP packet including MAC addresses and type)
 * @return ERR_OK if the packet could be sent
 *         an err_t value if the packet couldn't be sent
 *
 * @note Returning ERR_MEM here if a DMA queue of your MAC is full can lead to
 *       strange results. You might consider waiting for space in the DMA queue
 *       to become available since the stack doesn't retry to send a packet
 *       dropped because of memory failure (except for the TCP timers).
 */

static err_t
low_level_output(struct netif *netif, struct pbuf *p)
{
    struct ethernetif *ethernetif = netif->state;
    struct pbuf *q;

    initiate transfer();

#if ETH_PAD_SIZE
    pbuf_remove_header(p, ETH_PAD_SIZE); /* drop the padding word */
#endif

    for (q = p; q != NULL; q = q->next) {
        /* Send the data from the pbuf to the interface, one pbuf at a
             time. The size of the data in each pbuf is kept in the ->len
             variable. */
        send data from(q->payload, q->len);
    }

    signal that packet should be sent();

    MIB2_STATS_NETIF_ADD(netif, ifoutoctets, p->tot_len);
    if (((u8_t *)p->payload)[0] & 1) {
        /* broadcast or multicast packet*/
        MIB2_STATS_NETIF_INC(netif, ifoutnucastpkts);
    } else {
        /* unicast packet */
        MIB2_STATS_NETIF_INC(netif, ifoutucastpkts);
    }
    /* increase ifoutdiscards or ifouterrors on error */

#if ETH_PAD_SIZE
    pbuf_add_header(p, ETH_PAD_SIZE); /* reclaim the padding word */
#endif

    LINK_STATS_INC(link.xmit);

    return ERR_OK;
}

/**
 * Should allocate a pbuf and transfer the bytes of the incoming
 * packet from the interface into the pbuf.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return a pbuf filled with the received packet (including MAC header)
 *         NULL on memory error
 */
static struct pbuf *
low_level_input(struct netif *netif)
{
    struct ethernetif *ethernetif = netif->state;
    struct pbuf *p, *q;
    u16_t len;

    /* Obtain the size of the packet and put it into the "len"
         variable. */
    len = ;

#if ETH_PAD_SIZE
    len += ETH_PAD_SIZE; /* allow room for Ethernet padding */
#endif

    /* We allocate a pbuf chain of pbufs from the pool. */
    p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);

    if (p != NULL) {

#if ETH_PAD_SIZE
        pbuf_remove_header(p, ETH_PAD_SIZE); /* drop the padding word */
#endif

        /* We iterate over the pbuf chain until we have read the entire
         * packet into the pbuf. */
        for (q = p; q != NULL; q = q->next) {
            /* Read enough bytes to fill this pbuf in the chain. The
             * available data in the pbuf is given by the q->len
             * variable.
             * This does not necessarily have to be a memcpy, you can also preallocate
             * pbufs for a DMA-enabled MAC and after receiving truncate it to the
             * actually received size. In this case, ensure the tot_len member of the
             * pbuf is the sum of the chained pbuf len members.
             */
            read data into(q->payload, q->len);
        }
        acknowledge that packet has been read();

        MIB2_STATS_NETIF_ADD(netif, ifinoctets, p->tot_len);
        if (((u8_t *)p->payload)[0] & 1) {
            /* broadcast or multicast packet*/
            MIB2_STATS_NETIF_INC(netif, ifinnucastpkts);
        } else {
            /* unicast packet*/
            MIB2_STATS_NETIF_INC(netif, ifinucastpkts);
        }
#if ETH_PAD_SIZE
        pbuf_add_header(p, ETH_PAD_SIZE); /* reclaim the padding word */
#endif

        LINK_STATS_INC(link.recv);
    } else {
        drop packet();
        LINK_STATS_INC(link.memerr);
        LINK_STATS_INC(link.drop);
        MIB2_STATS_NETIF_INC(netif, ifindiscards);
    }

    return p;
}

/**
 * This function should be called when a packet is ready to be read
 * from the interface. It uses the function low_level_input() that
 * should handle the actual reception of bytes from the network
 * interface. Then the type of the received packet is determined and
 * the appropriate input function is called.
 *
 * @param netif the lwip network interface structure for this ethernetif
 */
static void ethernetif_input(void *arg) {
    struct netif *netif = (struct netif *)arg;
    struct eth_hdr *ethhdr;
    struct pbuf *p;

    for (;;) {
        waitSemaphore();

        while ((p = low_level_input(netif)) != NULL) {
            ethhdr = p->payload;
            switch (htons(ethhdr->type))
            {
            case ETHTYPE_IP:
            case ETHTYPE_ARP:
                /* full packet send to tcpip_thread to process */
                if (netif->input(p, netif) != ERR_OK) {
                    pbuf_free(p);
                }
                break;
            default:
                pbuf_free(p);
                break;
            }
        }
    }
}

/**
 * Should be called at the beginning of the program to set up the
 * network interface. It calls the function low_level_init() to do the
 * actual setup of the hardware.
 *
 * This function should be passed as a parameter to netif_add().
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return ERR_OK if the loopif is initialized
 *         ERR_MEM if private data couldn't be allocated
 *         any other err_t on error
 */
err_t ethernetif_init(struct netif *netif) {
    LWIP_ASSERT("netif != NULL", (netif != NULL));

    /*
     * Initialize the snmp variables and counters inside the struct netif.
     * The last argument should be replaced with your link speed, in units
     * of bits per second.
     */
    MIB2_INIT_NETIF(netif, snmp_ifType_ethernet_csmacd, LINK_SPEED_OF_YOUR_NETIF_IN_BPS);

    netif->name[0] = IFNAME0;
    netif->name[1] = IFNAME1;

    netif->output = etharp_output;
    netif->linkoutput = low_level_output;

    /* initialize the hardware */
    low_level_init(netif);

    netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP;// | NETIF_FLAG_LINK_UP;

    // sys_sem_new(&rx_semaphore, 0U);
    xTaskCreate(ethernetif_input, "ethernetif", configMINIMAL_STACK_SIZE * 3, NULL, configMAX_PRIORITIES - 2, NULL);

    netif_set_link_callback(netif, link_status_callback);
    xTaskCreate(link_status, "link_status", configMINIMAL_STACK_SIZE * 1, NULL, MAIN_TASK_PRIO, NULL);

    return ERR_OK;
}


void lwip_Init(const ethernet_t *eth_param) {
    IP4_ADDR(&ipaddr, eth_param->ip[0], eth_param->ip[1], eth_param->ip[2], eth_param->ip[3]);
    IP4_ADDR(&netmask, eth_param->mask[0], eth_param->mask[1], eth_param->mask[2], eth_param->mask[3]);
    IP4_ADDR(&gw, eth_param->gw[0], eth_param->gw[1], eth_param->gw[2], eth_param->gw[3]);

    /* Create tcp_ip stack thread */
    tcpip_init( NULL, NULL );
    netif_add(&xnetif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &tcpip_input);
    /*  Registers the default network interface.*/
    netif_set_default(&xnetif);

    dPrintf(("lwip starting!\r\n"));
}


void EXTI4_IRQHandler(void) {
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

    if (EXTI_GetITStatus(EXTI_Line4) != RESET) {

        xSemaphoreGiveFromISR(phyIRQSemaphore, &xHigherPriorityTaskWoken);

        /* Clear the  EXTI line 0 pending bit */
        EXTI_ClearITPendingBit(EXTI_Line4);
    }
    portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
}
