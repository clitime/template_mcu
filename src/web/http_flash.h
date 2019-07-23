#pragma once
#ifndef _HTTP_FLASH_H_
#define _HTTP_FLASH_H_

#include <stdint.h>

void flashEraseBlock(void);
uint8_t flashWriteData(volatile uint32_t *addr, uint32_t *data, uint32_t len);

#endif
