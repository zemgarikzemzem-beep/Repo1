#include <stdio.h>
#include "tft-menu.h"
#include "disp.h"
#include "string.h"
#include "eeprom.h"
#include "rtc.h"
#include "flash.h"
#include "alerts.h"
#include "data.h"

extern void delay(__IO uint32_t tck);

uint16_t MESS_NUM=0;

uint8_t Button_Selected=0; // Выбранный пункт меню
uint16_t Message_Selected=0; // Выбранное сообщение
uint8_t In_Mess=0;   // Флаг текущего нахождения в чтении сообщения
//char mess[MESS_MAX_SIZE]={0,};    // Буфер для сообщения
uint8_t Sel_Butt_Pos=0;                // Позиция меню на экране
extern struct Menu_Item* Current_Menu; // Текущее меню
char mess[MESS_MAX_SIZE]={0,};
uint8_t mess_menu_sel_pos=0;
extern uint8_t message_menu_flag;
int8_t Message_Current_Str=0;
uint8_t mess_rows_num;
char* Mess_Current_Position;

extern uint8_t message_received;
extern uint8_t is_menu;

uint16_t mess_menu_temp;

//char str_hour[5]={0,};
//char str_min[5]={0,};
//char str_day[5]={0,};
//char str_month[5]={0,};
//char str_year[5]={0,};
const struct Menu_Item menu_time[]={{"Часы+",50+HOURS,NULL,menu_settings}, {"Минуты+",MINUTES,NULL,menu_settings}, {"День+",DAY,NULL,menu_settings}, {"Месяц+",MONTH,NULL,menu_settings}, {"Год+",YEAR,NULL,menu_settings}}; // , {"День нед+",WEEKDAY,NULL,menu_settings}

const struct Menu_Item menu_mess[]={{"Читать",20+READ_MESS,NULL,NULL}, {"Инфо",INFO_MESS,NULL,NULL}};              // Первая цифра нулевого элемента - размер массива
const struct Menu_Item menu_autolock[]={{"АБл вкл.", 20+AUTOLOCK_ON, NULL, menu_settings}, {"АБл выкл.", AUTOLOCK_OFF, NULL, menu_settings}};
const struct Menu_Item menu_autosleep_time[]={{"ВрДоСна+", 10+SLEEP_TIME_PLUS, NULL, menu_settings}};
const struct Menu_Item menu_inversion[]={{"Инв вкл",20,NULL,menu_settings}, {"Инв выкл",1,NULL,menu_settings}};
const struct Menu_Item menu_brightness[]={{"Яркость+",20,NULL,menu_settings}, {"Яркость-",1,NULL,menu_settings}};
const struct Menu_Item menu_settings[]={{"Яркость",50+BRIGHTNESS,menu_brightness,menu_main}, {"Время",TIME,menu_time,menu_main}, {"Инверсия",INVERSION,menu_inversion,menu_main}, {"Автоблок",AUTOLOCK,menu_autolock,menu_main}, {"Время АБл",SLEEP_TIME,menu_autosleep_time,menu_main}};
const struct Menu_Item menu_main[]={{"Настройки",60+SETTINGS,menu_settings,NULL}, {"БЛОК",BUTTONS_LOCK,NULL,NULL}, {"Сообщения",MESS_ARCHIVE,NULL,NULL}, {"ПЛЛПА",MESSAGES,NULL,NULL}, {"Программ",PROGRAMMING,NULL,NULL}, {"Мощность",SIGNAL_POWER,NULL,NULL}};

struct Menu_Struct main_menu = {
	3,
	menu_main
};

//----------------------------------------------------------------------------------------------------------------------------

/*============Инициализация кнопок==============*/

void Buttons_Init(void){
	GPIOA->MODER&=~(GPIO_MODER_MODER11|GPIO_MODER_MODER12);
	GPIOA->PUPDR|=(GPIO_PUPDR_PUPDR11_0|GPIO_PUPDR_PUPDR12_0);
	GPIOB->MODER&=~(GPIO_MODER_MODER8|GPIO_MODER_MODER9);
	GPIOB->PUPDR|=(GPIO_PUPDR_PUPDR8_0|GPIO_PUPDR_PUPDR9_0);
}

//----------------------------------------------------------------------------------------------------------------------------

/*============Отрисовка меню==============*/

void Menu_Draw(const struct Menu_Item* menu){
	_CLEAR_MENU_SCREEN;
	for(uint8_t i=0; i<(((menu[0].value)/10<=MAX_MENU_ITEM_SCREEN)?((menu[0].value)/10):MAX_MENU_ITEM_SCREEN); ++i){  // Если пунктов не более 3-х либо - более
		TFT_Send_Str(MENU_CON_X, MENU_CON_Y+i*MENU_FONT_HEIGH, menu[i].name, strlen(menu[i].name), Font_11x18, WHITE, BLACK);
	}
	Sel_Butt_Pos=0;
}

/*============Выбор пункта меню==============*/

void Menu_Item_Select(const struct Menu_Item* menu, uint8_t item_num){
	TFT_Send_Str(MENU_CON_X, MENU_CON_Y+item_num*MENU_FONT_HEIGH, menu[item_num].name, strlen(menu[item_num].name), Font_11x18, BLACK, WHITE);
	Button_Selected=menu[item_num].value%10;
}

/*============Отмена выбора пункта меню==============*/

