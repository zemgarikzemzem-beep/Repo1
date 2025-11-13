#ifndef EEPROM_H_
#define EEPROM_H_
#include "stm32f0xx.h"                  // Device header

#define MESS_MAX_SIZE 				128
#define PAGE_SIZE							64
#define EEPROM_MESSAGES_ADDR	0x00000000
//#define EEPROM_SETTINGS_ADDR+PLA_ITEMS_NUM_1 		0x0003D090
#define EEPROM_SETTINGS_ADDR 	0x00006500            // Размер EEPROM - 256кбит, не килобайт !!!

void EEPROM_SendByte(uint32_t addr, uint8_t b);
void EEPROM_SendStr(uint32_t addr, uint8_t* str, uint8_t size);
void EEPROM_WriteBlock(uint32_t addr, uint8_t* str, uint8_t size);
uint8_t EEPROM_ReadByte(uint32_t addr);
void EEPROM_ReadStr(uint32_t addr, uint8_t* str, uint8_t size);

#endif /* EEPROM_H_ */
