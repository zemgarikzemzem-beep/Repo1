#include "cc2520.h"
#include "string.h"

#include "gpio.h"
#include "data.h"
#include "eeprom.h"
#include "disp.h"
#include "CRC.h"
#include "flash.h"
#include "alerts.h"


//#define ZOOM if(FLASH_ReadByte(FLASH_SETTINGS_ADDR+ALERT)&(1<<ZOOMER))GPIOA->BSRR|=(1<<9);delay(100);GPIOA->BRR|=(1<<9);

extern void delay(__IO uint32_t tck);

extern uint8_t label_on;

uint16_t current_eeprom_pos=0; // текущая позиция в памяти eeprom
uint16_t mess_num=0;
uint8_t mess_num_byte[2]={0,};

uint16_t label=999;

static uint8_t __attribute__ ((section(".VERS"))) version[6] = {7,25,4,0,1,0};

uint8_t rssi;

// меточные ячейки
uint8_t lrbuf[128], lrctr;
volatile uint16_t cc_tmt;
uint16_t prg;
volatile uint8_t ccrst;					// флаг рестарта чипкона

// приемный буфер
volatile uint16_t rbuf;
//volatile uint8_t rctr, bctr;			// счетчик принимаемых бит и бит для проверки шаблона вызова маяка
volatile uint8_t prg_en;				// разрешение программирования
uint8_t tmp_buf[128];					// временный буфер общего назначения
uint16_t ADDR;// = (uint16_t)&tmp_buf[0];

void echo();

#define SPI_DMA

/*============Посылка-чтение байта из чипкона==============*/

uint8_t spi1_write(uint8_t data)
{
	#ifdef SPI_DMA
	
	uint8_t res, 
			data1=data;
  while ((SPI1->SR & SPI_SR_BSY));
	
	DMA1_Channel3->CCR=0;
	DMA1_Channel3->CPAR=(uint32_t)(&SPI1->DR);
	DMA1_Channel3->CMAR=(uint32_t)(&data1);
	DMA1_Channel3->CNDTR=1;
	DMA1_Channel3->CCR|=(DMA_CCR_DIR);
	DMA1_Channel3->CCR|=DMA_CCR_EN;
	
	DMA1_Channel2->CCR=0;
	DMA1_Channel2->CPAR=(uint32_t)(&SPI1->DR);
	DMA1_Channel2->CMAR=(uint32_t)&res;
	DMA1_Channel2->CNDTR=1;
	DMA1_Channel2->CCR|=DMA_CCR_EN;
  while ((SPI1->SR & SPI_SR_BSY));
	
	return res;
	
	#else
	uint8_t a;
	
  while ((SPI1->SR & SPI_SR_BSY));
  *(__IO uint8_t *)&(SPI1->DR) = data; //
  while (!(SPI1->SR & SPI_SR_RXNE));
  return (uint8_t)(SPI1->DR);
	
	#endif
}

//void spi1_get_buf(uint8_t data, uint8_t bufsize){
//	
//	#ifdef SPI_DMA
//	
//	uint8_t data1=CC2520_INS_SNOP;
//  while ((SPI1->SR & SPI_SR_BSY));
//	
//	for(uint8_t i=1; i<=bufsize; ++i){
//		while ((SPI1->SR & SPI_SR_BSY));
//		DMA1_Channel3->CCR=0;
//		DMA1_Channel3->CPAR=(uint32_t)(&SPI1->DR);
//		DMA1_Channel3->CMAR=(uint32_t)(&data1);
//		DMA1_Channel3->CNDTR=1;
//		DMA1_Channel3->CCR|=(DMA_CCR_DIR);
//		DMA1_Channel3->CCR|=DMA_CCR_EN;
//		
//		while (!(SPI1->SR & SPI_SR_RXNE));
//		DMA1_Channel2->CCR=0;
//		DMA1_Channel2->CPAR=(uint32_t)(&SPI1->DR);
//		DMA1_Channel2->CMAR=(uint32_t)(ccbuf+i);
//		DMA1_Channel2->CNDTR=1;
//		DMA1_Channel2->CCR|=(DMA_CCR_MINC);
//		DMA1_Channel2->CCR|=DMA_CCR_EN;
//	}
//	
//	#else
//	
//	for(uint8_t i=1; i<=bufsize; ++i){
//		while ((SPI1->SR & SPI_SR_BSY));
//		*(__IO uint8_t *)&(SPI1->DR) = data; // 
//		while (!(SPI1->SR & SPI_SR_RXNE));
//		ccbuf[i]=(uint8_t)(SPI1->DR);
//	}
//	
//	#endif
//}

