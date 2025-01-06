#ifndef __PWM_H
#define __PWM_H

#include "stm32f10x.h"
#include "DS3231.h"
#include "Key.h"

void PWM_Init(void);
void PWM_Enable(void);
void PWM_Disable(void);
void PWM_Set(uint16_t Percent);
void PWM_Run(void);

#endif