void Menu_Item_Unselect(const struct Menu_Item* menu, uint8_t item_num){
	TFT_Send_Str(MENU_CON_X, MENU_CON_Y+item_num*MENU_FONT_HEIGH, menu[item_num].name, strlen(menu[item_num].name), Font_11x18, WHITE, BLACK);
}

//----------------------------------------------------------------------------------------------------------------------------

/*============Выбор сообщения==============*/

void Message_Menu_Item_Select(uint32_t addr, uint8_t item_num){
	FLASH_ReadStr(addr+MESS_MAX_SIZE*item_num, (uint8_t*)mess, MESS_MAX_ITEM_LENGTH);
	TFT_Send_Str(MENU_CON_X, MENU_CON_Y+item_num*18, mess, (strlen(mess)<MESS_MAX_ITEM_LENGTH)?strlen(mess):MESS_MAX_ITEM_LENGTH, Font_11x18, BLACK, WHITE);
	Message_Selected=item_num;
}

/*============Отмена выбора сообщения==============*/

void Message_Menu_Item_Unselect(uint32_t addr, uint8_t item_num){
	FLASH_ReadStr(addr+MESS_MAX_SIZE*item_num, (uint8_t*)mess, MESS_MAX_ITEM_LENGTH);
	TFT_Send_Str(MENU_CON_X, MENU_CON_Y+item_num*18, mess, (strlen(mess)<MESS_MAX_ITEM_LENGTH)?strlen(mess):MESS_MAX_ITEM_LENGTH, Font_11x18, WHITE, BLACK);
}

/*============Отрисовка списка сообщений==============*/

void Mess_Menu_Draw(uint32_t addr){
	_CLEAR_MENU_SCREEN;
	uint16_t mess_num;
//	switch(addr){
//		case FLASH_PLA_ADDR:
//			mess_num=(FLASH_ReadByte(FLASH_SETTINGS_ADDR+PLA_ITEMS_NUM_1)<<8)+(FLASH_ReadByte(FLASH_SETTINGS_ADDR+PLA_ITEMS_NUM_0)&0xFF);
//			break;
//		case FLASH_REC_MESS_ADDR:
//			mess_num=FLASH_ReadByte(FLASH_SETTINGS_ADDR+REC_MESS_NUM);
//			break;
//	}
	for(uint8_t i=0; i<(MAX_MESS_ITEM_SCREEN); ++i){ // (mess_num>MAX_MESS_ITEM_SCREEN)?:mess_num
		FLASH_ReadStr(addr+MESS_MAX_SIZE*i, (uint8_t*)mess, MESS_MAX_ITEM_LENGTH);
		TFT_Send_Str(MENU_CON_X, MENU_CON_Y+i*18, mess, (strlen(mess)<MESS_MAX_ITEM_LENGTH)?strlen(mess):MESS_MAX_ITEM_LENGTH, Font_11x18, WHITE, BLACK);
	}
	Message_Menu_Item_Select(addr, 0);
	mess_menu_sel_pos=0;
}

//----------------------------------------------------------------------------------------------------------------------------

/*============Навигация по меню списка сообщений==============*/