//void spi1_load_buf(uint8_t bufsize){
//	
//	#ifdef SPI_DMA
//	
//  while ((SPI1->SR & SPI_SR_BSY));
//	DMA1_Channel3->CCR=0;
//	DMA1_Channel3->CPAR=(uint32_t)(&SPI1->DR);
//	DMA1_Channel3->CMAR=(uint32_t)(ccbuf);
//	DMA1_Channel3->CNDTR=bufsize;
//	DMA1_Channel3->CCR|=(DMA_CCR_DIR|DMA_CCR_MINC); //msize8, psize16, !MINC
//	DMA1_Channel3->CCR|=DMA_CCR_EN;
//	while ((SPI1->SR & SPI_SR_BSY));
//	
//	#else
//	
//	for(uint8_t i=0; i<bufsize; ++i){
//		while ((SPI1->SR & SPI_SR_BSY));
//		*(__IO uint8_t *)&(SPI1->DR) = ccbuf[i]; // 
//	}
//	
//	#endif
//}

//

//------------------------------------------------------------

/*============Запись регистра чипкона==============*/

uint8_t writeMEM(uint8_t addr, uint8_t data)
{
  uint8_t res;
  CC_CSEN;
  spi1_write(CC2520_INS_MEMWR);
  spi1_write(addr);
  res = spi1_write(data);
  CC_CSDIS;
  return res;
}

//------------------------------------------------------------

/*============Чтение регистра чипкона==============*/

uint8_t readMEM(uint8_t addr)
{
	uint8_t res;
	CC_CSEN;
	spi1_write(CC2520_INS_MEMRD);
	spi1_write(addr);
	res = spi1_write(CC2520_INS_SNOP);
	CC_CSDIS;
	return res;
}
//------------------------------------------------------------

/*============Выполнить инструкцию==============*/

uint8_t writeINS(uint8_t data)
{
  uint8_t res;
  CC_CSEN;
  res = spi1_write(data);
  CC_CSDIS;
  return res;
}


uint8_t cc2520osc_on()
{
  timer1ms = 200;
  writeINS(CC2520_INS_SXOSCON);
  while (!(writeINS(CC2520_INS_SNOP) & 0x80)) {
    if (!timer1ms) return 0;
  }
  return 1;
}

uint8_t cc2520osc_off()
{
  timer1ms = 200;
  writeINS(CC2520_INS_SXOSCOFF);
  while (writeINS(CC2520_INS_SNOP) & 0x80) {
    if (!timer1ms) return 0;
  }
  return 1;
}

//------------------------------------------------------------

/*============Инициализация чипкона==============*/

