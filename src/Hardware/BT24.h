#ifndef __BT24_H
#define __BT24_H

#include "Delay.h"
#include "log.h"
#include "sys.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 蓝牙状态枚举（0=未连接，1=已连接）
typedef enum
{
    BT24_DISCONNECTED = 0, // 蓝牙未连接状态
    BT24_CONNECTED    = 1  // 蓝牙已连接状态
} BT24_Status;

// 波特率编码定义
#define baud_9600             3 // 9600波特率对应的AT命令
#define baud_115200           7 // 115200波特率对应的AT命令

// 蓝牙设备配置
#define BT_DEVICE_NAME        "Smart WakeUpLight" // 蓝牙设备默认更改名称
#define BT_DEVICE_BAUD        baud_115200         // 蓝牙设备默认更改通信波特率

// 设置AT指令内容
#define ATcmd_Set(message)    sprintf(AT_cmd, (message))

// AT指令相关配置
#define ATCMD_BUFF_LEN        32                // AT指令缓冲区长度
#define ATcmd_RxBuffer        BT24RxBuffer      // AT指令接收缓冲区
#define ATcmd_RxCounter       BT24RxCounter     // AT指令接收计数器
#define ATcmd_Clear_Buffer()  BT24_Clear_Buff() // AT指令清空缓冲区
#define ATcmd_UartInit(bound) UART1_Init(bound) // AT指令串口初始化

void BT24_GPIO_Init(void);
uint8_t BT24_AT_Init(char *DeviceName);
void BT24_Reset(void);
void BT24_Reset_Assert(void);
void BT24_Reset_Deassert(void);
void BT24_Disconnect(void);
BT24_Status BT24_GetStatus(void);
void BT24_PubString(char *str);

#endif
