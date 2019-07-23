#include "http_flash.h"
#include "stm32f4xx_flash.h"
#include "stm32f4xx.h"

void flashEraseBlock(void) {
  FLASH_Unlock();

  if(FLASH->SR & FLASH_SR_PGAERR)
    FLASH->SR |= FLASH_SR_PGAERR;
  if(FLASH->SR & FLASH_SR_PGPERR)
    FLASH->SR |= FLASH_SR_PGPERR;
  if(FLASH->SR & FLASH_SR_PGSERR)
    FLASH->SR |= FLASH_SR_PGSERR;

  __disable_irq();

  for (uint32_t ix = 5; ix != 12; ++ix) {
    while(FLASH->SR & FLASH_FLAG_BSY);

    FLASH->CR &= ~FLASH_CR_PSIZE;
    FLASH->CR |= FLASH_PSIZE_WORD;
    FLASH->CR |= FLASH_CR_SER;
    FLASH->CR &= ~FLASH_CR_SNB;
    FLASH->CR |= ix << 3;
    FLASH->CR |= FLASH_CR_STRT;

    // Check for the FLASH Status
    while(FLASH->SR & FLASH_FLAG_BSY);
  }
  __enable_irq();

  FLASH_Lock();
}

uint8_t flashWriteData (volatile uint32_t *addr, uint32_t *data, uint32_t len) {
  if (*addr % 4)
    return 3;

  if (!IS_FLASH_ADDRESS((*addr + len) - 4))
    return 2;

  FLASH_Unlock();

  if(FLASH->SR & FLASH_SR_PGAERR)
    FLASH->SR |= FLASH_SR_PGAERR;
  if(FLASH->SR & FLASH_SR_PGPERR)
    FLASH->SR |= FLASH_SR_PGPERR;
  if(FLASH->SR & FLASH_SR_PGSERR)
    FLASH->SR |= FLASH_SR_PGSERR;

  for (uint32_t i = 0; i < len; i++) {
    if(FLASH_ProgramWord(*addr, *(uint32_t*)(data + i)) != FLASH_COMPLETE) {
      FLASH_Lock();
      return 1;
    }

    *addr += 4;
  }

  FLASH_Lock();
  return 0;
}
