#include "stm32f0xx.h"                  // Device header
#include "string.h"
#include <stdio.h>
//#include "cmsis_armcc.h"

// Вероятно, из-за АЛЕРТА не удаётся полноценно записать ПЛА

// При перезагрузке в бутлоадер надо дополнительно включать

// Поднятие флага инициализации часов зацикливается

#include "alerts.h"
#include "rcc.h"
#include "SysTick.h"
#include "gpio.h"
#include "spi.h"
#include "i2c.h"
#include "rtc.h"
#include "tim.h"
#include "pwr.h"
#include "iwdg.h"
#include "disp.h"
#include "tft-menu.h"
#include "eeprom.h"
#include "adc.h"
#include "img.h"
#include "cc2520.h"
//#include "chipcon.h"
#include "data.h"
#include "CRC.h"
#include "flash.h"

#include "fft.h"

//#define MYADDR 0x0800F800

extern uint32_t Time_To_Sleep; // Количество переполнений таймера сна
	
uint8_t wait_to_sleep=1;    // Флаг ожидания сна(чтобы скважность ШИМ не менялась каждый раз)

extern uint8_t is_prog_mode; // Флаг режима общения с программатором

extern uint8_t autolock_flag, keys_lock_flag;

//extern uint32_t conv_res;

void delay(__IO uint32_t tck) // Задержка в мс (примерно)
{
	tck*=3000;
  while(tck)
  {
    tck--;
  }  
}



