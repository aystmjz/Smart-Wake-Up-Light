#ifndef __PWM_H
#define __PWM_H

#include "sys.h"

void PWM_Init(void);
void PWM_Enable(void);
void PWM_Disable(void);
void PWM_Set(uint16_t Percent);

#endif
