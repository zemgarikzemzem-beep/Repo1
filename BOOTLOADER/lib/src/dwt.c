#include "dwt.h"

void DWT_Init(void){
//	CoreDebug->DEMCR|=CoreDebug_DEMCR_TRCENA_Msk;
//	DWT->CTRL|=DWT_CTRL_CYCCNTENA_Msk;
//	DWT->CYCCNT=0;
	SCB_DEMCR|=(1<<24);
	DWT_CONTROL|=(1<<0);
	DWT_CYCCNT=0;
}
