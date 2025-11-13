#include <stdio.h>
#include "tft-menu.h"
#include "disp.h"
#include "string.h"
#include "eeprom.h"
#include "rtc.h"

extern void delay(__IO uint32_t tck);

uint8_t Button_Selected=0; // Выбранный пункт меню
uint8_t Message_Selected=0; // Выбранное сообщение
uint8_t In_Mess=0;   // Флаг текущего нахождения в чтении сообщения
uint8_t Sel_Butt_Pos=0;
extern struct Menu_Item* Current_Menu; // Текущее меню

//char str_hour[5]={0,};
//char str_min[5]={0,};
//char str_day[5]={0,};
//char str_month[5]={0,};
//char str_year[5]={0,};
const struct Menu_Item menu_time[]={{"Часы+",50+HOURS,NULL,menu_settings}, {"Минуты+",MINUTES,NULL,menu_settings}, {"День+",DAY,NULL,menu_settings}, {"Месяц+",MONTH,NULL,menu_settings}, {"Год+",YEAR,NULL,menu_settings}}; // , {"День нед+",WEEKDAY,NULL,menu_settings}

const struct Menu_Item menu_mess[]={{"Читать",20+READ_MESS,NULL,NULL}, {"Удалить",DELETE_MESS,NULL,NULL}};              // Первая цифра нулевого элемента - размер массива
const struct Menu_Item menu_1[]={{"Пункт1.1",30,NULL,menu_main}, {"Пункт1.2",1,NULL,menu_main}, {"Пункт1.3",2,NULL,menu_main}};
const struct Menu_Item menu_inversion[]={{"Инв вкл",20,NULL,menu_settings}, {"Инв выкл",1,NULL,menu_settings}};
const struct Menu_Item menu_brightness[]={{"Яркость+",20,NULL,menu_settings}, {"Яркость-",1,NULL,menu_settings}};
const struct Menu_Item menu_settings[]={{"Яркость",30,menu_brightness,menu_main}, {"Время",1,menu_time,menu_main}, {"Инверсия",2,menu_inversion,menu_main}};
const struct Menu_Item menu_main[]={{"Приём",60+RECEIVE,NULL,NULL}, {"Настройки",SETTINGS,menu_settings,NULL}, {"Сообщения",MESSAGES,NULL,NULL}, {"Пункт2",ITEM2,NULL,NULL}, {"Пункт3",ITEM3,NULL,NULL}, {"Пункт4",ITEM4,NULL,NULL}};

struct Menu_Struct main_menu = {
	3,
	menu_main
};

//----------------------------------------------------------------------------------------------------------------------------

void Buttons_Init(void){
	GPIOA->MODER&=~(GPIO_MODER_MODER11|GPIO_MODER_MODER12);
	GPIOA->PUPDR|=(GPIO_PUPDR_PUPDR11_0|GPIO_PUPDR_PUPDR12_0);
	GPIOB->MODER&=~(GPIO_MODER_MODER8|GPIO_MODER_MODER9);
	GPIOB->PUPDR|=(GPIO_PUPDR_PUPDR8_0|GPIO_PUPDR_PUPDR9_0);
}

//----------------------------------------------------------------------------------------------------------------------------

void Menu_Draw(const struct Menu_Item* menu){
	_CLEAR_MENU_SCREEN;
	for(uint8_t i=0; i<(((menu[0].value)/10<=MAX_MENU_ITEM_SCREEN)?((menu[0].value)/10):MAX_MENU_ITEM_SCREEN); ++i){  // Если пунктов не более 3-х либо - более
		TFT_Send_Str(MENU_CON_X, MENU_CON_Y+i*MENU_FONT_HEIGH, menu[i].name, strlen(menu[i].name), Font_11x18, WHITE, BLACK);
	}
	Sel_Butt_Pos=0;
	
//	sprintf(str_hour, "%d", ((RTC->TR>>RTC_TR_HT_Pos)&0b0011)*10+((RTC->TR>>RTC_TR_HU_Pos)&0x0F));
//	sprintf(str_min, "%d", ((RTC->TR>>RTC_TR_MNT_Pos)&0x0F)*10+((RTC->TR>>RTC_TR_MNU_Pos)&0x0F));
//	sprintf(str_day, "%d", ((RTC->DR>>RTC_DR_DT_Pos)&0b0011)*10+((RTC->DR>>RTC_DR_DU_Pos)&0x0F));
//	sprintf(str_month, "%d", ((RTC->DR>>RTC_DR_MU_Pos)&0x0F));
//	sprintf(str_year, "%d", ((RTC->DR>>RTC_DR_YT_Pos)&0x0F)*10+((RTC->DR>>RTC_DR_YU_Pos)&0x0F));
}

void Menu_Item_Select(const struct Menu_Item* menu, uint8_t item_num){
	TFT_Send_Str(MENU_CON_X, MENU_CON_Y+item_num*MENU_FONT_HEIGH, menu[item_num].name, strlen(menu[item_num].name), Font_11x18, BLACK, WHITE);
	Button_Selected=menu[item_num].value%10;
}

void Menu_Item_Unselect(const struct Menu_Item* menu, uint8_t item_num){
	TFT_Send_Str(MENU_CON_X, MENU_CON_Y+item_num*MENU_FONT_HEIGH, menu[item_num].name, strlen(menu[item_num].name), Font_11x18, WHITE, BLACK);
}

