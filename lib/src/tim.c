#include "string.h"
#include "tim.h"
#include "gpio.h"
#include "tft-menu.h"
#include "disp.h"
#include "eeprom.h"
#include "rtc.h"
#include "adc.h"
#include "bootloader.h"
#include "cc2520.h"
#include "data.h"
#include "flash.h"
#include "alerts.h"

#include "img.h"

#define NO_SIGNAL 2

void KeyPanel(void);
void TimeShow(void);
void BatteryShow(void);
void SignalAmplitude(void);

extern void delay(__IO uint32_t tck);

extern volatile uint16_t t_label;

extern uint8_t Button_Selected;
extern uint8_t Message_Selected;
extern uint8_t In_Mess;
extern uint8_t Sel_Butt_Pos;
struct Menu_Item* Current_Menu=menu_main;

char str_temp[20]={0,};

extern uint16_t adc[3];

uint8_t signal=NO_SIGNAL; // Принятый бит

uint8_t is_prog_mode=0, is_signal_power_mode=0;

uint32_t accident_show_time=80000;

uint16_t sign_max=0, sign_min=4095;
uint16_t refresh_time=4;

//uint32_t long_press=0;

//char mess_full[MESS_MAX_SIZE]={0,};

uint32_t Time_To_Sleep;

uint8_t Items_Num=0;

uint8_t receive_bit_show_flag=0,
				message_received=0,
				receive_start_flag=0,
				message_menu_flag=0,
				is_menu=1;

		static uint8_t offdelay = 10, SOS_delay=10, lock_delay=10;
		static uint8_t switch_flag = 0, two_keys_flag=0;
		uint8_t keys_lock_flag=0, autolock_flag=0;
		
		uint32_t messages_addr=0;

uint8_t batt_procent;
uint8_t lightning_on=1, lightning_off=1;




//--------------------------------------------------------------------------------------
//          ТАЙМЕР ТАЙМАУТОВ, НЕПРЕРЫВНОГО ВЫВОДА И ОТСЛЕЖИВАНИЯ НАЖАТИЙ КНОПОК
//--------------------------------------------------------------------------------------

void TIM3_Init(void){
	RCC->APB1ENR|=RCC_APB1ENR_TIM3EN;
	NVIC_EnableIRQ(TIM3_IRQn);
	NVIC_SetPriority(TIM3_IRQn, 1);
	TIM3->PSC=0;   // Prescaler = (f(APB1) / f) - 1
	TIM3->ARR=SystemCoreClock/1000-1;   // Period; Время переполнения = PSC*ARR/f(APB1)
	TIM3->CR1=0;
	TIM3->EGR|=TIM_EGR_UG;
	TIM3->SR&=~TIM_SR_UIF;
	TIM3->DIER=TIM_DIER_UIE;
	TIM3->CR1|=TIM_CR1_CEN;
	
	Time_To_Sleep=MINUTES_TO_SLEEP*60*1000;
}

uint16_t ms_count=0;

void TIM3_IRQHandler(void){
	TIM3->SR&=~TIM_SR_UIF;
	
//======Таймауты=====//
	if(Time_To_Sleep) --Time_To_Sleep;
//	if(offtimeout) --offtimeout;
	if(accident_show_time) --accident_show_time;
	else TIM17->CR1|=TIM_CR1_CEN;
	if(t_label) --t_label;
	if (timer1ms) timer1ms--;
	
	
//======Вывод часов=====//
	if(!--timeshow_refr_time){
		timeshow_refr_time=TIMESHOW_REFR_TIME;
		TimeShow();
	}
	
	
//======Вывод батареи======//
	if(show_battery_flag){
		BatteryShow();
	}

	
//=========Вывод принятого бита=========//
	if(receive_bit_show_flag) { ShowSignal(); message_received=1; receive_bit_show_flag=0;}
	
	
//=========Вывод амплитуды сигнала=========//
	if(is_signal_power_mode){
		SignalAmplitude();
	}
	
//======Отслеживание нажатий клавиатуры======//
	if(!--keypressed_refr_time){
		keypressed_refr_time=KEYPRESSED_REFR_TIME;
		KeyPanel();
	}
}



