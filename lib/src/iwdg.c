#include "iwdg.h"

void IWDG_Init(void){
	IWDG->KR=0xCCCC;
	IWDG->KR=0x5555;
	IWDG->PR=0b110;
	IWDG->RLR=2300; // T = (IWDG(presc)*counter_reload/F(rtc))
	while(!(IWDG->SR&IWDG_SR_PVU)&&!(IWDG->SR&IWDG_SR_RVU));
	IWDG->KR=0xAAAA;
}

void IWDG_Refresh(void){
	IWDG->KR=0xAAAA;
}
