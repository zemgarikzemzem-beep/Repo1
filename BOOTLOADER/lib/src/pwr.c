#include "pwr.h"
#include "exti.h"

#define SLEEP_MODE
//#define STOP_MODE
//#define STANDBY_MODE

extern void delay(__IO uint32_t tck);

void Sleep(void){
	CLEAR_BIT(SysTick->CTRL, SysTick_CTRL_TICKINT_Msk); // Остановка прерываний SYSTICK
	
	NVIC_DisableIRQ(TIM6_IRQn); // Остановка прерываний по таймеру кнопок
	NVIC_DisableIRQ(TIM14_IRQn); // Остановка прерываний по таймеру 
	NVIC_DisableIRQ(TIM15_IRQn); // Остановка прерываний по таймеру 
	NVIC_DisableIRQ(TIM16_IRQn); // Остановка прерываний по таймеру 
	NVIC_DisableIRQ(TIM17_IRQn); // Остановка прерываний по таймеру 
	
	
	EXTI_Init();
	
	#ifdef SLEEP_MODE
	CLEAR_BIT(SCB->SCR, ((uint32_t)SCB_SCR_SLEEPDEEP_Msk));
	#endif
	
	#ifdef STOP_MODE
	SET_BIT(SCB->SCR, ((uint32_t)SCB_SCR_SLEEPDEEP_Msk));
	CLEAR_BIT(PWR->CR, PWR_CR_PDDS);
	SET_BIT(PWR->CR, PWR_CR_LPDS);
	#endif
	
	#ifdef STANDBY_MODE
	SET_BIT(SCB->SCR, ((uint32_t)SCB_SCR_SLEEPDEEP_Msk));
	SET_BIT(PWR->CR, PWR_CR_PDDS|PWR_CR_CWUF);
	SET_BIT(PWR->CSR, PWR_CSR_EWUP1); // Пробуждение: А0-вверх
	#endif
	
	__WFI();
	SET_BIT(SysTick->CTRL, SysTick_CTRL_TICKINT_Msk); // Запуск прерываний SYSTICK
	
	NVIC_DisableIRQ(EXTI4_15_IRQn);
	
	delay(1000);
	
	NVIC_EnableIRQ(TIM6_IRQn); // Запуск прерываний по таймеру кнопок
	NVIC_EnableIRQ(TIM14_IRQn); // Запуск прерываний по таймеру 
	NVIC_EnableIRQ(TIM15_IRQn); // Запуск прерываний по таймеру 
	NVIC_EnableIRQ(TIM16_IRQn); // Запуск прерываний по таймеру 
	NVIC_EnableIRQ(TIM17_IRQn); // Запуск прерываний по таймеру 
	
	TIM1->CCR3=5000; // Восстановление подсветки
	TIM3->SR&=~TIM_SR_UIF; // Новый запуск таймера сна
	TIM3->DIER|=TIM_DIER_UIE;
	TIM3->CR1|=TIM_CR1_CEN;
}
