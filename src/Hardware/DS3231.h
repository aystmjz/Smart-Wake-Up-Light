#ifndef __DS3231_H__
#define __DS3231_H__
#include "sys.h"
#include "I2C.h"

extern int8_t TIME[7];
extern uint8_t AlarmStatus[2];
extern uint8_t Alarm_Set[8];
extern int8_t Alarm_Date[2];
extern int8_t Alarm_Date_Temp[2];
extern uint8_t Alarm_Enable;
extern int8_t PWM_Mod;

void DS3231_WriteByte(uint8_t WordAddress,uint8_t Data);
uint8_t DS3231_ReadByte(uint8_t WordAddress);
void DS3231_ReadTime();
void DS3231_WriteTime();
void DS3231_WriteAlarm();
void DS3231_Init();

#endif
