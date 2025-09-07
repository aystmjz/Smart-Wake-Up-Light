#ifndef __ASRPRO_H
#define __ASRPRO_H

#include "Delay.h"
#include "UART.h"
#include "sys.h"

// ASRPRO模块串口通信波特率
#define ASRPRO_DEVICE_BAUD 115200

void ASRPRO_Init(void);
void ASRPRO_Power_ON(void);
void ASRPRO_Power_OFF(void);
uint8_t ASRPRO_Power_Turn(void);
void ASRPRO_Mute_ON(void);
void ASRPRO_Mute_OFF(void);
uint8_t ASRPRO_Get_State(void);
uint8_t ASRPRO_Get_CMD(void);

#endif
