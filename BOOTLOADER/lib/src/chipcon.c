#include "cc2520.h"
#include "gpio.h"
#include "data.h"

extern void delay(__IO uint32_t tck);

void echo();

#define SPI1_DMA

uint8_t spi1_write(uint8_t data)
{
	#ifdef SPI1_DMA
	
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

void spi1_get_buf(uint8_t data, uint8_t bufsize){
	
	#ifdef SPI_DMA
	
	uint8_t data1=CC2520_INS_SNOP;
  while ((SPI1->SR & SPI_SR_BSY));
	
	for(uint8_t i=1; i<=bufsize; ++i){
		while ((SPI1->SR & SPI_SR_BSY));
		DMA1_Channel3->CCR=0;
		DMA1_Channel3->CPAR=(uint32_t)(&SPI1->DR);
		DMA1_Channel3->CMAR=(uint32_t)(&data1);
		DMA1_Channel3->CNDTR=1;
		DMA1_Channel3->CCR|=(DMA_CCR_DIR);
		DMA1_Channel3->CCR|=DMA_CCR_EN;
		
		while (!(SPI1->SR & SPI_SR_RXNE));
		DMA1_Channel2->CCR=0;
		DMA1_Channel2->CPAR=(uint32_t)(&SPI1->DR);
		DMA1_Channel2->CMAR=(uint32_t)(ccbuf+i);
		DMA1_Channel2->CNDTR=1;
		DMA1_Channel2->CCR|=(DMA_CCR_MINC);
		DMA1_Channel2->CCR|=DMA_CCR_EN;
	}
	
	#else
	
	for(uint8_t i=1; i<=bufsize; ++i){
		while ((SPI1->SR & SPI_SR_BSY));
		*(__IO uint8_t *)&(SPI1->DR) = data; // 
		while (!(SPI1->SR & SPI_SR_RXNE));
		ccbuf[i]=(uint8_t)(SPI1->DR);
	}
	
	#endif
}

void spi1_load_buf(uint8_t bufsize){
	
	#ifdef SPI_DMA
	
  while ((SPI1->SR & SPI_SR_BSY));
	DMA1_Channel3->CCR=0;
	DMA1_Channel3->CPAR=(uint32_t)(&SPI1->DR);
	DMA1_Channel3->CMAR=(uint32_t)(ccbuf);
	DMA1_Channel3->CNDTR=bufsize;
	DMA1_Channel3->CCR|=(DMA_CCR_DIR|DMA_CCR_MINC); //msize8, psize16, !MINC
	DMA1_Channel3->CCR|=DMA_CCR_EN;
	while ((SPI1->SR & SPI_SR_BSY));
	
	#else
	
	for(uint8_t i=0; i<bufsize; ++i){
		while ((SPI1->SR & SPI_SR_BSY));
		*(__IO uint8_t *)&(SPI1->DR) = ccbuf[i]; // 
	}
	
	#endif
}

// запись регистра чипкона

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

// чтение регистра чипкона

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

// выполнить инструкцию

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
  writeMEM(CC2520_TXPOWER, 0xF7);	// мощность будем ставить при переходе на канал
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

// прочитать буфер

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
// загрузка буфера передачи

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

// отправить пакет

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
    while (writeINS(CC2520_INS_SNOP) & 2) {; //ждем выключения передачи
      if (!timer1ms) return 0;
    }
    break;
  }
  return res;
}

// отправить подтверждение

void echo()
{					// эхо-ответ
  tbuf_load();
  tbuf_send();
}

//void send_ack()
//{
//  if (!cc_flag) return;
//  ccbuf[0] = 5;
//  ccbuf[1] = 0x35;
//  ccbuf[2] = 0x03;
//  ccbuf[3] = 0;
//  echo();
//}

// отправить отрицание

void send_nack()
{
  if (!cc_flag) return;
  ccbuf[0] = 5;
  ccbuf[1] = 0x35;
  ccbuf[2] = 0x03;
  ccbuf[3] = 0xff;
  echo();
}

static uint8_t __attribute__ ((section(".VERS"))) version[6] = {7,25,4,0,1,0};

void send_vers()
{
	for(uint8_t i=0; i<5; ++i) ccbuf[i]=version[i];
//  ccbuf[0] = 7;
//  ccbuf[1] = 25;
//  ccbuf[2] = 4;
//  ccbuf[3] = 0;
//  ccbuf[4] = 1;
//  ccbuf[5] = 0;
  echo();
}

// отправить метку
//uint16_t label=999;
uint16_t label=0;
void send_label()
{
//	chl_load(0);						// загрузка меточного канала
	ccbuf[0] = 6;						// загрузка метки в буфер
	ccbuf[1] = 0x28;
	ccbuf[2] = 0x41;
	label=*(__IO uint8_t*)(FLASH_LABEL_ADDR)+(*(__IO uint8_t*)(FLASH_LABEL_ADDR+1)<<8);
	ccbuf[3] = (uint8_t)(label >> 8);
	ccbuf[4] = (uint8_t)label;
	tbuf_load();
	tbuf_send();
	t_label = (2000 + (readMEM(CC2520_INS_RANDOM) & 0xf)); // перезагрузка периода метки
	writeINS(CC2520_INS_SFLUSHRX);	// очистить буфер
//	chl_load(1);				// загрузка программаторного канала
}

uint8_t chk_header(void){
	if (ccbuf[5] | ccbuf[6]) return 0;
//	switch (ccbuf[7]) {
//		case 0: if (ccbuf[0] != 9) return 0; break; // проверка связи
//		case 1: if (ccbuf[0] != 9) return 0; break;	// вызов в программатор программы
//		case 2: if (ccbuf[0] != 9) return 0; break;	// вызов в программатор бутлодера
//		case 3: if (ccbuf[0] != 9) return 0; break;
//		case 4:	if (ccbuf[0] != 75) return 0; break;
//		case 5: if (ccbuf[0] != 9) return 0; break;
//		case 6: if (ccbuf[0] != 11) return 0; break;
//		case 7: if (ccbuf[0] != 9) return 0; break;
//		default: return 0;
//	}

	if (ccbuf[1] == 0x28) {
		if (ccbuf[2] == 0x41) {
			if (ccbuf[3] == (label >> 8)) {
				if (ccbuf[4] == (uint8_t)label) return 1;
			}
		}
	}
	return 0;
}

void send_ack()
{
	if (!cc_flag) return;
  ccbuf[0] = 21;
	ccbuf[1] = 0x28;
	ccbuf[2] = 0x41;
	ccbuf[3] = (uint8_t)(label >> 8);
	ccbuf[4] = (uint8_t)label;
	ccbuf[5] = (uint8_t)(0 >> 8);
	ccbuf[6] = 0;
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
	ccbuf[17] = 0;
	ccbuf[18] = 0 >> 8;
	ccbuf[19] = 0;
	echo();
}
