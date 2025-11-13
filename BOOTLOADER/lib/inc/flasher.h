#ifndef __FLASHER_H__
#define __FLASHER_H__

#include <stdint.h>

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