uint8_t cc_init()
{
  static uint8_t cc2520_initval[] = {CC2520_TXCTRL, 0x94, CC2520_CCACTRL0, 0xF8, CC2520_MDMCTRL0, 0x85, CC2520_MDMCTRL1, 20,
				CC2520_RXCTRL, 0x3F, CC2520_FSCTRL, 0x5A, CC2520_FSCAL1, 0x2B, CC2520_AGCCTRL1, 0x11, CC2520_AGCCTRL2, 0xEB,
				CC2520_EXTCLOCK, 0,	CC2520_ADCTEST0, 0x10,CC2520_ADCTEST1, 0x0E, CC2520_ADCTEST2, 3, CC2520_FIFOPCTRL, 16, 0, 0};
  uint8_t *p = cc2520_initval;
  uint8_t addr, data;
  uint8_t osc_on, osc_off;
  
//  GPIOD->MODER |= 0x11;                         // порты CS3 и RST3 на вывод
//  GPIOD->ODR |= 5;                              // линии в 1
//  GPIOC->MODER |= 0x02A00000;                   // линии 10,11,12 альтернативные функции
//  GPIOC->AFR[1] |= 0x00066600;                  // линии 10,11,12 в AF6
				
	GPIOA->MODER&=~(GPIO_MODER_MODER1|GPIO_MODER_MODER4); // порты CS и RST на вывод
	GPIOA->MODER|=(GPIO_MODER_MODER1_0|GPIO_MODER_MODER4_0);
	GPIOA->OSPEEDR|=(GPIO_OSPEEDER_OSPEEDR1|GPIO_OSPEEDER_OSPEEDR4);
	GPIOA->BSRR|=((1<<1)|(1<<4));

  CC_RES;				// сброс чипкона
  delay(100);
  CC_START;
  
  osc_off = cc2520osc_off();
  osc_on = cc2520osc_on();
  
  if (!(osc_on && osc_off)) {                    // если включить не удалось
    SPI1->CR1 = 0;
    return 0;
  }
//  GPIOD->ODR |= (1<<13);                // для теста, показать чипкон
  cc_flag = 1;                          // выставить флаг чипкона

  while (*p) {
    addr = (*p++);
    data = (*p++); 
    writeMEM(addr, data);
  }
  writeMEM(CC2520_FRMFILT0, 0);
  writeINS(CC2520_INS_SFLUSHRX);
  writeINS(CC2520_INS_SFLUSHTX);
  writeMEM(CC2520_TXCTRL, 0x94);
//  writeMEM(CC2520_TXPOWER, 0xF7);	// мощность будем ставить при переходе на канал
  writeINS(CC2520_INS_SRXON);
  return 1;
}

// загрузка канала

uint8_t chl_load(uint8_t chl)
{	
  chl *= 5; chl += 11;
  writeMEM(CC2520_FREQCTRL, chl);		// загрузить в синтезатор
  cctimer = 100;
  while (!(readMEM(CC2520_FSMSTAT1) & 4)) { // дождаться захвата частоты
    if (!cctimer) return 0;		
  }
  return 1;
}

//------------------------------------------------------------

/*============Прочитать буфер==============*/

uint8_t rbuf_get()
{
  uint8_t *p = ccbuf;
  uint8_t res, ctr;

	
  CC_CSEN;
  res=spi1_write(CC2520_INS_RXBUF);
  ctr = spi1_write(CC2520_INS_SNOP);
  CC_CSDIS;
  res = ctr;

  if ((!ctr) || (ctr > 127)) {
    writeINS(CC2520_INS_SFLUSHRX);
    return 0;
  }
	
  *p++ = ctr;
  CC_CSEN;
  spi1_write(CC2520_INS_RXBUF);
//	spi1_get_buf(1, ctr);
  while (ctr--) *p++ = spi1_write(CC2520_INS_SNOP);
  CC_CSDIS;
  if (((*(p-1)) & 0x80) == 0) return 0;
//  if ((ccbuf[1] != 0x35)||(ccbuf[2] != 0x01)) return 0;
  return res;
}

//------------------------------------------------------------

/*============Загрузка буфера передачи==============*/

void tbuf_load()
{
  uint8_t *p = ccbuf;
  uint8_t ctr = (*p) - 1;				// счетчик равен размеру буфера без одного (RSSI чипкон сам добавит)

  writeINS(CC2520_INS_SFLUSHRX);		// очистить предыдущие данные
  writeMEM(CC2520_EXCFLAG0, 0);		// сброс флагов исключений
  writeMEM(CC2520_EXCFLAG1, 0);
  writeINS(CC2520_INS_SFLUSHTX);		// очистить предыдущие данные
  CC_CSEN;							// загрузка буфера передачи
  spi1_write(CC2520_INS_TXBUF);		// 
	
  while (ctr--) {
    spi1_write(*(p++));
  }
	
  CC_CSDIS;
}

//------------------------------------------------------------

/*============Отправить пакет==============*/

