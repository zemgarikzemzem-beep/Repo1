#ifndef FLASH_H_
#define FLASH_H_
//------------------------------------------------
#include "stm32f0xx.h"                  // Device header
//------------------------------------------------
#define FLASH_PLA_ADDR						0x08020000
#define FLASH_SETTINGS_ADDR				0x08030000
#define FLASH_SDFB_ADDR						0x08030014  // ShutDownedFromButton
#define FLASH_REC_MESS_ADDR				0x08030800
#define	FLASH_REC_MESS_TMPBUF			0x08031800
#define FLASH_PAGESIZE						0x800
#define SETTINGS_BYTES_NUM				6            // Чтобы не затиралось!
#define	REC_MESS_MAX_NUM					20

enum pager_settings{
	PAGER_NUM_LB,
	PAGER_NUM_HB,
	MINE,
	ALERT,
	Umin_LB,
	Umin_HB,
	PLA_ITEMS_NUM_1,
	PLA_ITEMS_NUM_0,
	PLA_CRC_0,
	PLA_CRC_1,
	PLA_CRC_2,
	PLA_CRC_3,
	PRG_WRITE,
	PRG_CRC_0,
	PRG_CRC_1,
	PRG_SIZE_0,
	PRG_SIZE_1,
	REC_MESS_NUM
};

enum pager_alerts{
	EMPTY_BYTE,
	LIGHT_UP_SCREEN,
	ZOOMER,
	SOUND,
	BIT_DEPTH
};

//------------------------------------------------
void FLASH_PageErase(uint32_t page_addr);
void FLASH_WriteStr_PLA(uint32_t flash_addr, uint8_t* data, uint8_t size);
void FLASH_WriteStr(uint32_t flash_addr, uint8_t* data, uint8_t size);
void FLASH_WriteByte(uint32_t flash_addr, uint8_t byte);
void FLASH_ReadStr(uint32_t flash_addr, uint8_t* buff, uint8_t size);
uint8_t FLASH_ReadByte(uint32_t flash_addr);
//------------------------------------------------
#endif /* FLASH_H_ */
