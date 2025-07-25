#ifndef __EXTI_H__
#define __EXTI_H__

#include "sys.h"

void EXTI0_Init(void);
void EXTI5_Init(void);
void EXTI9_Init(void);
uint8_t EXTI0_Get_Flag(void);
uint8_t EXTI5_Get_Flag(void);
uint8_t EXTI9_Get_Flag(void);

#endif