#include "adc.h"
#include "gpio.h"

	uint16_t adc[3];
extern void delay(__IO uint32_t tck);

void ADC_Init(void){
	RCC->APB2ENR|=(RCC_APB2ENR_ADCEN);
	
	RCC->CR2 |= RCC_CR2_HSI14ON;
	while (!(RCC->CR2 & RCC_CR2_HSI14RDY));
	
	RCC->AHBENR|=RCC_AHBENR_DMA1EN;
	
	GPIOB->MODER|=GPIO_MODER_MODER1; // B1 - вход АЦП
	GPIOA->MODER|=(GPIO_MODER_MODER2|GPIO_MODER_MODER3);
	
	ADC1->CHSELR|=ADC_CHSELR_CHSEL9;  // Принятый сигнал
	ADC1->CHSELR|=(ADC_CHSELR_CHSEL2|ADC_CHSELR_CHSEL3); // Батарея
	ADC1->CFGR1|=(ADC_CFGR1_CONT);   //continuous mode (disable DMA must)|ADC_CFGR_AUTDLY|ADC_CFGR1_SCANDIR|ADC_CFGR1_OVRMOD
	ADC1->SMPR=ADC_SMPR1_SMPR_0;
	
//	ADC1->IER = ADC_IER_EOSEQIE;
//	NVIC_EnableIRQ(ADC1_IRQn);
	
//	ADC1->IER = ADC_IER_OVRIE;
	
  ADC1->CR |= ADC_CR_ADCAL;
  while (ADC1->CR & ADC_CR_ADCAL);
	
	ADC1->CR|=ADC_CR_ADEN;
	while (!(ADC1->ISR&ADC_ISR_ADRDY)){};
		
//	ADC1->CR|=ADC_CR_ADSTART;
//	delay(5000);
	ADC1->CFGR1|=(ADC_CFGR1_DMAEN|ADC_CFGR1_DMACFG);//
//	while(!(ADC1->ISR&ADC_ISR_EOS));
	DMA1_Channel1->CCR=0;   // !!!
	DMA1_Channel1->CPAR=(uint32_t)(&ADC1->DR);
	DMA1_Channel1->CMAR=(uint32_t)(adc);
	DMA1_Channel1->CNDTR=3;
	DMA1_Channel1->CCR|=(DMA_CCR_MINC|DMA_CCR_MSIZE_0|DMA_CCR_PSIZE_0|DMA_CCR_CIRC);
	DMA1_Channel1->CCR|=DMA_CCR_EN;
	ADC1->CR |= ADC_CR_ADSTART;
		
//	DMA1_CSELR->CSELR|=(0b0000<<DMA_CSELR_C1S_Pos);
	
	
	//ADC1->IER|=ADC_IER_EOC;
	//NVIC_EnableIRQ(ADC1_IRQn);
}


uint16_t* ADC1_Result_DMA(void){
	ADC1->CR |= ADC_CR_ADSTART;
	while(!(ADC1->ISR&ADC_ISR_EOS));
	DMA1_Channel1->CCR=0;   // !!!
	DMA1_Channel1->CPAR=(uint32_t)(&ADC1->DR);
	DMA1_Channel1->CMAR=(uint32_t)(adc);
	DMA1_Channel1->CNDTR=3;
	DMA1_Channel1->CCR|=(DMA_CCR_MINC|DMA_CCR_MSIZE_0|DMA_CCR_PSIZE_0);
	DMA1_Channel1->CCR|=DMA_CCR_EN;
//	ADC1->CR |= ADC_CR_ADSTP;
	return adc;
}



void ADC1_IRQHandler(void){
	ADC1->ISR|=ADC_ISR_ADRDY;
	
	ADC1->CR |= ADC_CR_ADSTART;
	while (!(ADC1->ISR & ADC_ISR_EOC));
	adc[0]=ADC1->DR;
	while (!(ADC1->ISR & ADC_ISR_EOC));
	adc[1]=ADC1->DR;
	while (!(ADC1->ISR & ADC_ISR_EOC));
	adc[2]=ADC1->DR;
}
	//ADC1->IER|=ADC_IER_ADRDYIE;

void DMA1_Channel1_IRQHandler(void){};
