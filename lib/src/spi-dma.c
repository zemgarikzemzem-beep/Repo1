#include "spi-dma.h"

void SPI1_Init_DMA(void){
	RCC->APB2ENR|=RCC_APB2ENR_SPI1EN;
	RCC->AHBENR|=RCC_AHBENR_DMA1EN;
	GPIOA->CRL&=~0xF0F00000;
	GPIOA->CRL|=0xB0B00000;
	SPI1->CR1=0;
	SPI1->CR1|=(SPI_CR1_BR|SPI_CR1_MSTR|SPI_CR1_SSM|SPI_CR1_SSI);   //full-duplex 8-bit f/256
	SPI1->CR2=0x0000;
	SPI1->CR2|=(SPI_CR2_TXDMAEN|SPI_CR2_RXDMAEN);
	SPI1->CR1|=SPI_CR1_SPE;
}

void SPI1_Receive_DMA(uint8_t* data, uint16_t len){
	DMA1_Channel2->CCR=0;
	DMA1_Channel2->CPAR=(uint32_t)(&SPI1->DR);
	DMA1_Channel2->CMAR=(uint32_t)data;
	DMA1_Channel2->CNDTR=len;
	DMA1_Channel2->CCR|=(DMA_CCR3_PSIZE_0|DMA_CCR3_MINC); //msize8, psize16
	DMA1_Channel2->CCR|=DMA_CCR3_EN;
	
	static uint8_t _filler=0xF0;
	DMA1_Channel3->CCR=0;
	DMA1_Channel3->CPAR=(uint32_t)(&SPI1->DR);
	DMA1_Channel3->CMAR=(uint32_t)(&_filler);
	DMA1_Channel3->CNDTR=len;
	DMA1_Channel3->CCR|=(DMA_CCR3_PSIZE_0|DMA_CCR3_DIR); //msize8, psize16, !MINC
	DMA1_Channel3->CCR|=DMA_CCR3_EN;
}

void SPI1_Send_DMA(uint8_t* data, uint16_t len){
	DMA1_Channel3->CCR=0;
	DMA1_Channel3->CPAR=(uint32_t)(&SPI1->DR);
	DMA1_Channel3->CMAR=(uint32_t)data;
	DMA1_Channel3->CNDTR=len;
	DMA1_Channel3->CCR|=(DMA_CCR3_PSIZE_0|DMA_CCR3_MINC|DMA_CCR3_DIR); //msize8, psize16
	DMA1_Channel3->CCR|=DMA_CCR3_EN;
}