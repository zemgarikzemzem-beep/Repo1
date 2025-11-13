#include "disp.h"

#include <stdlib.h>

#include "spi.h"
#include "gpio.h"
//#include "img.h"

const uint8_t SHIFT_X = 0;
const uint8_t SHIFT_Y = 0;

extern void delay(__IO uint32_t tck);

void TFT_Send_Command(uint8_t com){
	SPI2_Send_Byte(0, com);
}

void TFT_Send_Data(uint8_t data){
	SPI2_Send_Byte(1, data);
}

void TFT_Send_Data_Mult(uint8_t* data, uint16_t size){
	TFT_CSEN;
	uint16_t data1[size];
	DMA1_Channel5->CMAR=(uint32_t)&data1;
	for(uint8_t i=0; i<size; ++i) data1[i]=(1<<8)|data[i];
	DMA1_Channel5->CCR&=~DMA_CCR_EN;
	DMA1_Channel5->CNDTR=size;
	DMA1_Channel5->CCR|=DMA_CCR_EN;
	while(SPI2->SR&SPI_SR_BSY);
	//for(uint8_t i=0; i<size; ++i) TFT_Send_Data(data[i]);
	TFT_CSDIS;
}

void TFT_Init(void){
	
	GPIOB->MODER&=~0x03000000;
	GPIOB->MODER|=GPIO_MODER_MODER12_0;
	GPIOB->OSPEEDR|=GPIO_OSPEEDR_OSPEEDR12;
	
	GPIOA->MODER&=~0x00030000;
	GPIOA->MODER|=GPIO_MODER_MODER8_0;
	GPIOA->OSPEEDR|=GPIO_OSPEEDR_OSPEEDR8;
	
	
	TFT_RES;
  
  TFT_START;
  delay(5);
  TFT_RES;
  delay(5);
  TFT_START;
  delay(5);
  
	TFT_CSEN;
  TFT_Send_Command(ST7735_SWRESET); // Ресет
  delay(150);
  TFT_Send_Command(ST7735_SLPOUT); // This command turns off sleep mode
  delay(500);
  TFT_Send_Command(ST7735_INVOFF); // This command is used to recover from display inversion mode
  TFT_Send_Command(ST7735_MADCTL); // This command defines read/ write scanning direction of frame memory (НУЖНО НЕ RGB, a BGR!!!)
  TFT_Send_Data(0xC8); // BGR и перевёрнутый дисплей
  TFT_Send_Command(ST7735_COLMOD); // Цветовой режим
  TFT_Send_Data(0x05); // 16-bit/pixel (5R-6G-5B)
	/*
  TFT_Send_Command(ST7735_FRMCTR1); // Set the frame frequency of the full colors normal mode
  TFT_Send_Data(0x01);
  TFT_Send_Data(0x2C);
  TFT_Send_Data(0x2D);
  TFT_Send_Command(ST7735_FRMCTR2); // Set the frame frequency of the Idle mode
  TFT_Send_Data(0x01);
  TFT_Send_Data(0x2C);
  TFT_Send_Data(0x2D);
  TFT_Send_Command(ST7735_FRMCTR3); // Set the frame frequency of the Partial mode/ full colors
  TFT_Send_Data(0x01);
  TFT_Send_Data(0x2C);
  TFT_Send_Data(0x2D);
  TFT_Send_Data(0x01);
  TFT_Send_Data(0x2C);
  TFT_Send_Data(0x2D);
  TFT_Send_Command(ST7735_INVCTR); // Display Inversion mode control
  TFT_Send_Data(0x07);
  TFT_Send_Command(ST7735_PWCTR1); // Power Control 1
  TFT_Send_Data(0xA2);
  TFT_Send_Data(0x02);
  TFT_Send_Data(0x84);
  TFT_Send_Command(ST7735_PWCTR2); // Set the VGH and VGL supply power level
  TFT_Send_Data(0xC5);
  TFT_Send_Command(ST7735_PWCTR3); // Set the amount of current in Operational amplifier in normal mode/full colors
  TFT_Send_Data(0x0A);
  TFT_Send_Data(0x00);
  TFT_Send_Command(ST7735_PWCTR4); // Set the amount of current in Operational amplifier in Idle mode/8 colors
  TFT_Send_Data(0x8A);
  TFT_Send_Data(0x2A);
  TFT_Send_Command(ST7735_PWCTR5); // Set the amount of current in Operational amplifier in Partial mode/ full-colors
  TFT_Send_Data(0x8A);
  TFT_Send_Data(0xEE);
  TFT_Send_Command(ST7735_VMCTR1); // VCOM voltage setting
  TFT_Send_Data(0x0E);
  TFT_Send_Command(ST7735_GMCTRP1); // Gamma (‘+’polarity) Correction Characteristics Setting
  TFT_Send_Data(0x02);
  TFT_Send_Data(0x1c);
  TFT_Send_Data(0x07);
  TFT_Send_Data(0x12);
  TFT_Send_Data(0x37);
  TFT_Send_Data(0x32);
  TFT_Send_Data(0x29);
  TFT_Send_Data(0x2d);
  TFT_Send_Data(0x29);
  TFT_Send_Data(0x25);
  TFT_Send_Data(0x2B);
  TFT_Send_Data(0x39);    
  TFT_Send_Data(0x00);
  TFT_Send_Data(0x01);
  TFT_Send_Data(0x03);
  TFT_Send_Data(0x10);
  TFT_Send_Command(ST7735_GMCTRN1); // Gamma ‘-’polarity Correction Characteristics Setting
  TFT_Send_Data(0x03);
  TFT_Send_Data(0x1d);
  TFT_Send_Data(0x07);
  TFT_Send_Data(0x06);
  TFT_Send_Data(0x2E);
  TFT_Send_Data(0x2C);
  TFT_Send_Data(0x29);
  TFT_Send_Data(0x2D);
  TFT_Send_Data(0x2E);
  TFT_Send_Data(0x2E);
  TFT_Send_Data(0x37);
  TFT_Send_Data(0x3F);    
  TFT_Send_Data(0x00);
  TFT_Send_Data(0x00);
  TFT_Send_Data(0x02);
  TFT_Send_Data(0x10);*/
  TFT_Send_Command(ST7735_NORON); // This command returns the display to normal mode
  delay(10);
  TFT_Send_Command(ST7735_DISPON); // Включаем!
  delay(100);
	
	TFT_CSDIS;
  TFT_START;
}

