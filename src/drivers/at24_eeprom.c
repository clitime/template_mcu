#include "at24_eeprom.h"
#include "stm32f4xx.h"
#include "stm32f4xx_i2c.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"


#define PCA_I2C_TIMEOUT         ((uint32_t)0x3FFFFF) /*!< I2C Time out */
#define PCA_I2C_SPEED      		  400000 /*!< I2C Speed */

#define PCA_FLAG_TIMEOUT        ((uint32_t)0xffff)
#define PCA_LONG_TIMEOUT        ((uint32_t)(30 * PCA_FLAG_TIMEOUT))

#define PCA_I2C                 I2C1

volatile uint32_t PCA_Timeout=PCA_LONG_TIMEOUT;


void I2C_config(void) {
    I2C_InitTypeDef I2C_InitStructure;

    GPIO_InitTypeDef GPIO_InitStruct;

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_Speed = GPIO_High_Speed;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStruct.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOB, &GPIO_InitStruct);
     
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_I2C1);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource8, GPIO_AF_I2C1);

    I2C_SoftwareResetCmd(PCA_I2C, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1,ENABLE);

    I2C_DeInit(PCA_I2C);
    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStructure.I2C_OwnAddress1 = 0;
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_InitStructure.I2C_ClockSpeed = PCA_I2C_SPEED;

    I2C_Cmd(PCA_I2C, ENABLE);
    I2C_Init(PCA_I2C, &I2C_InitStructure);
    I2C_AcknowledgeConfig(PCA_I2C, ENABLE);
}


uint8_t I2C_write(const uint16_t reg, const uint8_t *buffer, const uint32_t len) {
    uint32_t i;
    uint8_t slaveAdr = 0xa0;

    // slaveAdr |= (uint8_t)((reg >> 7) & 0x0e);

    PCA_Timeout = PCA_LONG_TIMEOUT;
    while (I2C_GetFlagStatus(PCA_I2C,I2C_FLAG_BUSY)) {
        if((PCA_Timeout--) == 0) {
            return I2C_BUS_BUSY;
        }
    }

    I2C_GenerateSTART(PCA_I2C, ENABLE);
    PCA_Timeout = PCA_FLAG_TIMEOUT;
    while(!I2C_CheckEvent(PCA_I2C, I2C_EVENT_MASTER_MODE_SELECT)) {
        if((PCA_Timeout--) == 0) {
            return I2C_ERROR;
        }
    }

    I2C_Send7bitAddress(PCA_I2C, slaveAdr, I2C_Direction_Transmitter);

    PCA_Timeout = PCA_FLAG_TIMEOUT;
    while(!I2C_CheckEvent(PCA_I2C, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)) {
        if((PCA_Timeout--) == 0) {
            return I2C_ERROR;
        }
    }

    I2C_SendData(PCA_I2C, (uint8_t)(reg & 0x00ff));
    PCA_Timeout = PCA_FLAG_TIMEOUT;
    while(!I2C_CheckEvent(PCA_I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED)) {
        if((PCA_Timeout--) == 0) {
            return I2C_ERROR;
        }
    }

    I2C_SendData(PCA_I2C, (uint8_t)(reg & 0xff00) >> 0x08);
    PCA_Timeout = PCA_FLAG_TIMEOUT;
    while(!I2C_CheckEvent(PCA_I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED)) {
        if((PCA_Timeout--) == 0) {
            return I2C_ERROR;
        }
    }
    /* Send BufferLength bytes to EEPROM */
    for(i=0; i<len; i++) {
        I2C_SendData(PCA_I2C, buffer[i]);
        PCA_Timeout = PCA_FLAG_TIMEOUT;
        while(!I2C_CheckEvent(PCA_I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED)) {
            if((PCA_Timeout--) == 0) {
                return I2C_ERROR;
            }
        }
    }
    I2C_GenerateSTOP(PCA_I2C, ENABLE);
    return I2C_OK;
}


