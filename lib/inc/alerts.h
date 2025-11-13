#ifndef ALERTS_H_
#define ALERTS_H_
#include "stm32f0xx.h"                  // Device header
#include "flash.h"
#include "tim.h"

extern void delay(__IO uint32_t tck);
extern uint32_t Time_To_Sleep;
extern uint8_t wait_to_sleep;
													
void ALERT_ForMessage(void);
void ALERT_ForNotification(void);
void ALERT_ForClick(void);
void ALERT_ForSOS(void);
void Boomer(void);
void Imperial_March(void);
void Ot_ulybki(void);

#endif /* ALERTS_H_ */
