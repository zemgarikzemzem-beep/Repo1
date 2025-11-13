#include "stm32f0xx.h"                  // Device header
#include "string.h"
#include <stdio.h>

#include "rcc.h"
#include "data.h"
#include "tim.h"
#include "gpio.h"
#include "chipcon.h"
#include "flash.h"
#include "disp.h"
#include "spi.h"
#include "fonts.h"
#include "cc2520.h"

 // 

//#define REC_VIEW    // Отображать принятые от программатора байты

unsigned int prgbuf[0x40000];

//----------------------------------------------------------

void delay(__IO uint32_t tck) // Задержка в мс (примерно)
{
	tck*=3000;
  while(tck)
  {
    tck--;
  }  
}

//------------------------------------------------------------------------------------

/*============Проверка контрольной суммы==============*/

uint8_t prg_check()
{
  uint32_t sum = 0;
  uint8_t *p = (uint8_t*)APPLICATION_ADDRESS;
//  if (prg_size > 65535) return 0;
  for (int i = 0; i < (prg_size); i++) {   //   * 64
		if(i==prg_size-2 || i==prg_size-1) sum+=0xFF;
		else sum += (uint8_t)*(p++);
    if (sum > 0xffff) sum -= 0xffff;
  }

  return (sum == prg_CRC)? 1:0;
}

//------------------------------------------------------------------------------------

/*============Прыжок в основную программу==============*/

void GotoApp(void){

  __disable_irq();//запрещаем прерывания
	SysTick->CTRL = 0;
  SysTick->LOAD = 0;
  SysTick->VAL = 0;
	
	for (uint8_t i=0;i<3;i++){
	  NVIC->ICER[i]=0xFFFFFFFF;
	  NVIC->ICPR[i]=0xFFFFFFFF;
  }
	
//	
	SET_BIT(RCC->CR, RCC_CR_HSION | RCC_CR_HSITRIM_4);
	while(READ_BIT(RCC->CR, RCC_CR_HSIRDY) == RESET);
	CLEAR_BIT(RCC->CFGR, RCC_CFGR_SW | RCC_CFGR_HPRE | RCC_CFGR_PPRE | RCC_CFGR_MCO);
	while (READ_BIT(RCC->CFGR, RCC_CFGR_SWS) != RESET);
	SystemCoreClock = 8000000;
	CLEAR_BIT(RCC->CR, RCC_CR_PLLON | RCC_CR_CSSON | RCC_CR_HSEON);
	CLEAR_BIT(RCC->CR, RCC_CR_HSEBYP);
	while(READ_BIT(RCC->CR, RCC_CR_PLLRDY) != RESET);
	CLEAR_REG(RCC->CFGR);
  CLEAR_REG(RCC->CFGR2);
  CLEAR_REG(RCC->CFGR3);
  CLEAR_REG(RCC->CIR);
	RCC->CSR |= RCC_CSR_RMVF;
	
	RCC->APB1RSTR = 0xFFFFFFFFU;
	RCC->APB1RSTR = 0x00000000U;
	RCC->APB2RSTR = 0xFFFFFFFFU;
	RCC->APB2RSTR = 0x00000000U;
	RCC->AHBRSTR = 0xFFFFFFFFU;
	RCC->AHBRSTR = 0x00000000U;

	memcpy((void*)0x20000000, (const void*)APPLICATION_ADDRESS, 0xC0);
	
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
    // remap memory to 0 (only for STM32F0)
  SYSCFG->CFGR1 = 0b11; __DSB(); __ISB();
	__enable_irq();
	
	__ASM volatile("msr msp, %0 \n bx %1" :: "r" (*(__IO uint32_t*) APPLICATION_ADDRESS), "r" (*( uint32_t*) (APPLICATION_ADDRESS+4)));
}

//------------------------------------------------------------------------------------

/*============Перезагрузка с сохранением последней присланной строки и атрибутов(CRC и размер)==============*/

void restart()
{
	
  uint8_t *p = (uint8_t*)APPLICATION_ADDRESS;
  uint32_t sum = 0;
	
	uint8_t prg_write=0;
	FLASH_WriteStr(FLASH_PRGFLAG_ADDR, &prg_write, 1);
  
 // if (prg_size > 16384) return;

  if (flash_buf_data) {
    flash_buf_save();
    Flash_lock();
    flash_buf_data = 0;
  }
	
	prg_CRC=flash_buf[62]+(flash_buf[63]<<8);
	prg_size=flash_buf_addr+0x40;
	
  for (int i = 0; i < (prg_size); i++) {   // проверка загруженной программы   * 64
		if(i==prg_size-2 || i==prg_size-1) sum+=0xFF;
		else sum += (uint8_t)*(p++);
    if (sum > 0xffff) sum -= 0xffff;
  }

  if (sum != prg_CRC) return;

  flash_attr_save();
  send_ack();                          
  timer1ms = 100;
  while (timer1ms);
	
	GPIOA->BSRR|=(1<<9);
	delay(100);
	GPIOA->BRR|=(1<<9);
	
	GotoApp();
}

