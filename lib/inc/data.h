#include "gpio.h"


#define SOS_CHANNEL		0
#define LABEL_CHANNEL	2
#define PROGR_CHANNEL	3

#define DOWN_KEY		!(GPIOB->IDR&(1<<9))
#define UP_KEY			!(GPIOA->IDR&(1<<11))
#define BACK_KEY		!(GPIOA->IDR&(1<<12))
#define ENTER_KEY		!(GPIOB->IDR&(1<<8))

#define BATT_REFR_TIME									SystemCoreClock/3
#define TIMESHOW_REFR_TIME							60000
#define RECEIVE_MESSAGE_SHOW_REFR_TIME	1000

#define KEYPRESSED_REFR_TIME						200

enum adc_tim{
	NO_SIG,
	ZERO_SIG,
	ONE_SIG
};

enum adc_in{
	SIGNAL_ADC,
	B_OR_CH_ADC
};

enum adc_in_battery{
	BATTERY_ADC,
	CHARGE_ADC
};


extern volatile int timer1ms;
extern volatile int cctimer;
extern volatile uint16_t t_label;
extern volatile uint32_t t_music;
extern volatile uint8_t MINUTES_TO_SLEEP;  // Количество минут до сна
extern volatile uint32_t SystemTime;
extern volatile uint8_t switch_flag_SOS;
extern uint8_t ccbuf[128];
extern uint8_t flash_buf[64];
extern uint8_t flash_buf_data;
extern uint32_t flash_buf_addr;
extern uint16_t prg_size;
extern uint16_t prg_CRC;
extern uint32_t prg_addr;
extern uint32_t prg_top;
extern uint8_t cc_flag;
extern int offtimeout;
extern uint32_t first_addr;
extern uint8_t lcd_type;

extern uint8_t show_battery_flag;

extern uint32_t batt_refr_time;
extern uint16_t timeshow_refr_time;
extern uint16_t receive_message_show_refr_time;

extern uint16_t keypressed_refr_time;

extern volatile uint8_t adc_in_flag;
extern volatile uint8_t adc_in_battery_flag;
