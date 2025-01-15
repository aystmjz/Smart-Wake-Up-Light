#ifndef __DS3231_H__
#define __DS3231_H__

#include "sys.h"
#include "I2C.h"
#include <time.h>
#include <stdio.h>
#include "UART.h"

#define DS3231_ADDRESS 0xD0
#define DS3231_CONTROL 0x0E
#define DS3231_STATUS 0x0F

#define Time_Year (Time.tm_year + 1900)
#define Time_Mon (Time.tm_mon + 1)
#define Time_Day (Time.tm_mday)
#define Time_Week (Time.tm_wday)
#define Time_Hour (Time.tm_hour)
#define Time_Min (Time.tm_min)
#define Time_Sec (Time.tm_sec)

#define Alarm_1 0
#define Alarm_2 1
#define Alarm_PerMin 0x0E
#define Alarm_MatchMin 0x0C
#define Alarm_MatchHour 0x08
#define Alarm_MatchDay 0x00
#define Alarm_MatchWeek 0x10

typedef struct
{
    int8_t Day;
    int8_t Week;
    int8_t Hour;
    int8_t Min;
    uint8_t Status;
    uint8_t Enable;
    uint8_t Num;
    uint8_t Mod;
} AlarmTypeDef;

void DS3231_WriteByte(uint8_t WordAddress, uint8_t Data);
uint8_t DS3231_ReadByte(uint8_t WordAddress);
void DS3231_ReadTime(struct tm *Time);
void DS3231_ReadStatus(AlarmTypeDef *Alarm);
time_t DS3231_GetTimeStamp(struct tm *Time);
void DS3231_WriteTime(struct tm *Time);
void DS3231_InitAlarm(AlarmTypeDef *Alarm);
void DS3231_WriteAlarm(AlarmTypeDef *Alarm);
void DS3231_ResetAlarm(void);
void DS3231_Init(struct tm *Time, AlarmTypeDef *Alarm);
void TimeJudge(struct tm *Time);
void Alarm_Judge(AlarmTypeDef *Alarm);
char *Get_Week_Str(uint8_t time_Week);

#endif
