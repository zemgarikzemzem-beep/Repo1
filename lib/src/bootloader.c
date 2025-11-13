#include "bootloader.h"
#include "disp.h"

#define APPLICATION_ADDRESS		0x1FFFC800 // 

void GotoApp(void){
	uint32_t app_jump_address;
	
  typedef void(*pFunction)(void); //объ€вл€ем пользовательский тип
  pFunction Jump_To_Application; //и создаЄм переменную этого типа

  __disable_irq();//запрещаем прерывани€
	
//	TIM3->CR1 = 0;
//  TIM3->DIER = 0;
//  TIM3->CCR1 = 0;
//	TIM14->CR1 = 0;
//  TIM14->DIER = 0;
//  TIM14->CCR1 = 0;
//	TIM15->CR1 = 0;
//  TIM15->DIER = 0;
//  TIM15->CCR1 = 0;
//	TIM16->CR1 = 0;
//  TIM16->DIER = 0;
//  TIM16->CCR1 = 0;
//	TIM17->CR1 = 0;
//  TIM17->DIER = 0;
//  TIM17->CCR1 = 0;
//	SysTick->CTRL = 0;
//  SysTick->LOAD = 0;
//  SysTick->VAL = 0;
//	
//	
//	for (uint8_t i=0;i<3;i++){
//	  NVIC->ICER[i]=0xFFFFFFFF;
//	  NVIC->ICPR[i]=0xFFFFFFFF;
//  }
	
//	SET_BIT(RCC->CR, RCC_CR_HSION | RCC_CR_HSITRIM_4);
//	while(READ_BIT(RCC->CR, RCC_CR_HSIRDY) == RESET);
//	CLEAR_BIT(RCC->CFGR, RCC_CFGR_SW | RCC_CFGR_HPRE | RCC_CFGR_PPRE | RCC_CFGR_MCO);
//	while (READ_BIT(RCC->CFGR, RCC_CFGR_SWS) != RESET);
//	SystemCoreClock = 48000000;
//	CLEAR_BIT(RCC->CR, RCC_CR_PLLON | RCC_CR_CSSON | RCC_CR_HSEON);
//	CLEAR_BIT(RCC->CR, RCC_CR_HSEBYP);
//	while(READ_BIT(RCC->CR, RCC_CR_PLLRDY) != RESET);
//	CLEAR_REG(RCC->CFGR);
//  CLEAR_REG(RCC->CFGR2);
//  CLEAR_REG(RCC->CFGR3);
//  CLEAR_REG(RCC->CIR);
//	RCC->CSR |= RCC_CSR_RMVF;
	
	RCC->APB1RSTR = 0xFFFFFFFFU;
	RCC->APB1RSTR = 0x00000000U;
	RCC->APB2RSTR = 0xFFFFFFFFU;
	RCC->APB2RSTR = 0x00000000U;
	RCC->AHBRSTR = 0xFFFFFFFFU;
	RCC->AHBRSTR = 0x00000000U;
	
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
    // remap memory to 0 (only for STM32F0)
  SYSCFG->CFGR1 = 0x01; __DSB(); __ISB();
	
//	volatile uint32_t *VectorTable = (volatile uint32_t *)0x20000000;
//    uint32_t ui32_VectorIndex = 0;
//    for(ui32_VectorIndex = 0; ui32_VectorIndex < 48; ui32_VectorIndex++)
//    {
//        VectorTable[ui32_VectorIndex] = *(__IO uint32_t*)((uint32_t)APPLICATION_ADDRESS + (ui32_VectorIndex << 2));
//    }
//	RCC->AHBRSTR = 0xFFFFFFFFU;
//	SET_BIT(RCC->APB2ENR, RCC_APB2ENR_SYSCFGEN);
//	RCC->AHBRSTR = 0x00000000U;
//	SYSCFG->CFGR1 &= ~(SYSCFG_CFGR1_MEM_MODE);
//	SYSCFG->CFGR1 |= (SYSCFG_CFGR1_MEM_MODE_0 | SYSCFG_CFGR1_MEM_MODE_1);
//	__enable_irq();
	
//		 
//	__enable_irq();
	
//	SysTick->CTRL = 0;
////HAL_RCC_DeInit();
// 
////  оличество регистров сброса и ожидани€ прерывани€ зависит от €дра. ” ћ3 их 3
//     for (uint8_t i=0;i<3;i++)
//     {
//	  NVIC->ICER[i]=0xFFFFFFFF;
//	  NVIC->ICPR[i]=0xFFFFFFFF;
//     }	
//__enable_irq();
		
  app_jump_address = *( uint32_t*) (APPLICATION_ADDRESS+4);    // извлекаем адрес перехода из вектора Reset
  Jump_To_Application = (pFunction)app_jump_address;            //приводим его к пользовательскому типу
  __set_MSP(*(__IO uint32_t*) APPLICATION_ADDRESS);          //устанавливаем SP приложени€                                           
  Jump_To_Application();		                        //запускаем приложение	
}
