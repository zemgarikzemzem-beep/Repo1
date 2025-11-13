#ifndef TIM_H_
#define TIM_H_
#include "stm32f0xx.h"                  // Device header

		
#define SLEEP_TIMER

enum opcodes{
	READY_TO_RECEIVE,
	RECEIVE_BYTES,
	BYTE1_RECEIVE,
	BYTE2_RECEIVE,
	BYTE3_RECEIVE,
	WAIT_17_BIT
};

void TIM3_Init(void);
void TIM1_PWM_Init(void);
void TIM6_Init(void);
void TIM14_Init(void);
void TIM15_Init(void);
void TIM16_Init(void);
void TIM17_Init(void);

#endif /* TIM_H_ */
