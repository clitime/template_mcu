#include "stm32f4x7_eth.h"

#include "stm32f4xx_rcc.h"


#include "stm32f4x7_eth_conf.h"


/**
 * errata 2.10.5
 * A write to a register might not be fully taken into account if a previous write to the same
 * register is performed within a time period of four TX_CLK/RX_CLK clock cycles. When this
 * error occurs, reading the register returns the most recently written value, but the Ethernet
 * MAC continues to operate as if the latest write operation never occurred
 *
 * Two workarounds could be applicable:
 * • Ensure a delay of four TX_CLK/RX_CLK clock cycles between the successive write
 * operations to the same register.
 * • Make several successive write operations without delay, then read the register when all
 * the operations are complete, and finally reprogram it after a delay of four
 * TX_CLK/RX_CLK clock cycles.
 */
#define waitWriteOperation(reg)             \
    {                                       \
        volatile uint32_t tmpreg = reg;     \
        eth_delay(ETH_REG_WRITE_DELAY);     \
        reg = tmpreg;                       \
    }


void ETH_DeInit(void) {
    RCC_AHB1PeriphResetCmd(RCC_AHB1Periph_ETH_MAC, ENABLE);
    RCC_AHB1PeriphResetCmd(RCC_AHB1Periph_ETH_MAC, DISABLE);
}


void ETH_StructInit(ETH_InitTypeDef* ETH_InitStruct) {
    /*------------------------   MAC Configuration   ---------------------------*/
    ETH_InitStruct->ETH_AutoNegotiation = ETH_AutoNegotiation_Disable;
    ETH_InitStruct->ETH_Speed = ETH_Speed_100M;
    ETH_InitStruct->ETH_Mode = ETH_Mode_FullDuplex;
    ETH_InitStruct->ETH_Watchdog = ETH_Watchdog_Enable;
    ETH_InitStruct->ETH_Jabber = ETH_Jabber_Enable;
    ETH_InitStruct->ETH_InterFrameGap = ETH_InterFrameGap_96Bit;
    ETH_InitStruct->ETH_CarrierSense = ETH_CarrierSense_Enable;
    ETH_InitStruct->ETH_ReceiveOwn = ETH_ReceiveOwn_Enable;
    ETH_InitStruct->ETH_LoopbackMode = ETH_LoopbackMode_Disable;
    #ifdef CHECKSUM_BY_HARDWARE
        ETH_InitStruct->ETH_ChecksumOffload = ETH_ChecksumOffload_Enable;
    #endif
    ETH_InitStruct->ETH_RetryTransmission = ETH_RetryTransmission_Disable;
    ETH_InitStruct->ETH_AutomaticPadCRCStrip = ETH_AutomaticPadCRCStrip_Disable;
    ETH_InitStruct->ETH_BackOffLimit = ETH_BackOffLimit_10;
    ETH_InitStruct->ETH_DeferralCheck = ETH_DeferralCheck_Disable;
    ETH_InitStruct->ETH_ReceiveAll = ETH_ReceiveAll_Disable;
    ETH_InitStruct->ETH_SourceAddrFilter = ETH_SourceAddrFilter_Disable;
    ETH_InitStruct->ETH_PassControlFrames = ETH_PassControlFrames_BlockAll;
    ETH_InitStruct->ETH_BroadcastFramesReception = ETH_BroadcastFramesReception_Enable;
    ETH_InitStruct->ETH_DestinationAddrFilter = ETH_DestinationAddrFilter_Normal;
    ETH_InitStruct->ETH_PromiscuousMode = ETH_PromiscuousMode_Disable;
    ETH_InitStruct->ETH_MulticastFramesFilter = ETH_MulticastFramesFilter_Perfect;
    ETH_InitStruct->ETH_UnicastFramesFilter = ETH_UnicastFramesFilter_Perfect;
    ETH_InitStruct->ETH_HashTableHigh = 0x0;
    ETH_InitStruct->ETH_HashTableLow = 0x0;
    ETH_InitStruct->ETH_PauseTime = 0x0;
    ETH_InitStruct->ETH_ZeroQuantaPause = ETH_ZeroQuantaPause_Disable;
    ETH_InitStruct->ETH_PauseLowThreshold = ETH_PauseLowThreshold_Minus4;
    ETH_InitStruct->ETH_UnicastPauseFrameDetect = ETH_UnicastPauseFrameDetect_Disable;
    ETH_InitStruct->ETH_ReceiveFlowControl = ETH_ReceiveFlowControl_Disable;
    ETH_InitStruct->ETH_TransmitFlowControl = ETH_TransmitFlowControl_Disable;
    ETH_InitStruct->ETH_VLANTagComparison = ETH_VLANTagComparison_16Bit;
    ETH_InitStruct->ETH_VLANTagIdentifier = 0x0;
    /*---------------------- DMA Configuration   -------------------------------*/
    ETH_InitStruct->ETH_DropTCPIPChecksumErrorFrame = ETH_DropTCPIPChecksumErrorFrame_Enable;
    ETH_InitStruct->ETH_ReceiveStoreForward = ETH_ReceiveStoreForward_Enable;
    ETH_InitStruct->ETH_FlushReceivedFrame = ETH_FlushReceivedFrame_Enable;
    ETH_InitStruct->ETH_TransmitStoreForward = ETH_TransmitStoreForward_Enable;
    ETH_InitStruct->ETH_TransmitThresholdControl = ETH_TransmitThresholdControl_64Bytes;
    ETH_InitStruct->ETH_ForwardErrorFrames = ETH_ForwardErrorFrames_Disable;
    ETH_InitStruct->ETH_ForwardUndersizedGoodFrames = ETH_ForwardUndersizedGoodFrames_Disable;
    ETH_InitStruct->ETH_ReceiveThresholdControl = ETH_ReceiveThresholdControl_64Bytes;
    ETH_InitStruct->ETH_SecondFrameOperate = ETH_SecondFrameOperate_Enable;
    ETH_InitStruct->ETH_AddressAlignedBeats = ETH_AddressAlignedBeats_Enable;
    ETH_InitStruct->ETH_FixedBurst = ETH_FixedBurst_Enable;
    ETH_InitStruct->ETH_RxDMABurstLength = ETH_RxDMABurstLength_32Beat;
    ETH_InitStruct->ETH_TxDMABurstLength = ETH_TxDMABurstLength_32Beat;
    ETH_InitStruct->ETH_DescriptorSkipLength = 0x0;
    ETH_InitStruct->ETH_DMAArbitration = ETH_DMAArbitration_RoundRobin_RxTx_2_1;
}


