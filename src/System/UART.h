#ifndef __UART_H__
#define __UART_H__

#include "sys.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

// 串口接收缓冲区长度
#define ASRPRO_UART_REC_LEN 255  // ASRPRO语音识别模块接收缓冲区长度
#define BT24_UART_REC_LEN   1024 // BT24蓝牙模块接收缓冲区长度
#define DEBUG_BUFF_LEN      255  // 调试信息缓冲区长度

// 声明外部变量
extern volatile uint16_t ASRPRORxCounter;                    // ASRPRO模块接收计数器
extern volatile uint16_t BT24RxCounter;                      // BT24模块接收计数器
extern volatile uint8_t ASRPRORxBuffer[ASRPRO_UART_REC_LEN]; // ASRPRO接收缓冲区
extern volatile uint8_t BT24RxBuffer[BT24_UART_REC_LEN];     // BT24接收缓冲区

void UART1_Init(uint32_t bound);
void UART2_Init(uint32_t bound);
void Uart_Init(uint32_t bound);
void ASRPRO_printf(const char *format, ...);
void Debug_printf(const char *format, ...);
void BT24_printf(char *SendBuf);
void ASRPRO_Clear_Buff(void);
void BT24_Clear_Buff(void);

#endif
