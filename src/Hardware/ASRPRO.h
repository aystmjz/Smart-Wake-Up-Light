#ifndef __ASRPRO_H
#define __ASRPRO_H

#include "sys.h"
#include "UART.h"
#include "Delay.h"

#define ASRPRO_DEVICE_BAUD 115200

void ASRPRO_Init(void);
void ASRPRO_Power_ON(void);
void ASRPRO_Power_OFF(void);
void ASRPRO_Power_Control(uint8_t state);
uint8_t ASRPRO_Power_Turn(void);
void ASRPRO_Mute_ON(void);
void ASRPRO_Mute_OFF(void);
uint8_t ASRPRO_Get_State(void);
uint8_t ASRPRO_Get_CMD(void);

#endif
