#include "alerts.h"
#include "data.h"



void play_note(uint32_t freq, uint16_t ms){
	uint32_t t;
	for(uint16_t i=0; i<ms*freq/1760; ++i){
		GPIOA->BSRR|=(1<<0);
		t=2351102/freq;
		while(t--);
		GPIOA->BRR|=(1<<0);
		t=2351102/freq;
		while(t--);
	}
}



void Boomer(void){
	play_note(440*2, 250);
	play_note(523*2, 1000);
	delay(750);
	play_note(523*2, 250);
	play_note(440*2, 1000);
	delay(750);
	play_note(587*2, 250);
	play_note(523*2, 250);
	play_note(587*2, 250);
	play_note(523*2, 250);
	play_note(587*2, 250);
	play_note(523*2, 250);
	play_note(587*2, 250);
	play_note(523*2, 250);
	play_note(587*2, 250);
	play_note(659*2, 1000);
}

void Imperial_March(void){
	play_note(784, 750);
	delay(250);
	play_note(784, 750);
	delay(250);
	play_note(784, 750);
	delay(250);
	play_note(622, 750);
	play_note(932, 250);
	play_note(784, 1000);
	play_note(622, 750);
	play_note(932, 250);
	play_note(784, 2000);
	
	play_note(1175, 750);
	delay(250);
	play_note(1175, 750);
	delay(250);
	play_note(1175, 750);
	delay(250);
	play_note(1244, 750);
	play_note(932, 250);
	play_note(740, 1000);
	play_note(622, 750);
	play_note(932, 250);
	play_note(784, 2000);
}

void Ot_ulybki(void){
	play_note(784, 250);
	play_note(659, 250);
	play_note(880, 500);
	play_note(784, 500);
	play_note(587, 250);
	play_note(698, 250);
	play_note(659, 250);
	play_note(587, 250);
	play_note(523, 1000);
}


inline void ALERT_ForMessage(void){
	if(FLASH_ReadByte(FLASH_SETTINGS_ADDR+ALERT)&(1<<ZOOMER)){
		GPIOA->BSRR|=(1<<9);
		delay(500);
		GPIOA->BRR|=(1<<9);
//		delay(500);
//		GPIOA->BSRR|=(1<<9);
//		delay(500);
//		GPIOA->BRR|=(1<<9);
//		delay(500);
//		GPIOA->BSRR|=(1<<9);
//		delay(500);
//		GPIOA->BRR|=(1<<9);
	}
	if(FLASH_ReadByte(FLASH_SETTINGS_ADDR+ALERT)&(1<<LIGHT_UP_SCREEN)){
		Time_To_Sleep=MINUTES_TO_SLEEP*60*1000;
		wait_to_sleep=1;
		TIM1->CCR3=5000;
	}
	if(FLASH_ReadByte(FLASH_SETTINGS_ADDR+ALERT)&(1<<SOUND)) Ot_ulybki();
}

inline void ALERT_ForNotification(void){
	if(FLASH_ReadByte(FLASH_SETTINGS_ADDR+ALERT)&(1<<ZOOMER)){
		GPIOA->BSRR|=(1<<9);
		delay(100);
		GPIOA->BRR|=(1<<9);
	}
	if(FLASH_ReadByte(FLASH_SETTINGS_ADDR+ALERT)&(1<<LIGHT_UP_SCREEN)){
		Time_To_Sleep=MINUTES_TO_SLEEP*60*1000;
		wait_to_sleep=1;
		TIM1->CCR3=5000;
	}
	if(FLASH_ReadByte(FLASH_SETTINGS_ADDR+ALERT)&(1<<SOUND)) play_note(880, 2000);
}

inline void ALERT_ForSOS(void){
	Boomer();
	GPIOA->BSRR|=(1<<9);
	delay(100);
	GPIOA->BRR|=(1<<9);
	delay(100);
	GPIOA->BSRR|=(1<<9);
	delay(100);
	GPIOA->BRR|=(1<<9);
	delay(100);
	GPIOA->BSRR|=(1<<9);
	delay(100);
	GPIOA->BRR|=(1<<9);
	delay(100);
	GPIOA->BSRR|=(1<<9);
	delay(100);
	GPIOA->BRR|=(1<<9);
	delay(100);
	GPIOA->BSRR|=(1<<9);
	delay(100);
	GPIOA->BRR|=(1<<9);
}

inline void ALERT_ForClick(void){
	if(FLASH_ReadByte(FLASH_SETTINGS_ADDR+ALERT)&(1<<ZOOMER)){
		GPIOA->BSRR|=(1<<9);
		delay(10);
		GPIOA->BRR|=(1<<9);
	}
	if(FLASH_ReadByte(FLASH_SETTINGS_ADDR+ALERT)&(1<<SOUND)) play_note(220, 100);
}
