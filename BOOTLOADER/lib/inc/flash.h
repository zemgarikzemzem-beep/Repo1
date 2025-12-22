#ifndef __FLASHER_H__
#define __FLASHER_H__

#include <stdint.h>

#define APPLICATION_ADDRESS		0x08004000
#define FLASH_PRGFLAG_ADDR		0x0803000C
#define FLASH_CRC_ADDR				0x0803000E
#define FLASH_SDFB_ADDR				0x08030014  // Флаг выключения с кнопки, чтоб не прыгал при перезагрузке
#define FLASH_BOOTLOADER_END	0x00004000
#define FLASH_PAGESIZE				0x800
#define FLASH_SECTOR_SIZE			0x1000
#define FLASH_PAGES_IN_SECTOR	FLASH_SECTOR_SIZE/FLASH_PAGESIZE
#define FLASH_ATTRIB_SECTOR		15
#define PRG_WRITE							4

typedef struct
{
  volatile uint32_t ACR;
  volatile uint32_t KEYR;
  volatile uint32_t OPTKEYR;
  volatile uint32_t SR;
  volatile uint32_t CR;
  volatile uint32_t AR;
  volatile uint32_t RESERVED;
  volatile uint32_t OBR;
  volatile uint32_t WRPR;
} FLASH_T;

typedef enum
{ 
  Flash_BUSY = 1,
  Flash_ERROR_PG,
  Flash_ERROR_WRP,
  Flash_COMPLETE,
  Flash_TIMEOUT
}Flash_Status;

/* memory mapping struct for System Control Block */
typedef struct
{
  volatile const  uint32_t CPUID;                 /*!< CPU ID Base Register                                     */
  volatile uint32_t ICSR;                         /*!< Interrupt Control State Register                         */
  volatile uint32_t VTOR;                         /*!< Vector Table Offset Register                             */
  volatile uint32_t AIRCR;                        /*!< Application Interrupt / Reset Control Register           */
  volatile uint32_t SCR;                          /*!< System Control Register                                  */
  volatile uint32_t CCR;                          /*!< Configuration Control Register                           */
  volatile uint8_t  SHP[12];                      /*!< System Handlers Priority Registers (4-7, 8-11, 12-15)    */
  volatile uint32_t SHCSR;                        /*!< System Handler Control and State Register                */
  volatile uint32_t CFSR;                         /*!< Configurable Fault Status Register                       */
  volatile uint32_t HFSR;                         /*!< Hard Fault Status Register                               */
  volatile uint32_t DFSR;                         /*!< Debug Fault Status Register                              */
  volatile uint32_t MMFAR;                        /*!< Mem Manage Address Register                              */
  volatile uint32_t BFAR;                         /*!< Bus Fault Address Register                               */
  volatile uint32_t AFSR;                         /*!< Auxiliary Fault Status Register                          */
  volatile const  uint32_t PFR[2];                /*!< Processor Feature Register                               */
  volatile const  uint32_t DFR;                   /*!< Debug Feature Register                                   */
  volatile const  uint32_t ADR;                   /*!< Auxiliary Feature Register                               */
  volatile const  uint32_t MMFR[4];               /*!< Memory Model Feature Register                            */
  volatile const  uint32_t ISAR[5];               /*!< ISA Feature Register                                     */
} SCB_Typedef;

void flash_update(uint32_t*);

#endif

//#define FLASH_KEY1               ((uint32_t)0x45670123)
//#define FLASH_KEY2               ((uint32_t)0xCDEF89AB)

#define EraseTimeout		 ((uint32_t)0x000B0000)
#define ProgramTimeout        	 ((uint32_t)0x00002000)

#define CR_SER_Set               ((uint32_t)0x00000002) // строб стирания сектора
//#define CR_PER_Reset             ((uint32_t)0x00001FFD)
#define CR_PG_Set                ((uint32_t)0x00000001)
//#define CR_PG_Reset              ((uint32_t)0x00001FFE)
#define CR_STRT_Set              ((uint32_t)0x00010000)
#define CR_LOCK_Set              ((uint32_t)0x80000000)

#define FLASH_FLAG_BSY           ((uint32_t)0x00010000)  // флаг занятой flash 
#define FLASH_FLAG_EOP           ((uint32_t)0x00000001)  // конец операции
void save_settings(uint32_t*);
void write_prg();
void write_prg_CRC();
void write_prg_size();
void flash_buf_save();
void flash_attr_save();
void Flash_lock();

void FLASH_PageErase(uint32_t page_addr);
void FLASH_WriteBlock(uint32_t flash_addr, uint8_t* data, uint8_t size);
void FLASH_WriteStr(uint32_t flash_addr, uint8_t* data, uint8_t size);
void FLASH_WriteByte(uint32_t flash_addr, uint8_t byte);
uint8_t* FLASH_Read8byAddr(uint32_t flash_addr, uint8_t size);
void FLASH_ReadStr(uint32_t flash_addr, uint8_t* buff, uint8_t size);
void write_program(void);
