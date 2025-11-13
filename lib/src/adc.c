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
//	ADC1->CHSELR|=(ADC_CHSELR_CHSEL2|ADC_CHSELR_CHSEL3); // Батарея
//	ADC1->CFGR1|=(ADC_CFGR1_OVRMOD);   //continuous mode (disable DMA must)|ADC_CFGR_AUTDLY|ADC_CFGR1_SCANDIR|ADC_CFGR1_CONT
	
//	ADC1->SMPR=ADC_SMPR1_SMPR_1;
	
	
  ADC1->CR |= ADC_CR_ADCAL;
  while (ADC1->CR & ADC_CR_ADCAL);
	
	ADC1->CR|=ADC_CR_ADEN;
	while (!(ADC1->ISR&ADC_ISR_ADRDY)){};

		
//	ADC1->CFGR1|=(ADC_CFGR1_DMAEN|ADC_CFGR1_DMACFG);//
//	DMA1_Channel1->CCR=0;   // !!!
//	DMA1_Channel1->CPAR=(uint32_t)(&ADC1->DR);
//	DMA1_Channel1->CMAR=(uint32_t)(adc);
//	DMA1_Channel1->CNDTR=3;
//	DMA1_Channel1->CCR|=(DMA_CCR_MINC|DMA_CCR_MSIZE_0|DMA_CCR_PSIZE_0|DMA_CCR_CIRC);
//	DMA1_Channel1->CCR|=DMA_CCR_EN;
		
	#ifdef ADC_INTERRUPT
		ADC1->IER = ADC_IER_EOCIE;
		NVIC_EnableIRQ(ADC1_IRQn);
		NVIC_SetPriority(ADC1_IRQn, 0);
	#endif // ADC_INTERRUPT
	
//	ADC1->CR |= ADC_CR_ADSTART;
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

extern uint32_t dacc01, dacc02, dacc03, dacc04, dout01, dout02, dout03, dout04;
extern uint32_t dacc11, dacc12, dacc13, dacc14, dout11, dout12, dout13, dout14;
extern uint8_t level0_tmp, level1_tmp; 
extern uint8_t adc_timer_flag, signal_timer_flag, adc_in_flag, adc_in_battery_flag;

void ADC1_IRQHandler(void){
	switch(adc_in_flag){
		case SIGNAL_ADC:
			if(adc_timer_flag!=NO_SIG){ 
				adc[2]=ADC1->DR;

				if(adc_timer_flag==ZERO_SIG){
					dacc01 = dacc01 + (adc[2] - dout01); // ADC1->DR
					dout01 = dacc01 >> 4;
					dacc02 = dacc02 + (dout01 - dout02);
					dout02 = dacc02 >> 4;
					dacc03 = dacc03 + (dout02 - dout03);
					dout03 = dacc03 >> 4;
					dacc04 = dacc04 + (dout03 - dout04);
					dout04 = dacc04 >> 4;
					
					if (dout04 > 2056) level0_tmp = 1; // 2056
					else if (dout04 < 2040) level0_tmp = 0; //2040
				}
				
				if(adc_timer_flag==ONE_SIG){
					dacc11 = dacc11 + (adc[2] - dout11); // ADC1->DR
					dout11 = dacc11 >> 4;
					dacc12 = dacc12 + (dout11 - dout12);
					dout12 = dacc12 >> 4;
					dacc13 = dacc13 + (dout12 - dout13);
					dout13 = dacc13 >> 4;
					dacc14 = dacc14 + (dout13 - dout14);
					dout14 = dacc14 >> 4;
					
					if (dout14 > 2056) level1_tmp = 1; // 2056
					else if (dout14 < 2040) level1_tmp = 0; //2040
				}
				
				adc_timer_flag=NO_SIG;	
				ADC1->CR |= ADC_CR_ADSTP; 
			}
			else{
				ADC1->ISR|=ADC_ISR_EOC;
			}
		break;
		
		case B_OR_CH_ADC:
			switch(adc_in_battery_flag){
				case BATTERY_ADC:
					adc[0]=ADC1->DR;
				break;
				
				case CHARGE_ADC:
					adc[1]=ADC1->DR;
				break;
			}
			adc_in_flag=SIGNAL_ADC;
			ADC1->CHSELR=ADC_CHSELR_CHSEL9;
		break;
		
	}
	
}

void DMA1_Channel1_IRQHandler(void){};

	
inline void ADC_Polling(uint16_t ms){
	uint32_t adc_timer=ms*SystemCoreClock/1000;
		while (!(ADC1->ISR & ADC_ISR_EOC) && (--adc_timer));
		if(!adc_timer){
//			ADC1->CR|=ADC_CR_ADDIS;
//			while ((ADC1->CR&ADC_CR_ADEN));
//			ADC1->CR|=ADC_CR_ADEN;
//			while (!(ADC1->ISR&ADC_ISR_ADRDY));
//			ADC1->CR |= ADC_CR_ADSTART;
			ADC1->ISR|=ADC_ISR_EOC;
		}
//		else adc[2]=ADC1->DR;
}
