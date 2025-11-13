#ifndef SPIDMA_H_
#define SPIDMA_H_
#include "stm32f10x.h"                  // Device header

void SPI1_Init_DMA(void);
void SPI1_Receive_DMA(uint8_t* data, uint16_t len);
void SPI1_Send_DMA(uint8_t* data, uint16_t len);

#endif /* SPIDMA_H_ */
