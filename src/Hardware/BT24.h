#ifndef __BT24_H
#define __BT24_H

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "sys.h"
#include "UART.h"
#include "Delay.h"
#include "W25Q128.h"
#include "DS3231.h"
#include "cJSON.h"
#include "SHT30.h"
#include "PWM.h"
#include "UART.h"

#define baud_9600 3
#define baud_115200 7

#define BT_DEVICE_NAME "Smart WakeUpLight"
#define BT_DEVICE_BAUD baud_115200

typedef struct
{
    SHT30TypeDef *SHT;
    AlarmTypeDef *Alarm;
    SettingTypeDef *Set;
    struct tm *Time;
} PubDataTypeDef;

#define ATCMD_BUFF_LEN 32

#define ATcmd_Set(message) sprintf(AT_cmd, (message))
#define ATcmd_RxBuffer BT24RxBuffer
#define ATcmd_RxCounter BT24RxCounter
#define ATcmd_Clear_Buffer BT24_Clear_Buff
#define ATcmd_UartInit uart1_init

void BT24_Init(char *DeviceName);
void BT24_Reset(void);
uint8_t BT24_GetStatus(void);
uint8_t BT24_FindValidJson(char *buffer, uint16_t length, char *json_str);
void BT24_PubData(PubDataTypeDef *PubData);
int8_t BT24_ParseCmd(const char *json_str);
uint8_t BT24_ParseData(const char *json_str, PubDataTypeDef *PubData, char *Extra);

#endif
