#ifndef DISP_H_
#define DISP_H_
#include "stm32f0xx.h"                  // Device header
#include "fonts.h"

#define DISP_WIDTH	128
#define DISP_HEIGHT 160

#define TFT_RES GPIOA->BRR|=(1<<8)
#define TFT_START GPIOA->BSRR|=(1<<8)
#define TFT_CSEN GPIOB->BRR|=(1<<12)
#define TFT_CSDIS GPIOB->BSRR|=(1<<12)

#define WHITE       0xFFFF
#define BLACK       0x0000
#define BLUE        0x001F
#define RED         0xF800
#define MAGENTA     0xF81F
#define GREEN       0x07E0
#define CYAN        0x7FFF
#define YELLOW      0xFFE0
#define GRAY        0X8430
#define BRED        0XF81F
#define GRED        0XFFE0
#define GBLUE       0X07FF
#define BROWN       0XBC40
#define BRRED       0XFC07
#define DARKBLUE    0X01CF
#define LIGHTBLUE   0X7D7C
#define GRAYBLUE    0X5458

#define LIGHTGREEN  0X841F
#define LGRAY       0XC618
#define LGRAYBLUE   0XA651
#define LBBLUE      0X2B12

#define ST7735_X_SIZE                                                   128
#define ST7735_Y_SIZE                                                   160
#define ST7735_NOP                                                      0x00
#define ST7735_SWRESET                                                  0x01
#define ST7735_RDDID                                                    0x04
#define ST7735_RDDST                                                    0x09
#define ST7735_SLPIN                                                    0x10
#define ST7735_SLPOUT                                                   0x11
#define ST7735_PTLON                                                    0x12
#define ST7735_NORON                                                    0x13
#define ST7735_INVOFF                                                   0x20
#define ST7735_INVON                                                    0x21
#define ST7735_DISPOFF                                                  0x28
#define ST7735_DISPON                                                   0x29
#define ST7735_CASET                                                    0x2A
#define ST7735_RASET                                                    0x2B
#define ST7735_RAMWR                                                    0x2C
#define ST7735_RAMRD                                                    0x2E
#define ST7735_PTLAR                                                    0x30
#define ST7735_COLMOD                                                   0x3A
#define ST7735_MADCTL                                                   0x36
#define ST7735_FRMCTR1                                                  0xB1
#define ST7735_FRMCTR2                                                  0xB2
#define ST7735_FRMCTR3                                                  0xB3
#define ST7735_INVCTR                                                   0xB4
#define ST7735_DISSET5                                                  0xB6
#define ST7735_PWCTR1                                                   0xC0
#define ST7735_PWCTR2                                                   0xC1
#define ST7735_PWCTR3                                                   0xC2
#define ST7735_PWCTR4                                                   0xC3
#define ST7735_PWCTR5                                                   0xC4
#define ST7735_VMCTR1                                                   0xC5
#define ST7735_RDID1                                                    0xDA
#define ST7735_RDID2                                                    0xDB
#define ST7735_RDID3                                                    0xDC
#define ST7735_RDID4                                                    0xDD
#define ST7735_PWCTR6                                                   0xFC
#define ST7735_GMCTRP1                                                  0xE0
#define ST7735_GMCTRN1                                                  0xE1
#define ST7735_SPI_TIMEOUT                                              100

void TFT_Init(void);
void TFT_Send_Command(uint8_t com);
void TFT_Fill_Color(uint16_t color);
void TFT_Fill_Area(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
void TFT_DrawPixel(uint16_t x, uint16_t y, uint16_t color);
void TFT_Send_Char(uint16_t x, uint16_t y, char ch, FontDef font, uint16_t fontcolor,  uint16_t backcolor);
void TFT_Send_Str(uint16_t x, uint16_t y, char* str, uint8_t size, FontDef font, uint16_t fontcolor,  uint16_t backcolor);
void TFT_Draw_Image(uint16_t* img_arr, uint16_t img_width, uint16_t img_height, uint16_t x, uint16_t y);
void TFT_Draw_Image_Mono(uint8_t* img_arr, uint16_t img_width, uint16_t img_height, uint16_t x, uint16_t y, uint16_t fontcolor,  uint16_t backcolor);
void TFT_Set_Area(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

#endif /* DISP_H_ */
