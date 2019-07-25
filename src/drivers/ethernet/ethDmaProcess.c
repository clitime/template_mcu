#include "ethDmaProcess.h"
#include "stm32f4x7_eth.h"
#include "lwipopts.h"


static ETH_DMADESCTypeDef  DMARxDscrTab[ETH_RXBUFNB] __attribute__ ((aligned (4))); /* Ethernet Rx DMA Descriptor */
static ETH_DMADESCTypeDef  DMATxDscrTab[ETH_TXBUFNB] __attribute__ ((aligned (4))); /* Ethernet Tx DMA Descriptor */
static uint8_t Rx_Buff[ETH_RXBUFNB][ETH_RX_BUF_SIZE] __attribute__ ((aligned (4))); /* Ethernet Receive Buffer */
static uint8_t Tx_Buff[ETH_TXBUFNB][ETH_TX_BUF_SIZE] __attribute__ ((aligned (4))); /* Ethernet Transmit Buffer */

static __IO ETH_DMADESCTypeDef  *DMATxDescToSet;
static __IO ETH_DMADESCTypeDef  *DMARxDescToGet;
static __IO ETH_DMA_Rx_Frame_infos RX_Frame_Descriptor;
__IO ETH_DMA_Rx_Frame_infos *DMA_RX_FRAME_infos;


static void ETH_DMARxDescChainInit(ETH_DMADESCTypeDef *DMARxDescTab, uint8_t *RxBuff, uint32_t rxBuffCount);
static void ETH_DMATxDescChainInit(ETH_DMADESCTypeDef *DMATxDescTab, uint8_t* TxBuff, uint32_t TxBuffCount);

static inline void ETH_DMARxDescReceiveITConfig(ETH_DMADESCTypeDef *DMARxDesc, FunctionalState newState);
#ifdef CHECKSUM_BY_HARDWARE
static inline void ETH_DMATxDescChecksumInsertionConfig(ETH_DMADESCTypeDef *DMATxDesc, uint32_t DMATxDesc_Checksum);
#endif
static inline uint32_t ETH_GetDMARxDescFrameLength(__IO ETH_DMADESCTypeDef *DMARxDesc);


void ETH_DMA_chainInit(uint32_t MacAddr, uint8_t *Addr) {
	//initialize MAC address in ethernet MAC
	ETH_MACAddressConfig(MacAddr, Addr);
	//Initialize Tx Descriptors list: Chain Mode
	ETH_DMATxDescChainInit(DMATxDscrTab, &Tx_Buff[0][0], ETH_TXBUFNB);
	//Initialize Rx Descriptors list: Chain Mode
	ETH_DMARxDescChainInit(DMARxDscrTab, &Rx_Buff[0][0], ETH_RXBUFNB);


    for(uint32_t i = 0; i < ETH_RXBUFNB; i++) {
        ETH_DMARxDescReceiveITConfig(&DMARxDscrTab[i], ENABLE);
    }

#ifdef CHECKSUM_BY_HARDWARE
	//Enable the checksum insertion for the Tx frames
    for(uint32_t i = 0; i < ETH_TXBUFNB; i++) {
        ETH_DMATxDescChecksumInsertionConfig(&DMATxDscrTab[i], ETH_DMATxDesc_ChecksumTCPUDPICMPFull);
    }
#endif
}


FrameTypeDef ETH_Get_Received_Frame(void) {
    FrameTypeDef frame = {0,0,0};

    /* Get the Frame Length of the received packet: substruct 4 bytes of the CRC */
    #define CRC_SIZE 4
    frame.length = ETH_GetDMARxDescFrameLength(DMARxDescToGet) - CRC_SIZE;
    #undef CRC_SIZE
    /* Get the address of the first frame descriptor and the buffer start address */
    frame.descriptor = DMA_RX_FRAME_infos->FS_Rx_Desc;
    frame.buffer =(DMA_RX_FRAME_infos->FS_Rx_Desc)->Buffer1Addr;
    /* Update the ETHERNET DMA global Rx descriptor with next Rx descriptor */
    /* Chained Mode */
    /* Selects the next DMA Rx descriptor list for next buffer to read */
    DMARxDescToGet = (ETH_DMADESCTypeDef*) (DMARxDescToGet->Buffer2NextDescAddr);
    /* Return Frame */
    return (frame);
}


