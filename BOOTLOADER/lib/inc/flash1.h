#ifndef FLASH_H_
#define FLASH_H_
//------------------------------------------------
#include "stm32f0xx.h"                  // Device header
//------------------------------------------------
void FLASH_PageErase(uint32_t page_addr);
void FLASH_WritebyAddr(uint32_t flash_addr, uint8_t* data, uint8_t size);
uint8_t* FLASH_Read8byAddr(uint32_t flash_addr, uint8_t size);
//------------------------------------------------
#endif /* FLASH_H_ */
