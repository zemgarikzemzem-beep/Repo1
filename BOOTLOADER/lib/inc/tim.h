#ifndef TIM_H_
#define TIM_H_
#include "stm32f0xx.h"                  // Device header

enum opcodes{
	READY_TO_RECEIVE,
	RECEIVE_BYTES,
	BYTE1_RECEIVE,
	BYTE2_RECEIVE,
	BYTE3_RECEIVE,
	WAIT_13_BIT
};

void TIM3_Init(void);
void TIM6_Init(void);

#endif /* TIM_H_ */