uint8_t tbuf_send()
{
  uint8_t ctr = 100;					// сделать 100 попыток передать
  uint8_t res = 0;					// результат операции
  if (!cc_flag) return 0;
  while (ctr--) {
    writeINS(CC2520_INS_STXONCCA);	// старт передатчика
    timer1ms = 2;
    while (timer1ms) {				// ожидание включения
      if (writeINS(CC2520_INS_SNOP) & 2) {
        res = 1;
        break;
      }
    }     
    if (!res) continue;
    timer1ms = 100;
    while (writeINS(CC2520_INS_SNOP) & 2) {   //ждем выключения передачи
      if (!timer1ms) return 0;
    }
    break;
  }
  return res;
}

//------------------------------------------------------------

/*============Отправить эхо-ответ==============*/

void echo()
{
  tbuf_load();
  tbuf_send();
}

//-------------------------------------------------------------------------------------------------------------------------------------

/*============Отправить подтверждение==============*/

extern uint16_t adc[3];

void send_ack()
{
	if (!cc_flag) return;
  ccbuf[0] = 21;
	ccbuf[1] = 0x28;
	ccbuf[2] = 0x41;
	ccbuf[3] = (uint8_t)(label >> 8);
	ccbuf[4] = (uint8_t)label;
	ccbuf[5] = (uint8_t)(prg >> 8);
	ccbuf[6] = (uint8_t)prg;
	ccbuf[7] = 0;

	ccbuf[8] = version[0];
	ccbuf[9] = version[1];
	ccbuf[10] = version[2];
	ccbuf[11] = version[3];
	ccbuf[12] = version[4];
	ccbuf[13] = 0;
	ccbuf[14] = 0;
	ccbuf[15] = 0;
	ccbuf[16] = 0;
	ccbuf[17] = rssi;
	ccbuf[18] = adc[0] >> 8;
	ccbuf[19] = (uint8_t)adc[0];
	echo();
}

//------------------------------------------------------------

/*============Отправить отрицание==============*/

void send_nack()
{
  if (!cc_flag) return;
  ccbuf[0] = 5;
  ccbuf[1] = 0x35;
  ccbuf[2] = 0x03;
  ccbuf[3] = 0xff;
  echo();
}


//------------------------------------------------------------

/*============Отправка версии==============*/

void send_vers()
{
	//for(uint8_t i=0; i<5; ++i) ccbuf[i]=version[i];
  ccbuf[0] = 7;
  ccbuf[1] = 25;
  ccbuf[2] = 4;
  ccbuf[3] = 0;
  ccbuf[4] = 1;
  ccbuf[5] = 0;
  echo();
}

//------------------------------------------------------------

/*============Отправить метку==============*/

void send_label()
{
//	chl_load(0);						// загрузка меточного канала
	ccbuf[0] = 6;						// загрузка метки в буфер
	ccbuf[1] = 0x28;
	ccbuf[2] = 0x41;
	ccbuf[3] = (uint8_t)(label >> 8);
	ccbuf[4] = (uint8_t)label;
	tbuf_load();
	tbuf_send();
	t_label = (1000 + (readMEM(CC2520_INS_RANDOM) & 0xf)*10); // перезагрузка периода метки 
	writeINS(CC2520_INS_SFLUSHRX);	// очистить буфер
//	chl_load(1);				// загрузка программаторного канала
}

void send_SOS_label()
{
	chl_load(0);
	ccbuf[0] = 6;						// загрузка метки в буфер
	ccbuf[1] = 0x00;
	ccbuf[2] = 0x41;
	ccbuf[3] = (uint8_t)((label) >> 8);   // Поднятый 13-й бит у метки - сигнал SOS +(1<<13)
	ccbuf[4] = (uint8_t)(label); // +(1<<13)
	tbuf_load();
	tbuf_send();
	t_label = (1000 + (readMEM(CC2520_INS_RANDOM) & 0xf)*10); // перезагрузка периода метки 
	writeINS(CC2520_INS_SFLUSHRX);	// очистить буфер
}

//------------------------------------------------------------------------------------------

/*============Проверка заголовка==============*/

