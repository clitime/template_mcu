#ifndef ETH_DMA_PROCESS_H_
#define ETH_DMA_PROCESS_H_


#include <stdint.h>
#include "stm32f4xx.h"
#include "stm32f4x7_eth_conf.h"


typedef struct {
    volatile uint32_t   Status;           /*!< Status */
    uint32_t   ControlBufferSize;     /*!< Control and Buffer1, Buffer2 lengths */
    uint32_t   Buffer1Addr;           /*!< Buffer1 address pointer */
    uint32_t   Buffer2NextDescAddr;   /*!< Buffer2 or next descriptor address pointer */
    /* Enhanced ETHERNET DMA PTP Descriptors */
    #ifdef USE_ENHANCED_DMA_DESCRIPTORS
        uint32_t   ExtendedStatus;        /* Extended status for PTP receive descriptor */
        uint32_t   Reserved1;             /* Reserved */
        uint32_t   TimeStampLow;          /* Time Stamp Low value for transmit and receive */
        uint32_t   TimeStampHigh;         /* Time Stamp High value for transmit and receive */
    #endif /* USE_ENHANCED_DMA_DESCRIPTORS */
} ETH_DMADESCTypeDef;


typedef struct  {
    volatile ETH_DMADESCTypeDef *FS_Rx_Desc;          /*!< First Segment Rx Desc */
    volatile ETH_DMADESCTypeDef *LS_Rx_Desc;          /*!< Last Segment Rx Desc */
    volatile uint32_t Seg_Count;                     /*!< Segment count */
} ETH_DMA_Rx_Frame_infos;


typedef struct{
    uint32_t buffer;
    volatile ETH_DMADESCTypeDef *descriptor;
    uint16_t length;
} FrameTypeDef;


void ETH_DMA_chainInit(uint32_t MacAddr, uint8_t *addr);
FrameTypeDef ETH_Get_Received_Frame(void);
FrameTypeDef ETH_Get_Received_Frame_interrupt(void);
void ETH_SetDMARxDescOwnBit(volatile ETH_DMADESCTypeDef *DMARxDesc);
ETH_DMADESCTypeDef *ETH_setNextDMADescriptor(volatile ETH_DMADESCTypeDef *DMARxDesc);
ETH_DMA_Rx_Frame_infos *getFrameInfos(void);
ETH_DMADESCTypeDef *getDMATxDescToSet(void);



// uint32_t ETH_CheckFrameReceived(void);
// uint32_t ETH_GetRxPktSize(ETH_DMADESCTypeDef *DMARxDesc);
uint32_t ETH_Prepare_Transmit_Descriptors(u16 FrameLength);
// FrameTypeDef ETH_Get_Received_Frame(void);
// FlagStatus ETH_GetDMATxDescFlagStatus(ETH_DMADESCTypeDef *DMATxDesc, uint32_t ETH_DMATxDescFlag);
// uint32_t ETH_GetDMATxDescCollisionCount(ETH_DMADESCTypeDef *DMATxDesc);
// void ETH_SetDMATxDescOwnBit(ETH_DMADESCTypeDef *DMATxDesc);
// void ETH_DMATxDescTransmitITConfig(ETH_DMADESCTypeDef *DMATxDesc, FunctionalState NewState);
// void ETH_DMATxDescFrameSegmentConfig(ETH_DMADESCTypeDef *DMATxDesc, uint32_t DMATxDesc_FrameSegment);
// void ETH_DMATxDescCRCCmd(ETH_DMADESCTypeDef *DMATxDesc, FunctionalState NewState);
// void ETH_DMATxDescSecondAddressChainedCmd(ETH_DMADESCTypeDef *DMATxDesc, FunctionalState NewState);
// void ETH_DMATxDescShortFramePaddingCmd(ETH_DMADESCTypeDef *DMATxDesc, FunctionalState NewState);
// void ETH_DMATxDescBufferSizeConfig(ETH_DMADESCTypeDef *DMATxDesc, uint32_t BufferSize1, uint32_t BufferSize2);
// FlagStatus ETH_GetDMARxDescFlagStatus(ETH_DMADESCTypeDef *DMARxDesc, uint32_t ETH_DMARxDescFlag);

// #ifdef USE_ENHANCED_DMA_DESCRIPTORS
//     FlagStatus ETH_GetDMAPTPRxDescExtendedFlagStatus(ETH_DMADESCTypeDef *DMAPTPRxDesc, uint32_t ETH_DMAPTPRxDescExtendedFlag);
// #endif


// uint32_t ETH_GetDMARxDescFrameLength(__IO ETH_DMADESCTypeDef *DMARxDesc);
// void ETH_DMARxDescSecondAddressChainedCmd(ETH_DMADESCTypeDef *DMARxDesc, FunctionalState NewState);
// uint32_t ETH_GetDMARxDescBufferSize(ETH_DMADESCTypeDef *DMARxDesc, uint32_t DMARxDesc_Buffer);



#endif
