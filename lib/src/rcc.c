#include "rcc.h"

int Clock_Init(void){
	FLASH->ACR=0x00000000;
	FLASH->ACR|=(1<<FLASH_ACR_LATENCY_Pos);
	
	RCC->CR=0x00000083;
	RCC->CR|=RCC_CR_HSEON;
	while(!(RCC->CR&(RCC_CR_HSERDY)));
		
	RCC->CFGR=0;
	RCC->CFGR|=(RCC_CFGR_PLLSRC_HSE_PREDIV|RCC_CFGR_PLLMUL3);
	
	RCC->CR|=RCC_CR_PLLON;
	while ((RCC->CR & RCC_CR_PLLRDY) == 0);
	
//	RCC->CSR |= (RCC_CSR_LSION);               // For RTC enable
//	while (!(RCC->CSR & RCC_CSR_LSIRDY));
	
	RCC->CFGR|=RCC_CFGR_SW_PLL;
	while((RCC->CFGR&RCC_CFGR_SWS_PLL) != RCC_CFGR_SWS_PLL);
	
	RCC->APB2ENR|=RCC_APB2ENR_SYSCFGEN;
	RCC->APB1ENR|=RCC_APB1ENR_PWREN;
	
//	RCC->CR&=~RCC_CR_HSION;  // Для работы с памятью - не вырубать !!!
	return 0;
}