//--------------------------------------------------------
//          			ТАЙМЕР ШИМ ПОДСВЕТКИ
//--------------------------------------------------------


void TIM1_PWM_Init(void){
	RCC->APB2ENR|=RCC_APB2ENR_TIM1EN;
	TIM1->PSC=0;   // Prescaler = (f(APB1) / f) - 1
	TIM1->ARR=5000;   //65535 Period
	TIM1->CR1=0;
	TIM1->EGR|=TIM_EGR_UG;
	
	TIM1->CCMR2|=((0b110<<TIM_CCMR2_OC3M_Pos)|TIM_CCMR2_OC3PE);
	TIM1->CCR3=5000;	//5000 Заполнение
	GPIOA->MODER|=(0b10<<GPIO_MODER_MODER10_Pos);
	GPIOA->AFR[1]|=(0b0010<<GPIO_AFRH_AFSEL10_Pos);
	TIM1->CCER|=TIM_CCER_CC3E;
	TIM1->BDTR |= TIM_BDTR_MOE;
	TIM1->CR1|=TIM_CR1_CEN;
}



//-----------------------------------------------------------------------
//               ТАЙМЕРЫ ОБРАБОТКИ ВХОДЯЩЕГО СИГНАЛА
//-----------------------------------------------------------------------


#define HETER_ALG

#define HETERODINE_FREQ 1065
#ifdef HETER_ALG
	#define FREQ_0					1065		// /2
	#define FREQ_1					1071		// /2
#else
	#define FREQ_0					1066		// /2
	#define FREQ_1					1070		// /2
#endif
#define BIT_PERIOD			1

uint16_t lev_count0=0,
				lev_summ0=0,
				lev_count1=0,
				lev_summ1=0;
uint8_t rec_buf[18];
uint8_t rec_buf_count=0;
uint8_t sign0_acc=0,
				sign1_acc=0;

	// фильтровые ячейки
volatile uint32_t dacc01, dacc02, dacc03, dacc04, dout01, dout02, dout03, dout04;
volatile uint32_t dacc11, dacc12, dacc13, dacc14, dout11, dout12, dout13, dout14;
volatile uint8_t flen0, flen01, flen1, flen11;
volatile uint8_t level0_tmp, level1_tmp; 
volatile uint8_t phase0, phase1;
volatile uint8_t bit, last_bit=0;
volatile uint8_t level0, level1;		// уровни огибающей ПЧ
volatile uint8_t div10ms = 10;
volatile uint16_t div512ms = 766;

// приемный буфер
extern uint16_t rbuf;
volatile uint8_t rctr, bctr;			// счетчик принимаемых бит и бит для проверки шаблона вызова маяка


volatile uint8_t adc_timer_flag=NO_SIG;   // Флаг занятости для варианта с прерыванием АЦП
volatile uint8_t signal_timer_flag=1;   // Флаг занятости для варианта без прерываний АЦП



	volatile uint16_t err0=0, err1=0;
	volatile uint16_t count0=0, count1=0;
	volatile uint16_t adc_buf0[3]={0,};
	volatile uint16_t adc_buf1[3]={0,};
	volatile uint16_t pred0=2048, pred1=2048;
	const uint16_t dv=100;



	uint32_t receive_word=0;
	uint16_t	byte1=0,
						byte2=0,
						byte3=0;
	uint8_t opcode=READY_TO_RECEIVE;
	uint8_t receive_count=0;

	uint32_t recw1=0, recw2=0;
	
	static inline uint16_t median3(uint16_t a, uint16_t b, uint16_t c) {
    return (a < b) ? ((b < c) ? b : ((c < a) ? a : c)) : ((a < c) ? a : ((c < b) ? b : c));
	}


/*============Таймер нулей==============*/