inline void Message_Menu_Navigation(uint32_t addr){
	switch(addr){
		case FLASH_PLA_ADDR:
			MESS_NUM=(FLASH_ReadByte(FLASH_SETTINGS_ADDR+PLA_ITEMS_NUM_1)<<8)+(FLASH_ReadByte(FLASH_SETTINGS_ADDR+PLA_ITEMS_NUM_0)&0xFF);
			break;
		case FLASH_REC_MESS_ADDR:
			MESS_NUM=FLASH_ReadByte(FLASH_SETTINGS_ADDR+REC_MESS_NUM);
			break;
	}
	
	
	if(DOWN_KEY){
		if(!In_Mess){                                // Прокрутка списка сообщений вниз
			if(MESS_NUM<=MAX_MESS_ITEM_SCREEN){
				Message_Menu_Item_Unselect(addr, Message_Selected);
				Message_Menu_Item_Select(addr, (Message_Selected+1)%MESS_NUM);
			}
			else{
				if((Message_Selected+1)==MESS_NUM){
					Mess_Menu_Draw(addr);
				}
				else
					if(mess_menu_sel_pos<MAX_MESS_ITEM_SCREEN-1){
						FLASH_ReadStr(addr+MESS_MAX_SIZE*Message_Selected, (uint8_t*)mess, MESS_MAX_ITEM_LENGTH);
						TFT_Send_Str(MENU_CON_X, MENU_CON_Y+(mess_menu_sel_pos++)*18, mess, (strlen(mess)<MESS_MAX_ITEM_LENGTH)?strlen(mess):MESS_MAX_ITEM_LENGTH, Font_11x18, WHITE, BLACK);
						FLASH_ReadStr(addr+MESS_MAX_SIZE*(++Message_Selected), (uint8_t*)mess, MESS_MAX_ITEM_LENGTH);
						TFT_Send_Str(MENU_CON_X, MENU_CON_Y+mess_menu_sel_pos*18, mess, (strlen(mess)<MESS_MAX_ITEM_LENGTH)?strlen(mess):MESS_MAX_ITEM_LENGTH, Font_11x18, BLACK, WHITE);
					}
					else{
						_CLEAR_MENU_SCREEN;
						++Message_Selected;
						mess_menu_temp=Message_Selected-MAX_MESS_ITEM_SCREEN+1;
						for(uint8_t i=0; i<MAX_MESS_ITEM_SCREEN-1; ++i){
							FLASH_ReadStr(addr+MESS_MAX_SIZE*(mess_menu_temp+i), (uint8_t*)mess, MESS_MAX_ITEM_LENGTH);
							TFT_Send_Str(MENU_CON_X, MENU_CON_Y+i*18, mess, (strlen(mess)<MESS_MAX_ITEM_LENGTH)?strlen(mess):MESS_MAX_ITEM_LENGTH, Font_11x18, WHITE, BLACK);
						}
						FLASH_ReadStr(addr+MESS_MAX_SIZE*(mess_menu_temp+MAX_MESS_ITEM_SCREEN-1), (uint8_t*)mess, MESS_MAX_ITEM_LENGTH);
						TFT_Send_Str(MENU_CON_X, MENU_CON_Y+(MAX_MESS_ITEM_SCREEN-1)*18, mess, (strlen(mess)<MESS_MAX_ITEM_LENGTH)?strlen(mess):MESS_MAX_ITEM_LENGTH, Font_11x18, BLACK, WHITE);
					}
			}
		}
		else{                                // Прокрутка сообщения вниз
			_CLEAR_MENU_SCREEN;
			if(++Message_Current_Str+MAX_MENU_ITEM_SCREEN-1>=mess_rows_num) --Message_Current_Str;
			Mess_Current_Position=mess+MAX_SYM_IN_STR*Message_Current_Str;
			TFT_Send_Str(0, MENU_CON_Y, Mess_Current_Position,
					(strlen(Mess_Current_Position)>=MESS_MAX_DISP) ? MESS_MAX_DISP : strlen(Mess_Current_Position), Font_11x18, RED, CYAN);
		}
	}
		
	if(UP_KEY){
		if(!In_Mess){                                // Прокрутка списка сообщений вверх
			if(MESS_NUM<=MAX_MESS_ITEM_SCREEN){
				Message_Menu_Item_Unselect(addr, Message_Selected);
				Message_Menu_Item_Select(addr, (Message_Selected>0)?(Message_Selected-1):2);
			}
			else
				if(mess_menu_sel_pos>0){
					FLASH_ReadStr(addr+MESS_MAX_SIZE*Message_Selected, (uint8_t*)mess, MESS_MAX_ITEM_LENGTH);
					TFT_Send_Str(MENU_CON_X, MENU_CON_Y+(mess_menu_sel_pos--)*18, mess, (strlen(mess)<MESS_MAX_ITEM_LENGTH)?strlen(mess):MESS_MAX_ITEM_LENGTH, Font_11x18, WHITE, BLACK);
					FLASH_ReadStr(addr+MESS_MAX_SIZE*(--Message_Selected), (uint8_t*)mess, MESS_MAX_ITEM_LENGTH);
					TFT_Send_Str(MENU_CON_X, MENU_CON_Y+mess_menu_sel_pos*18, mess, (strlen(mess)<MESS_MAX_ITEM_LENGTH)?strlen(mess):MESS_MAX_ITEM_LENGTH, Font_11x18, BLACK, WHITE);
				}
				else{
					_CLEAR_MENU_SCREEN;
					Message_Selected=(Message_Selected+MESS_NUM-1)%MESS_NUM;
					FLASH_ReadStr(addr+MESS_MAX_SIZE*Message_Selected, (uint8_t*)mess, MESS_MAX_ITEM_LENGTH);
					TFT_Send_Str(MENU_CON_X, MENU_CON_Y+0*18, mess, (strlen(mess)<MESS_MAX_ITEM_LENGTH)?strlen(mess):MESS_MAX_ITEM_LENGTH, Font_11x18, BLACK, WHITE);
					
					for(uint8_t i=1; i<MAX_MESS_ITEM_SCREEN; ++i){
						FLASH_ReadStr(addr+MESS_MAX_SIZE*((Message_Selected+i)%MESS_NUM), (uint8_t*)mess, MESS_MAX_ITEM_LENGTH);
						TFT_Send_Str(MENU_CON_X, MENU_CON_Y+i*18, mess, (strlen(mess)<MESS_MAX_ITEM_LENGTH)?strlen(mess):MESS_MAX_ITEM_LENGTH, Font_11x18, WHITE, BLACK);
					}
				}
			
		}
		else{                                // Прокрутка сообщения вверх
			_CLEAR_MENU_SCREEN;
			if(--Message_Current_Str<0) Message_Current_Str=0;
			Mess_Current_Position=mess+MAX_SYM_IN_STR*Message_Current_Str;
			TFT_Send_Str(0, MENU_CON_Y, Mess_Current_Position,
					(strlen(Mess_Current_Position)>=MESS_MAX_DISP) ? MESS_MAX_DISP : strlen(Mess_Current_Position), Font_11x18, RED, CYAN);
			
		}
	}
		
	if(ENTER_KEY){             // Переход в меню выбора действий с сообщением
//		Current_Menu=menu_mess;
//		Menu_Draw(Current_Menu);
//		Menu_Item_Select(Current_Menu, 0);
//		message_menu_flag=0;
		Message_Read(addr);
	}
		
	if(BACK_KEY){
		if(In_Mess && !message_received){           // Выход из сообщения
			In_Mess=0;
			Message_Current_Str=0;
			_CLEAR_MENU_SCREEN;
			for(uint8_t i=0; i<MAX_MESS_ITEM_SCREEN; ++i){
				FLASH_ReadStr(addr+MESS_MAX_SIZE*((Message_Selected-mess_menu_sel_pos+i)%MESS_NUM), (uint8_t*)mess, MESS_MAX_ITEM_LENGTH);
				TFT_Send_Str(MENU_CON_X, MENU_CON_Y+i*18, mess, (strlen(mess)<MESS_MAX_ITEM_LENGTH)?strlen(mess):MESS_MAX_ITEM_LENGTH, Font_11x18, (mess_menu_sel_pos!=i)?WHITE:BLACK, (mess_menu_sel_pos!=i)?BLACK:WHITE);
			}
		}
		else{           // Выход из меню выбора действий с сообщением или из пришедшего сообщения
			Current_Menu=menu_main;
			Menu_Draw(menu_main);
			Menu_Item_Select(menu_main, 0);
			message_menu_flag=0;
			if(message_received){
				message_received=0;
				is_menu=1;
			}
			Sel_Butt_Pos=0;
		}
	}
}
//----------------------------------------------------------------------------------------------------------------------------

