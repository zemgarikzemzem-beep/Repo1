#ifndef TFTMENU_H_
#define TFTMENU_H_
#include "stm32f0xx.h"                  // Device header

#define MENU_CON_X 						20
#define MENU_CON_Y 						100
#define MENU_FONT_HEIGH 			18
#define MAX_MENU_ITEM_SCREEN	3

#define _CLEAR_MENU_SCREEN    TFT_Fill_Area(0, MENU_CON_Y, DISP_WIDTH, DISP_HEIGHT, BLACK)

struct Menu_Struct{
	uint8_t menu_size;
	const char** menu_n;
};

struct Menu_Item{
	char* name;
	uint8_t value;
	const struct Menu_Item* submenu;
	const struct Menu_Item* premenu;
};

enum main_menu_items{
	RECEIVE,
	SETTINGS,
	MESSAGES,
	ITEM2,
	ITEM3,
	ITEM4
};

enum menu_mess_items{
	READ_MESS,
	DELETE_MESS
};

enum menu_sett_items{
	BRIGHTNESS,
	TIME,
	INVERSION
};

enum menu_time_items{
	HOURS,
	MINUTES,
	DAY,
	MONTH,
	YEAR,
	WEEKDAY
};

extern const struct Menu_Item menu_main[];
extern const struct Menu_Item menu_mess[];
extern const struct Menu_Item menu_settings[];
extern const struct Menu_Item menu_inversion[];
extern const struct Menu_Item menu_brightness[];
extern const struct Menu_Item menu_time[];
extern struct Menu_Struct menu1;

void Buttons_Init(void);
void Menu_Draw(const struct Menu_Item* menu);
void Menu_Item_Select(const struct Menu_Item* menu, uint8_t item_num);
void Menu_Item_Unselect(const struct Menu_Item* menu, uint8_t item_num);
void Mess_Menu_Draw(void);
void Message_Menu_Navigation(void);
void Message_Read(void);
void Menu_Down(void);
void Menu_Up(void);
void Hour_Plus(void);
void Min_Plus(void);
void Day_Plus(void);
void Month_Plus(void);
void Year_Plus(void);
void WeekDay_Plus(void);
void WeekDay_Calc(void);
void ShowSignal(void);

#endif /* TFTMENU_H_ */