void TFT_Set_Area(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2){
	
	TFT_CSEN;
	TFT_Send_Command(ST7735_CASET); // Column addr set
	TFT_Send_Data((SHIFT_X+x1)>>8);
	TFT_Send_Data((SHIFT_X+x1)&0xFF);
	TFT_Send_Data((SHIFT_X+x2)>>8);
	TFT_Send_Data((SHIFT_X+x2)&0xFF);
	TFT_Send_Command(ST7735_RASET); // Row addr set
	TFT_Send_Data((SHIFT_Y+y1)>>8);
	TFT_Send_Data((SHIFT_Y+y1)&0xFF);
	TFT_Send_Data((SHIFT_Y+y2)>>8);
	TFT_Send_Data((SHIFT_Y+y2)&0xFF);
	TFT_Send_Command(ST7735_RAMWR); // Memory Write
	TFT_CSDIS;
}



void TFT_Fill_Color(uint16_t color){
	uint8_t c1=color >> 8;
	uint8_t c2=color & 0xFF;
	
	TFT_Set_Area(0,0,DISP_WIDTH,DISP_HEIGHT);
	TFT_CSEN;
	
	for (uint32_t i = 0; i < DISP_WIDTH*DISP_HEIGHT; ++i){
		TFT_Send_Data(c1);
		TFT_Send_Data(c2);
				}
				
	TFT_CSDIS;
}

void TFT_Fill_Area(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color){
	uint8_t c1=color >> 8;
	uint8_t c2=color & 0xFF;
	
	TFT_Set_Area(x1,y1,x2,y2);
	TFT_CSEN;
	
	for (uint32_t i = x1*y1; i < x2*y2; ++i){
		TFT_Send_Data(c1);
		TFT_Send_Data(c2);
				}
				
	TFT_CSDIS;
}

void TFT_DrawPixel(uint16_t x, uint16_t y, uint16_t color){
	TFT_Set_Area(x,y,x,y);
	TFT_CSEN;
	TFT_Send_Data(color >> 8);
	TFT_Send_Data(color & 0xFF);
	TFT_CSDIS;
}