void TIM15_Init(void){
	
	RCC->APB2ENR|=RCC_APB2ENR_TIM15EN;
	NVIC_EnableIRQ(TIM15_IRQn);
	NVIC_SetPriority(TIM15_IRQn, 1);
	TIM15->PSC=0;   // Prescaler = (f(APB1) / f) - 1
	TIM15->ARR=SystemCoreClock/FREQ_0-1;   // Period; Время переполнения = PSC*ARR/f(APB1)
	TIM15->CR1=0;
	TIM15->EGR|=TIM_EGR_UG;
	TIM15->SR&=~TIM_SR_UIF;
	TIM15->DIER=TIM_DIER_UIE;
	TIM15->CR1|=TIM_CR1_CEN;
}

void TIM15_IRQHandler(void){
	TIM15->SR&=~TIM_SR_UIF;
	
	
//======Фильтр на частоте 1066Гц=====//
	
	#ifdef ADC_INTERRUPT
			if(adc_timer_flag==NO_SIG && adc_in_flag==SIGNAL_ADC){
				adc_timer_flag=ZERO_SIG;
				ADC1->CR |= ADC_CR_ADSTART;
			}
		#else
		if(signal_timer_flag){
			signal_timer_flag=0;
			ADC1->CR |= ADC_CR_ADSTART;
//			while (!(ADC1->ISR & ADC_ISR_EOC));
			ADC_Polling(10);
			adc[2]=ADC1->DR;
	//		ADC1->CR |= ADC_CR_ADSTP;
			signal_timer_flag=1;
		}
		
//	int sum=dout04*31 + adc[2] + err0;
//	dout04 = sum >> 5; // ADC1->DR - dout01					filt = (A * filt + B * signal) >> k;
//	err0=sum&0x1F;
		
		
		dacc01 = dacc01 + (adc[2] - dout01); // ADC1->DR
		dout01 = dacc01 >> 4;
		dacc02 = dacc02 + (dout01 - dout02);
		dout02 = dacc02 >> 4;
		dacc03 = dacc03 + (dout02 - dout03);
		dout03 = dacc03 >> 4;
		dacc04 = dacc04 + (dout03 - dout04);
		dout04 = dacc04 >> 4;
		
		if (dout04 > 2056) level0_tmp = 1; // 2056
		else if (dout04 < 2040) level0_tmp = 0; //2040
	#endif // ADC_INTERRUPT
	
	#ifndef HETER_ALG
//======Счётчик на частоте 1066Гц=====//

//	lev_summ0+=level0_tmp;
//	++lev_count0;
//	
//	if(lev_count0>=140){  // FREQ_0/4
//		if(lev_summ0%lev_count0==0) ++sign0_acc; // signal=0
//		lev_summ0=0;
//		lev_count0=0;
//	}
	#endif // HETER_ALG
}

//--------------------------------

/*============Таймер единичек==============*/

void TIM16_Init(void){
	
	RCC->APB2ENR|=RCC_APB2ENR_TIM16EN;
	NVIC_EnableIRQ(TIM16_IRQn);
	NVIC_SetPriority(TIM16_IRQn, 1);
	TIM16->PSC=0;   // Prescaler = (f(APB1) / f) - 1
	TIM16->ARR=SystemCoreClock/FREQ_1-1;   // Period; Время переполнения = PSC*ARR/f(APB1)
	TIM16->CR1=0;
	TIM16->EGR|=TIM_EGR_UG;
	TIM16->SR&=~TIM_SR_UIF;
	TIM16->DIER=TIM_DIER_UIE;
	TIM16->CR1|=TIM_CR1_CEN;
}

