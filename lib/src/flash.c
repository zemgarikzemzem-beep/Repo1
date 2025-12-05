#include "flash.h"

/*============Блокировка записи во флэш==============*/

inline static void FLASH_Lock(void){
	SET_BIT(FLASH->CR, FLASH_CR_LOCK);
}

//------------------------------------------------------------------------

/*============Разблокировка записи во флэш==============*/

inline static void FLASH_Unlock(void){
	while ((FLASH->SR & FLASH_SR_BSY) != 0);
	if ((FLASH->CR & FLASH_CR_LOCK) != 0){
		FLASH->KEYR = FLASH_KEY1;
		FLASH->KEYR = FLASH_KEY2;
	}
}

//-------------------------------------------------------------------------

/*============Стирание страницы с указанного адреса ( для 030С8Т6 - 1Кб)==============*/

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

//--------------------------------------------------------------------

/*============Чтение последовательности байт по адресу==============*/

void FLASH_ReadStr(uint32_t flash_addr, uint8_t* buff, uint8_t size){
	//uint32_t addr=flash_addr;
	for(uint8_t i=0; i<size; ++i){
		buff[i]=*(__IO uint8_t*)(flash_addr+i);
	}
}

//-------------------------------------------------------------------------

uint8_t FLASH_ReadByte(uint32_t flash_addr){
	return *(__IO uint8_t*)flash_addr;
}

//-------------------------------------------------------------------------

/*============Запись ПЛЛПА по адресу==============*/

void FLASH_WriteStr_Mess(uint32_t flash_addr, uint8_t* data, uint8_t size){
//	uint8_t pagebytes[255]={0,};
//	uint32_t pageaddr=(flash_addr/FLASH_PAGESIZE)*FLASH_PAGESIZE;
//	FLASH_ReadStr(pageaddr, pagebytes, 255);
//	for(uint8_t i=flash_addr%FLASH_PAGESIZE; i<flash_addr%FLASH_PAGESIZE+size; ++i){}
//	pagebytes[flash_addr%FLASH_PAGESIZE]=byte;
//	FLASH_PageErase(pageaddr);
	
	uint32_t addr=flash_addr;
	if(addr%FLASH_PAGESIZE==0) FLASH_PageErase(addr);
	FLASH_Unlock();
	
	FLASH->CR |= FLASH_CR_PG;
	
	if((size%2)==0){
		for(uint8_t i=0; i<size; i+=2){
			*(__IO uint16_t*)(addr) = (data[i]&0xFF)|(data[i+1]<<8);
			while ((FLASH->SR & FLASH_SR_BSY) != 0);
			if ((FLASH->SR & FLASH_SR_EOP) != 0) FLASH->SR = FLASH_SR_EOP;
			addr+=2;
		}
	}
	else{
		for(uint8_t i=0; i<size-1; i+=2){
			*(__IO uint16_t*)(addr) = (data[i]&0xFF)|(data[i+1]<<8);
			while ((FLASH->SR & FLASH_SR_BSY) != 0);
			if ((FLASH->SR & FLASH_SR_EOP) != 0) FLASH->SR = FLASH_SR_EOP;
			addr+=2;
		}
		*(__IO uint16_t*)(addr) = (data[size-1]&0xFF)|(0xFF<<8);
	}
	
	FLASH->CR &= ~FLASH_CR_PG;
	
	FLASH_Lock();
}

//-------------------------------------------------------------------------

/*============Запись последовательности байт по адресу==============*/

void FLASH_WriteStr(uint32_t flash_addr, uint8_t* data, uint8_t size){
	uint8_t pagebytes[MESS_MAX_SIZE]={0,};
	uint8_t offset=flash_addr%FLASH_PAGESIZE;
	uint32_t pageaddr=(flash_addr/FLASH_PAGESIZE)*FLASH_PAGESIZE;
	FLASH_ReadStr(pageaddr, pagebytes, MESS_MAX_SIZE);
	for(uint8_t i=0; i<size; ++i){
		pagebytes[offset+i]=data[i];
	}
	
	FLASH_PageErase(pageaddr);
//	if(addr%FLASH_PAGESIZE==0) FLASH_PageErase(addr);
	FLASH_Unlock();
	
	FLASH->CR |= FLASH_CR_PG;
	
	for(uint8_t i=0; i<MESS_MAX_SIZE; i+=2){  // 
		*(__IO uint16_t*)(pageaddr) = (pagebytes[i]&0xFF)|(pagebytes[i+1]<<8);
		while ((FLASH->SR & FLASH_SR_BSY) != 0);
		if ((FLASH->SR & FLASH_SR_EOP) != 0) FLASH->SR = FLASH_SR_EOP;
		pageaddr+=2;
	}
	
	FLASH->CR &= ~FLASH_CR_PG;
	
	FLASH_Lock();
}

//-------------------------------------------------------------------------

void FLASH_WriteByte(uint32_t flash_addr, uint8_t byte){
	uint8_t pagebytes[MESS_MAX_SIZE]={0,};
	uint32_t pageaddr=(flash_addr/FLASH_PAGESIZE)*FLASH_PAGESIZE;
	FLASH_ReadStr(pageaddr, pagebytes, MESS_MAX_SIZE);
	pagebytes[flash_addr%FLASH_PAGESIZE]=byte;
	FLASH_PageErase(pageaddr);
	FLASH_WriteStr(pageaddr, pagebytes, MESS_MAX_SIZE);
}