void TFT_Send_Char(uint16_t x, uint16_t y, char ch, FontDef font, uint16_t fontcolor,  uint16_t backcolor){
	uint32_t b;
	
	uint8_t cf1=fontcolor >> 8;
	uint8_t cf2=fontcolor & 0xFF;
	uint8_t cb1=backcolor >> 8;
	uint8_t cb2=backcolor & 0xFF;
	
	
	TFT_Set_Area(x, y, x+font.width-1, y+font.height-1);
	
	TFT_CSEN;
	for(uint32_t i=0; i<font.height; ++i){
		if (ch>=192) b=font.data[font.height*(ch-96)+i];
		else if(ch==168) b=font.data[font.height*(160)+i];
		else if(ch==184) b=font.data[font.height*(161)+i];
		else b=font.data[font.height*(ch-32)+i];
		for(uint32_t j=0; j<font.width; ++j){
			if((b<<j)&0x8000){
				TFT_Send_Data(cf1);
				TFT_Send_Data(cf2);
			}
			else{
				TFT_Send_Data(cb1);
				TFT_Send_Data(cb2);
			}
		}
	}
	TFT_CSDIS;
}

void TFT_Send_Str(uint16_t x, uint16_t y, char* str, uint8_t size, FontDef font, uint16_t fontcolor,  uint16_t backcolor){
	uint16_t xn=x,yn=y;
	for(uint8_t i=0; i<size; ++i){
		TFT_Send_Char(xn,yn,str[i],font, fontcolor, backcolor);
		xn+=font.width;
		if(xn>DISP_WIDTH-font.width) {xn=x; yn+=font.height;}
	}

}

void TFT_Draw_Image_Mono(uint8_t* img_arr, uint16_t img_width, uint16_t img_height, uint16_t x, uint16_t y, uint16_t piccolor,  uint16_t backcolor){
	uint16_t imgsize=img_width*img_height/8;
	
	TFT_Set_Area(x,y, x+img_width-1, y+img_height-1);
	
	TFT_CSEN;
	
	for(uint16_t i=0; i<imgsize; ++i){
		for(uint8_t j=0; j<8; ++j){
			if((img_arr[i]>>(7-j))&1) {TFT_Send_Data(piccolor>>8); TFT_Send_Data(piccolor&0xFF);}
			else {TFT_Send_Data(backcolor>>8); TFT_Send_Data(backcolor>>8);}
		}
	}
	
//	for(uint16_t i=0; i<imgsize; ++i){
//		TFT_Send_Data(img_arr[i]>>8);
//		TFT_Send_Data(img_arr[i]&0xFF);
////		delay(1); // Анимация!
//	}
	
	TFT_CSDIS;
}

void TFT_Draw_Image(uint16_t* img_arr, uint16_t img_width, uint16_t img_height, uint16_t x, uint16_t y){
	/**/
	uint16_t imgsize=img_width*img_height;
	
	TFT_Set_Area(x,y, x+img_width-1, y+img_height-1);
	
	TFT_CSEN;
	
	#ifdef IMG_DMA
	
	DMA1_Channel5->CMAR=(uint32_t)&img_arr;
	DMA1_Channel5->CCR&=~DMA_CCR_EN;
	DMA1_Channel5->CNDTR=img_width*img_height;
	DMA1_Channel5->CCR|=DMA_CCR_EN;
	while(SPI2->SR&SPI_SR_BSY);
	
	//SET_BIT(FLASH->CR, FLASH_CR_PER);
  //WRITE_REG(FLASH->AR, img1);
  //SET_BIT(FLASH->CR, FLASH_CR_STRT);
	//free((void*)img1);
	
	#ifdef ARR2
	DMA1_Channel5->CMAR=(uint32_t)&img2;
	DMA1_Channel5->CCR&=~DMA_CCR_EN;
	DMA1_Channel5->CNDTR=sizeof(img2);
	DMA1_Channel5->CCR|=DMA_CCR_EN;
	while(SPI2->SR&SPI_SR_BSY);
	#endif // ARR2
	
	#else
	
	for(uint16_t i=0; i<imgsize; ++i){
		TFT_Send_Data(img_arr[i]>>8);
		TFT_Send_Data(img_arr[i]&0xFF);
//		delay(1); // Анимация!
	}
	
	#endif // IMG_DMA
	TFT_CSDIS;
}