uint8_t chk_header(void){
	if (ccbuf[5] | ccbuf[6]) return 0;
	switch (ccbuf[7]) {
		case 0: if (ccbuf[0] != 9) return 0; break; // проверка связи
		case 1: if (ccbuf[0] != 9) return 0; break;	// вызов в программатор программы
		case 2: if (ccbuf[0] != 9) return 0; break;	// вызов в программатор бутлодера
		case 3: if (ccbuf[0] != 0x4B) return 0; break; // получение ПЛА
		case 4:	if (ccbuf[0] != 0x4B) return 0; break; // 10 байт(преамбула) + 64(пакет) + 1(чек-сумма)
		case 5: if (ccbuf[0] != 9) return 0; break;
		case 6: if (ccbuf[0] != 0xB) return 0; break; // 8 байт(преамбула) + 2(номер пакета)
		case 7: if (ccbuf[0] != 9) return 0; break;
		case 8: if (ccbuf[0] != 9) return 0; break;
		default: return 0;
	}

	if (ccbuf[1] == 0x28) {   // 0x28-0x2C - пейджеры
		if (ccbuf[2] == 0x41) {
			if (ccbuf[3] == (label >> 8)) {
				if (ccbuf[4] == (uint8_t)label) return 1;
			}
		}
	}
	return 0;
}


//----------------------------------------------------------------------------------

/*============Чтение и отправка настроек из памяти==============*/

void data_read()
{
	uint16_t addr = ccbuf[9] + ((uint16_t)ccbuf[8] << 8);
	ccbuf[0] = 6+SETTINGS_BYTES_NUM;
	ccbuf[1] = 0x28;
	ccbuf[2] = 0x41;
//	for(uint8_t i=0; i<SETTINGS_BYTES_NUM;++i) ccbuf[5+i]=EEPROM_ReadByte(EEPROM_SETTINGS_ADDR+i);
	FLASH_ReadStr(FLASH_SETTINGS_ADDR, ccbuf+5, SETTINGS_BYTES_NUM);
	echo();
	
	TFT_Fill_Area(0, 100, DISP_WIDTH, DISP_HEIGHT, BLACK);
	TFT_Send_Str(0, 110, "Получение  настроек...", 22, Font_11x18, 0xE7E7, BLACK);
}

//------------------------------------------------------------------------------------

/*============Сохранение присланных данных в памяти==============*/

void save_data()
{
//	TIMSK1 = 0;
//	asm (
//		"lds	r26,ADDR\n"
//		"lds	r27,ADDR+1\n"
//		"ldi	r30,0\n"
//		"ldi	r31,0x37\n"
//		"call	0x386a"	
//	);
//	TIMSK1 = 1<<ICIE1;
//	EEPROM_SendStr(current_eeprom_pos, tmp_buf, strlen((const char*)tmp_buf));
//	current_eeprom_pos+=128;
//	
//	mess_num++;
//	mess_num_byte[0]=mess_num>>8;
//	mess_num_byte[1]=mess_num&0xFF;
//	EEPROM_SendStr(EEPROM_SETTINGS_ADDR+PLA_ITEMS_NUM_1, mess_num_byte, 2);
//	
//	ZOOM;
}

//------------------------------------------------------------------------------------

/*============Сохранение присланных данных в памяти==============*/

//uint16_t label1=0;

void data_write()
{
	int sum = 0;
//	uint16_t addr = ccbuf[8];
//	addr <<= 8;
//	addr |= ccbuf[9];
	memcpy(&tmp_buf[0], &ccbuf[10], SETTINGS_BYTES_NUM);
	for (uint8_t i = 0; i < 64; i++) {
		sum += ccbuf[i + 10];
		if (sum > 0xff) sum -= 0xff;
	}
	
//	if(ccbuf[75]!=sum) return;
	
//	label1=ccbuf[10]+(ccbuf[11]<<8);
	
	ccbuf[0] = 12;
	ccbuf[1] = 0x28;
	ccbuf[10] = (uint8_t)sum;
	echo();
//	if (addr == 0x40) save_data();
	
	FLASH_WriteStr(FLASH_SETTINGS_ADDR, tmp_buf, SETTINGS_BYTES_NUM);
//	EEPROM_SendStr(EEPROM_SETTINGS_ADDR, tmp_buf, strlen((const char*)tmp_buf));
	
	TFT_Fill_Area(0, 100, DISP_WIDTH, DISP_HEIGHT, BLACK);
	TFT_Send_Str(0, 110, "Настройки  записаны!", 20, Font_11x18, 0xBFBF, BLACK);
	
	ALERT_ForNotification();
}