void ETH_MACInit (ETH_InitTypeDef* ETH_InitStruct) {
    /*---------------------- ETHERNET MACMIIAR Configuration -------------------*/
    RCC_ClocksTypeDef  rcc_clocks;
    uint32_t hclk = 60000000;
    /* Get the ETHERNET MACMIIAR value */
    volatile uint32_t tmpreg = ETH->MACMIIAR;
    /* Clear CSR Clock Range CR[2:0] bits */
    tmpreg &= MACMIIAR_CR_MASK;
    /* Get hclk frequency value */
    RCC_GetClocksFreq(&rcc_clocks);
    hclk = rcc_clocks.HCLK_Frequency;

    /* Set CR bits depending on hclk value */
    if((hclk >= 20000000)&&(hclk < 35000000)) {
        /* CSR Clock Range between 20-35 MHz */
        tmpreg |= (uint32_t)ETH_MACMIIAR_CR_Div16;
    } else if((hclk >= 35000000)&&(hclk < 60000000)) {
        /* CSR Clock Range between 35-60 MHz */
        tmpreg |= (uint32_t)ETH_MACMIIAR_CR_Div26;
    } else if((hclk >= 60000000)&&(hclk < 100000000)) {
        /* CSR Clock Range between 60-100 MHz */
        tmpreg |= (uint32_t)ETH_MACMIIAR_CR_Div42;
    } else if((hclk >= 100000000)&&(hclk < 150000000)) {
        /* CSR Clock Range between 100-150 MHz */
        tmpreg |= (uint32_t)ETH_MACMIIAR_CR_Div62;
    } else {    /* ((hclk >= 150000000)&&(hclk <= 168000000)) */
        /* CSR Clock Range between 150-168 MHz */
        tmpreg |= (uint32_t)ETH_MACMIIAR_CR_Div102;
    }

    /* Write to ETHERNET MAC MIIAR: Configure the ETHERNET CSR Clock Range */
    ETH->MACMIIAR = (uint32_t)tmpreg;
    /*------------------------ ETHERNET MACCR Configuration --------------------*/
    /* Get the ETHERNET MACCR value */
    tmpreg = ETH->MACCR;
    /* Clear WD, PCE, PS, TE and RE bits */
    tmpreg &= MACCR_CLEAR_MASK;
    /* Set the WD bit according to ETH_Watchdog value */
    /* Set the JD: bit according to ETH_Jabber value */
    /* Set the IFG bit according to ETH_InterFrameGap value */
    /* Set the DCRS bit according to ETH_CarrierSense value */
    /* Set the FES bit according to ETH_Speed value */
    /* Set the DO bit according to ETH_ReceiveOwn value */
    /* Set the LM bit according to ETH_LoopbackMode value */
    /* Set the DM bit according to ETH_Mode value */
    /* Set the IPCO bit according to ETH_ChecksumOffload value */
    /* Set the DR bit according to ETH_RetryTransmission value */
    /* Set the ACS bit according to ETH_AutomaticPadCRCStrip value */
    /* Set the BL bit according to ETH_BackOffLimit value */
    /* Set the DC bit according to ETH_DeferralCheck value */
    tmpreg |= (uint32_t)(ETH_InitStruct->ETH_Watchdog |
                        ETH_InitStruct->ETH_Jabber |
                        ETH_InitStruct->ETH_InterFrameGap |
                        ETH_InitStruct->ETH_CarrierSense |
                        ETH_InitStruct->ETH_Speed |
                        ETH_InitStruct->ETH_ReceiveOwn |
                        ETH_InitStruct->ETH_LoopbackMode |
                        ETH_InitStruct->ETH_Mode |
                        ETH_InitStruct->ETH_ChecksumOffload |
                        ETH_InitStruct->ETH_RetryTransmission |
                        ETH_InitStruct->ETH_AutomaticPadCRCStrip |
                        ETH_InitStruct->ETH_BackOffLimit |
                        ETH_InitStruct->ETH_DeferralCheck);
    /* Write to ETHERNET MACCR */
    ETH->MACCR = (uint32_t)tmpreg;

    waitWriteOperation(ETH->MACCR);
    /*----------------------- ETHERNET MACFFR Configuration --------------------*/
    /* Set the RA bit according to ETH_ReceiveAll value */
    /* Set the SAF and SAIF bits according to ETH_SourceAddrFilter value */
    /* Set the PCF bit according to ETH_PassControlFrames value */
    /* Set the DBF bit according to ETH_BroadcastFramesReception value */
    /* Set the DAIF bit according to ETH_DestinationAddrFilter value */
    /* Set the PR bit according to ETH_PromiscuousMode value */
    /* Set the PM, HMC and HPF bits according to ETH_MulticastFramesFilter value */
    /* Set the HUC and HPF bits according to ETH_UnicastFramesFilter value */
    /* Write to ETHERNET MACFFR */
    ETH->MACFFR = (uint32_t)(ETH_InitStruct->ETH_ReceiveAll |
                            ETH_InitStruct->ETH_SourceAddrFilter |
                            ETH_InitStruct->ETH_PassControlFrames |
                            ETH_InitStruct->ETH_BroadcastFramesReception |
                            ETH_InitStruct->ETH_DestinationAddrFilter |
                            ETH_InitStruct->ETH_PromiscuousMode |
                            ETH_InitStruct->ETH_MulticastFramesFilter |
                            ETH_InitStruct->ETH_UnicastFramesFilter);

    waitWriteOperation(ETH->MACFFR);
    /*--------------- ETHERNET MACHTHR and MACHTLR Configuration ---------------*/
    /* Write to ETHERNET MACHTHR */
    ETH->MACHTHR = (uint32_t)ETH_InitStruct->ETH_HashTableHigh;

    /* Write to ETHERNET MACHTLR */
    ETH->MACHTLR = (uint32_t)ETH_InitStruct->ETH_HashTableLow;
    /*----------------------- ETHERNET MACFCR Configuration --------------------*/

    /* Get the ETHERNET MACFCR value */
    tmpreg = ETH->MACFCR;
    /* Clear xx bits */
    tmpreg &= MACFCR_CLEAR_MASK;

    /* Set the PT bit according to ETH_PauseTime value */
    /* Set the DZPQ bit according to ETH_ZeroQuantaPause value */
    /* Set the PLT bit according to ETH_PauseLowThreshold value */
    /* Set the UP bit according to ETH_UnicastPauseFrameDetect value */
    /* Set the RFE bit according to ETH_ReceiveFlowControl value */
    /* Set the TFE bit according to ETH_TransmitFlowControl value */
    tmpreg |= (uint32_t)((ETH_InitStruct->ETH_PauseTime << 16) |
                            ETH_InitStruct->ETH_ZeroQuantaPause |
                            ETH_InitStruct->ETH_PauseLowThreshold |
                            ETH_InitStruct->ETH_UnicastPauseFrameDetect |
                            ETH_InitStruct->ETH_ReceiveFlowControl |
                            ETH_InitStruct->ETH_TransmitFlowControl);
    /* Write to ETHERNET MACFCR */
    ETH->MACFCR = (uint32_t)tmpreg;

    waitWriteOperation(ETH->MACFCR);
    /*----------------------- ETHERNET MACVLANTR Configuration -----------------*/
    /* Set the ETV bit according to ETH_VLANTagComparison value */
    /* Set the VL bit according to ETH_VLANTagIdentifier value */
    ETH->MACVLANTR = (uint32_t)(ETH_InitStruct->ETH_VLANTagComparison |
                                                            ETH_InitStruct->ETH_VLANTagIdentifier);

    waitWriteOperation(ETH->MACVLANTR);
    /*-------------------------------- DMA Config ------------------------------*/
    /*----------------------- ETHERNET DMAOMR Configuration --------------------*/

    /* Get the ETHERNET DMAOMR value */
    tmpreg = ETH->DMAOMR;
    /* Clear xx bits */
    tmpreg &= DMAOMR_CLEAR_MASK;

    /* Set the DT bit according to ETH_DropTCPIPChecksumErrorFrame value */
    /* Set the RSF bit according to ETH_ReceiveStoreForward value */
    /* Set the DFF bit according to ETH_FlushReceivedFrame value */
    /* Set the TSF bit according to ETH_TransmitStoreForward value */
    /* Set the TTC bit according to ETH_TransmitThresholdControl value */
    /* Set the FEF bit according to ETH_ForwardErrorFrames value */
    /* Set the FUF bit according to ETH_ForwardUndersizedGoodFrames value */
    /* Set the RTC bit according to ETH_ReceiveThresholdControl value */
    /* Set the OSF bit according to ETH_SecondFrameOperate value */
    tmpreg |= (uint32_t)(ETH_InitStruct->ETH_DropTCPIPChecksumErrorFrame |
                        ETH_InitStruct->ETH_ReceiveStoreForward |
                        ETH_InitStruct->ETH_FlushReceivedFrame |
                        ETH_InitStruct->ETH_TransmitStoreForward |
                        ETH_InitStruct->ETH_TransmitThresholdControl |
                        ETH_InitStruct->ETH_ForwardErrorFrames |
                        ETH_InitStruct->ETH_ForwardUndersizedGoodFrames |
                        ETH_InitStruct->ETH_ReceiveThresholdControl |
                        ETH_InitStruct->ETH_SecondFrameOperate);
    /* Write to ETHERNET DMAOMR */
    ETH->DMAOMR = (uint32_t)tmpreg;

    waitWriteOperation(ETH->DMAOMR);
    /*----------------------- ETHERNET DMABMR Configuration --------------------*/
    /* Set the AAL bit according to ETH_AddressAlignedBeats value */
    /* Set the FB bit according to ETH_FixedBurst value */
    /* Set the RPBL and 4*PBL bits according to ETH_RxDMABurstLength value */
    /* Set the PBL and 4*PBL bits according to ETH_TxDMABurstLength value */
    /* Set the DSL bit according to ETH_DesciptorSkipLength value */
    /* Set the PR and DA bits according to ETH_DMAArbitration value */
    ETH->DMABMR = (uint32_t)(ETH_InitStruct->ETH_AddressAlignedBeats |
                            ETH_InitStruct->ETH_FixedBurst |
                            ETH_InitStruct->ETH_RxDMABurstLength | /* !! if 4xPBL is selected for Tx or Rx it is applied for the other */
                            ETH_InitStruct->ETH_TxDMABurstLength |
                            (ETH_InitStruct->ETH_DescriptorSkipLength << 2) |
                            ETH_InitStruct->ETH_DMAArbitration |
                            ETH_DMABMR_USP); /* Enable use of separate PBL for Rx and Tx */

    waitWriteOperation(ETH->DMABMR);

    #ifdef USE_ENHANCED_DMA_DESCRIPTORS
        /* Enable the Enhanced DMA descriptors */
        ETH->DMABMR |= ETH_DMABMR_EDE;

        waitWriteOperation(ETH->DMABMR);
    #endif /* USE_ENHANCED_DMA_DESCRIPTORS */
}


