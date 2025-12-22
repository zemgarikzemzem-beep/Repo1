#ifndef ADC_H_
#define ADC_H_
#include "stm32f0xx.h"                  // Device header

#define ADC_INTERRUPT

void ADC_Init(void);
uint16_t* ADC1_Result_DMA(void);
void ADC_Polling(uint16_t ms);

#endif /* ADC_H_ */