//------------------------------------------------------------------------------------

/*============Прыжок в бутлоадер==============*/

void goto_boot()
{
	uint32_t app_jump_address;
	
  typedef void(*pFunction)(void); //объявляем пользовательский тип
  pFunction Jump_To_Application; //и создаём переменную этого типа
	
	uint8_t prg_write=1;
	FLASH_WriteStr(FLASH_SETTINGS_ADDR+PRG_WRITE, &prg_write, 1);
	
	ccbuf[1] = 0;
	echo();
	
	__disable_irq();
	
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
	
	memcpy((void*)0x20000000, (const void*)BOOTLOADER_ADDRESS, 0xC0);
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
    // remap memory to 0 (only for STM32F0)
  SYSCFG->CFGR1 = 0b00; __DSB(); __ISB();
	__enable_irq();
	
	app_jump_address = *( uint32_t*) (BOOTLOADER_ADDRESS+4);    // извлекаем адрес перехода из вектора Reset
  Jump_To_Application = (pFunction)app_jump_address;            //приводим его к пользовательскому типу
  __set_MSP(*(__IO uint32_t*) BOOTLOADER_ADDRESS);          //устанавливаем SP приложения                                           
  Jump_To_Application();
}

//------------------------------------------------------------------------------------

/*============Установка режима программирования==============*/

void prg_on()
{
	if (!prg_en) prg_en = 1;
	mess_num=0;    // Чтобы не перезагружать перед заливкой ПЛА
	send_ack();
	
	TFT_Fill_Area(0, 100, DISP_WIDTH, DISP_HEIGHT, BLACK);
	TFT_Send_Str(0, 120, "Есть ответ!", 11, Font_11x18, GREEN, BLACK);
}

//------------------------------------------------------------------------------------

/*============Сброс режима программирования==============*/

void prg_off()
{
	send_ack();
	prg_en = 0;
	
	
	label=FLASH_ReadByte(FLASH_SETTINGS_ADDR+PAGER_NUM_LB)+(FLASH_ReadByte(FLASH_SETTINGS_ADDR+PAGER_NUM_HB)<<8);
	
	FLASH_ReadStr(FLASH_SETTINGS_ADDR+PLA_ITEMS_NUM_1, mess_num_byte, 2);
	mess_num = (mess_num_byte[0]<<8)+(mess_num_byte[1]&0xFF);
	current_eeprom_pos=0;
	
	CRC->CR |= CRC_CR_RESET;
	
	TFT_Fill_Area(0, 100, DISP_WIDTH, DISP_HEIGHT, BLACK);
	TFT_Send_Str(0, 120, "Завершено.", 10, Font_11x18, LIGHTBLUE, BLACK);
}

//------------------------------------------------------------------------------------

/*============Получить ПЛА==============*/

uint32_t Pla_CRC=0;
uint32_t current_flash_addr_pla=0;
uint8_t EEPROM_MAXIMAL_SEND=200;

