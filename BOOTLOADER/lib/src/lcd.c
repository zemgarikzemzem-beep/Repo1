#include "lcd.h"
#include "i2c.h"

extern void delay(__IO uint32_t tck);

char str1[100];
uint8_t buf[1]={0};
uint8_t portlcd=0; //ячейка для хранения данных порта микросхемы расширения


//                                   ДЛЯ ДИСПЛЕЕВ С ПОДДЕРЖКОЙ КИРИЛЛИЦЫ
//uint8_t Russian_ABC[66][2] = {{/*А*/192,0x41},{/*Б*/193,0xA0},{/*В*/194,0x42},{/*Г*/195,0xA1},{/*Д*/196,0xE0},{/*Е*/197,0x45},{/*Ё*/136,0xA2},{/*Ж*/198,0xA3},
//                      {/*З*/199,0xA4},{/*И*/200,0xA5},{/*Й*/201,0xA6},{/*К*/202,0x4B},{/*Л*/203,0xA7},{/*М*/204,0x4D},{/*Н*/205,0x48},{/*О*/206,0x4F},
//                  {/*П*/207,0xA8},{/*Р*/208,0x50},{/*С*/209,0x43},{/*Т*/210,0x54},{/*У*/211,0xA9},{/*Ф*/212,0xAA},{/*Х*/213,0x58},{/*Ц*/214,0xE1},
//                  {/*Ч*/215,0xAB},{/*Ш*/216,0xAC},{/*Щ*/217,0xE2},{/*Ъ*/218,0xAD},{/*Ы*/219,0xAE},{/*Э*/221,0xAF},{/*Ю*/222,0xB0},{/*Я*/223,0xB1},
//                              //Маленькие буквы
//                  {224,0x61},{225,0xB2},{226,0xB3},{227,0xB4},{228,0xE3},{229,0x65},{152,0xB5},{230,0xB6},{231,0xB7},{232,0xB8},{233,0xB9},{234,0xBA},
//                              {235,0xBB},{236,0xBC},{237,0xBD},{238,0x6F},{239,0xBE},{240,0x70},{241,0x63},{242,0xBF},{243,0x79},{244,0xE4},{245,0xD5},{246,0xE5},
//                              {247,0xC0},{248,0xC1},{249,0xE6},{250,0xC2},{251,0xC3},{252,0xC4},{253,0xC5},{254,0xC6},{255,0xC7}};



uint8_t CustomChar1[8]={0b11011,0b11011,0b00000,0b00100,0b10001,0b11011,0b01110,0b00000};
uint8_t CustomChar2[8]={0b00111,0b00011,0b00101,0b01000,0b11111,0b00010,0b00100,0b01000};

void LCD_WriteByteI2CLCD (uint8_t b){
	I2C1_SendByte(b);
}

//------------------------------------------------
__STATIC_INLINE void delay_ms(__IO uint32_t micros)
{
	micros *=(SystemCoreClock / 10000);//
	while (micros--);
}
//------------------------------------------------
__STATIC_INLINE void DelayNano(__IO uint32_t nanos)
{
  nanos = nanos * (SystemCoreClock / 1000000) / 9000;
  while (nanos--);
}

void sendhalfbyte(uint8_t c)
{
  c<<=4;
  LCD_WriteByteI2CLCD(portlcd|c);
  LCD_WriteByteI2CLCD((portlcd|=0x04)|c);
  DelayNano(200);
  LCD_WriteByteI2CLCD((portlcd&=~0x04)|c);
}
//------------------------------------------------
void sendbyte(uint8_t c, uint8_t mode)
{
	if(mode==0) rs_reset();
	else rs_set();
	uint8_t hc=0;
	hc=c>>4;
	sendhalfbyte(hc);sendhalfbyte(c);
}
//------------------------------------------------
void LCD_Clear(void)
{
	sendbyte(0x01,0);
	delay_ms(2);
}
//------------------------------------------------
void LCD_SendChar(char ch)
{
	sendbyte(ch,1);
}

void LCD_SendCustomChar1(uint8_t x, uint8_t y){                    //https://maxpromer.github.io/LCD-Character-Creator/
	sendbyte(0b01000000,0);     /* + (ch << 3)*/
  for(uint8_t line = 0; line < 8; line++) {
  sendbyte(CustomChar1[line],1);
  }
	LCD_SetPos(x,y);
	sendbyte(0,1);
}
void LCD_SendCustomChar2(uint8_t x, uint8_t y){                    //https://maxpromer.github.io/LCD-Character-Creator/
	sendbyte(0b01000000+8,0);     /* + (ch << 3)*/
  for(uint8_t line = 0; line < 8; line++) {
  sendbyte(CustomChar2[line],1);
  }
	LCD_SetPos(x,y);
	sendbyte(1,1);
}
//------------------------------------------------
void LCD_String(char* st, uint8_t size)
{
	for(uint8_t i=0;i<size;++i) sendbyte(st[i],1);
//	uint8_t i=0;
//	while(st[i]!=0)
//	{
//    sendbyte(st[i],1);
//		i++;   
//   }
		
	
}

//------------------------------------------------
void LCD_SetPos(uint8_t x, uint8_t y)
{
	switch(y)
	{
		case 0:
			sendbyte(x|0x80,0);
			break;
		case 1:
			sendbyte((0x40+x)|0x80,0);
			break;
	}
}
//------------------------------------------------
void LCD_Init(void)
{
  delay(50);
  LCD_WriteByteI2CLCD(0);
  setwrite();//запись
  delay(100);
  sendhalfbyte(0x03);
  delay(5);
  sendhalfbyte(0x03);
  delay(5);
  sendhalfbyte(0x03);
  delay(100);
  sendhalfbyte(0x02);
  sendbyte(0x28,0);//режим 4 бит, 2 линии (для нашего большого дисплея это 4 линии, шрифт 5х8
  sendbyte(0x08,0);//дисплей пока выключаем
  delay(1);
  sendbyte(0x01,0);// уберем мусор
  delay(2);
  sendbyte(0x06,0);// пишем влево
  delay(1);
  sendbyte(0x0C,0);//дисплей включаем (D=1), курсоры никакие не нужны
  sendbyte(0x02,0);//курсор на место
  delay(2);
  setled();//подсветка
}

