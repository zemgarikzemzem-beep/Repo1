#ifndef __DWT_H
#define __DWT_H

#include "stm32f0xx.h"

#define    DWT_CYCCNT    *(volatile uint32_t*)0xE0001004
#define    DWT_CONTROL   *(volatile uint32_t*)0xE0001000
#define    SCB_DEMCR     *(volatile uint32_t*)0xE000EDFC

void DWT_Init(void);

#endif
