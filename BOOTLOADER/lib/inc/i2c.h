#ifndef I2C_H_
#define I2C_H_
#include "stm32f0xx.h"                  // Device header

void I2C1_Init(void);
void I2C1_SendByte (uint8_t b);
void I2C1_Init_DMA(void);
void I2C1_Send_DMA(uint8_t* data, uint16_t len);
void I2C1_Receive_DMA(uint8_t* data, uint16_t len);

#endif /* I2C_H_ */
