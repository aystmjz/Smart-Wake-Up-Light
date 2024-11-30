#ifndef __KEY_H
#define __KEY_H

#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "Buzzer.h"

void Timer_Init(void);
void Key_Init(void);
uint8_t Key_Read(void);
uint8_t Key_GetNumber(void);
uint8_t Key_Clear(void);
uint8_t Key_GetState(void);
void Key_Entry(void);

#endif