FrameTypeDef ETH_Get_Received_Frame_interrupt(void) {
    FrameTypeDef frame = {.length = 0, .buffer = 0, .descriptor = NULL};
    /* scan descriptors owned by CPU */
    for (uint32_t i = 0; i < ETH_RXBUFNB; i++) {
        if ((DMARxDescToGet->Status & ETH_DMARxDesc_OWN) != (uint32_t)RESET) {
            break;
        }
        /* check if first segment in frame */
        if (((DMARxDescToGet->Status & ETH_DMARxDesc_FS) != (uint32_t)RESET)&&
            ((DMARxDescToGet->Status & ETH_DMARxDesc_LS) == (uint32_t)RESET)) {
            DMA_RX_FRAME_infos->FS_Rx_Desc = DMARxDescToGet;
            DMA_RX_FRAME_infos->Seg_Count = 1;
            DMARxDescToGet = (ETH_DMADESCTypeDef*)(DMARxDescToGet->Buffer2NextDescAddr);
        } else if (((DMARxDescToGet->Status & ETH_DMARxDesc_LS) == (uint32_t)RESET)&&
                   ((DMARxDescToGet->Status & ETH_DMARxDesc_FS) == (uint32_t)RESET)) {

            DMA_RX_FRAME_infos->Seg_Count++;
            DMARxDescToGet = (ETH_DMADESCTypeDef*)(DMARxDescToGet->Buffer2NextDescAddr);
        } else {
            /* last segment */
            DMA_RX_FRAME_infos->LS_Rx_Desc = DMARxDescToGet;

            DMA_RX_FRAME_infos->Seg_Count++;

            /* first segment is last segment */
            if ((DMA_RX_FRAME_infos->Seg_Count) == 1) {
                DMA_RX_FRAME_infos->FS_Rx_Desc = DMARxDescToGet;
            }

            /* Get the Frame Length of the received packet: substruct 4 bytes of the CRC */
            #define CRC_SIZE 4
            frame.length = ETH_GetDMARxDescFrameLength(DMARxDescToGet) - CRC_SIZE;
            #undef CRC_SIZE
            /* Get the address of the buffer start address */
            /* Check if more than one segment in the frame */
            if (DMA_RX_FRAME_infos->Seg_Count > 1) {
                frame.buffer = (DMA_RX_FRAME_infos->FS_Rx_Desc)->Buffer1Addr;
            } else {
                frame.buffer = DMARxDescToGet->Buffer1Addr;
            }

            frame.descriptor = DMA_RX_FRAME_infos->FS_Rx_Desc;
            /* Update the ETHERNET DMA global Rx descriptor with next Rx descriptor */
            DMARxDescToGet = (ETH_DMADESCTypeDef*)(DMARxDescToGet->Buffer2NextDescAddr);
            return frame;
        }
    }
    return frame;
}


void ETH_SetDMARxDescOwnBit(volatile ETH_DMADESCTypeDef *DMARxDesc) {
    /* Set the DMA Rx Desc Own bit */
    DMARxDesc->Status |= ETH_DMARxDesc_OWN;
}


ETH_DMADESCTypeDef *ETH_setNextDMADescriptor(volatile ETH_DMADESCTypeDef *DMARxDesc) {
    return (ETH_DMADESCTypeDef *)(DMARxDesc->Buffer2NextDescAddr);
}


ETH_DMA_Rx_Frame_infos *getFrameInfos(void) {
    return (ETH_DMA_Rx_Frame_infos *)DMA_RX_FRAME_infos;
}