void ETH_SoftwareReset(void) {
    /* Set the SWR bit: resets all MAC subsystem internal registers and logic */
    /* After reset all the registers holds their respective reset values */
    ETH->DMABMR |= ETH_DMABMR_SR;
}


FlagStatus ETH_GetSoftwareResetStatus(void) {
    if((ETH->DMABMR & ETH_DMABMR_SR) != (uint32_t)RESET) {
        return SET;
    }
    return RESET;
}


void ETH_Start(void) {
    /* Enable transmit state machine of the MAC for transmission on the MII */
    ETH_MACTransmissionCmd(ENABLE);

    /* Enable receive state machine of the MAC for reception from the MII */
    ETH_MACReceptionCmd(ENABLE);

    /* Flush Transmit FIFO */
    ETH_FlushTransmitFIFO();

    /* Start DMA transmission */
    ETH_DMATransmissionCmd(ENABLE);

    /* Start DMA reception */
    ETH_DMAReceptionCmd(ENABLE);
}


void ETH_Stop(void) {
    /* Stop DMA transmission */
    ETH_DMATransmissionCmd(DISABLE);
    /* Stop DMA reception */
    ETH_DMAReceptionCmd(DISABLE);
    /* Disable receive state machine of the MAC for reception from the MII */
    ETH_MACReceptionCmd(DISABLE);
    /* Flush Transmit FIFO */
    ETH_FlushTransmitFIFO();
    /* Disable transmit state machine of the MAC for transmission on the MII */
    ETH_MACTransmissionCmd(DISABLE);
}


// uint32_t ETH_GetRxPktSize(ETH_DMADESCTypeDef *DMARxDesc) {
//     if (((DMARxDescToGet->Status & ETH_DMARxDesc_OWN) == (uint32_t)RESET) &&
//         ((DMARxDescToGet->Status & ETH_DMARxDesc_ES) == (uint32_t)RESET) &&
//         ((DMARxDescToGet->Status & ETH_DMARxDesc_LS) != (uint32_t)RESET)
//     ) {
//         return ETH_GetDMARxDescFrameLength(DMARxDesc);
//     }
//     return 0;
// }


#ifdef USE_ENHANCED_DMA_DESCRIPTORS
void ETH_EnhancedDescriptorCmd(FunctionalState NewState) {
    __IO uint32_t tmpreg = 0;

    if (NewState != DISABLE) {
        ETH->DMABMR |= ETH_DMABMR_EDE; /* Enable enhanced descriptor structure */
    } else {
        ETH->DMABMR &= ~ETH_DMABMR_EDE; /* Disable enhanced descriptor structure */
    }

    waitWriteOperation(ETH->DMABMR);
}
#endif


void ETH_MACTransmissionCmd(FunctionalState NewState) {
    __IO uint32_t tmpreg = 0;

    if (NewState != DISABLE) {
        /* Enable the MAC transmission */
        ETH->MACCR |= ETH_MACCR_TE;
    } else {
        /* Disable the MAC transmission */
        ETH->MACCR &= ~ETH_MACCR_TE;
    }

    waitWriteOperation(ETH->MACCR);
}


void ETH_MACReceptionCmd(FunctionalState NewState) {
    __IO uint32_t tmpreg = 0;

    if (NewState != DISABLE) {
        /* Enable the MAC reception */
        ETH->MACCR |= ETH_MACCR_RE;
    } else {
        /* Disable the MAC reception */
        ETH->MACCR &= ~ETH_MACCR_RE;
    }

    waitWriteOperation(ETH->MACCR);
}