/*============Вывод пункта ПЛА==============*/

void Message_Read(uint32_t addr){
	FLASH_ReadStr(addr+MESS_MAX_SIZE*Message_Selected, (uint8_t*)mess, MESS_MAX_SIZE);                // + привязка ко времени!+1
	_CLEAR_MENU_SCREEN;
	TFT_Send_Str(0, MENU_CON_Y, mess, (strlen(mess)<=MESS_MAX_DISP)?strlen(mess):MESS_MAX_DISP, Font_11x18, RED, CYAN);
	mess_rows_num=(strlen(mess)%MAX_SYM_IN_STR==0)?(strlen(mess)/MAX_SYM_IN_STR):(strlen(mess)/MAX_SYM_IN_STR+1);
	message_menu_flag=1;
	In_Mess=1;
}

//------------------------------------------------------------------------------------------------------------------------------

extern uint8_t Items_Num;

/*============Переход вниз по меню==============*/

inline void Menu_Down(void){
	if(Items_Num<=MAX_MENU_ITEM_SCREEN){
				Menu_Item_Unselect(Current_Menu, Button_Selected);
				Menu_Item_Select(Current_Menu, (Button_Selected+1)%Items_Num);
			}
			else{
				Button_Selected=(Button_Selected+1)%Items_Num;
				if(Button_Selected==0){
					Menu_Draw(Current_Menu);
					Menu_Item_Select(Current_Menu, 0);
					Sel_Butt_Pos=0;
				}
				else if(Sel_Butt_Pos<MAX_MENU_ITEM_SCREEN-1){
					++Sel_Butt_Pos;
					TFT_Send_Str(MENU_CON_X, MENU_CON_Y+(Sel_Butt_Pos-1)*MENU_FONT_HEIGH, Current_Menu[(Button_Selected+Items_Num-1)%Items_Num].name,
						strlen(Current_Menu[(Button_Selected+Items_Num-1)%Items_Num].name), Font_11x18, WHITE, BLACK);
					TFT_Send_Str(MENU_CON_X, MENU_CON_Y+Sel_Butt_Pos*MENU_FONT_HEIGH, Current_Menu[Button_Selected].name,
						strlen(Current_Menu[Button_Selected].name), Font_11x18, BLACK, WHITE);
				}
				else{
					Sel_Butt_Pos=MAX_MENU_ITEM_SCREEN-1;
					_CLEAR_MENU_SCREEN;
					for(uint8_t i=0; i<MAX_MENU_ITEM_SCREEN-1; ++i){
						TFT_Send_Str(MENU_CON_X, MENU_CON_Y+i*MENU_FONT_HEIGH, Current_Menu[(Button_Selected+Items_Num-MAX_MENU_ITEM_SCREEN+1+i)%Items_Num].name,
						strlen(Current_Menu[(Button_Selected+Items_Num-MAX_MENU_ITEM_SCREEN+1+i)%Items_Num].name), Font_11x18, WHITE, BLACK);
					}
					TFT_Send_Str(MENU_CON_X, MENU_CON_Y+Sel_Butt_Pos*MENU_FONT_HEIGH, Current_Menu[(Button_Selected)%Items_Num].name,
						strlen(Current_Menu[(Button_Selected)%Items_Num].name), Font_11x18, BLACK, WHITE);
				}
			}
}

/*============Переход вверх по меню==============*/

