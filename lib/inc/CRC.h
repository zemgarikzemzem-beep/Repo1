#ifndef CRC_H_
#define CRC_H_
#include "stm32f0xx.h"                  // Device header

void CRC_Init(void);
uint32_t CRC_Result(uint8_t buff[], uint32_t buffsize);
uint32_t crc32_fast(uint8_t *buff, uint32_t buffsize);

#endif /* CRC_H_ */
