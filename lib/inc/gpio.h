#ifndef GPIO_H_
#define GPIO_H_
#include "stm32f0xx.h"                  // Device header

#define TogglePin(GPIOx, n) if(GPIOx->ODR&(1<<n)){GPIOx->BSRR|=(1<<(16+n));} else {GPIOx->BSRR|=(1<<n);}
#define SetPin(GPIOx, n) GPIOx->BSRR|=(1<<n);
#define ResetPin(GPIOx, n) GPIOx->BSRR|=(1<<(16+n));

#define GPIO_MODE_INPUT												0x00
#define GPIO_CNF_INPUT_ANALOG									0x00
#define GPIO_CNF_INPUT_FLOATING								0x01
#define GPIO_CNF_INPUT_PULLUP_PULLDOWN				0x10

#define GPIO_MODE_OUTPUT_LOW									0x10
#define GPIO_MODE_OUTPUT_MEDIUM								0x01
#define GPIO_MODE_OUTPUT_HIGH									0x11
#define GPIO_CNF_OUTPUT_PUSHPULL							0x00
#define GPIO_CNF_OUTPUT_OPENDRAIN             0x01
#define GPIO_CNF_ALTERNATE_PUSHPULL						0x10
#define GPIO_CNF_ALTERNATE_OPENDRAIN					0x11

//#define GPIO_CRH_CNF8_Pos                    (2U)
//#define GPIO_CRH_MODE8_Pos                   (0U)
//#define GPIO_CRH_CNF12_Pos                   (18U)
//#define GPIO_CRH_MODE12_Pos                  (16U)

void GPIO_Init(void);


#endif /* GPIO_H_ */
