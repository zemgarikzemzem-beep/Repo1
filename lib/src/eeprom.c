#include "eeprom.h"
#include "i2c.h"

#define EEPROM_ADDR		0xA0

extern void delay(__IO uint32_t tck);




/*===========Функция отправки байта по адресу==============*/

void EEPROM_SendByte(uint32_t addr, uint8_t b){
	while(I2C1->ISR&I2C_ISR_BUSY);
	I2C1->CR2=(I2C_CR2_RELOAD|(2<<I2C_CR2_NBYTES_Pos)|(EEPROM_ADDR<<I2C_CR2_SADD_Pos)|I2C_CR2_START);
	while(!(I2C1->ISR&I2C_ISR_TXIS));
	I2C1->TXDR = (addr>>8);
	while(!(I2C1->ISR&I2C_ISR_TXIS));
	I2C1->TXDR = addr&0xFF;
	while(!(I2C1->ISR&I2C_ISR_TCR));
	I2C1->TXDR = b;
	I2C1->CR2=(I2C_CR2_AUTOEND|(1<<I2C_CR2_NBYTES_Pos)|(EEPROM_ADDR<<I2C_CR2_SADD_Pos)|I2C_CR2_START);
	delay(5);
}




/*===========Функция отправки пункта ПЛА по адресу==============*/

void EEPROM_SendStr_PLA(uint32_t addr, uint8_t* str, uint8_t size){
	while(I2C1->ISR&I2C_ISR_BUSY);
	I2C1->CR2=(I2C_CR2_RELOAD|(2<<I2C_CR2_NBYTES_Pos)|(EEPROM_ADDR<<I2C_CR2_SADD_Pos)|I2C_CR2_START);
	while(!(I2C1->ISR&I2C_ISR_TXIS));
	I2C1->TXDR = (addr>>8);
	while(!(I2C1->ISR&I2C_ISR_TXIS));
	I2C1->TXDR = addr&0xFF;
	while(!(I2C1->ISR&I2C_ISR_TCR));
	I2C1->CR2=(I2C_CR2_AUTOEND|(PAGE_SIZE<<I2C_CR2_NBYTES_Pos)|(EEPROM_ADDR<<I2C_CR2_SADD_Pos));
	I2C1->CR2|=I2C_CR2_START;
	for(uint8_t i=0; i<PAGE_SIZE; ++i){
		if(i<size) I2C1->TXDR = str[i];
		else I2C1->TXDR = 0;
		while(!(I2C1->ISR&I2C_ISR_TXE));
	}
	delay(5);
	
	if(size>PAGE_SIZE){
		while(I2C1->ISR&I2C_ISR_BUSY);
		I2C1->CR2=(I2C_CR2_RELOAD|(2<<I2C_CR2_NBYTES_Pos)|(EEPROM_ADDR<<I2C_CR2_SADD_Pos)|I2C_CR2_START);
		while(!(I2C1->ISR&I2C_ISR_TXIS));
		I2C1->TXDR = ((addr+0x40)>>8);
		while(!(I2C1->ISR&I2C_ISR_TXIS));
		I2C1->TXDR = (addr+0x40)&0xFF;
		while(!(I2C1->ISR&I2C_ISR_TCR));
		I2C1->CR2=(I2C_CR2_AUTOEND|(PAGE_SIZE<<I2C_CR2_NBYTES_Pos)|(EEPROM_ADDR<<I2C_CR2_SADD_Pos));
		I2C1->CR2|=I2C_CR2_START;
		for(uint8_t i=PAGE_SIZE; i<MESS_MAX_SIZE; ++i){
			if(i<size) I2C1->TXDR = str[i];
			else I2C1->TXDR = 0;
			while(!(I2C1->ISR&I2C_ISR_TXE));
		}
		delay(5);
	}
//	I2C1->CR2|=I2C_CR2_STOP;
}




/*===========Функция отправки группы байт по адресу==============*/

