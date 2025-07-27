#ifndef __UART_H__
#define __UART_H__

#include "sys.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#define ASRPRO_UART_REC_LEN 255
#define BT24_UART_REC_LEN 1024
#define DEBUG_BUFF_LEN 255

extern volatile uint16_t ASRPRORxCounter;
extern volatile uint16_t BT24RxCounter;
extern volatile uint8_t ASRPRORxBuffer[ASRPRO_UART_REC_LEN]; // 接收缓冲
extern volatile uint8_t BT24RxBuffer[BT24_UART_REC_LEN];	 // 接收缓冲
extern char Debug_str[DEBUG_BUFF_LEN];

void uart1_init(uint32_t bound);
void uart2_init(uint32_t bound);
void Uart_Init(uint32_t bound);
void ASRPRO_printf(const char *format, ...);
void Debug_printf(const char *format, ...);
void BT24_printf(char *SendBuf);
void ASRPRO_Clear_Buff(void);
void BT24_Clear_Buff(void);

#endif