void get_pla(void)
{
	if(!(ccbuf[8]|ccbuf[9])){
		TFT_Fill_Area(0, 100, DISP_WIDTH, DISP_HEIGHT, BLACK);
		TFT_Send_Str(0, 100, "Идёт приём ПЛА!", 15, Font_11x18, 0xEAEA, BLACK);
		mess_num=0;
	}
	int sum = 0;
	uint16_t addr = (ccbuf[8]<<8)+ccbuf[9];
	
	for (uint8_t i = 0; i < 64; i++) {
		sum += ccbuf[i + 10];
		if (sum > 0xff) sum -= 0xff;
	}
	
	if(addr!=flash_buf_addr){  //  && addr%MESS_MAX_SIZE==0x40
		if(tmp_buf[0]||tmp_buf[1]){
			FLASH_WriteStr_Mess(FLASH_PLA_ADDR+flash_buf_addr, tmp_buf, MESS_MAX_SIZE/2);
		}
		flash_buf_addr=addr;
		++mess_num;
		memcpy(tmp_buf, ccbuf+10, 64);
	}
	else memcpy(tmp_buf, ccbuf+10, 64);
	
	if(!(tmp_buf[0]||tmp_buf[1])){
		mess_num/=2;
		mess_num_byte[0]=mess_num>>8;
		mess_num_byte[1]=mess_num&0xFF;
		FLASH_WriteStr(FLASH_SETTINGS_ADDR+PLA_ITEMS_NUM_1, mess_num_byte, 2);
		
		Pla_CRC=0;
		for(uint16_t i=0; i<mess_num;++i){
			FLASH_ReadStr(FLASH_PLA_ADDR+i*MESS_MAX_SIZE, tmp_buf, MESS_MAX_SIZE);
			Pla_CRC=CRC_Result(tmp_buf, strlen((const char*)tmp_buf));
		}
		FLASH_WriteStr(FLASH_SETTINGS_ADDR+PLA_CRC_0, (uint8_t*)&Pla_CRC, 4);
		Pla_CRC=0;
		
		TFT_Fill_Area(0, 100, DISP_WIDTH, DISP_HEIGHT, BLACK);
		TFT_Send_Str(0, 100, "ПЛА успешно записан!", 20, Font_11x18, GREEN, BLACK);
//		ALERT_ForNotification();
	}
	
	ccbuf[0] = 12;
	ccbuf[1] = 0x28;
	ccbuf[10] = (uint8_t)sum;
	echo();
	
//	if (addr%MESS_MAX_SIZE==0x40 && (tmp_buf[0]||tmp_buf[1])){
//	
//		if(mess_num%EEPROM_MAXIMAL_SEND==0 && mess_num!=0){																			// Если число сообщений в EEPROM достигло определённого значения - сливаем во FLASH
//			uint8_t mess_buf[MESS_MAX_SIZE]={0,};
//			current_flash_addr_pla=FLASH_PLA_ADDR+(mess_num-EEPROM_MAXIMAL_SEND)*MESS_MAX_SIZE;
//			for(uint16_t i=0; i<EEPROM_MAXIMAL_SEND; ++i){
//				EEPROM_ReadStr(EEPROM_MESSAGES_ADDR+i*MESS_MAX_SIZE, mess_buf, MESS_MAX_SIZE);
//				FLASH_WriteStr_Mess(current_flash_addr_pla+i*MESS_MAX_SIZE, mess_buf, strlen((const char*)mess_buf)+1);
//			}
//		}
////		FLASH_WriteStr_Mess(FLASH_PLA_ADDR+(addr/MESS_MAX_SIZE)*MESS_MAX_SIZE, tmp_buf, strlen((const char*)tmp_buf)+1);
//		EEPROM_SendStr_PLA(EEPROM_MESSAGES_ADDR+(mess_num%EEPROM_MAXIMAL_SEND)*MESS_MAX_SIZE, tmp_buf, strlen((const char*)tmp_buf)); // (addr/MESS_MAX_SIZE)*MESS_MAX_SIZE
////		current_eeprom_pos+=128;
////				Pla_CRC=CRC_Result(tmp_buf, strlen((const char*)tmp_buf));
////				EEPROM_SendStr(EEPROM_SETTINGS_ADDR+PLA_CRC_0, (uint8_t*)&Pla_CRC, 4);
//		
//		
//		mess_num++;
//	}
//	
//	if(addr%MESS_MAX_SIZE==0x40 && !(tmp_buf[0]||tmp_buf[1])){
//		
//		mess_num_byte[0]=mess_num>>8;
//		mess_num_byte[1]=mess_num&0xFF;
//		
//		FLASH_WriteStr(FLASH_SETTINGS_ADDR+PLA_ITEMS_NUM_1, mess_num_byte, 2);
//		
//		if(mess_num<EEPROM_MAXIMAL_SEND) current_flash_addr_pla=FLASH_PLA_ADDR;
//		else current_flash_addr_pla=FLASH_PLA_ADDR+((mess_num-1)-(mess_num-1)%EEPROM_MAXIMAL_SEND)*MESS_MAX_SIZE;
//		
////		uint8_t mess_buf[MESS_MAX_SIZE]={0,};
//		
//		for(uint16_t i=0; i<mess_num%EEPROM_MAXIMAL_SEND; ++i){
//			EEPROM_ReadStr(EEPROM_MESSAGES_ADDR+i*MESS_MAX_SIZE, tmp_buf, MESS_MAX_SIZE);
////			Pla_CRC=CRC_Result(tmp_buf, strlen((const char*)tmp_buf));
////			EEPROM_SendStr(EEPROM_SETTINGS_ADDR+PLA_CRC_0, (uint8_t*)&Pla_CRC, 4);
//			FLASH_WriteStr_Mess(current_flash_addr_pla+i*MESS_MAX_SIZE, tmp_buf, strlen((const char*)tmp_buf)+1); // 
//		}
//		
//		for(uint16_t i=0; i<mess_num;++i){
//			FLASH_ReadStr(FLASH_PLA_ADDR+i*MESS_MAX_SIZE, tmp_buf, MESS_MAX_SIZE);
//			Pla_CRC=CRC_Result(tmp_buf, strlen((const char*)tmp_buf));
//		}
//		FLASH_WriteStr(FLASH_SETTINGS_ADDR+PLA_CRC_0, (uint8_t*)&Pla_CRC, 4);
//		Pla_CRC=0;
//		
//		TFT_Fill_Area(0, 100, DISP_WIDTH, DISP_HEIGHT, BLACK);
//		TFT_Send_Str(0, 100, "ПЛА успешно записан!", 20, Font_11x18, GREEN, BLACK);
////		ALERT_FOR_MESSAGE;
//		ALERT_ForNotification();
//	}
}