void TIM16_IRQHandler(void){
	TIM16->SR&=~TIM_SR_UIF;
	
//======Фильтр на частоте 1070Гц=====//
	
	#ifdef ADC_INTERRUPT
			if(adc_timer_flag==NO_SIG && adc_in_flag==SIGNAL_ADC){
				adc_timer_flag=ONE_SIG;
				ADC1->CR |= ADC_CR_ADSTART;
			}
	#else
		if(signal_timer_flag){
			signal_timer_flag=0;
			ADC1->CR |= ADC_CR_ADSTART;
//			while (!(ADC1->ISR & ADC_ISR_EOC));
			ADC_Polling(10);
			adc[2]=ADC1->DR;
	//		ADC1->CR |= ADC_CR_ADSTP;
			signal_timer_flag=1;
		}
		
//		int sum=dout14*31 + adc[2] + err1;
//		dout14 = sum >> 5; // ADC1->DR - dout01					filt = (A * filt + B * signal) >> k;
//		err1=sum&0x1F;
		
		dacc11 = dacc11 + (adc[2] - dout11); // ADC1->DR
		dout11 = dacc11 >> 4;
		dacc12 = dacc12 + (dout11 - dout12);
		dout12 = dacc12 >> 4;
		dacc13 = dacc13 + (dout12 - dout13);
		dout13 = dacc13 >> 4;
		dacc14 = dacc14 + (dout13 - dout14);
		dout14 = dacc14 >> 4;
		
		if (dout14 > 2056) level1_tmp = 1; // 2056
		else if (dout14 < 2040) level1_tmp = 0; //2040
	#endif // ADC_INTERRUPT
	
	#ifndef HETER_ALG
	
//======Счётчик на частоте 1070Гц=====//
	
//	lev_summ1+=level1_tmp;
//	++lev_count1;
//	
//	if(lev_count1>=140){ // FREQ_1/4
//			if(lev_summ1%lev_count1==0) ++sign1_acc; // signal=1
//			lev_summ1=0;
//			lev_count1=0;
//	}
	#endif // HETER_ALG
	
}

//--------------------------------------

/*============Таймер обработки очереди принятых бит==============*/

void TIM17_Init(void){
	
	RCC->APB2ENR|=RCC_APB2ENR_TIM17EN;
	NVIC_EnableIRQ(TIM17_IRQn);
	NVIC_SetPriority(TIM17_IRQn, 1);
	TIM17->PSC=0;   // Prescaler = (f(APB1) / f) - 1
	TIM17->ARR=SystemCoreClock/1000-1;   // Period; Время переполнения = PSC*ARR/f(APB1)
	TIM17->CR1=0;
	TIM17->EGR|=TIM_EGR_UG;
	TIM17->SR&=~TIM_SR_UIF;
	TIM17->DIER=TIM_DIER_UIE;
	TIM17->CR1|=TIM_CR1_CEN;
}