inline void Menu_Up(void){
	if(Items_Num<=MAX_MENU_ITEM_SCREEN){
				Menu_Item_Unselect(Current_Menu, Button_Selected);
				Menu_Item_Select(Current_Menu, (Button_Selected>0)?(Button_Selected-1):((Current_Menu[0].value)/10-1));
			}
			else{
				Button_Selected=(Button_Selected+Items_Num-1)%Items_Num;
				if(Sel_Butt_Pos>0){
					--Sel_Butt_Pos;
					TFT_Send_Str(MENU_CON_X, MENU_CON_Y+(Sel_Butt_Pos+1)*MENU_FONT_HEIGH, Current_Menu[(Button_Selected+1)%Items_Num].name,
						strlen(Current_Menu[(Button_Selected+1)%Items_Num].name), Font_11x18, WHITE, BLACK);
					TFT_Send_Str(MENU_CON_X, MENU_CON_Y+Sel_Butt_Pos*MENU_FONT_HEIGH, Current_Menu[Button_Selected].name,
						strlen(Current_Menu[Button_Selected].name), Font_11x18, BLACK, WHITE);
				}
				else{
					Sel_Butt_Pos=0;
					_CLEAR_MENU_SCREEN;
					for(uint8_t i=1; i<MAX_MENU_ITEM_SCREEN; ++i){
						TFT_Send_Str(MENU_CON_X, MENU_CON_Y+i*MENU_FONT_HEIGH, Current_Menu[(Button_Selected+i)%Items_Num].name,
							strlen(Current_Menu[(Button_Selected+i)%Items_Num].name), Font_11x18, WHITE, BLACK);
					}
					TFT_Send_Str(MENU_CON_X, MENU_CON_Y+Sel_Butt_Pos*MENU_FONT_HEIGH, Current_Menu[Button_Selected%Items_Num].name,
						strlen(Current_Menu[Button_Selected%Items_Num].name), Font_11x18, BLACK, WHITE);
				}
			}
}

//------------------------------------------------------------------------------------------------------------------------------

/*============Настройка часов==============*/

uint8_t time;
inline void Hour_Plus(void){
//	RCC->APB1ENR|=RCC_APB1ENR_PWREN;
	PWR->CR|=PWR_CR_DBP;
//	RCC->BDCR|=RCC_BDCR_RTCEN;
	time=((RTC->TR>>RTC_TR_HT_Pos)&0b0011)*10+((RTC->TR>>RTC_TR_HU_Pos)&0x0F);
	RTC->WPR=0xCA;
	RTC->WPR=0x53;
	RTC->ISR |= RTC_ISR_INIT;
	while ((RTC->ISR & RTC_ISR_INITF) != RTC_ISR_INITF);
	RTC->TR&=~(RTC_TR_HU_Msk|RTC_TR_HT_Msk);
	RTC->ISR&=~RTC_ISR_INIT;
	while ((RTC->ISR & RTC_ISR_RSF) != RTC_ISR_RSF);
	RTC->WPR=0xFF;
	
	//delay(5);
	RTC->WPR=0xCA;
	RTC->WPR=0x53;
	RTC->ISR |= RTC_ISR_INIT;
	while ((RTC->ISR & RTC_ISR_INITF) != RTC_ISR_INITF);
	if(time<23){
		RTC->TR|=((((time+1)/10)<<RTC_TR_HT_Pos)|(((time+1)%10)<<RTC_TR_HU_Pos));
	}
	RTC->ISR&=~RTC_ISR_INIT;
	RTC->WPR=0xFF;
}

/*============Настройка минут==============*/

inline void Min_Plus(void){
//	RCC->APB1ENR|=RCC_APB1ENR_PWREN;
	PWR->CR|=PWR_CR_DBP;
//	RCC->BDCR|=RCC_BDCR_RTCEN;
	time=((RTC->TR>>RTC_TR_MNT_Pos)&0x0F)*10+((RTC->TR>>RTC_TR_MNU_Pos)&0x0F);
	RTC->WPR=0xCA;
	RTC->WPR=0x53;
	RTC->ISR |= RTC_ISR_INIT;
	while ((RTC->ISR & RTC_ISR_INITF) != RTC_ISR_INITF);
	RTC->TR&=~(RTC_TR_MNU_Msk|RTC_TR_MNT_Msk);
	RTC->ISR&=~RTC_ISR_INIT;
	while ((RTC->ISR & RTC_ISR_RSF) != RTC_ISR_RSF);
	RTC->WPR=0xFF;
	
	RTC->WPR=0xCA;
	RTC->WPR=0x53;
	RTC->ISR |= RTC_ISR_INIT;
	while ((RTC->ISR & RTC_ISR_INITF) != RTC_ISR_INITF);
	if(time<59){
		RTC->TR|=((((time+1)/10)<<RTC_TR_MNT_Pos)|(((time+1)%10)<<RTC_TR_MNU_Pos));
	}
	RTC->ISR&=~RTC_ISR_INIT;
	RTC->WPR=0xFF;
}

//uint8_t month, day, year, weekday;
//	if(((RTC->DR>>RTC_DR_MU_Pos)&0x0F)<3){                      // Чтобы определять по формуле день недели
//		month+=10;
//		year-=1;
//	}
//	else{
//		month-=2;
//	}
//	weekday=abs(int(2.6*(double)m-0.2)+d+y div 4+y-35)%7;

/*============Расчёт дня недели по дате==============*/

inline void WeekDay_Calc(void){
	uint8_t month, day, year, weekday;
	month=((RTC->DR>>RTC_DR_MT_Pos)&0x01)*10+((RTC->DR>>RTC_DR_MU_Pos)&0x0F);
	day=((RTC->DR>>RTC_DR_DT_Pos)&0b0011)*10+((RTC->DR>>RTC_DR_DU_Pos)&0x0F);
	year=((RTC->DR>>RTC_DR_YT_Pos)&0x0F)*10+((RTC->DR>>RTC_DR_YU_Pos)&0x0F);
	if(month<3){                      // Чтобы определять по формуле день недели
		month+=10;
		year-=1;
	}
	else{
		month-=2;
	}
	weekday=abs((uint8_t)(2.6*(double)month-0.2)+day+year/4+year-35)%7;
	RTC->WPR=0xCA;
	RTC->WPR=0x53;
	RTC->ISR |= RTC_ISR_INIT;
	while ((RTC->ISR & RTC_ISR_INITF) != RTC_ISR_INITF);
	
	RTC->DR=(RTC->DR&(~RTC_DR_WDU_Msk))|(((weekday!=0)?weekday:7)<<RTC_DR_WDU_Pos);
	
	RTC->ISR&=~RTC_ISR_INIT;
	RTC->WPR=0xFF;
}

