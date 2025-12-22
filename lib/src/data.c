//#include "main.h"
//#include "stm32f4xx_it.h"
//#include "string.h"
#include "gpio.h"
//#include "stdio.h"
//#include "user.h"
#include "data.h"

volatile int timer1ms;
volatile int cctimer;
volatile uint16_t t_label=1000;
volatile uint32_t t_music=0;
volatile uint8_t switch_flag_SOS = 0;
volatile uint8_t MINUTES_TO_SLEEP=5;  // Количество минут до сна
volatile uint32_t SystemTime=0;

uint8_t flash_buf[64]={0,};          // временный буфер страницы
uint8_t flash_buf_data = 0;
uint32_t flash_buf_addr=0;        // адрес flash, в который предназначается блок

uint16_t prg_size = 0;          // размер программы в блоках
uint16_t prg_CRC=0;               // контрольная сумма программы
uint32_t first_addr = 0xffffffff;
uint32_t prg_addr;              // адрес текущего блока
uint32_t prg_top;               // адрес конца блока
uint8_t *flash_ptr;
uint8_t cc_flag = 0;
uint8_t locked = 1;
uint8_t ccbuf[128];
int offtimeout = 180000;           // таймаут выключения
uint8_t lcd_type = 1;           // временно тип дисплея 7789 SPI

uint8_t show_battery_flag=0;

uint16_t batt_refr_time=2;
uint16_t timeshow_refr_time=1000;
uint16_t receive_message_show_refr_time=2;

uint16_t keypressed_refr_time=2;