void TIM17_IRQHandler(void){
	
	TIM17->SR&=~TIM_SR_UIF;
	
	static uint8_t div1ms = 16;
	static uint8_t plevel0, plevel1;
	static uint8_t bit0, bit1, pbit, bit_tmp, bit_vote = 128;
	static int bitrate = 512;
	static uint8_t lvl0ctr = 10, lvl1ctr = 10;
	
	#ifdef HETER_ALG
	
	if (level0) {
		if (level0_tmp) lvl0ctr = 20;
		else if (!--lvl0ctr) {
			level0 = 0;
			lvl0ctr = 20;
		}
	}
	else if (!level0_tmp) lvl0ctr = 20;
	else if (!--lvl0ctr) {
		level0 = 1;
		lvl0ctr = 20;
	}

	if (level1) {
		if (level1_tmp) lvl1ctr = 20;
		else if (!--lvl1ctr) {
			level1 = 0;
			lvl1ctr = 20;
		}
	}
	else if (!level1_tmp) lvl1ctr = 20;
	else if (!--lvl1ctr) {
		level1 = 1;
		lvl1ctr = 20;
	}

	// опреденение частот по ПЧ 1Гц

	if (flen0 < 150) if (++flen0 == 150) bit0 = 0;
	if (flen1 < 150) if (++flen1 == 150) bit1 = 1;

// определение частот по ПЧ 5Гц

	if (level0 != plevel0) {
		plevel0 = level0;
		flen01 = flen0;
		flen0 = 0;

		if (flen01 < 70) bit0 = 0xff;
		if ((flen01 > 70) && (flen01 < 150)) bit0 = 1;
	}

	if (level1 != plevel1) {
		plevel1 = level1;
		flen11 = flen1;
		flen1 = 0;

		if (flen11 < 80) bit1 = 0xff;
		if ((flen11 > 80) && (flen11 < 150)) bit1 = 0;
	}

	if (bit0 == 0xff) {
		if (bit1 != 0xff) bit_tmp = bit1;
	}
	else if ((bit0 == bit1) || (bit1 == 0xff)) bit_tmp = bit0;

	if (bit_tmp) bit_vote++;
	else bit_vote--;

	if (bit_vote > 128) bit = 1;
	if (bit_vote < 128) bit = 0;
	bit_vote = 128;
	
	#else
	
	lev_summ0+=level0_tmp;
	++lev_count0;
	
	if(lev_count0>=250){  // FREQ_0/4
		if(lev_summ0%lev_count0==0) signal=0; // ++sign0_acc
		bit=signal;
		lev_summ0=0;
		lev_count0=0;
	}
	
	lev_summ1+=level1_tmp;
	++lev_count1;
	
	if(lev_count1>=250){ // FREQ_1/4
		if(lev_summ1%lev_count1==0) signal=1; // ++sign1_acc
		bit=signal;
		lev_summ1=0;
		lev_count1=0;
	}
	
//	if(sign0_acc || sign1_acc){ // 
//		if(sign0_acc>sign1_acc) {signal=0; } // 
//		if(sign0_acc<sign1_acc) {signal=1; }
//		bit=signal;
//		
//		sign0_acc=0;
//		sign1_acc=0;
//	}
		
	
	#endif // HETER_ALG
	
	if (bit != pbit) {		// проверка фронта бита
		pbit = bit;
		bitrate = 256;
	}
	else if (!(--bitrate)) {
		
		bitrate = 512;
		signal=bit;

		switch(opcode){
			case READY_TO_RECEIVE:
				receive_word=(receive_word<<1)+signal;
			
				if((receive_word&0xFF)==0xAA) {
					batt_refr_time=40000;
				}
			
				if((receive_word&0x3FFFF)==0x2AAA9){
					opcode=BYTE1_RECEIVE; // (receive_word&0xFF)==0xA9
					TFT_Send_Str(100, 16, "!", 1, Font_11x18, WHITE, BLACK);
				}
			break;
				
			case BYTE1_RECEIVE:
				recw1=(recw1<<1)+signal;
				++receive_count;
				if(receive_count==32){
					opcode=BYTE2_RECEIVE;
					receive_count=0;
				}
			break;
				
			case BYTE2_RECEIVE:
				recw2=(recw2<<1)+signal;
				++receive_count;
				if(receive_count==11){
					opcode=READY_TO_RECEIVE;
					receive_count=0;
					receive_bit_show_flag=1;
					dout04=0;
					dout14=0;
					err0=0;
					err1=0;
		//			accident_show_time=80000;
				}
			break;
		}
	}
}



//-----------------------------------------------------------------------
//               ВСПОМОГАТЕЛЬНЫЕ ПРОЦЕДУРЫ
//-----------------------------------------------------------------------