uint8_t I2C_read(const uint16_t reg, uint8_t *buffer, const uint32_t len) {
    uint32_t i;

    uint8_t slaveAdr = 0xa0;

    // slaveAdr |= (uint8_t)((reg >> 7) & 0x0e);

    PCA_Timeout = PCA_LONG_TIMEOUT;
    while (I2C_GetFlagStatus(PCA_I2C,I2C_FLAG_BUSY)) {
        if((PCA_Timeout--) == 0) {
            return I2C_BUS_BUSY;
        }
    }
    I2C_GenerateSTART(PCA_I2C, ENABLE);
    PCA_Timeout = PCA_FLAG_TIMEOUT;
    while(!I2C_CheckEvent(PCA_I2C, I2C_EVENT_MASTER_MODE_SELECT)) {
        if((PCA_Timeout--) == 0) {
            return I2C_ERROR;
        }
    }

    I2C_Send7bitAddress(PCA_I2C, slaveAdr, I2C_Direction_Transmitter);

    PCA_Timeout = PCA_FLAG_TIMEOUT;
    while(!I2C_CheckEvent(PCA_I2C, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)) {
        if((PCA_Timeout--) == 0) {
            return I2C_ERROR;
        }
    }

    I2C_SendData(PCA_I2C, (uint8_t)(reg & 0x00ff));
    PCA_Timeout = PCA_FLAG_TIMEOUT;
    while(!I2C_CheckEvent(PCA_I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED)) {
        if((PCA_Timeout--) == 0) {
            return I2C_ERROR;
        }
    }

    I2C_SendData(PCA_I2C, (uint8_t)(reg & 0xff00) >> 0x08);
    PCA_Timeout = PCA_FLAG_TIMEOUT;
    while(!I2C_CheckEvent(PCA_I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED)) {
        if((PCA_Timeout--) == 0) {
            return I2C_ERROR;
        }
    }
    /* Second START */
    I2C_GenerateSTART(PCA_I2C, ENABLE);
    PCA_Timeout = PCA_FLAG_TIMEOUT;
    while(!I2C_CheckEvent(PCA_I2C, I2C_EVENT_MASTER_MODE_SELECT)) {
        if((PCA_Timeout--) == 0) {
            return I2C_ERROR;
        }
    }
    /* Send adress read */
    I2C_Send7bitAddress(PCA_I2C, slaveAdr, I2C_Direction_Receiver);

    PCA_Timeout = PCA_FLAG_TIMEOUT;
    while(!I2C_CheckEvent(PCA_I2C, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED)) {
        if((PCA_Timeout--) == 0) {
            return I2C_ERROR;
        }
    }
    /* Read bytes to buffer */
    for(i=0; i<(len-1); i++) {
        I2C_AcknowledgeConfig(PCA_I2C, ENABLE);
        PCA_Timeout = PCA_FLAG_TIMEOUT;
        while(!I2C_CheckEvent(PCA_I2C, I2C_EVENT_MASTER_BYTE_RECEIVED)) {
            if((PCA_Timeout--) == 0) {
                return I2C_ERROR;
            }
        }
        buffer[i] = I2C_ReceiveData(PCA_I2C);
    }
    I2C_AcknowledgeConfig(PCA_I2C, DISABLE);
    I2C_GenerateSTOP(PCA_I2C, ENABLE);
    PCA_Timeout = PCA_FLAG_TIMEOUT;
    while(!I2C_CheckEvent(PCA_I2C, I2C_EVENT_MASTER_BYTE_RECEIVED)) {
        if((PCA_Timeout--) == 0) {
            return I2C_ERROR;
        }
    }
    buffer[len-1] = I2C_ReceiveData(PCA_I2C);
    return I2C_OK;
}


uint8_t readData (const uint16_t adr, uint8_t* data, const uint16_t len) {
    uint8_t result = I2C_OK;

    for (uint16_t ix = 0; ix != len; ++ix) {
        result = I2C_read(adr + ix, &data[ix], 1);
        if (result != I2C_OK) {
            I2C_config();
            result = I2C_read(adr + ix, &data[ix], 1);
        }
    }

    return result;
}



#include "FreeRTOS.h"
#include "task.h"

uint8_t writeData(const uint16_t adr, const uint8_t* data, const uint16_t len) {
    uint8_t result = I2C_OK;

    uint16_t ix = 0;
    while (ix < len) {
        uint16_t tmp_len = 1;

        result = I2C_write(adr + ix, &data[ix], tmp_len);

        ix += tmp_len;
        vTaskDelay(5);
    }

    return result;
}