#include "gpio.h"
#include "string.h"
//#include "user.h"
//#include "lcd.h"
#include "flash.h"
//#include "flasher.h"
#include "data.h"
#include "cc2520.h"
#include "eeprom.h"
#include "disp.h"

//------------------------------------------------------------------------------
// сохранение настроек
//------------------------------------------------------------------------------

void Flash_lock();

// сбросить флаги статуса
void Flash_ClearFlag(uint16_t FLASH_FLAG)
{
  FLASH->SR = FLASH_FLAG;
}
// получить статус                                
Flash_Status Flash_GetStatus(void)
{
  Flash_Status flashstatus = Flash_COMPLETE;
  
  if((FLASH->SR & FLASH_FLAG_BSY) == FLASH_FLAG_BSY) flashstatus = Flash_BUSY;
  else flashstatus = Flash_COMPLETE;
  return flashstatus;
}
// дождатьс€ окончани€ предыдущей операции
void Flash_WaitForLastOperation()
{ 
  Flash_Status status = Flash_GetStatus();
  while(status == Flash_BUSY) status = Flash_GetStatus();
}

void Flash_ProgramWord(uint32_t* Address, uint32_t Data)
{
  Flash_WaitForLastOperation();

  FLASH->CR |= CR_PG_Set;
  *Address = Data;
  Flash_WaitForLastOperation();
  FLASH->CR &= ~CR_PG_Set;
}

//-----------------------------------------------------------------------------

inline static void FLASH_Lock(void){
	SET_BIT(FLASH->CR, FLASH_CR_LOCK);
}

//-----------------------------------------------------------------------------

inline static void FLASH_Unlock(void){
	while ((FLASH->SR & FLASH_SR_BSY) != 0);
	if ((FLASH->CR & FLASH_CR_LOCK) != 0){
		FLASH->KEYR = FLASH_KEY1;
		FLASH->KEYR = FLASH_KEY2;
	}
}

//--------------------------------------------------------------------

/*============„тение последовательности байт по адресу==============*/

void FLASH_ReadStr(uint32_t flash_addr, uint8_t* buff, uint8_t size){
	//uint32_t addr=flash_addr;
	for(uint8_t i=0; i<size; ++i){
		buff[i]=*(__IO uint8_t*)(flash_addr+i);
	}
}

//-----------------------------------------------------------------------------

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

//-----------------------------------------------------------------------------

