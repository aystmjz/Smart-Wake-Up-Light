#ifndef __ENCODER_H
#define __ENCODER_H

#include "stm32f10x.h"                  // Device header
#include "Buzzer.h"


void Encoder_Init(void);
int16_t Encoder_GetCounter(void);
void Encoder_Set(int16_t Num);
void Encoder_Clear(void);
int16_t Encoder_Get_Div4(void);

#endif