FlagStatus ETH_GetFlowControlBusyStatus(void) {
    /* The Flow Control register should not be written to until this bit is cleared */
    if ((ETH->MACFCR & ETH_MACFCR_FCBBPA) != (uint32_t)RESET) {
        return SET;
    }
    return RESET;
}


void ETH_InitiatePauseControlFrame(void)  {
    __IO uint32_t tmpreg = 0;
    /* When Set In full duplex MAC initiates pause control frame */
    ETH->MACFCR |= ETH_MACFCR_FCBBPA;

    waitWriteOperation(ETH->MACFCR);
}


void ETH_BackPressureActivationCmd(FunctionalState NewState) {
    if (NewState != DISABLE) {
        /* Activate the MAC BackPressure operation */
        /* In Half duplex: during backpressure, when the MAC receives a new frame,
        the transmitter starts sending a JAM pattern resulting in a collision */
        ETH->MACFCR |= ETH_MACFCR_FCBBPA;
    } else {
        /* Desactivate the MAC BackPressure operation */
        ETH->MACFCR &= ~ETH_MACFCR_FCBBPA;
    }

    waitWriteOperation(ETH->MACFCR);
}


FlagStatus ETH_GetMACFlagStatus(uint32_t ETH_MAC_FLAG) {
    if ((ETH->MACSR & ETH_MAC_FLAG) != (uint32_t)RESET) {
        return SET;
    }
    return RESET;
}


ITStatus ETH_GetMACITStatus(uint32_t ETH_MAC_IT) {
  if ((ETH->MACSR & ETH_MAC_IT) != (uint32_t)RESET) {
        return SET;
    }
    return RESET;
}


void ETH_MACITConfig(uint32_t ETH_MAC_IT, FunctionalState NewState) {
    if (NewState != DISABLE) {
        /* Enable the selected ETHERNET MAC interrupts */
        ETH->MACIMR &= (~(uint32_t)ETH_MAC_IT);
    } else {
        /* Disable the selected ETHERNET MAC interrupts */
        ETH->MACIMR |= ETH_MAC_IT;
    }
}


void ETH_MACAddressConfig(uint32_t MacAddr, uint8_t *Addr) {
    uint32_t tmpreg;
    /* Calculate the selected MAC address high register */
    tmpreg = ((uint32_t)Addr[5] << 8) | (uint32_t)Addr[4];
    /* Load the selected MAC address high register */
    (*(__IO uint32_t *) (ETH_MAC_ADDR_HBASE + MacAddr)) = tmpreg;
    /* Calculate the selected MAC address low register */
    tmpreg = ((uint32_t)Addr[3] << 24) | ((uint32_t)Addr[2] << 16) | ((uint32_t)Addr[1] << 8) | Addr[0];

    /* Load the selected MAC address low register */
    (*(__IO uint32_t *) (ETH_MAC_ADDR_LBASE + MacAddr)) = tmpreg;
}


void ETH_GetMACAddress(uint32_t MacAddr, uint8_t *Addr) {
    uint32_t tmpreg;

    /* Get the selected MAC address high register */
    tmpreg =(*(__IO uint32_t *) (ETH_MAC_ADDR_HBASE + MacAddr));

    /* Calculate the selected MAC address buffer */
    Addr[5] = ((tmpreg >> 8) & (uint8_t)0xFF);
    Addr[4] = (tmpreg & (uint8_t)0xFF);
    /* Load the selected MAC address low register */
    tmpreg =(*(__IO uint32_t *) (ETH_MAC_ADDR_LBASE + MacAddr));
    /* Calculate the selected MAC address buffer */
    Addr[3] = ((tmpreg >> 24) & (uint8_t)0xFF);
    Addr[2] = ((tmpreg >> 16) & (uint8_t)0xFF);
    Addr[1] = ((tmpreg >> 8 ) & (uint8_t)0xFF);
    Addr[0] = (tmpreg & (uint8_t)0xFF);
}


void ETH_MACAddressPerfectFilterCmd(uint32_t MacAddr, FunctionalState NewState) {
    __IO uint32_t tmpreg = 0;

    if (NewState != DISABLE) {
        /* Enable the selected ETHERNET MAC address for perfect filtering */
        (*(__IO uint32_t *) (ETH_MAC_ADDR_HBASE + MacAddr)) |= ETH_MACA1HR_AE;
    } else {
        /* Disable the selected ETHERNET MAC address for perfect filtering */
        (*(__IO uint32_t *) (ETH_MAC_ADDR_HBASE + MacAddr)) &=(~(uint32_t)ETH_MACA1HR_AE);
    }

    waitWriteOperation((*(__IO uint32_t *) (ETH_MAC_ADDR_HBASE + MacAddr)));
}


void ETH_MACAddressFilterConfig(uint32_t MacAddr, uint32_t Filter) {
    __IO uint32_t tmpreg = 0;

    if (Filter != ETH_MAC_AddressFilter_DA) {
        /* The selected ETHERNET MAC address is used to compare with the SA fields of the
        received frame. */
        (*(__IO uint32_t *) (ETH_MAC_ADDR_HBASE + MacAddr)) |= ETH_MACA1HR_SA;
    } else {
        /* The selected ETHERNET MAC address is used to compare with the DA fields of the
        received frame. */
        (*(__IO uint32_t *) (ETH_MAC_ADDR_HBASE + MacAddr)) &=(~(uint32_t)ETH_MACA1HR_SA);
    }

    waitWriteOperation((*(__IO uint32_t *) (ETH_MAC_ADDR_HBASE + MacAddr)));
}


void ETH_MACAddressMaskBytesFilterConfig(uint32_t MacAddr, uint32_t MaskByte) {
    __IO uint32_t tmpreg = 0;

    /* Clear MBC bits in the selected MAC address  high register */
    (*(__IO uint32_t *) (ETH_MAC_ADDR_HBASE + MacAddr)) &=(~(uint32_t)ETH_MACA1HR_MBC);

    waitWriteOperation((*(__IO uint32_t *) (ETH_MAC_ADDR_HBASE + MacAddr)));

    /* Set the selected Filter mask bytes */
    (*(__IO uint32_t *) (ETH_MAC_ADDR_HBASE + MacAddr)) |= MaskByte;

    waitWriteOperation((*(__IO uint32_t *) (ETH_MAC_ADDR_HBASE + MacAddr)));
}