//------------------------------------------------------------------------------------

void send_PLA_CRC(void){
//	uint32_t Pla_CRC=0;
	
	CRC->CR |= CRC_CR_RESET;
	
	FLASH_ReadStr(FLASH_SETTINGS_ADDR+PLA_ITEMS_NUM_1, mess_num_byte, 2);
	mess_num = (mess_num_byte[0]<<8)+(mess_num_byte[1]&0xFF);
	ccbuf[0] = 12;
	ccbuf[1] = 0x28;
	ccbuf[2] = 0x41;
	for(uint16_t i=0; i<mess_num;++i){
		FLASH_ReadStr(FLASH_PLA_ADDR+i*MESS_MAX_SIZE, tmp_buf, MESS_MAX_SIZE);
		Pla_CRC=CRC_Result(tmp_buf, strlen((const char*)tmp_buf));
	}
	ccbuf[7]=Pla_CRC&0xFF;
	ccbuf[8]=(Pla_CRC>>8)&0xFF;
	ccbuf[9]=(Pla_CRC>>16)&0xFF;
	ccbuf[10]=(Pla_CRC>>24)&0xFF;
	echo();
	
	TFT_Fill_Area(0, 100, DISP_WIDTH, DISP_HEIGHT, BLACK);
	TFT_Send_Str(0, 110, "Идёт расчёт CRC ПЛЛПА.", 22, Font_11x18, 0xE7E7, BLACK);
}

//------------------------------------------------------------------------------------

/*============Выбор действий в зависимости от запроса программатора==============*/

void program(void){
	if (!rbuf_get()) return;
	if (!chk_header()) return;
//	label_on=0;
	prg = ccbuf[5] << 8;
	prg |= ccbuf[6];
	rssi = ccbuf[8];				// RSSI для ACK

	switch (ccbuf[7]) {
		case 0: send_ack();	break;				// проверка связи
		case 1: prg_on(); break;				// вызов в программатор программы
		case 2: goto_boot();					// вызов в программатор бутлодера
		case 3: if (!prg_en) return; get_pla(); break; // запись ПЛА в память
		case 4:	if (!prg_en) return; data_write(); break; // запись настроек только из режима программирования
		case 5: goto_boot(); break;																																								// надо нажимать кнопку включения !!!
		case 6: data_read(); break; // чтение настроек
		case 7: prg_off();  break; // label_on=1;
		case 8: send_PLA_CRC(); break;
		default: send_nack();
	}
//	label_on=1;
	offtimeout = 10000;
}
