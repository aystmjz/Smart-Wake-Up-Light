#ifndef __USART_H
#define __USART_H

#include "dma.h"
#include "sys.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#define UART1_TX_SIZE       128
#define UART1_RX_SIZE       2048
#define UART1_RX_MAX        256 // 串口一次能接受的数据量
#define BLOCK_NUM           10

#define UART_DATA_AVAILABLE (UCB_DATA.RxDataOUT != UCB_DATA.RxDataIN)

typedef struct
{
    uint8_t *start; // 缓冲区起始地址指针
    uint8_t *end;   // 缓冲区结束地址指针
} UCB_RxBuffptr;

typedef struct
{
    uint16_t RxCounter;                 // 接收计数器
    UCB_RxBuffptr RxDataPtr[BLOCK_NUM]; // 接收数据缓冲区数组
    UCB_RxBuffptr *RxDataIN;            // 当前写入缓冲区指针
    UCB_RxBuffptr *RxDataOUT;           // 当前读取缓冲区指针
    UCB_RxBuffptr *RxDataEND;           // 缓冲区数组结束指针
} UCB_DATE;

extern UCB_DATE UCB_DATA;

void uart1_init(uint32_t bound);
void uart1_printf(const char *format, ...);
void uart1_send_byte(uint8_t byte);
uint16_t uart1_read_data(uint8_t *buffer, uint16_t max_len);
uint8_t uart1_scanf(const char *format, ...);
void uart1_update_endptr(void);

#endif