inline void KeyPanel(void){
	Items_Num=Current_Menu[0].value/10;  // Количество пунктов текущего меню
	if(message_menu_flag){
		Message_Menu_Navigation(messages_addr);
	}
	else{
		if(DOWN_KEY && !two_keys_flag && !keys_lock_flag){
			ALERT_ForClick();
			if(is_menu) Menu_Down();                // Переход на пункт меню вниз
		}
		
		if(UP_KEY && !two_keys_flag){
			if(!keys_lock_flag){
				ALERT_ForClick();
				if(is_menu) Menu_Up();                 // Переход на пункт меню вверх
			}
			if (!lock_delay--) {
				keys_lock_flag=0;
				TFT_Send_Str(MENU_CON_X, MENU_CON_Y+18, "БЛОК", 4, Font_11x18, WHITE, BLACK);
			}
		}
		else lock_delay=10;
		
		if(ENTER_KEY && !two_keys_flag && !keys_lock_flag){
			ALERT_ForClick();
			
			//======Переход в подменю======//
			if(Current_Menu[Button_Selected].submenu != 0){
				Current_Menu=Current_Menu[Button_Selected].submenu;
				if(Current_Menu!=0){
					Menu_Draw(Current_Menu);
					Menu_Item_Select(Current_Menu, 0);
				}
			}
			
			//======Главное меню======//
			else if(Current_Menu==menu_main){
				switch(Button_Selected){
					case BUTTONS_LOCK:
						TFT_Send_Str(MENU_CON_X, MENU_CON_Y+18, "БЛОК", 4, Font_11x18, BLUE, RED);
						keys_lock_flag=1;
						break;
					case MESS_ARCHIVE:
						messages_addr=FLASH_REC_MESS_ADDR;
						Mess_Menu_Draw(messages_addr);
						message_menu_flag=1;
						break;
					case MESSAGES:
						messages_addr=FLASH_PLA_ADDR;
						Mess_Menu_Draw(messages_addr);
						message_menu_flag=1;
						break;
					case PROGRAMMING:
//						cc_init();
						is_menu=0;
						_CLEAR_MENU_SCREEN;
						TFT_Send_Str(0, 120, "Ожидание...", 11, Font_11x18, MAGENTA, BLACK);
//						label_on=1;
						is_prog_mode=1;
						break;
					case SIGNAL_POWER:
						is_menu=0;
						_CLEAR_MENU_SCREEN;
						sign_max=0;
						sign_min=4095;
						is_signal_power_mode=1;
						break;
				}
			}
			
			//======Меню действий с сообщением======//
			else if(Current_Menu==menu_mess){
				switch(Button_Selected){
					case READ_MESS:
						Message_Read(FLASH_PLA_ADDR);
						break;
					case INFO_MESS:
						break;
				}
			}
			
			//======Меню инверсии======//
			else if(Current_Menu==menu_inversion){
				if(Button_Selected==0){TFT_CSEN; TFT_Send_Command(ST7735_INVON); TFT_CSDIS;}
				else {TFT_CSEN; TFT_Send_Command(ST7735_INVOFF); TFT_CSDIS;}
			}
			
			//======Меню настройки яркости======//
			else if(Current_Menu==menu_brightness){
				if(Button_Selected==0) { if(TIM1->CCR3<5000) TIM1->CCR3+=1000; else TIM1->CCR3=1000;}
				else { if(TIM1->CCR3>1000) TIM1->CCR3-=1000; else TIM1->CCR3=5000;}
			}
			
			//======Меню настройки времени======//
			else if(Current_Menu==menu_time){
				switch(Button_Selected){
					case HOURS:
						Hour_Plus();
						break;
					case MINUTES:
						Min_Plus();
						break;
					case DAY:
						Day_Plus();
						break;
					case MONTH:
						Month_Plus();
						break;
					case YEAR:
						Year_Plus();
						break;
				}
			}
			
			else if(Current_Menu==menu_autolock){
				switch(Button_Selected){
					case AUTOLOCK_ON:
						autolock_flag=1;
						TFT_Send_Str(0, 0, "АБл", 3, Font_7x9, GREEN, BLACK);
						break;
					case AUTOLOCK_OFF:
						autolock_flag=0;
						TFT_Fill_Area(0, 0, 22, 9, BLACK);
						break;
				}
			}
			
			else if(Current_Menu==menu_autosleep_time){
				switch(Button_Selected){
					case SLEEP_TIME_PLUS:
						if(Button_Selected==SLEEP_TIME_PLUS){
							//is_menu=0;
							char locktime_str[2];
							if(++MINUTES_TO_SLEEP>10) MINUTES_TO_SLEEP=1;
							sprintf(locktime_str, "%2d мин.", MINUTES_TO_SLEEP);
							TFT_Send_Str(MENU_CON_X, MENU_CON_Y+18, locktime_str, strlen(locktime_str), Font_11x18, CYAN, BLACK);
							Time_To_Sleep=MINUTES_TO_SLEEP*60*1000;
						}
						break;
				}
			}
			
		}
//---------
		
		if(BACK_KEY && !two_keys_flag && !keys_lock_flag){
			
			ALERT_ForClick();
			
			//======Выключение с долгого нажатия кнопки======//
			if (!offdelay--) {
					__disable_irq();//запрещаем прерывания
					TFT_Fill_Color(BLACK);
//					TIM1->CCR3=0; // Подсветка
					FLASH_WriteByte(FLASH_SDFB_ADDR, 1);	// Чтобы перезагрузка не вырубала пейджер
					GPIOA->BRR|=(1<<15); // Отключение
				}
			
			//======Выход в подменю (если есть)======//
			if(Current_Menu[Button_Selected].premenu!=0){
				Current_Menu=Current_Menu[Button_Selected].premenu;   // Определение наличия предменю
				Menu_Draw(Current_Menu);
				Menu_Item_Select(Current_Menu, 0);
				Sel_Butt_Pos=0;
			}
			
			//======Выход из пунктов ПРОГРАММИРОВАНИЕ и МОЩНОСТЬ в главное меню======//
			if(Current_Menu==menu_main && (Button_Selected==PROGRAMMING || Button_Selected==SIGNAL_POWER)){
//				TIM15->CR1&=~TIM_CR1_CEN;
//				TIM16->CR1&=~TIM_CR1_CEN;
//				TIM17->CR1&=~TIM_CR1_CEN;
				Menu_Draw(Current_Menu);
				Menu_Item_Select(Current_Menu, 0);
//				receive_bit_show_flag=0;
				is_menu=1;
				is_prog_mode=0;
				is_signal_power_mode=0;
			}
			
			//======Выход из демонстрации аварии======//
			if(message_received){
//				receive_bit_show_flag=0;
				is_menu=1;
				TIM17->CR1&=~TIM_CR1_CEN;
				Menu_Draw(Current_Menu);
				Menu_Item_Select(Current_Menu, 0);
			}
			
			//======Выход из меню действий с сообщением в список сообщений======//
			if(Current_Menu==menu_mess){
				Message_Current_Str=0;
				_CLEAR_MENU_SCREEN;
				for(uint8_t i=0; i<MAX_MESS_ITEM_SCREEN; ++i){
					FLASH_ReadStr(FLASH_PLA_ADDR+MESS_MAX_SIZE*((Message_Selected-mess_menu_sel_pos+i)%MESS_NUM), (uint8_t*)mess, MESS_MAX_ITEM_LENGTH);
					TFT_Send_Str(MENU_CON_X, MENU_CON_Y+i*18, mess, MESS_MAX_ITEM_LENGTH, Font_11x18, (mess_menu_sel_pos!=i)?WHITE:BLACK, (mess_menu_sel_pos!=i)?BLACK:WHITE);
				}
				message_menu_flag=1;
			}
			
			
		}
		else offdelay=10;  // Если кнопка отпущена - процесс отключения прерван
		
		
//			if (GPIOA->IDR&(1<<12)) {
//				if (!switch_flag) switch_flag = 1;
//				offdelay = 10;
//			}
//			else if (switch_flag == 1) {
//				if (!offdelay) {
//					TIM1->CCR3=0; // Подсветка
//					GPIOA->BRR|=(1<<15); // Отключение
//					switch_flag = 2;
//				}
//			}
//			if (offdelay) offdelay--;
		
			//======SOS-звонок======//
		if(UP_KEY && DOWN_KEY ||             // При зажатии любых двух клавиш - СОС-звонок, либо - выход из СОС-звонка
			UP_KEY && BACK_KEY ||
			UP_KEY && ENTER_KEY ||
			DOWN_KEY && BACK_KEY ||
			DOWN_KEY && ENTER_KEY ||
			BACK_KEY && ENTER_KEY){
				
			two_keys_flag=1;
			if(!SOS_delay--){
				SOS_delay=10;
				if(switch_flag_SOS){
					switch_flag_SOS=0;
					TFT_Fill_Area(90, 16, 122, 34, BLACK);
				}
				else {
					switch_flag_SOS=1;
					TFT_Send_Str(90, 16, "SOS", 3, Font_11x18, RED, CYAN);
					t_music=0;
				}
			}
		}
		else{
			SOS_delay=10;
			two_keys_flag=0;
		}
	}
}