/**
  * @brief  Checks whether the specified ETHERNET DMA flag is set or not.
  * @param  ETH_DMA_FLAG: specifies the flag to check.
  *   This parameter can be one of the following values:
  *     @arg ETH_DMA_FLAG_TST : Time-stamp trigger flag
  *     @arg ETH_DMA_FLAG_PMT : PMT flag
  *     @arg ETH_DMA_FLAG_MMC : MMC flag
  *     @arg ETH_DMA_FLAG_DataTransferError : Error bits 0-data buffer, 1-desc. access
  *     @arg ETH_DMA_FLAG_ReadWriteError    : Error bits 0-write trnsf, 1-read transfr
  *     @arg ETH_DMA_FLAG_AccessError       : Error bits 0-Rx DMA, 1-Tx DMA
  *     @arg ETH_DMA_FLAG_NIS : Normal interrupt summary flag
  *     @arg ETH_DMA_FLAG_AIS : Abnormal interrupt summary flag
  *     @arg ETH_DMA_FLAG_ER  : Early receive flag
  *     @arg ETH_DMA_FLAG_FBE : Fatal bus error flag
  *     @arg ETH_DMA_FLAG_ET  : Early transmit flag
  *     @arg ETH_DMA_FLAG_RWT : Receive watchdog timeout flag
  *     @arg ETH_DMA_FLAG_RPS : Receive process stopped flag
  *     @arg ETH_DMA_FLAG_RBU : Receive buffer unavailable flag
  *     @arg ETH_DMA_FLAG_R   : Receive flag
  *     @arg ETH_DMA_FLAG_TU  : Underflow flag
  *     @arg ETH_DMA_FLAG_RO  : Overflow flag
  *     @arg ETH_DMA_FLAG_TJT : Transmit jabber timeout flag
  *     @arg ETH_DMA_FLAG_TBU : Transmit buffer unavailable flag
  *     @arg ETH_DMA_FLAG_TPS : Transmit process stopped flag
  *     @arg ETH_DMA_FLAG_T   : Transmit flag
  * @retval The new state of ETH_DMA_FLAG (SET or RESET).
  */
FlagStatus ETH_GetDMAFlagStatus(uint32_t ETH_DMA_FLAG) {
    if ((ETH->DMASR & ETH_DMA_FLAG) != (uint32_t)RESET) {
        return SET;
    }
    return RESET;
}


/**
  * @brief  Clears the ETHERNET�s DMA pending flag.
  * @param  ETH_DMA_FLAG: specifies the flag to clear.
  *   This parameter can be any combination of the following values:
  *     @arg ETH_DMA_FLAG_NIS : Normal interrupt summary flag
  *     @arg ETH_DMA_FLAG_AIS : Abnormal interrupt summary flag
  *     @arg ETH_DMA_FLAG_ER  : Early receive flag
  *     @arg ETH_DMA_FLAG_FBE : Fatal bus error flag
  *     @arg ETH_DMA_FLAG_ETI : Early transmit flag
  *     @arg ETH_DMA_FLAG_RWT : Receive watchdog timeout flag
  *     @arg ETH_DMA_FLAG_RPS : Receive process stopped flag
  *     @arg ETH_DMA_FLAG_RBU : Receive buffer unavailable flag
  *     @arg ETH_DMA_FLAG_R   : Receive flag
  *     @arg ETH_DMA_FLAG_TU  : Transmit Underflow flag
  *     @arg ETH_DMA_FLAG_RO  : Receive Overflow flag
  *     @arg ETH_DMA_FLAG_TJT : Transmit jabber timeout flag
  *     @arg ETH_DMA_FLAG_TBU : Transmit buffer unavailable flag
  *     @arg ETH_DMA_FLAG_TPS : Transmit process stopped flag
  *     @arg ETH_DMA_FLAG_T   : Transmit flag
  * @retval None
  */
void ETH_DMAClearFlag(uint32_t ETH_DMA_FLAG) {
    /* Clear the selected ETHERNET DMA FLAG */
    ETH->DMASR = (uint32_t) ETH_DMA_FLAG;
}


/**
  * @brief  Checks whether the specified ETHERNET DMA interrupt has occurred or not.
  * @param  ETH_DMA_IT: specifies the interrupt source to check.
  *   This parameter can be one of the following values:
  *     @arg ETH_DMA_IT_TST : Time-stamp trigger interrupt
  *     @arg ETH_DMA_IT_PMT : PMT interrupt
  *     @arg ETH_DMA_IT_MMC : MMC interrupt
  *     @arg ETH_DMA_IT_NIS : Normal interrupt summary
  *     @arg ETH_DMA_IT_AIS : Abnormal interrupt summary
  *     @arg ETH_DMA_IT_ER  : Early receive interrupt
  *     @arg ETH_DMA_IT_FBE : Fatal bus error interrupt
  *     @arg ETH_DMA_IT_ET  : Early transmit interrupt
  *     @arg ETH_DMA_IT_RWT : Receive watchdog timeout interrupt
  *     @arg ETH_DMA_IT_RPS : Receive process stopped interrupt
  *     @arg ETH_DMA_IT_RBU : Receive buffer unavailable interrupt
  *     @arg ETH_DMA_IT_R   : Receive interrupt
  *     @arg ETH_DMA_IT_TU  : Underflow interrupt
  *     @arg ETH_DMA_IT_RO  : Overflow interrupt
  *     @arg ETH_DMA_IT_TJT : Transmit jabber timeout interrupt
  *     @arg ETH_DMA_IT_TBU : Transmit buffer unavailable interrupt
  *     @arg ETH_DMA_IT_TPS : Transmit process stopped interrupt
  *     @arg ETH_DMA_IT_T   : Transmit interrupt
  * @retval The new state of ETH_DMA_IT (SET or RESET).
  */
ITStatus ETH_GetDMAITStatus(uint32_t ETH_DMA_IT) {
  if ((ETH->DMASR & ETH_DMA_IT) != (uint32_t)RESET) {
        return SET;
    }
    return RESET;
}


/**
  * @brief  Clears the ETHERNET�s DMA IT pending bit.
  * @param  ETH_DMA_IT: specifies the interrupt pending bit to clear.
  *   This parameter can be any combination of the following values:
  *     @arg ETH_DMA_IT_NIS : Normal interrupt summary
  *     @arg ETH_DMA_IT_AIS : Abnormal interrupt summary
  *     @arg ETH_DMA_IT_ER  : Early receive interrupt
  *     @arg ETH_DMA_IT_FBE : Fatal bus error interrupt
  *     @arg ETH_DMA_IT_ETI : Early transmit interrupt
  *     @arg ETH_DMA_IT_RWT : Receive watchdog timeout interrupt
  *     @arg ETH_DMA_IT_RPS : Receive process stopped interrupt
  *     @arg ETH_DMA_IT_RBU : Receive buffer unavailable interrupt
  *     @arg ETH_DMA_IT_R   : Receive interrupt
  *     @arg ETH_DMA_IT_TU  : Transmit Underflow interrupt
  *     @arg ETH_DMA_IT_RO  : Receive Overflow interrupt
  *     @arg ETH_DMA_IT_TJT : Transmit jabber timeout interrupt
  *     @arg ETH_DMA_IT_TBU : Transmit buffer unavailable interrupt
  *     @arg ETH_DMA_IT_TPS : Transmit process stopped interrupt
  *     @arg ETH_DMA_IT_T   : Transmit interrupt
  * @retval None
  */
void ETH_DMAClearITPendingBit(uint32_t ETH_DMA_IT) {
    /* Clear the selected ETHERNET DMA IT */
    ETH->DMASR = (uint32_t) ETH_DMA_IT;
}


