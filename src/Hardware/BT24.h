#ifndef __BT24_H
#define __BT24_H

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "sys.h"
#include "Delay.h"
#include "log.h"

#define baud_9600 3
#define baud_115200 7

#define BT_DEVICE_NAME "Smart WakeUpLight"
#define BT_DEVICE_BAUD baud_115200

#define ATCMD_BUFF_LEN 32

#define ATcmd_Set(message) sprintf(AT_cmd, (message))
#define ATcmd_RxBuffer BT24RxBuffer
#define ATcmd_RxCounter BT24RxCounter
#define ATcmd_Clear_Buffer BT24_Clear_Buff
#define ATcmd_UartInit uart1_init

void BT24_Init(char *DeviceName);
void BT24_Reset(void);
uint8_t BT24_GetStatus(void);
void BT24_PubString(char *str);

#endif
