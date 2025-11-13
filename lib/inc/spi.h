#ifndef SPI_H_
#define SPI_H_
#include "stm32f0xx.h"                  // Device header

void SPI1_Init(void);
void SPI2_Init(void);
void SPI2_Send_Byte(uint8_t op, uint8_t b);

#endif /* SPI_H_ */