/**
  * @brief  Returns the ETHERNET DMA Transmit Process State.
  * @param  None
  * @retval The new ETHERNET DMA Transmit Process State:
  *   This can be one of the following values:
  *     - ETH_DMA_TransmitProcess_Stopped   : Stopped - Reset or Stop Tx Command issued
  *     - ETH_DMA_TransmitProcess_Fetching  : Running - fetching the Tx descriptor
  *     - ETH_DMA_TransmitProcess_Waiting   : Running - waiting for status
  *     - ETH_DMA_TransmitProcess_Reading   : Running - reading the data from host memory
  *     - ETH_DMA_TransmitProcess_Suspended : Suspended - Tx Descriptor unavailable
  *     - ETH_DMA_TransmitProcess_Closing   : Running - closing Rx descriptor
  */
uint32_t ETH_GetTransmitProcessState(void) {
    return ((uint32_t)(ETH->DMASR & ETH_DMASR_TS));
}


/**
  * @brief  Returns the ETHERNET DMA Receive Process State.
  * @param  None
  * @retval The new ETHERNET DMA Receive Process State:
  *   This can be one of the following values:
  *     - ETH_DMA_ReceiveProcess_Stopped   : Stopped - Reset or Stop Rx Command issued
  *     - ETH_DMA_ReceiveProcess_Fetching  : Running - fetching the Rx descriptor
  *     - ETH_DMA_ReceiveProcess_Waiting   : Running - waiting for packet
  *     - ETH_DMA_ReceiveProcess_Suspended : Suspended - Rx Descriptor unavailable
  *     - ETH_DMA_ReceiveProcess_Closing   : Running - closing descriptor
  *     - ETH_DMA_ReceiveProcess_Queuing   : Running - queuing the receive frame into host memory
  */
uint32_t ETH_GetReceiveProcessState(void) {
    return ((uint32_t)(ETH->DMASR & ETH_DMASR_RS));
}


/**
  * @brief  Clears the ETHERNET transmit FIFO.
  * @param  None
  * @retval None
  */
void ETH_FlushTransmitFIFO(void) {
    __IO uint32_t tmpreg = 0;
    /* Set the Flush Transmit FIFO bit */
    ETH->DMAOMR |= ETH_DMAOMR_FTF;

    waitWriteOperation(ETH->DMAOMR);
}


FlagStatus ETH_GetFlushTransmitFIFOStatus(void) {
  if ((ETH->DMAOMR & ETH_DMAOMR_FTF) != (uint32_t)RESET) {
        return SET;
    }
    return RESET;
}


void ETH_DMATransmissionCmd(FunctionalState NewState) {
    if (NewState != DISABLE) {
        /* Enable the DMA transmission */
        ETH->DMAOMR |= ETH_DMAOMR_ST;
    } else {
        /* Disable the DMA transmission */
        ETH->DMAOMR &= ~ETH_DMAOMR_ST;
    }
}


void ETH_DMAReceptionCmd(FunctionalState NewState) {
    if (NewState != DISABLE) {
        /* Enable the DMA reception */
        ETH->DMAOMR |= ETH_DMAOMR_SR;
    } else {
        /* Disable the DMA reception */
        ETH->DMAOMR &= ~ETH_DMAOMR_SR;
    }
}


/**
  * @brief  Enables or disables the specified ETHERNET DMA interrupts.
  * @param  ETH_DMA_IT: specifies the ETHERNET DMA interrupt sources to be
  *   enabled or disabled.
  *   This parameter can be any combination of the following values:
  *     @arg ETH_DMA_IT_NIS : Normal interrupt summary
  *     @arg ETH_DMA_IT_AIS : Abnormal interrupt summary
  *     @arg ETH_DMA_IT_ER  : Early receive interrupt
  *     @arg ETH_DMA_IT_FBE : Fatal bus error interrupt
  *     @arg ETH_DMA_IT_ET  : Early transmit interrupt
  *     @arg ETH_DMA_IT_RWT : Receive watchdog timeout interrupt
  *     @arg ETH_DMA_IT_RPS : Receive process stopped interrupt
  *     @arg ETH_DMA_IT_RBU : Receive buffer unavailable interrupt
  *     @arg ETH_DMA_IT_R   : Receive interrupt
  *     @arg ETH_DMA_IT_TU  : Underflow interrupt
  *     @arg ETH_DMA_IT_RO  : Overflow interrupt
  *     @arg ETH_DMA_IT_TJT : Transmit jabber timeout interrupt
  *     @arg ETH_DMA_IT_TBU : Transmit buffer unavailable interrupt
  *     @arg ETH_DMA_IT_TPS : Transmit process stopped interrupt
  *     @arg ETH_DMA_IT_T   : Transmit interrupt
  * @param  NewState: new state of the specified ETHERNET DMA interrupts.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void ETH_DMAITConfig(uint32_t ETH_DMA_IT, FunctionalState NewState) {
    if (NewState != DISABLE) {
        /* Enable the selected ETHERNET DMA interrupts */
        ETH->DMAIER |= ETH_DMA_IT;
    } else {
        /* Disable the selected ETHERNET DMA interrupts */
        ETH->DMAIER &=(~(uint32_t)ETH_DMA_IT);
    }
}


FlagStatus ETH_GetDMAOverflowStatus(uint32_t ETH_DMA_Overflow) {
  if ((ETH->DMAMFBOCR & ETH_DMA_Overflow) != (uint32_t)RESET) {
        return SET;
    }
    return RESET;
}


uint32_t ETH_GetRxOverflowMissedFrameCounter(void) {
    return ((uint32_t)((ETH->DMAMFBOCR & ETH_DMAMFBOCR_MFA)>>ETH_DMA_RX_OVERFLOW_MISSEDFRAMES_COUNTERSHIFT));
}


uint32_t ETH_GetBufferUnavailableMissedFrameCounter(void) {
    return ((uint32_t)(ETH->DMAMFBOCR) & ETH_DMAMFBOCR_MFC);
}


uint32_t ETH_GetCurrentTxDescStartAddress(void) {
    return ((uint32_t)(ETH->DMACHTDR));
}


uint32_t ETH_GetCurrentRxDescStartAddress(void) {
    return ((uint32_t)(ETH->DMACHRDR));
}


uint32_t ETH_GetCurrentTxBufferAddress(void) {
    return ((uint32_t)(ETH->DMACHTBAR));
}


uint32_t ETH_GetCurrentRxBufferAddress(void) {
    return ((uint32_t)(ETH->DMACHRBAR));
}


void ETH_ResumeDMATransmission(void) {
    ETH->DMATPDR = 0;
}


void ETH_ResumeDMAReception(void) {
    ETH->DMARPDR = 0;
}


void ETH_SetReceiveWatchdogTimer(uint8_t Value) {
    ETH->DMARSWTR = Value;
}


void ETH_ResetWakeUpFrameFilterRegisterPointer(void) {
    __IO uint32_t tmpreg = 0;
    /* Resets the Remote Wake-up Frame Filter register pointer to 0x0000 */
    ETH->MACPMTCSR |= ETH_MACPMTCSR_WFFRPR;

    waitWriteOperation(ETH->MACPMTCSR);
}


