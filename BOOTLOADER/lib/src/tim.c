#include "string.h"
#include "tim.h"
#include "gpio.h"
#include "disp.h"
#include "data.h"

extern void delay(__IO uint32_t tck);

uint32_t long_press=0;

//-----------------------------------------------------------------------

//void TIM6_Init(void){
//	
//	RCC->APB1ENR|=RCC_APB1ENR_TIM6EN;
//	NVIC_EnableIRQ(TIM6_IRQn);
//	TIM6->PSC=SystemCoreClock/1000-1;   // Prescaler = (f(APB1) / f) - 1
//	TIM6->ARR=200-1;   // Period; Время переполнения = PSC*ARR/f(APB1)
//	TIM6->CR1=0;
//	TIM6->EGR|=TIM_EGR_UG;
//	TIM6->SR&=~TIM_SR_UIF;
//	TIM6->DIER=TIM_DIER_UIE;
//	TIM6->CR1|=TIM_CR1_CEN;
//}

//  static int div10ms = 10;

//void TIM6_IRQHandler(void){
//	TIM6->SR&=~TIM_SR_UIF;
//	
//  static uint8_t switch_flag = 0;
//	
//	if (GPIOA->IDR&(1<<12)) {
//    if (!switch_flag) switch_flag = 1;
//    offdelay = 10;
//  }
//  else if (switch_flag == 1) {
//    if (!offdelay) {
//      GPIOA->BRR|=(1<<10); // Подсветка
//      GPIOA->BRR|=(1<<15); // Отключение
//      switch_flag = 2;
//    }
//  }
//	if (offdelay) offdelay--;
////  if (--div1s) return;
////  div1s = 1000;
////  GPIOD->ODR ^= (1<<15);
//}

//-----------------------------------------------------------------------

static uint16_t offdelay = 2000;

void TIM3_Init(void){
	
	RCC->APB1ENR|=RCC_APB1ENR_TIM3EN;
	NVIC_EnableIRQ(TIM3_IRQn);
	TIM3->PSC=0;   // Prescaler = (f(APB1) / f) - 1
	TIM3->ARR=SystemCoreClock/1000-1;   // Period; Время переполнения = PSC*ARR/f(APB1)
	TIM3->CR1=0;
	TIM3->EGR|=TIM_EGR_UG;
	TIM3->SR&=~TIM_SR_UIF;
	TIM3->DIER=TIM_DIER_UIE;
	TIM3->CR1|=TIM_CR1_CEN;
}

void TIM3_IRQHandler(void){
	TIM3->SR&=~TIM_SR_UIF;
	static uint8_t switch_flag = 0;
	
	if (GPIOA->IDR&(1<<12)) {
    if (!switch_flag) switch_flag = 1;
    offdelay = 2000;
  }
  else if (switch_flag == 1) {
    if (!offdelay) {
      GPIOA->BRR|=(1<<10); // Подсветка
      GPIOA->BRR|=(1<<15); // Отключение
      switch_flag = 2;
    }
  }
	if (offdelay) offdelay--;
  if (!--offtimeout) GPIOA->BRR |= (1<<15);
  if (timer1ms) timer1ms--;
  if (cctimer) cctimer--;
	if (t_label) t_label--;
}
