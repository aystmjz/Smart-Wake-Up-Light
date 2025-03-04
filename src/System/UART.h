#ifndef __UART_H__
#define __UART_H__

#include "sys.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#define ASRPRO_UART_REC_LEN 255
#define BT24_UART_REC_LEN 1024
#define DEBUG_BUFF_LEN 24

extern uint16_t ASRPRORxCounter;
extern uint16_t BT24RxCounter;
extern uint8_t ASRPRORxBuffer[ASRPRO_UART_REC_LEN]; // 接收缓冲
extern uint8_t BT24RxBuffer[BT24_UART_REC_LEN];	 // 接收缓冲
extern char Debug_str[DEBUG_BUFF_LEN];

void Uart_Init(uint32_t bound);
void ASRPRO_printf(const char *format, ...);
void BT24_printf(char *SendBuf);
void Debug_printf(char *SendBuf);
void ASRPRO_Clear_Buff(void);
void BT24_Clear_Buff(void);

#endif
