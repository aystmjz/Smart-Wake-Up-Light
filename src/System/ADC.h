#ifndef __ADC_H
#define __ADC_H

#include "stm32f10x.h"

void AD_Init(void);
uint16_t AD_GetValue(void);

#endif
