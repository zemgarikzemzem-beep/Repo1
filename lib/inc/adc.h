#ifndef ADC_H_
#define ADC_H_
#include "stm32f0xx.h"                  // Device header

#define ADC_INTERRUPT

enum adc_tim{
	NO_SIG,
	ZERO_SIG,
	ONE_SIG
};

enum adc_in{
	SIGNAL_ADC,
	B_OR_CH_ADC
};

enum adc_in_battery{
	BATTERY_ADC,
	CHARGE_ADC
};

void ADC_Init(void);
uint16_t* ADC1_Result_DMA(void);
void ADC_Polling(uint16_t ms);

#endif /* ADC_H_ */