//----------------------------------------------------------

int main(void){
	
  uint16_t* p = (uint16_t*)FLASH_CRC_ADDR;
	uint8_t prg_write=*(__IO uint8_t*)(FLASH_PRGFLAG_ADDR);
	uint8_t sdfb=*(__IO uint8_t*)(FLASH_SDFB_ADDR);
	
	Clock_Init(); // Инициализация тактирования
	
//	FLASH_WriteStr(FLASH_CRC_ADDR, &prg_CRC, 4);
  prg_size = *p++;
  prg_CRC = *p;
	
	GPIO_Init(); // Инициализация пинов ввода-вывода
	
	if (prg_write!=1 && sdfb) {GPIOA->BRR|=(1<<15);delay(1000);}	// Фишка для включения с кнопки (чтобы перезагрузка не вырубала пейджер)
	FLASH_WriteByte(FLASH_SDFB_ADDR, 0);
	
//	TIM6_Init();  // Инициализация таймера кнопок
	TIM3_Init();  // Инициализация таймера таймаутов и обработки кнопки выключения
	SPI1_Init(); // Инициализация SPI для общения с чипконом
	SPI2_Init(); // Инициализация SPI для вывода на дисплей
	TFT_Init(); // Инициализация дисплея
	
//	TFT_Fill_Color(BLACK); // Заливка дисплея чёрным
	
	offtimeout = 30000;
  timer1ms = 1000;
	
  if (prg_write!=1) {                       // Если контрольная сумма совпадает, при зажатой несколько секунд кнопке "Вверх" вызывается режим бутлоадера // prg_check() && 
    while (timer1ms) {
      if ((GPIOA->IDR&(1<<11))) GotoApp(); //  && (GPIOB->IDR&(1<<9))
    }
  }
	
	GPIOA->BSRR|=(1<<15);
	
	uint8_t prg_flag=0;
	FLASH_WriteStr(FLASH_PRGFLAG_ADDR, &prg_flag, 1);
	
	TFT_Fill_Color(YELLOW); // Заливка экрана жёлтым
	
	cc_init(); // Инициализация чипкона
	
	if (cc_flag){
		chl_load(LABEL_CHANNEL);
		send_label();													// Передача метки в эфир на втором канале
		chl_load(PROGR_CHANNEL);                          // Переход на программирующий канал
	}
	
#ifdef REC_VIEW
	
	while(1){
		if(!t_label){
			chl_load(0);
			send_label();
			chl_load(1);
		}
		
		if (cc_flag)
    if (GPIOB->IDR & (1<<0)) {
      if (rbuf_get()) {
				TFT_Fill_Color(YELLOW);
				for(uint8_t i=0; i<35; ++i){
					sprintf(str, "%2X", ccbuf[i]);
					TFT_Send_Str((i%5)*24, i/5*20, str, strlen(str), Font_11x18, RED, YELLOW);
				}
			}
		}
	}
	
#endif

	if (cc_flag) {
		TFT_Send_Str(10, 50, "СУБР ПОИСК", 10, Font_11x18, RED, YELLOW);
		TFT_Send_Str(10, 80, "БУТЛОАДЕР", 9, Font_11x18, RED, YELLOW);
		TFT_Send_Str(30, 110, "V 2.0", 5, Font_11x18, RED, YELLOW);
  }
  else {
		TFT_Send_Str(10, 80, "НЕТ РАДИОБЛОКА", 14, Font_11x18, RED, YELLOW);
  }
	

	
	while(1){
		if(!t_label){
			chl_load(LABEL_CHANNEL);
			send_label();													// Передача метки в эфир на втором канале
			chl_load(PROGR_CHANNEL);
		}
		
//--------------------------------------------------------------------------------------------------

/*============Выбор действия в зависимости от строки, принятой от программатора==============*/
		
		if (cc_flag)
    if (GPIOB->IDR & (1<<0)) {
      if (rbuf_get() && chk_header()) {
        switch (ccbuf[7]) {
          case 0: if (ccbuf[0] == 9) send_ack(); break;
          case 1: if (ccbuf[0] == 9) send_ack(); break;
          case 2: if (ccbuf[0] == 5) send_vers(); break;
          case 3: if (ccbuf[0] == 7) write_prg_size(); break;
          case 4: if (ccbuf[0] == 7) write_prg_CRC(); break;
          case 5: if (ccbuf[0] == 75) write_program(); break;
          case 7: if (ccbuf[0] == 9) {
            restart();
            send_nack();
          }
          default: send_nack();
        }
        offtimeout = 30000; // При простое больше 3-х минут - выключение устройства
      }
    }
  }
}

void HardFault_Handler(void){
	uint8_t i=1;
	while(i);
}