/*============Настройка дня==============*/

inline void Day_Plus(void){
//	RCC->APB1ENR|=RCC_APB1ENR_PWREN;
	PWR->CR|=PWR_CR_DBP;
//	RCC->BDCR|=RCC_BDCR_RTCEN;
	time=((RTC->DR>>RTC_DR_DT_Pos)&0b0011)*10+((RTC->DR>>RTC_DR_DU_Pos)&0x0F);
	RTC->WPR=0xCA;
	RTC->WPR=0x53;
	RTC->ISR |= RTC_ISR_INIT;
	while ((RTC->ISR & RTC_ISR_INITF) != RTC_ISR_INITF);
	RTC->DR=(RTC->DR&(~(RTC_DR_DU_Msk|RTC_DR_DT_Msk)));
	RTC->ISR&=~RTC_ISR_INIT;
	while ((RTC->ISR & RTC_ISR_RSF) != RTC_ISR_RSF);
	RTC->WPR=0xFF;
	
	RTC->WPR=0xCA;
	RTC->WPR=0x53;
	RTC->ISR |= RTC_ISR_INIT;
	while ((RTC->ISR & RTC_ISR_INITF) != RTC_ISR_INITF);
	if(time<31){
		RTC->DR|=((((time+1)/10)<<RTC_DR_DT_Pos)|(((time+1)%10)<<RTC_DR_DU_Pos));
	}
	else RTC->DR=(RTC->DR&(~(RTC_DR_DU_Msk|RTC_DR_DT_Msk)))|RTC_DR_DU_0;//
	RTC->ISR&=~RTC_ISR_INIT;
	while ((RTC->ISR & RTC_ISR_RSF) != RTC_ISR_RSF);
	RTC->WPR=0xFF;
	
//	delay(5);
	WeekDay_Calc();
}

/*============Настройка месяца==============*/

inline void Month_Plus(void){
//	RCC->APB1ENR|=RCC_APB1ENR_PWREN;
	PWR->CR|=PWR_CR_DBP;
//	RCC->BDCR|=RCC_BDCR_RTCEN;
	time=((RTC->DR>>RTC_DR_MT_Pos)&0x01)*10+((RTC->DR>>RTC_DR_MU_Pos)&0x0F);
	RTC->WPR=0xCA;
	RTC->WPR=0x53;
	RTC->ISR |= RTC_ISR_INIT;
	while ((RTC->ISR & RTC_ISR_INITF) != RTC_ISR_INITF);
	RTC->DR=(RTC->DR&(~(RTC_DR_MU_Msk|RTC_DR_MT_Msk)));
	RTC->ISR&=~RTC_ISR_INIT;
	while ((RTC->ISR & RTC_ISR_RSF) != RTC_ISR_RSF);
	RTC->WPR=0xFF;
	
	RTC->WPR=0xCA;
	RTC->WPR=0x53;
	RTC->ISR |= RTC_ISR_INIT;
	while ((RTC->ISR & RTC_ISR_INITF) != RTC_ISR_INITF);
	if(time<12){
		RTC->DR|=((((time+1)/10)<<RTC_DR_MT_Pos)|(((time+1)%10)<<RTC_DR_MU_Pos));
	}
	else RTC->DR=(RTC->DR&(~(RTC_DR_MU_Msk|RTC_DR_MT_Msk)))|RTC_DR_MU_0;
	RTC->ISR&=~RTC_ISR_INIT;
	while ((RTC->ISR & RTC_ISR_RSF) != RTC_ISR_RSF);
	RTC->WPR=0xFF;
	
//	delay(5);
	WeekDay_Calc();
}

/*============Настройка года==============*/

inline void Year_Plus(void){
//	RCC->APB1ENR|=RCC_APB1ENR_PWREN;
	PWR->CR|=PWR_CR_DBP;
//	RCC->BDCR|=RCC_BDCR_RTCEN;
	time=((RTC->DR>>RTC_DR_YT_Pos)&0x0F)*10+((RTC->DR>>RTC_DR_YU_Pos)&0x0F);
	RTC->WPR=0xCA;
	RTC->WPR=0x53;
	RTC->ISR |= RTC_ISR_INIT;
	while ((RTC->ISR & RTC_ISR_INITF) != RTC_ISR_INITF);
	RTC->DR&=~(RTC_DR_YU_Msk|RTC_DR_YT_Msk);
	RTC->ISR&=~RTC_ISR_INIT;
	while ((RTC->ISR & RTC_ISR_RSF) != RTC_ISR_RSF);
	RTC->WPR=0xFF;
	
	RTC->WPR=0xCA;
	RTC->WPR=0x53;
	RTC->ISR |= RTC_ISR_INIT;
	while ((RTC->ISR & RTC_ISR_INITF) != RTC_ISR_INITF);
	if(time<99){
		RTC->DR|=((((time+1)/10)<<RTC_DR_YT_Pos)|(((time+1)%10)<<RTC_DR_YU_Pos));
	}
	else{
		RTC->DR|=((0<<RTC_DR_YT_Pos)|(0<<RTC_DR_YU_Pos));
	}
	RTC->ISR&=~RTC_ISR_INIT;
	while ((RTC->ISR & RTC_ISR_RSF) != RTC_ISR_RSF);
	RTC->WPR=0xFF;
	
//	delay(5);
	WeekDay_Calc();
}
extern uint8_t signal;
extern char str_temp[];