//----------------------------------------------------------------------------------------------------------------------------

char mess[MESS_MAX_SIZE]={0,};

void Message_Menu_Item_Select(uint8_t item_num){
	EEPROM_ReadStr(Eeprom_Addr_Array[item_num], (uint8_t*)mess, MESS_MAX_SIZE);
	TFT_Send_Str(MENU_CON_X, MENU_CON_Y+item_num*9, mess, 15, Font_7x9, BLACK, WHITE);
	Message_Selected=item_num;
}

void Message_Menu_Item_Unselect(uint8_t item_num){
	EEPROM_ReadStr(Eeprom_Addr_Array[item_num], (uint8_t*)mess, MESS_MAX_SIZE);
	TFT_Send_Str(MENU_CON_X, MENU_CON_Y+item_num*9, mess, 15, Font_7x9, WHITE, BLACK);
}

void Mess_Menu_Draw(void){
	_CLEAR_MENU_SCREEN;
	for(uint8_t i=0; i<3; ++i){
		EEPROM_ReadStr(Eeprom_Addr_Array[i], (uint8_t*)mess, MESS_MAX_SIZE);
		TFT_Send_Str(MENU_CON_X, MENU_CON_Y+i*9, mess, 15, Font_7x9, WHITE, BLACK);
	}
	Message_Menu_Item_Select(0);
}

//----------------------------------------------------------------------------------------------------------------------------

extern uint8_t message_menu_flag;

inline void Message_Menu_Navigation(void){
	if(!(GPIOB->IDR&(1<<9))){
		if(!In_Mess){
			Message_Menu_Item_Unselect(Message_Selected);
			Message_Menu_Item_Select((Message_Selected+1)%3);
		}
		
	}
		
	if(!(GPIOA->IDR&(1<<11))){
		if(!In_Mess){
			Message_Menu_Item_Unselect(Message_Selected);
			Message_Menu_Item_Select((Message_Selected>0)?(Message_Selected-1):2);
		}
	}
		
	if(!(GPIOB->IDR&(1<<8))){
		Current_Menu=menu_mess;
		Menu_Draw(Current_Menu);
		Menu_Item_Select(Current_Menu, 0);
		message_menu_flag=0;
	}
		
	if(!(GPIOA->IDR&(1<<12))){
		if(In_Mess){
			Mess_Menu_Draw();
			In_Mess=0;
		}
		else{
			Current_Menu=menu_main;
			Menu_Draw(menu_main);
			Menu_Item_Select(menu_main, 0);
			message_menu_flag=0;
			Sel_Butt_Pos=0;
		}
	}
}
//----------------------------------------------------------------------------------------------------------------------------


char mess_full[MESS_MAX_SIZE]={0,};

void Message_Read(void){
	EEPROM_ReadStr(Eeprom_Addr_Array[Message_Selected], (uint8_t*)mess_full, MESS_MAX_SIZE);                // + привязка ко времени!
	_CLEAR_MENU_SCREEN;
	TFT_Send_Str(MENU_CON_X, MENU_CON_Y, mess_full, strlen(mess_full), Font_7x9, WHITE, BLACK);
	message_menu_flag=1;
	In_Mess=1;
}

//------------------------------------------------------------------------------------------------------------------------------

extern uint8_t Items_Num;

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

inline void WeekDay_Calc(void){
	uint8_t month, day, year, weekday;
	month=((RTC->DR>>RTC_DR_MU_Pos)&0x0F);
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
	RTC->ISR&=~RTC_ISR_INIT;
	while ((RTC->ISR & RTC_ISR_RSF) != RTC_ISR_RSF);
	RTC->WPR=0xFF;
	
//	delay(5);
	WeekDay_Calc();
}
extern uint8_t signal;
extern char str_temp[];

//-----------------------------------------------------------------------------

extern uint8_t rec_buf[];
extern uint8_t receive_bit_show_flag;
uint8_t dig1=0, dig2=0;
extern uint16_t	byte1, byte2, byte3;
uint16_t	byte;

inline void ShowSignal(void){
	if((byte1&0xFFF)==(byte2&0xFFF) || (byte1&0xFFF)==(byte3&0xFFF)) byte=byte1;
	if((byte2&0xFFF)==(byte3&0xFFF)) byte=byte2;
	if((byte&0xC00)==0xC00){
		if((byte&0x003)==0b11) TFT_Send_Str(0, 100, "Авария-1", 8, Font_11x18, BLACK, WHITE);
		if((byte&0x003)==0b00) TFT_Send_Str(0, 100, "Авария-2", 8, Font_11x18, BLACK, WHITE);
		
		if((byte&0x0F0)==0x030) TFT_Send_Str(0, 118, "Рудник-1", 8, Font_11x18, BLACK, WHITE);
		if((byte&0x0F0)==0x0C0) TFT_Send_Str(0, 118, "Рудник-2", 8, Font_11x18, BLACK, WHITE);
		if((byte&0x0F0)==0x0F0) TFT_Send_Str(0, 118, "Рудник-3", 8, Font_11x18, BLACK, WHITE);
		if((byte&0x0F0)==0x000) TFT_Send_Str(0, 118, "Рудник-4", 8, Font_11x18, BLACK, WHITE);
	}
	
}

//-----------------------------------------------------------------------------


