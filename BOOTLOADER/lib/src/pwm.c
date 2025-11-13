#include "pwm.h"
#include "tim.h"

void PWM_Init(void){
	TIM1->CCMR2|=((0b110<<TIM_CCMR2_OC3M_Pos)|TIM_CCMR2_OC3PE);
	TIM1->CCR3=0;
	GPIOA->MODER|=(0b10<<GPIO_MODER_MODER10_Pos);
}