ETH_DMADESCTypeDef *getDMATxDescToSet(void) {
    return (ETH_DMADESCTypeDef *)DMATxDescToSet;
}


static void ETH_DMARxDescChainInit(ETH_DMADESCTypeDef *DMARxDescTab, uint8_t *RxBuff, uint32_t rxBuffCount) {
    /* Set the DMARxDescToGet pointer with the first one of the DMARxDescTab list */
    DMARxDescToGet = DMARxDescTab;
    /* Fill each DMARxDesc descriptor with the right values */
    for (uint32_t i = 0; i < rxBuffCount; i++) {
        /* Get the pointer on the ith member of the Rx Desc list */
        ETH_DMADESCTypeDef *DMARxDesc = DMARxDescTab + i;
        /* Set Own bit of the Rx descriptor Status */
        DMARxDesc->Status = ETH_DMARxDesc_OWN;

        /* Set Buffer1 size and Second Address Chained bit */
        DMARxDesc->ControlBufferSize = ETH_DMARxDesc_RCH | (uint32_t)ETH_RX_BUF_SIZE;
        /* Set Buffer1 address pointer */
        DMARxDesc->Buffer1Addr = (uint32_t)(&RxBuff[i*ETH_RX_BUF_SIZE]);

        /* Initialize the next descriptor with the Next Descriptor Polling Enable */
        if (i < (rxBuffCount - 1)) {
            /* Set next descriptor address register with next descriptor base address */
            DMARxDesc->Buffer2NextDescAddr = (uint32_t)(DMARxDescTab + i + 1);
        } else {
            /* For last descriptor, set next descriptor address register equal to the first descriptor base address */
            DMARxDesc->Buffer2NextDescAddr = (uint32_t)(DMARxDescTab);
        }
    }

    /* Set Receive Descriptor List Address Register */
    ETH->DMARDLAR = (uint32_t) DMARxDescTab;
    DMA_RX_FRAME_infos = &RX_Frame_Descriptor;
}


static void ETH_DMATxDescChainInit(ETH_DMADESCTypeDef *DMATxDescTab, uint8_t* TxBuff, uint32_t TxBuffCount) {
    /* Set the DMATxDescToSet pointer with the first one of the DMATxDescTab list */
    DMATxDescToSet = DMATxDescTab;
    /* Fill each DMATxDesc descriptor with the right values */
    for (uint32_t i = 0; i < TxBuffCount; i++) {
        /* Get the pointer on the ith member of the Tx Desc list */
        ETH_DMADESCTypeDef *DMATxDesc = DMATxDescTab + i;
        /* Set Second Address Chained bit */
        DMATxDesc->Status = ETH_DMATxDesc_TCH;
        /* Set Buffer1 address pointer */
        DMATxDesc->Buffer1Addr = (uint32_t)(&TxBuff[i*ETH_TX_BUF_SIZE]);
        /* Initialize the next descriptor with the Next Descriptor Polling Enable */
        if (i < (TxBuffCount-1)) {
            /* Set next descriptor address register with next descriptor base address */
            DMATxDesc->Buffer2NextDescAddr = (uint32_t)(DMATxDescTab + i + 1);
        } else {
            /* For last descriptor, set next descriptor address register equal to the first descriptor base address */
            DMATxDesc->Buffer2NextDescAddr = (uint32_t) DMATxDescTab;
        }
    }
    /* Set Transmit Desciptor List Address Register */
    ETH->DMATDLAR = (uint32_t) DMATxDescTab;
}


static inline void ETH_DMARxDescReceiveITConfig(ETH_DMADESCTypeDef *DMARxDesc, FunctionalState newState) {
    if (newState == ENABLE) {
        /* Enable the DMA Rx Desc receive interrupt */
        DMARxDesc->ControlBufferSize &=(~(uint32_t)ETH_DMARxDesc_DIC);
    } else {
        /* Disable the DMA Rx Desc receive interrupt */
        DMARxDesc->ControlBufferSize |= ETH_DMARxDesc_DIC;
    }
}

