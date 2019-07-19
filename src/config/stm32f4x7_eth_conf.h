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


/**********************************************************************************************************/

#define ETH_MAX_PACKET_SIZE    1524    /*!< ETH_HEADER + ETH_EXTRA + VLAN_TAG + MAX_ETH_PAYLOAD + ETH_CRC */
#define ETH_HEADER               14    /*!< 6 byte Dest addr, 6 byte Src addr, 2 byte length/type */
#define ETH_CRC                   4    /*!< Ethernet CRC */
#define ETH_EXTRA                 2    /*!< Extra bytes in some cases */
#define VLAN_TAG                  4    /*!< optional 802.1q VLAN Tag */
#define MIN_ETH_PAYLOAD          46    /*!< Minimum Ethernet payload size */
#define MAX_ETH_PAYLOAD        1500    /*!< Maximum Ethernet payload size */
#define JUMBO_FRAME_PAYLOAD    9000    /*!< Jumbo frame payload size */

/* Ethernet driver receive buffers are organized in a chained linked-list, when
    an ethernet packet is received, the Rx-DMA will transfer the packet from RxFIFO
    to the driver receive buffers memory.

    Depending on the size of the received ethernet packet and the size of
    each ethernet driver receive buffer, the received packet can take one or more
    ethernet driver receive buffer.

    In below are defined the size of one ethernet driver receive buffer ETH_RX_BUF_SIZE
    and the total count of the driver receive buffers ETH_RXBUFNB.

    The configured value for ETH_RX_BUF_SIZE and ETH_RXBUFNB are only provided as
    example, they can be reconfigured in the application layer to fit the application
    needs */

/* Here we configure each Ethernet driver receive buffer to fit the Max size Ethernet
   packet */
#ifndef ETH_RX_BUF_SIZE
 #define ETH_RX_BUF_SIZE         ETH_MAX_PACKET_SIZE
#endif

/* 5 Ethernet driver receive buffers are used (in a chained linked list)*/
#ifndef ETH_RXBUFNB
 #define ETH_RXBUFNB             5
#endif

 /* Ethernet driver transmit buffers are organized in a chained linked-list, when
    an ethernet packet is transmitted, Tx-DMA will transfer the packet from the
    driver transmit buffers memory to the TxFIFO.

    Depending on the size of the Ethernet packet to be transmitted and the size of
    each ethernet driver transmit buffer, the packet to be transmitted can take
    one or more ethernet driver transmit buffer.

    In below are defined the size of one ethernet driver transmit buffer ETH_TX_BUF_SIZE
    and the total count of the driver transmit buffers ETH_TXBUFNB.

    The configured value for ETH_TX_BUF_SIZE and ETH_TXBUFNB are only provided as
    example, they can be reconfigured in the application layer to fit the application
    needs */

/* Here we configure each Ethernet driver transmit buffer to fit the Max size Ethernet
   packet */
#ifndef ETH_TX_BUF_SIZE
 #define ETH_TX_BUF_SIZE         ETH_MAX_PACKET_SIZE
#endif

/* 5 ethernet driver transmit buffers are used (in a chained linked list)*/
#ifndef ETH_TXBUFNB
 #define ETH_TXBUFNB             5
#endif

#define  ETH_DMARxDesc_FrameLengthShift           16

/**********************************************************************************************************/


#endif