//-----------------------------------------------------------------------

inline void TimeShow(void){
		if((RTC->ISR & RTC_ISR_RSF) == RTC_ISR_RSF){
			strcpy(str_temp, RTC_GetDate());
			TFT_Send_Str(0, 10, str_temp, strlen(str_temp), Font_7x9, WHITE, BLACK);
			strcpy(str_temp, RTC_GetTime());
			TFT_Send_Str(0, 20, str_temp, strlen(str_temp), Font_11x18, WHITE, BLACK);
		}
}

//-----------------------------------------------------------------------

inline void BatteryShow(void){ // !--batt_refr_time
		#ifndef ADC_INTERRUPT
			if(signal_timer_flag){
				signal_timer_flag=0;
			ADC1->CHSELR=ADC_CHSELR_CHSEL2;
			ADC1->CR |= ADC_CR_ADSTART;
			while (!(ADC1->ISR & ADC_ISR_EOC));
			adc[0]=ADC1->DR;
//			ADC1->CR |= ADC_CR_ADSTP;
			
			ADC1->CHSELR=ADC_CHSELR_CHSEL3;
			ADC1->CR |= ADC_CR_ADSTART;
			while (!(ADC1->ISR & ADC_ISR_EOC));
			adc[1]=ADC1->DR;
//			ADC1->CR |= ADC_CR_ADSTP;
			
			ADC1->CHSELR=ADC_CHSELR_CHSEL9;
			ADC1->CR |= ADC_CR_ADSTART;
			while (!(ADC1->ISR & ADC_ISR_EOC));
			adc[2]=ADC1->DR;
//			ADC1->CR |= ADC_CR_ADSTP;
				signal_timer_flag=1;
			}
		#endif // ADC_INTERRUPT
			
//		batt_refr_time=180;     // Значение батареи обновляется раз в 30сек.

		show_battery_flag=0;
		
		batt_procent=(adc[0]-2100)/4; // (adc[0]*100)/2339 2120==0% -2035)/3
		if(batt_procent>100) batt_procent=100;
		
		sprintf(str_temp, "%3d%%", batt_procent);
		
		if(batt_procent>60) TFT_Send_Str(92, 4, str_temp, strlen(str_temp), Font_6x8, GREEN, BLACK);
		if(batt_procent<=60 && batt_procent>20) TFT_Send_Str(92, 4, str_temp, strlen(str_temp), Font_6x8, YELLOW, BLACK);
		if(batt_procent<=20) TFT_Send_Str(92, 4, str_temp, strlen(str_temp), Font_6x8, RED, BLACK);
			
		if(adc[1]>1000 && lightning_on) {
			TFT_Draw_Image_Mono(lightning_mono_arr, lightning_mono_width, lightning_mono_height, 80, 0, 0xE3E3, BLACK);
			lightning_on=0;
			lightning_off=1;
			ALERT_ForNotification();
		}
		if(adc[1]<1000 && lightning_off) {
			TFT_Fill_Area(80, 0, 80+lightning_mono_width, 0+lightning_mono_height, BLACK);
			lightning_on=1;
			lightning_off=0;
//			ALERT_ForNotification();
		}
}

//-----------------------------------------------------------------------

inline void SignalAmplitude(void){
		if(adc[2]>sign_max) sign_max=adc[2];
		if(adc[2]<sign_min) sign_min=adc[2];
		if((sign_max-sign_min)*100/4096>0){
			sprintf(str_temp, "%3d%%", (sign_max-sign_min)*100/4096);
			TFT_Send_Str(0, 120, str_temp, strlen(str_temp), Font_11x18, WHITE, BLACK);
		}
		if(!--refresh_time){
			refresh_time=750;
			sign_max=2048;
			sign_min=2048;
		}
}
