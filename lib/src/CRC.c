#include "CRC.h"

void CRC_Init(void){
	SET_BIT(RCC->AHBENR, RCC_AHBENR_CRCEN);
	WRITE_REG(CRC->INIT, 0xFFFFFFFF);
	CRC->CR |= CRC_CR_RESET;
}

uint32_t CRC_Result(uint8_t buff[], uint32_t buffsize){
	uint32_t index=0;
//	CRC->CR |= CRC_CR_RESET;
	for(index=0; index<buffsize/4; index++){
		CRC->DR=((buff[4*index]<<24)|(buff[4*index+1]<<16)|(buff[4*index+2]<<8)|(buff[4*index+3]));
	}
	if(buffsize%4!=0){
		switch(buffsize%4){
			case 1: CRC->DR=buff[index]; break;
			case 2: CRC->DR=((buff[4*index]<<8)|(buff[4*index+1])); break;
			case 3: CRC->DR=((buff[4*index]<<16)|(buff[4*index+1]<<8)|(buff[4*index+2])); break;
		}
	}
	return CRC->DR;
}

//--------------------------------------------------------------------------------------------------

//static const uint32_t CrcTable[16] = { 
//    0x00000000, 0x04C11DB7, 0x09823B6E, 0x0D4326D9,
//		0x130476DC, 0x17C56B6B, 0x1A864DB2, 0x1E475005,
//    0x2608EDB8, 0x22C9F00F, 0x2F8AD6D6, 0x2B4BCB61,
//		0x350C9B64, 0x31CD86D3, 0x3C8EA00A, 0x384FBDBD 
//};

//uint32_t crc32_fast(uint8_t *buff, uint32_t buffsize)
//{
//	int Crc = -1;
//	uint32_t index=0;
//	
//	for(index=0; index<buffsize/4; index++){
//		Crc^=((buff[4*index]<<24)|(buff[4*index+1]<<16)|(buff[4*index+2]<<8)|(buff[4*index+3]));
//		for(uint8_t i=0; i<8; ++i) Crc = (Crc << 4) ^ CrcTable[(Crc >> 28)&0x0F];
//	}
//	if(buffsize%4!=0){
//		switch(buffsize%4){
//			case 1: Crc^=buff[index]; break;
//			case 2: Crc^=((buff[4*index]<<8)|(buff[4*index+1])); break;
//			case 3: Crc^=((buff[4*index]<<16)|(buff[4*index+1]<<8)|(buff[4*index+2])); break;
//		}
//		for(uint8_t i=0; i<8; ++i) Crc = (Crc << 4) ^ CrcTable[(Crc >> 28)&0x0F];
//	}

//	return Crc;
//}