void FLASH_WriteBlock(uint32_t flash_addr, uint8_t* data, uint8_t size){
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

/*============«апись последовательности байт по адресу==============*/

void FLASH_WriteStr(uint32_t flash_addr, uint8_t* data, uint8_t size){
	uint8_t pagebytes[254]={0,};
	uint8_t offset=flash_addr%FLASH_PAGESIZE;
	uint32_t pageaddr=(flash_addr/FLASH_PAGESIZE)*FLASH_PAGESIZE;
	FLASH_ReadStr(pageaddr, pagebytes, 254);
	for(uint8_t i=0; i<size; ++i){
		pagebytes[offset+i]=data[i];
	}
	
	FLASH_PageErase(pageaddr);
//	if(addr%FLASH_PAGESIZE==0) FLASH_PageErase(addr);
	FLASH_Unlock();
	
	FLASH->CR |= FLASH_CR_PG;
	
	for(uint8_t i=0; i<254; i+=2){
		*(__IO uint16_t*)(pageaddr) = (pagebytes[i]&0xFF)|(pagebytes[i+1]<<8);
		while ((FLASH->SR & FLASH_SR_BSY) != 0);
		if ((FLASH->SR & FLASH_SR_EOP) != 0) FLASH->SR = FLASH_SR_EOP;
		pageaddr+=2;
	}
	
	FLASH->CR &= ~FLASH_CR_PG;
	
	FLASH_Lock();
}


void FLASH_WriteByte(uint32_t flash_addr, uint8_t byte){
	uint8_t pagebytes[255]={0,};
	uint32_t pageaddr=(flash_addr/FLASH_PAGESIZE)*FLASH_PAGESIZE;
	FLASH_ReadStr(pageaddr, pagebytes, 255);
	pagebytes[flash_addr%FLASH_PAGESIZE]=byte;
	FLASH_PageErase(pageaddr);
	FLASH_WriteStr(pageaddr, pagebytes, 255);
}

//-----------------------------------------------------------------------------

uint8_t saved_str[100]={0,};

uint8_t* FLASH_Read8byAddr(uint32_t flash_addr, uint8_t size){
	//uint32_t addr=flash_addr;
	for(uint8_t i=0; i<size; ++i){
		saved_str[i]=*(__IO uint8_t*)(flash_addr+i);
	}
	return saved_str;
}

//-----------------------------------------------------------------------------

// стереть страницу                                                                                                                                            u 
void Flash_ErasePage(uint8_t sector)
{
  Flash_WaitForLastOperation(); 
  FLASH->CR &= ~(15<<3);        // установить адрес 11 сектора
  FLASH->CR |= (sector<<3);
  FLASH->CR |= 0x200;           // psize установить в 32 бита
  FLASH->CR |= CR_SER_Set;
  FLASH->CR |= CR_STRT_Set;
  Flash_WaitForLastOperation();
}

void Flash_unlock()
{
  /* Authorize the FPEC Access */
  FLASH->KEYR = FLASH_KEY1;
  FLASH->KEYR = FLASH_KEY2;
}
void sector_erase(uint8_t sector)
{
//  Flash_unlock();
//  Flash_ErasePage(sector);
	FLASH_Unlock();
	for(uint8_t i=0; i<FLASH_PAGES_IN_SECTOR; ++i) FLASH_PageErase(FLASH_BASE+sector*FLASH_SECTOR_SIZE+i*FLASH_PAGESIZE);
  flash_buf_data = 1;
}

void Flash_lock()
{
    /* Set the Lock Bit to lock the FPEC and the FCR */
  FLASH->CR |= CR_LOCK_Set;
}

void flash_buf_save()
{
  uint32_t ptr = flash_buf_addr;
//  uint32_t *src = (uint32_t*)flash_buf;
  ptr += APPLICATION_ADDRESS;
////  __disable_irq();
////  for (int i = 0; i < 64; i += 4) Flash_ProgramWord((uint32_t*)(ptr + i), (uint32_t)*(src++));
////  __enable_irq();
	FLASH_WriteBlock(ptr, flash_buf, 64);
	
}

void flash_attr_save()
{
  uint32_t data = (uint16_t)prg_CRC;
  data <<= 16;
  data |= (uint16_t)prg_size;
//  sector_erase(FLASH_ATTRIB_SECTOR);
	FLASH_WriteStr(FLASH_CRC_ADDR, (uint8_t*)&data, 4);
//  Flash_ProgramWord((uint32_t*)FLASH_CRC_ADDR, data);
  Flash_lock();
}

void write_prg_size()
{
  prg_size = ccbuf[4];
  prg_size <<= 8;
  prg_size |= ccbuf[5];
  ccbuf[2] = 3;
  echo();
}

void write_prg_CRC()
{
  prg_CRC = ccbuf[4];
  prg_CRC <<= 8;
  prg_CRC |= ccbuf[5];
  
  ccbuf[2] = 3;
  echo();
}
// записать программный блок
void write_prg()
{
  uint16_t sum = 0;
  uint32_t addr = ccbuf[4];
  addr <<= 8;
  addr |= ccbuf[5];
  addr *= 64;            // ???
	
//	if(addr==0) memcpy(flash_buf, ccbuf+6, 64);

  if (addr < FLASH_BOOTLOADER_END) {                     // защита области бутлодера
    send_nack();
    return;
  }
  if (addr < first_addr) first_addr = addr;     // установить младший адрес записываемой программы
  

  if (addr != flash_buf_addr) {
    if (flash_buf_data) flash_buf_save();               // если в буфере есть данные
    
//		if(addr%FLASH_SECTOR_SIZE==0) sector_erase((addr+APPLICATION_ADDRESS-FLASH_BASE)/FLASH_SECTOR_SIZE);
		
//    switch (addr) {
////      case 0: sector_erase(0); break;         // сектора с бутлодером не стираютс€
////      case 0x4000: if (flash_buf_data) Flash_lock(); sector_erase(1); break;
//      case 0x8000: if (flash_buf_data) Flash_lock(); sector_erase(2); break; // 
//      case 0xC000: if (flash_buf_data) Flash_lock(); sector_erase(3); break; // 
//      case 0x10000: if (flash_buf_data) Flash_lock(); sector_erase(4); break;  // 
//      case 0x20000: if (flash_buf_data) Flash_lock(); sector_erase(5); break; // 
//      case 0x40000: if (flash_buf_data) Flash_lock(); sector_erase(6); break; // 
//      case 0x60000: if (flash_buf_data) Flash_lock(); sector_erase(7); break; // 
//      case 0x80000: if (flash_buf_data) Flash_lock(); sector_erase(8); break; // 
//      case 0xA0000: if (flash_buf_data) Flash_lock(); sector_erase(9); break; // 
//      case 0xC0000: if (flash_buf_data) Flash_lock(); sector_erase(10); break; // 
//      case 0xE0000: if (flash_buf_data) Flash_lock(); sector_erase(11); break; // 
//    }
    flash_buf_data = 1;                                 // прием очередного пакета
    flash_buf_addr = addr+APPLICATION_ADDRESS;                              // запомнить адрес пакета
    memcpy(flash_buf, ccbuf + 6, 64);                   // сохранить прин€тый пакет в промежуточном буфере
  }
	else memcpy(flash_buf, ccbuf + 6, 64);
	
  for (int i = 0; i < 64; i++) {
    sum += ccbuf[6 + i];
    if (sum > 0xff) sum -= 0xff;
  }
  
  ccbuf[0] = 7;
  ccbuf[2] = 3;
  ccbuf[3] = ccbuf[4];
  ccbuf[4] = ccbuf[5];
  ccbuf[5] = sum;
  echo();
}

//-------------------------------------------------------------------------------------------------------------

uint32_t mess_num=0;
uint8_t tmp_buf[MESS_MAX_SIZE]={0,};
uint8_t EEPROM_MAXIMAL_SEND=200;
uint32_t current_flash_addr=0;

void write_program(void){
	if(!(ccbuf[8]|ccbuf[9])){
		TFT_Fill_Color(YELLOW);
		TFT_Send_Str(5, 60, "»дЄт запись прошивки!", 21, Font_11x18, RED, YELLOW);
		mess_num=0;
	}
	int sum = 0;
	uint16_t addr = (ccbuf[8]<<8)+ccbuf[9];
	
	for (uint8_t i = 0; i < 64; i++) {
		sum += ccbuf[i + 10];
		if (sum > 0xff) sum -= 0xff;
	}
	
	if(addr!=flash_buf_addr){  //  && addr%MESS_MAX_SIZE==0x40
		FLASH_WriteBlock(APPLICATION_ADDRESS+flash_buf_addr, flash_buf, MESS_MAX_SIZE/2);
		flash_buf_addr=addr;
		++mess_num;
		memcpy(flash_buf, ccbuf+10, 64);
	}
	else memcpy(flash_buf, ccbuf+10, 64);
	
  flash_buf_data = 1; 
	
	ccbuf[0] = 12;
	ccbuf[1] = 0x28;
	ccbuf[10] = (uint8_t)sum;
	echo();
}
        