//-----------------------------------------------------------------------------

/*============Вывод сигнала об аварии==============*/

extern uint8_t rec_buf[];
extern uint8_t receive_bit_show_flag, is_menu;
uint8_t dig1=0, dig2=0;
extern uint16_t	byte1, byte2, byte3;
uint16_t	byte=0;
uint16_t PLA_num;
extern uint16_t label;
	uint16_t byte1_14d, byte2_14d, byte3_14d, byte_14d=0;

extern uint32_t recw1, recw2;

void MessRecToArch(char* data){
	char tmp_str[MESS_MAX_SIZE] = {0,};
	uint8_t mess_shift=FLASH_ReadByte(FLASH_SETTINGS_ADDR+REC_MESS_NUM);
	if(mess_shift==0xFF) mess_shift=0;
	strcpy(tmp_str,RTC_GetDate());//+RTC_GetTime()+data;
	strcat(tmp_str, RTC_GetTime());
	strcat(tmp_str, "   ");
	strcat(tmp_str, data);
//	sprintf(tmp_str, "%s", RTC_GetDate()); //  %s %s, RTC_GetTime(), data
	if(mess_shift == REC_MESS_MAX_NUM){
		mess_shift=REC_MESS_MAX_NUM-1;
		FLASH_PageErase(FLASH_REC_MESS_TMPBUF);
		FLASH_WriteStr_PLA(FLASH_REC_MESS_TMPBUF+(REC_MESS_MAX_NUM-1)*MESS_MAX_SIZE, (uint8_t*)tmp_str, MESS_MAX_SIZE);
		for(uint8_t i=1; i<REC_MESS_MAX_NUM; ++i){
			FLASH_ReadStr(FLASH_REC_MESS_ADDR+i*MESS_MAX_SIZE, (uint8_t*)tmp_str, MESS_MAX_SIZE);
			FLASH_WriteStr_PLA(FLASH_REC_MESS_TMPBUF+(i-1)*MESS_MAX_SIZE, (uint8_t*)tmp_str, MESS_MAX_SIZE); 
		}
		FLASH_PageErase(FLASH_REC_MESS_ADDR);
		FLASH_PageErase(FLASH_REC_MESS_ADDR+FLASH_PAGESIZE);
		for(uint8_t i=0; i<REC_MESS_MAX_NUM; ++i){
			FLASH_ReadStr(FLASH_REC_MESS_TMPBUF+i*MESS_MAX_SIZE, (uint8_t*)tmp_str, MESS_MAX_SIZE);
			FLASH_WriteStr_PLA(FLASH_REC_MESS_ADDR+i*MESS_MAX_SIZE, (uint8_t*)tmp_str, MESS_MAX_SIZE); 
		}
	}
	else FLASH_WriteStr_PLA(FLASH_REC_MESS_ADDR+((mess_shift<20)?mess_shift:0)*MESS_MAX_SIZE, (uint8_t*)tmp_str, strlen(tmp_str)+1);
//	FLASH_WriteByte(FLASH_SETTINGS_ADDR+CURRENT_REC_MESS, mess_shift+1);
	FLASH_WriteByte(FLASH_SETTINGS_ADDR+REC_MESS_NUM, mess_shift+1);
}