#ifdef CHECKSUM_BY_HARDWARE
static inline void ETH_DMATxDescChecksumInsertionConfig(ETH_DMADESCTypeDef *DMATxDesc, uint32_t DMATxDesc_Checksum) {
    /* Set the selected DMA Tx desc checksum insertion control */
    DMATxDesc->Status |= DMATxDesc_Checksum;
}
#endif


static inline uint32_t ETH_GetDMARxDescFrameLength(__IO ETH_DMADESCTypeDef *DMARxDesc) {
    /* Return the Receive descriptor frame length */
    return ((DMARxDesc->Status & ETH_DMARxDesc_FL) >> ETH_DMARXDESC_FRAME_LENGTHSHIFT);
}













uint32_t ETH_Prepare_Transmit_Descriptors(u16 FrameLength)
{
  uint32_t buf_count =0, size=0,i=0;
  __IO ETH_DMADESCTypeDef *DMATxDesc;

  /* Check if the descriptor is owned by the ETHERNET DMA (when set) or CPU (when reset) */
  if((DMATxDescToSet->Status & ETH_DMATxDesc_OWN) != (u32)RESET)
  {
    /* Return ERROR: OWN bit set */
    return ETH_ERROR;
  }

  DMATxDesc = DMATxDescToSet;

  if (FrameLength > ETH_TX_BUF_SIZE)
  {
    buf_count = FrameLength/ETH_TX_BUF_SIZE;
    if (FrameLength%ETH_TX_BUF_SIZE) buf_count++;
  }
  else buf_count =1;

  if (buf_count ==1)
  {
    /*set LAST and FIRST segment */
    DMATxDesc->Status |=ETH_DMATxDesc_FS|ETH_DMATxDesc_LS;
    /* Set frame size */
    DMATxDesc->ControlBufferSize = (FrameLength & ETH_DMATxDesc_TBS1);
    /* Set Own bit of the Tx descriptor Status: gives the buffer back to ETHERNET DMA */
    DMATxDesc->Status |= ETH_DMATxDesc_OWN;
    DMATxDesc= (ETH_DMADESCTypeDef *)(DMATxDesc->Buffer2NextDescAddr);
  }
  else
  {
    for (i=0; i< buf_count; i++)
    {
      /* Clear FIRST and LAST segment bits */
      DMATxDesc->Status &= ~(ETH_DMATxDesc_FS | ETH_DMATxDesc_LS);

      if (i==0)
      {
        /* Setting the first segment bit */
        DMATxDesc->Status |= ETH_DMATxDesc_FS;
      }

      /* Program size */
      DMATxDesc->ControlBufferSize = (ETH_TX_BUF_SIZE & ETH_DMATxDesc_TBS1);

      if (i== (buf_count-1))
      {
        /* Setting the last segment bit */
        DMATxDesc->Status |= ETH_DMATxDesc_LS;
        size = FrameLength - (buf_count-1)*ETH_TX_BUF_SIZE;
        DMATxDesc->ControlBufferSize = (size & ETH_DMATxDesc_TBS1);
      }

      /* Set Own bit of the Tx descriptor Status: gives the buffer back to ETHERNET DMA */
      DMATxDesc->Status |= ETH_DMATxDesc_OWN;

      DMATxDesc = (ETH_DMADESCTypeDef *)(DMATxDesc->Buffer2NextDescAddr);
    }
  }

  DMATxDescToSet = DMATxDesc;

  /* When Tx Buffer unavailable flag is set: clear it and resume transmission */
  if ((ETH->DMASR & ETH_DMASR_TBUS) != (u32)RESET)
  {
    /* Clear TBUS ETHERNET DMA flag */
    ETH->DMASR = ETH_DMASR_TBUS;
    /* Resume DMA transmission*/
    ETH->DMATPDR = 0;
  }

  /* Return SUCCESS */
  return ETH_SUCCESS;
}
