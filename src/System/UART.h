#ifndef __UART_H__
#define __UART_H__

#include "sys.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#define UART_REC_LEN 200

void Uart_Init(uint32_t bound);
void ASRPRO_Send_byte(uint8_t byte);
void ASRPRO_Send_bytes(uint8_t *bytes, uint8_t length);
void ASRPRO_printf(const char *format, ...);
void Debug_printf(char *SendBuf);

#endif