inline void ShowSignal(void){
	uint8_t is_12or14d=0;        // флаг разрядности
	char tmp_str[MESS_MAX_SIZE] = {0,};
	
	MESS_NUM=(FLASH_ReadByte(FLASH_SETTINGS_ADDR+PLA_ITEMS_NUM_1)<<8)+(FLASH_ReadByte(FLASH_SETTINGS_ADDR+PLA_ITEMS_NUM_0)&0xFF);
	
	TFT_Fill_Area(90, 16, 122, 34, BLACK);
	
	byte1=(recw1>>20)&0xFFF;
	byte2=(recw1>>8)&0xFFF;
	byte3=((recw1&0x7F)<<5)+((recw2&0x7FF)>>6)&0xFFF;
	
	byte1_14d=(recw1>>18)&0x3FFF;
	byte2_14d=(recw1>>4)&0x3FFF;
	byte3_14d=(((recw1&0x7)<<11)+(recw2&0x7FF))&0x3FFF;
	
	if((byte1&0xFFF)==(byte2&0xFFF) || (byte1&0xFFF)==(byte3&0xFFF)){ byte=byte1; is_12or14d=12;}
	if((byte2&0xFFF)==(byte3&0xFFF)) { byte=byte2; is_12or14d=12;}
	
	if((byte1_14d&0xFFF)==(byte2_14d&0xFFF) || (byte1_14d&0xFFF)==(byte3_14d&0xFFF)) { byte_14d=byte1_14d; is_12or14d=14;}
	if((byte2_14d&0xFFF)==(byte3_14d&0xFFF)) { byte_14d=byte2_14d; is_12or14d=14;}
	
	if(is_12or14d==12){
		if((byte>>10)==0b00){
			if((byte&0x03FF)==label){
				is_menu=0;
				_CLEAR_MENU_SCREEN;
				TFT_Send_Str(0, 100, "Персональн. вызов!!!", 20, Font_11x18, RED, CYAN);
				ALERT_ForMessage();
				MessRecToArch("Персональн. вызов!!!");
			};
		}
		if((byte>>10)==0b11){
			is_menu=0;
			_CLEAR_MENU_SCREEN;
			if((byte&0x003)==0b11) {TFT_Send_Str(0, 100, "Авария-1", 8, Font_11x18, RED, CYAN); MessRecToArch("Авария-1");}
			if((byte&0x003)==0b00) {TFT_Send_Str(0, 100, "Авария-2", 8, Font_11x18, RED, CYAN); MessRecToArch("Авария-2");}
			
			if((byte&0x0F0)==0x030) {TFT_Send_Str(0, 118, "Рудник-1", 8, Font_11x18, RED, CYAN);}
			if((byte&0x0F0)==0x0C0) {TFT_Send_Str(0, 118, "Рудник-2", 8, Font_11x18, RED, CYAN);}
			if((byte&0x0F0)==0x0F0) {TFT_Send_Str(0, 118, "Рудник-3", 8, Font_11x18, RED, CYAN);}
			if((byte&0x0F0)==0x000) {TFT_Send_Str(0, 118, "Рудник-4", 8, Font_11x18, RED, CYAN);}
			ALERT_ForMessage();
		}
		else if((byte>>10)==0b01){
			is_menu=0;
			PLA_num=((byte&0x03FF)-8)/2; // Пункты ПЛА начинаются с 1 !!! -1
			
			if(PLA_num+1<=MESS_NUM){
				_CLEAR_MENU_SCREEN;
				Message_Selected=PLA_num;
				Message_Read(FLASH_PLA_ADDR);
				In_Mess=1;
				message_menu_flag=1;
				ALERT_ForMessage();
				FLASH_ReadStr(FLASH_PLA_ADDR+Message_Selected, (uint8_t*)tmp_str, MESS_MAX_SIZE);
				MessRecToArch(tmp_str);
			}
		}
	}
	
	if(is_12or14d==14){
		if((byte_14d>>12)==0b00){
			if((byte_14d&0x0FFF)==label){
				is_menu=0;
				_CLEAR_MENU_SCREEN;
				TFT_Send_Str(0, 100, "Персональн. вызов!!!", 20, Font_11x18, RED, CYAN);
				ALERT_ForMessage();
				MessRecToArch("Персональн. вызов!!!");
			}
		}
		if((byte_14d>>12)==0b11){
			is_menu=0;
			_CLEAR_MENU_SCREEN;
			if((byte_14d&0x00C)==0b1100) {TFT_Send_Str(0, 100, "Авария-1", 8, Font_11x18, RED, CYAN); MessRecToArch("Авария-1");}
			if((byte_14d&0x00C)==0b0000) {TFT_Send_Str(0, 100, "Авария-2", 8, Font_11x18, RED, CYAN); MessRecToArch("Авария-2");}
			
			if((byte_14d&0x3C0)==0x0C0) TFT_Send_Str(0, 118, "Рудник-1", 8, Font_11x18, RED, CYAN);
			if((byte_14d&0x3C0)==0x300) TFT_Send_Str(0, 118, "Рудник-2", 8, Font_11x18, RED, CYAN);
			if((byte_14d&0x3C0)==0x3C0) TFT_Send_Str(0, 118, "Рудник-3", 8, Font_11x18, RED, CYAN);
			if((byte_14d&0x3C0)==0x000) TFT_Send_Str(0, 118, "Рудник-4", 8, Font_11x18, RED, CYAN);
			ALERT_ForMessage();
		}
		if((byte_14d>>12)==0b01){
			is_menu=0;
			PLA_num=(((byte_14d&0x0FFF)>>2)-8)/2; // Пункты ПЛА начинаются с 1 !!! -1
			
			if(PLA_num+1<=MESS_NUM){
				_CLEAR_MENU_SCREEN;
				Message_Selected=PLA_num;
				Message_Read(FLASH_PLA_ADDR);
				In_Mess=1;
				message_menu_flag=1;
				ALERT_ForMessage();
				FLASH_ReadStr(FLASH_PLA_ADDR+Message_Selected, (uint8_t*)tmp_str, MESS_MAX_SIZE);
				MessRecToArch(tmp_str);
			}
			
			
		}
	}
	
	
	RCC->APB2ENR|=RCC_APB2ENR_SPI1EN;  // Включаем периферию, выключенную на сне
	RCC->APB1ENR|=RCC_APB1ENR_TIM3EN;
	
	TIM15->CR1&=~TIM_CR1_CEN;
	TIM16->CR1&=~TIM_CR1_CEN;
	TIM17->CR1&=~TIM_CR1_CEN;
	TIM15->CR1|=TIM_CR1_CEN;
	TIM16->CR1|=TIM_CR1_CEN;
	TIM17->CR1|=TIM_CR1_CEN;
	
}

//-----------------------------------------------------------------------------