void ETH_SetWakeUpFrameFilterRegister(uint32_t *Buffer) {
    __IO uint32_t tmpreg = 0;

    /* Fill Remote Wake-up Frame Filter register with Buffer data */
    for (uint32_t i = 0; i < ETH_WAKEUP_REGISTER_LENGTH; i++) {
        /* Write each time to the same register */
        ETH->MACRWUFFR = Buffer[i];

        waitWriteOperation(ETH->MACRWUFFR);
    }
}


void ETH_GlobalUnicastWakeUpCmd(FunctionalState NewState) {
    __IO uint32_t tmpreg = 0;
    if (NewState != DISABLE) {
        /* Enable the MAC Global Unicast Wake-Up */
        ETH->MACPMTCSR |= ETH_MACPMTCSR_GU;
    } else {
        /* Disable the MAC Global Unicast Wake-Up */
        ETH->MACPMTCSR &= ~ETH_MACPMTCSR_GU;
    }

    waitWriteOperation(ETH->MACPMTCSR);
}


/**
  * @brief  Checks whether the specified ETHERNET PMT flag is set or not.
  * @param  ETH_PMT_FLAG: specifies the flag to check.
  *   This parameter can be one of the following values:
  *     @arg ETH_PMT_FLAG_WUFFRPR : Wake-Up Frame Filter Register Pointer Reset
  *     @arg ETH_PMT_FLAG_WUFR    : Wake-Up Frame Received
  *     @arg ETH_PMT_FLAG_MPR     : Magic Packet Received
  * @retval The new state of ETHERNET PMT Flag (SET or RESET).
  */
FlagStatus ETH_GetPMTFlagStatus(uint32_t ETH_PMT_FLAG) {
    if ((ETH->MACPMTCSR & ETH_PMT_FLAG) != (uint32_t)RESET) {
        return SET;
    }
    return RESET;
}


void ETH_WakeUpFrameDetectionCmd(FunctionalState NewState) {
    __IO uint32_t tmpreg = 0;

    if (NewState != DISABLE) {
        /* Enable the MAC Wake-Up Frame Detection */
        ETH->MACPMTCSR |= ETH_MACPMTCSR_WFE;
    } else {
        /* Disable the MAC Wake-Up Frame Detection */
        ETH->MACPMTCSR &= ~ETH_MACPMTCSR_WFE;
    }

    waitWriteOperation(ETH->MACPMTCSR);
}


void ETH_MagicPacketDetectionCmd(FunctionalState NewState) {
    __IO uint32_t tmpreg = 0;

    if (NewState != DISABLE) {
        /* Enable the MAC Magic Packet Detection */
        ETH->MACPMTCSR |= ETH_MACPMTCSR_MPE;
    } else {
        /* Disable the MAC Magic Packet Detection */
        ETH->MACPMTCSR &= ~ETH_MACPMTCSR_MPE;
    }

    waitWriteOperation(ETH->MACPMTCSR);
}


void ETH_PowerDownCmd(FunctionalState NewState) {
    __IO uint32_t tmpreg = 0;

    if (NewState != DISABLE) {
        /* Enable the MAC Power Down */
        /* This puts the MAC in power down mode */
        ETH->MACPMTCSR |= ETH_MACPMTCSR_PD;
    } else {
        /* Disable the MAC Power Down */
        ETH->MACPMTCSR &= ~ETH_MACPMTCSR_PD;
    }

    waitWriteOperation(ETH->MACPMTCSR);
}


/******************************************************************************/
/*                              MMC functions                                 */
/******************************************************************************/
void ETH_MMCCounterFullPreset(void) {
    /* Preset and Initialize the MMC counters to almost-full value */
    ETH->MMCCR |= ETH_MMCCR_MCFHP | ETH_MMCCR_MCP;
}


void ETH_MMCCounterHalfPreset(void) {
    /* Preset the MMC counters to almost-full value */
    ETH->MMCCR &= ~ETH_MMCCR_MCFHP;

    /* Initialize the MMC counters to almost-half value */
    ETH->MMCCR |= ETH_MMCCR_MCP;
}


void ETH_MMCCounterFreezeCmd(FunctionalState NewState) {
    if (NewState != DISABLE) {
        /* Enable the MMC Counter Freeze */
        ETH->MMCCR |= ETH_MMCCR_MCF;
    } else {
        /* Disable the MMC Counter Freeze */
        ETH->MMCCR &= ~ETH_MMCCR_MCF;
    }
}


void ETH_MMCResetOnReadCmd(FunctionalState NewState) {
    if (NewState != DISABLE) {
        /* Enable the MMC Counter reset on read */
        ETH->MMCCR |= ETH_MMCCR_ROR;
    } else {
        /* Disable the MMC Counter reset on read */
        ETH->MMCCR &= ~ETH_MMCCR_ROR;
    }
}


void ETH_MMCCounterRolloverCmd(FunctionalState NewState) {
    if (NewState != DISABLE) {
        /* Disable the MMC Counter Stop Rollover  */
        ETH->MMCCR &= ~ETH_MMCCR_CSR;
    } else {
        /* Enable the MMC Counter Stop Rollover */
        ETH->MMCCR |= ETH_MMCCR_CSR;
    }
}


void ETH_MMCCountersReset(void) {
    /* Resets the MMC Counters */
    ETH->MMCCR |= ETH_MMCCR_CR;
}


/**
  * @brief  Enables or disables the specified ETHERNET MMC interrupts.
  * @param  ETH_MMC_IT: specifies the ETHERNET MMC interrupt sources to be enabled or disabled.
  *   This parameter can be any combination of Tx interrupt or
  *   any combination of Rx interrupt (but not both)of the following values:
  *     @arg ETH_MMC_IT_TGF   : When Tx good frame counter reaches half the maximum value
  *     @arg ETH_MMC_IT_TGFMSC: When Tx good multi col counter reaches half the maximum value
  *     @arg ETH_MMC_IT_TGFSC : When Tx good single col counter reaches half the maximum value
  *     @arg ETH_MMC_IT_RGUF  : When Rx good unicast frames counter reaches half the maximum value
  *     @arg ETH_MMC_IT_RFAE  : When Rx alignment error counter reaches half the maximum value
  *     @arg ETH_MMC_IT_RFCE  : When Rx crc error counter reaches half the maximum value
  * @param  NewState: new state of the specified ETHERNET MMC interrupts.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void ETH_MMCITConfig(uint32_t ETH_MMC_IT, FunctionalState NewState) {
    if ((ETH_MMC_IT & (uint32_t)0x10000000) != (uint32_t)RESET) {
        /* Remove Register mak from IT */
        ETH_MMC_IT &= 0xEFFFFFFF;

        /* ETHERNET MMC Rx interrupts selected */
        if (NewState != DISABLE) {
            /* Enable the selected ETHERNET MMC interrupts */
            ETH->MMCRIMR &=(~(uint32_t)ETH_MMC_IT);
        } else {
            /* Disable the selected ETHERNET MMC interrupts */
            ETH->MMCRIMR |= ETH_MMC_IT;
        }
    } else {
        /* ETHERNET MMC Tx interrupts selected */
        if (NewState != DISABLE) {
            /* Enable the selected ETHERNET MMC interrupts */
            ETH->MMCTIMR &=(~(uint32_t)ETH_MMC_IT);
        } else {
            /* Disable the selected ETHERNET MMC interrupts */
            ETH->MMCTIMR |= ETH_MMC_IT;
        }
    }
}


