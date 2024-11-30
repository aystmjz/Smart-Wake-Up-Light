#ifndef __PWM_H
#define __PWM_H

#include "stm32f10x.h"

void PWM_Init(void);
void PWM_Enable(void);
void PWM_Disable(void);
void PWM_Set(uint8_t Spead);

#endif
