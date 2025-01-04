#ifndef __ASRPRO_H
#define __ASRPRO_H

#include "stm32f10x.h"
#include "UART.h"
#include "Delay.h"

void ASRPRO_Init(void);
void ASRPRO_Power_ON(void);
void ASRPRO_Power_OFF(void);
void ASRPRO_Power_Turn(void);
void ASRPRO_Mute_ON(void);
void ASRPRO_Mute_OFF(void);


#endif