int main(void)
{
	Clock_Init(); // Инициализация тактирования
	SYSTICK_Init();
//	RTC_Init(); // Инициализация часов
	GPIO_Init(); // Инициализация портов ввода-вывода
	ADC_Init();  // Инициализация АЦП (сигнал приёма, батареи и зарядки)
	TIM1_PWM_Init(); // Инициализация таймера ШИМ для подсветки
	SPI1_Init(); // Инициализация SPI для чипкона
	SPI2_Init(); // Инициализация SPI для дисплея
	TFT_Init();  // Инициализация дисплея
	I2C1_Init(); // Инициализация I2C (для EEPROM)
	
	#ifndef ADC_INTERRUPT
		IWDG_Init(); // Инициализация сторожевого таймера
	#endif
	
	TIM3_Init(); // Инициализация таймера для сна и таймаутов
	TIM15_Init(); // Таймер частоты 1066Гц
	TIM16_Init(); // Таймер частоты 1070Гц
	TIM17_Init(); // Таймер цепочки бит
	Buttons_Init(); // Инициализация кнопок
//	TIM6_Init(); // Инициализация таймера обработки кнопок
	cc_init(); // Инициализация чипкона
	CRC_Init(); // Инициализация CRC-калькулятора
	
//	GPIOA->MODER|=(0b10<<GPIO_MODER_MODER8_Pos);
//	RCC->CFGR|=RCC_CFGR_MCOSEL_HSE;

	//FLASH_WriteByte(FLASH_SETTINGS_ADDR+REC_MESS_NUM, 0x0);

	label=FLASH_ReadByte(FLASH_SETTINGS_ADDR+PAGER_NUM_LB)+(FLASH_ReadByte(FLASH_SETTINGS_ADDR+PAGER_NUM_HB)<<8);  // Присвоение метке номера из памяти
	
	uint8_t mess_shift=FLASH_ReadByte(FLASH_SETTINGS_ADDR+REC_MESS_NUM);
	if(mess_shift==0xFF){
		mess_shift=0;
		FLASH_WriteByte(FLASH_SETTINGS_ADDR+REC_MESS_NUM, 0);
	}
	
	TFT_Fill_Color(BLACK); // Заполнение экрана чёрным цветом
	
	TFT_Draw_Image_Mono(logo_mono_arr, logo_mono_width, logo_mono_height, 4, 50, WHITE, BLACK); // Вывод логотипа
	TFT_Draw_Image_Mono(battery_mono_arr, battery_mono_width, battery_mono_height, 90, 0, WHITE, BLACK); // Вывод батарейки
	Menu_Draw(menu_main); // Отрисовка главного меню
	Menu_Item_Select(menu_main, 0); // A11 - UP, B9 - DOWN, A12 - BACK, B8 - OK
		
//	Boomer();
//	Imperial_March();
//	Ot_ulybki();
	
//	TIM14_Init(); // Инициализация таймера непрерывного вывода на дисплей (обязательно - после прорисовки статичных рисунков, иначе прерывание вклинивается и вырубает CS !!!)
	
	
	
	if (cc_flag){
			chl_load(LABEL_CHANNEL);
			send_label();													// Передача метки в эфир на втором канале
			chl_load(PROGR_CHANNEL);
	}
	
	
//	
//	double x[65]={0,};
//  double y[65]={0,};
//  uint16_t N=64;
//	
//	
//	
//	for(uint16_t i=1; i<=N; ++i){
////		x[i]=readMEM(CC2520_INS_RANDOM)&0xFF;
////		writeINS(CC2520_INS_SFLUSHRX);
//		ADC1->CR |= ADC_CR_ADSTART;
////			while (!(ADC1->ISR & ADC_ISR_EOC));
//		ADC_Polling(10);
//		x[i]=ADC1->DR;
//	}
//    
//  FFT(x, y, N);
	
	
	
	
	while(1){
		
		if(!--batt_refr_time){
			batt_refr_time=BATT_REFR_TIME;
			#ifdef ADC_INTERRUPT
				adc_in_flag=B_OR_CH_ADC;
				adc_in_battery_flag=(adc_in_battery_flag+1)&0b1;
				switch(adc_in_battery_flag){
							case BATTERY_ADC:
								ADC1->CHSELR=ADC_CHSELR_CHSEL2; // ??????????????????????????
								ADC1->CR |= ADC_CR_ADSTART;
							break;
							
							case CHARGE_ADC:
								ADC1->CHSELR=ADC_CHSELR_CHSEL3;
								ADC1->CR |= ADC_CR_ADSTART;
							break;
				}
			#endif // ADC_INTERRUPT
			show_battery_flag=1;
		}
		
		if(switch_flag_SOS){
			if(!t_label){
				chl_load(SOS_CHANNEL);
				send_SOS_label();													// Передача метки в эфир на нулевом канале с поднятым 13-м битом (SOS)
			}
			if(!t_music--){
				t_music=6000000;                         // Раз в 15 секунд звучит мелодия
				ALERT_ForSOS();
			}
		}
		
		if(is_prog_mode){
			if(!t_label){
				chl_load(LABEL_CHANNEL);
				send_label();													// Передача метки в эфир на втором канале
				chl_load(PROGR_CHANNEL);
			}
			if (cc_flag && (GPIOB->IDR & (1<<0))) program(); // Если в пункте меню "Программирование", и пришла строка - отвечаем
//			if(prg_en && !offtimeout) prg_off();  // Простой в общении более offtimeout - выключаем режим программирования
		}
		
		#ifdef SLEEP_TIMER
			if(!Time_To_Sleep && wait_to_sleep){     // Уход в спящий режим по таймеру с выключением подсветки
//				TIM3->DIER&=~TIM_DIER_UIE;
//				TIM3->CR1&=~TIM_CR1_CEN;
				if(autolock_flag) keys_lock_flag=1;
				TIM1->CCR3=0;
				wait_to_sleep=0;
				//----------Для экономии батарейки----------//
				RCC->APB2ENR&=~RCC_APB2ENR_SPI1EN;
//				RCC->APB1ENR&=~RCC_APB1ENR_TIM3EN;
//				RCC->APB2ENR&=~RCC_APB2ENR_TIM1EN;
			}
			if((!(GPIOA->IDR&(1<<11)) || !(GPIOA->IDR&(1<<12)) || !(GPIOB->IDR&(1<<8)) || !(GPIOB->IDR&(1<<9))) && !wait_to_sleep && !keys_lock_flag){
				Time_To_Sleep=MINUTES_TO_SLEEP*60*1000;
				wait_to_sleep=1;
				TIM1->CCR3=5000;
				//----------Для экономии батарейки----------//
				RCC->APB2ENR|=RCC_APB2ENR_SPI1EN;
//				RCC->APB1ENR|=RCC_APB1ENR_TIM3EN;
//				RCC->APB2ENR|=RCC_APB2ENR_TIM1EN;
			}
		#endif // SLEEP_TIMER
			
		IWDG->KR=0xAAAA;
	}
}

void HardFault_Handler(void){
	uint8_t i=1;
	while(i);
}
