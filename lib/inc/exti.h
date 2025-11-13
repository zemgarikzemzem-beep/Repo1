#ifndef EXTI_H_
#define EXTI_H_
#include "stm32f0xx.h"                  // Device header

void EXTI_Init(void);
void EXTI0_IRQHandler(void);

#endif /* EXTI_H_ */