/**
  * @brief  Checks whether the specified ETHERNET MMC IT is set or not.
  * @param  ETH_MMC_IT: specifies the ETHERNET MMC interrupt.
  *   This parameter can be one of the following values:
  *     @arg ETH_MMC_IT_TxFCGC: When Tx good frame counter reaches half the maximum value
  *     @arg ETH_MMC_IT_TxMCGC: When Tx good multi col counter reaches half the maximum value
  *     @arg ETH_MMC_IT_TxSCGC: When Tx good single col counter reaches half the maximum value
  *     @arg ETH_MMC_IT_RxUGFC: When Rx good unicast frames counter reaches half the maximum value
  *     @arg ETH_MMC_IT_RxAEC : When Rx alignment error counter reaches half the maximum value
  *     @arg ETH_MMC_IT_RxCEC : When Rx crc error counter reaches half the maximum value
  * @retval The value of ETHERNET MMC IT (SET or RESET).
  */
ITStatus ETH_GetMMCITStatus(uint32_t ETH_MMC_IT) {
    ITStatus bitstatus = RESET;

    if ((ETH_MMC_IT & (uint32_t)0x10000000) != (uint32_t)RESET) {
        /* ETHERNET MMC Rx interrupts selected */
        /* Check if the ETHERNET MMC Rx selected interrupt is enabled and occurred */
        if ((((ETH->MMCRIR & ETH_MMC_IT) != (uint32_t)RESET)) && ((ETH->MMCRIMR & ETH_MMC_IT) == (uint32_t)RESET)) {
            bitstatus = SET;
        } else {
            bitstatus = RESET;
        }
    } else {
        /* ETHERNET MMC Tx interrupts selected */
        /* Check if the ETHERNET MMC Tx selected interrupt is enabled and occurred */
        if ((((ETH->MMCTIR & ETH_MMC_IT) != (uint32_t)RESET)) && ((ETH->MMCRIMR & ETH_MMC_IT) == (uint32_t)RESET)) {
            bitstatus = SET;
        } else {
            bitstatus = RESET;
        }
    }

    return bitstatus;
}


/**
  * @brief  Get the specified ETHERNET MMC register value.
  * @param  ETH_MMCReg: specifies the ETHERNET MMC register.
  *   This parameter can be one of the following values:
  *     @arg ETH_MMCCR      : MMC CR register
  *     @arg ETH_MMCRIR     : MMC RIR register
  *     @arg ETH_MMCTIR     : MMC TIR register
  *     @arg ETH_MMCRIMR    : MMC RIMR register
  *     @arg ETH_MMCTIMR    : MMC TIMR register
  *     @arg ETH_MMCTGFSCCR : MMC TGFSCCR register
  *     @arg ETH_MMCTGFMSCCR: MMC TGFMSCCR register
  *     @arg ETH_MMCTGFCR   : MMC TGFCR register
  *     @arg ETH_MMCRFCECR  : MMC RFCECR register
  *     @arg ETH_MMCRFAECR  : MMC RFAECR register
  *     @arg ETH_MMCRGUFCR  : MMC RGUFCRregister
  * @retval The value of ETHERNET MMC Register value.
  */
uint32_t ETH_GetMMCRegister(uint32_t ETH_MMCReg) {
    /* Return the selected register value */
    return (*(__IO uint32_t *)(ETH_MAC_BASE + ETH_MMCReg));
}


#define PHY_READ_TO                     ((uint32_t)0x0004FFFF)
#define PHY_WRITE_TO                    ((uint32_t)0x0004FFFF)

uint16_t ETH_ReadPHYRegister(uint16_t PHYAddress, uint16_t PHYReg) {
    uint32_t tmpreg = 0;
    __IO uint32_t timeout = 0;

    /* Get the ETHERNET MACMIIAR value */
    tmpreg = ETH->MACMIIAR;
    /* Keep only the CSR Clock Range CR[2:0] bits value */
    tmpreg &= ~MACMIIAR_CR_MASK;
    /* Prepare the MII address register value */
    tmpreg |=(((uint32_t)PHYAddress<<11) & ETH_MACMIIAR_PA);/* Set the PHY device address */
    tmpreg |=(((uint32_t)PHYReg<<6) & ETH_MACMIIAR_MR);      /* Set the PHY register address */
    tmpreg &= ~ETH_MACMIIAR_MW;                              /* Set the read mode */
    tmpreg |= ETH_MACMIIAR_MB;                               /* Set the MII Busy bit */
    /* Write the result value into the MII Address register */
    ETH->MACMIIAR = tmpreg;
    /* Check for the Busy flag */
    do {
        timeout++;
        tmpreg = ETH->MACMIIAR;
    } while ((tmpreg & ETH_MACMIIAR_MB) && (timeout < (uint32_t)PHY_READ_TO));
    /* Return ERROR in case of timeout */
    if(timeout == PHY_READ_TO) {
        return (uint16_t)ETH_ERROR;
    }

    /* Return data register value */
    return (uint16_t)(ETH->MACMIIDR);
}


uint32_t ETH_WritePHYRegister(uint16_t PHYAddress, uint16_t PHYReg, uint16_t PHYValue) {
    uint32_t tmpreg = 0;
    __IO uint32_t timeout = 0;

    /* Get the ETHERNET MACMIIAR value */
    tmpreg = ETH->MACMIIAR;
    /* Keep only the CSR Clock Range CR[2:0] bits value */
    tmpreg &= ~MACMIIAR_CR_MASK;
    /* Prepare the MII register address value */
    tmpreg |=(((uint32_t)PHYAddress<<11) & ETH_MACMIIAR_PA); /* Set the PHY device address */
    tmpreg |=(((uint32_t)PHYReg<<6) & ETH_MACMIIAR_MR);      /* Set the PHY register address */
    tmpreg |= ETH_MACMIIAR_MW;                               /* Set the write mode */
    tmpreg |= ETH_MACMIIAR_MB;                               /* Set the MII Busy bit */
    /* Give the value to the MII data register */
    ETH->MACMIIDR = PHYValue;
    /* Write the result value into the MII Address register */
    ETH->MACMIIAR = tmpreg;
    /* Check for the Busy flag */
    do {
        timeout++;
        tmpreg = ETH->MACMIIAR;
    } while ((tmpreg & ETH_MACMIIAR_MB) && (timeout < (uint32_t)PHY_WRITE_TO));
    /* Return ERROR in case of timeout */
    if(timeout == PHY_WRITE_TO) {
        return ETH_ERROR;
    }

    /* Return SUCCESS */
    return ETH_SUCCESS;
}
