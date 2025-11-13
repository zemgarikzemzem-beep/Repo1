#include "flash1.h"


inline static void FLASH_Lock(void){
	SET_BIT(FLASH->CR, FLASH_CR_LOCK);
}

inline static void FLASH_Unlock(void){
	while ((FLASH->SR & FLASH_SR_BSY) != 0);
	if ((FLASH->CR & FLASH_CR_LOCK) != 0){
		FLASH->KEYR = FLASH_KEY1;
		FLASH->KEYR = FLASH_KEY2;
	}
}

void FLASH_PageErase(uint32_t page_addr){
	FLASH_Unlock();
	
	while (FLASH->SR & FLASH_SR_BSY);
	FLASH->CR |= FLASH_CR_PER;
	FLASH->AR = page_addr;
	FLASH->CR |= FLASH_CR_STRT;
	while (FLASH->SR & FLASH_SR_BSY);
	if ((FLASH->SR & FLASH_SR_EOP) != 0) FLASH->SR = FLASH_SR_EOP;
	FLASH->CR &= ~FLASH_CR_PER;
	
	FLASH_Lock();
}

void FLASH_WritebyAddr(uint32_t flash_addr, uint8_t* data, uint8_t size){
	uint32_t addr=flash_addr;
	
	while (FLASH->SR & FLASH_SR_BSY);
	FLASH_Unlock();
	
	FLASH->CR |= FLASH_CR_PG;
	
  __disable_irq();
	
	if((size%2)==0){
		for(uint8_t i=0; i<size; i+=2){
			*(__IO uint16_t*)(addr) = (data[i]&0xFF)|(data[i+1]<<8);
			while (FLASH->SR & FLASH_SR_BSY);
			if ((FLASH->SR & FLASH_SR_EOP) != 0) FLASH->SR = FLASH_SR_EOP;
			addr+=2;
		}
	}
	else{
		for(uint8_t i=0; i<size-1; i+=2){
			*(__IO uint16_t*)(addr) = (data[i]&0xFF)|(data[i+1]<<8);
			while (FLASH->SR & FLASH_SR_BSY);
			if ((FLASH->SR & FLASH_SR_EOP) != 0) FLASH->SR = FLASH_SR_EOP;
			addr+=2;
		}
		*(__IO uint16_t*)(addr) = (data[size-1]&0xFF)|(0xFF<<8);
	}
	
	__enable_irq();
	
	FLASH->CR &= ~FLASH_CR_PG;
	
	FLASH_Lock();
}

uint8_t saved_str[100]={0,};

uint8_t* FLASH_Read8byAddr(uint32_t flash_addr, uint8_t size){
	//uint32_t addr=flash_addr;
	for(uint8_t i=0; i<size; ++i){
		saved_str[i]=*(__IO uint8_t*)(flash_addr+i);
	}
	return saved_str;
}
