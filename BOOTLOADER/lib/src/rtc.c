#include "rtc.h"
#include <stdio.h>
#include "string.h"

void RTC_Init(void){
	RCC->APB1ENR|=RCC_APB1ENR_PWREN;
	PWR->CR|=PWR_CR_DBP;
	SET_BIT(RCC->BDCR, RCC_BDCR_BDRST);
	CLEAR_BIT(RCC->BDCR, RCC_BDCR_BDRST);
	
	RCC->BDCR|=RCC_BDCR_LSEON;
	while (!(RCC->BDCR & RCC_BDCR_LSERDY));
	RCC->BDCR|=RCC_BDCR_RTCSEL_LSE;
	
//	RCC->CSR |= (RCC_CSR_LSION);               // For RTC enable
//	while (!(RCC->CSR & RCC_CSR_LSIRDY));
//	RCC->BDCR|=RCC_BDCR_RTCSEL_LSI;
	
	RCC->BDCR|=RCC_BDCR_RTCEN;
	RTC->WPR=0xCA;
	RTC->WPR=0x53;
	RTC->ISR |= RTC_ISR_INIT;
	while ((RTC->ISR & RTC_ISR_INITF) != RTC_ISR_INITF);
	RTC->PRER=0x007F00FF; // (0x7F+1)*(0xFF+1)=32768(kHz) îò LSE
	RTC->DR=0x0025A620; // YY-(Week<<1)-MM-DD
	RTC->TR=0x00163000; // HH-MM-SS
	RTC->ISR&=~RTC_ISR_INIT;
	RTC->WPR=0xFF;
}

char* RTC_GetTime(void){
	char str[100], str1[5];
	uint8_t Hours=((RTC->TR>>RTC_TR_HT_Pos)&0b0011)*10+((RTC->TR>>RTC_TR_HU_Pos)&0x0F);
	uint8_t Minutes=((RTC->TR>>RTC_TR_MNT_Pos)&0x0F)*10+((RTC->TR>>RTC_TR_MNU_Pos)&0x0F);
	uint8_t Seconds=((RTC->TR>>RTC_TR_ST_Pos)&0x0F)*10+((RTC->TR>>RTC_TR_SU_Pos)&0x0F);
	
	if(Hours/10==0) sprintf(str, "0%d:",  Hours); else sprintf(str, "%d:",  Hours);
	if(Minutes/10==0) sprintf(str1, "0%d:",  Minutes); else sprintf(str1, "%d:",  Minutes);
	strcat(str, str1);
	if(Seconds/10==0) sprintf(str1, "0%d",  Seconds); else sprintf(str1, "%d",  Seconds);
	strcat(str, str1);
	
	return str;
}

char* RTC_GetDate(void){
	char str[100], str1[5];
	uint8_t Year=((RTC->DR>>RTC_DR_YT_Pos)&0x0F)*10+((RTC->DR>>RTC_DR_YU_Pos)&0x0F);
	uint8_t Month=((RTC->DR>>RTC_DR_MT_Pos)&0x01)*10+((RTC->DR>>RTC_DR_MU_Pos)&0x0F);
	uint8_t WeekDay=((RTC->DR>>RTC_DR_WDU_Pos)&0b0111);
	uint8_t Date=((RTC->DR>>RTC_DR_DT_Pos)&0b0011)*10+((RTC->DR>>RTC_DR_DU_Pos)&0x0F);
	
	if(Date/10==0) sprintf(str, "0%d-",  Date); else sprintf(str, "%d-",  Date);
	if(Month/10==0) sprintf(str1, "0%d-",  Month); else sprintf(str1, "%d-",  Month);
	strcat(str, str1);
	if(Year/10==0) sprintf(str1, "0%d ",  Year); else sprintf(str1, "%d ",  Year);
	strcat(str, str1);
	switch(WeekDay){
		case 1: strcpy(str1, "Ïí"); break;
		case 2: strcpy(str1, "Âò"); break;
		case 3: strcpy(str1, "Ñð"); break;
		case 4: strcpy(str1, "×ò"); break;
		case 5: strcpy(str1, "Ïò"); break;
		case 6: strcpy(str1, "Ñá"); break;
		case 7: strcpy(str1, "Âñ"); break;
	}
	//sprintf(str1, " %d",  WeekDay);
	strcat(str, str1);
	
	return str;
}
