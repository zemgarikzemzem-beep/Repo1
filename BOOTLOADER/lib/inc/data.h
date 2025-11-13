#include "gpio.h"

#define LABEL_CHANNEL	2
#define PROGR_CHANNEL	3

extern volatile int timer1ms;
extern volatile int cctimer;
extern volatile uint16_t t_label;
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