void EEPROM_SendStr(uint32_t addr, uint8_t* str, uint8_t size){
	while(I2C1->ISR&I2C_ISR_BUSY);
	I2C1->CR2=(I2C_CR2_RELOAD|(2<<I2C_CR2_NBYTES_Pos)|(EEPROM_ADDR<<I2C_CR2_SADD_Pos)|I2C_CR2_START);
	while(!(I2C1->ISR&I2C_ISR_TXIS));
	I2C1->TXDR = (addr>>8);
	while(!(I2C1->ISR&I2C_ISR_TXIS));
	I2C1->TXDR = addr&0xFF;
	while(!(I2C1->ISR&I2C_ISR_TCR));
	
	if(size<PAGE_SIZE){
		I2C1->CR2=(I2C_CR2_AUTOEND|((size)<<I2C_CR2_NBYTES_Pos)|(EEPROM_ADDR<<I2C_CR2_SADD_Pos));
		I2C1->CR2|=I2C_CR2_START;
		for(uint8_t i=0; i<size; ++i){
			while(!(I2C1->ISR&I2C_ISR_TXE));
			I2C1->TXDR = str[i];
		}
//		while(!(I2C1->ISR&I2C_ISR_TXE));
//		I2C1->TXDR = 0;
		
		delay(5);
	}
	
	else{
		I2C1->CR2=(I2C_CR2_AUTOEND|(PAGE_SIZE<<I2C_CR2_NBYTES_Pos)|(EEPROM_ADDR<<I2C_CR2_SADD_Pos));
		I2C1->CR2|=I2C_CR2_START;
		for(uint8_t i=0; i<PAGE_SIZE; ++i){
			while(!(I2C1->ISR&I2C_ISR_TXE));
			I2C1->TXDR = str[i];
		}
		
		delay(5);
		
		while(I2C1->ISR&I2C_ISR_BUSY);
		I2C1->CR2=(I2C_CR2_RELOAD|(2<<I2C_CR2_NBYTES_Pos)|(EEPROM_ADDR<<I2C_CR2_SADD_Pos)|I2C_CR2_START);
		while(!(I2C1->ISR&I2C_ISR_TXIS));
		I2C1->TXDR = ((addr+0x40)>>8);
		while(!(I2C1->ISR&I2C_ISR_TXIS));
		I2C1->TXDR = (addr+0x40)&0xFF;
		while(!(I2C1->ISR&I2C_ISR_TCR));
		I2C1->CR2=(I2C_CR2_AUTOEND|((size-PAGE_SIZE)<<I2C_CR2_NBYTES_Pos)|(EEPROM_ADDR<<I2C_CR2_SADD_Pos));
		I2C1->CR2|=I2C_CR2_START;
		for(uint8_t i=PAGE_SIZE; i<size; ++i){
			while(!(I2C1->ISR&I2C_ISR_TXE));
			I2C1->TXDR = str[i];
		}
//		while(!(I2C1->ISR&I2C_ISR_TXE));
//		I2C1->TXDR = 0;
		
		delay(5);
	}
//	I2C1->CR2|=I2C_CR2_STOP;
}




/*============Функция чтения байта по адресу==============*/

uint8_t EEPROM_ReadByte(uint32_t addr){
	while(I2C1->ISR&I2C_ISR_BUSY);
	I2C1->CR2=((2<<I2C_CR2_NBYTES_Pos)|(EEPROM_ADDR<<I2C_CR2_SADD_Pos)|I2C_CR2_START);
	while(!(I2C1->ISR&I2C_ISR_TXIS));
	I2C1->TXDR = (addr>>8);
	while(!(I2C1->ISR&I2C_ISR_TXIS));
	I2C1->TXDR = addr&0xFF;
	while(!(I2C1->ISR&I2C_ISR_TC));
	I2C1->CR2=(I2C_CR2_AUTOEND|(1<<I2C_CR2_NBYTES_Pos)|(EEPROM_ADDR<<I2C_CR2_SADD_Pos)|I2C_CR2_RD_WRN|I2C_CR2_START);
	while(!(I2C1->ISR&I2C_ISR_RXNE));
	return (I2C1->RXDR);
}



/*============Функция чтения группы байт по адресу==============*/

void EEPROM_ReadStr(uint32_t addr, uint8_t* buff, uint8_t size){
	memset(buff, 0, size); // MESS_MAX_SIZE
	while(I2C1->ISR&I2C_ISR_BUSY);
	I2C1->CR2=((2<<I2C_CR2_NBYTES_Pos)|(EEPROM_ADDR<<I2C_CR2_SADD_Pos)|I2C_CR2_START);
	while(!(I2C1->ISR&I2C_ISR_TXIS));
	I2C1->TXDR = (addr>>8);
	while(!(I2C1->ISR&I2C_ISR_TXIS));
	I2C1->TXDR = addr&0xFF;
	while(!(I2C1->ISR&I2C_ISR_TC));
	I2C1->CR2=(I2C_CR2_AUTOEND|(size<<I2C_CR2_NBYTES_Pos)|(EEPROM_ADDR<<I2C_CR2_SADD_Pos)|I2C_CR2_RD_WRN|I2C_CR2_START);
	for(uint8_t i=0; i<size; ++i){ // MESS_MAX_SIZE
		while(!(I2C1->ISR&I2C_ISR_RXNE));
		buff[i]=I2C1->RXDR;
	}
}
