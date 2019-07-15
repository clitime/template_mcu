#ifndef I2C_H_
#define I2C_H_

#include <stdint.h>

enum {
  I2C_OK = 0,
  I2C_ERROR,
  I2C_BUS_BUSY,
};


void I2C_config(void);
uint8_t I2C_write(const uint16_t reg, const uint8_t *buffer, const uint32_t len);
uint8_t I2C_read(const uint16_t reg, uint8_t *buffer, const uint32_t len);

uint8_t readData(const uint16_t adr, uint8_t* data, const uint16_t len);
uint8_t writeData(const uint16_t adr, const uint8_t* data, const uint16_t len);

#endif