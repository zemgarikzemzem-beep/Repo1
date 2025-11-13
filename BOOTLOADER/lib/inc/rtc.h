#ifndef RTC_H_
#define RTC_H_
#include "stm32f0xx.h"                  // Device header

void RTC_Init(void);
char* RTC_GetTime(void);
char* RTC_GetDate(void);

#endif /* SPI_H_ */
