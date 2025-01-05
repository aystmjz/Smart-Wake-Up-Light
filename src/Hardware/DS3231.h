#ifndef __DS3231_H__
#define __DS3231_H__
#include "sys.h"
#include "I2C.h"
#include <time.h>

#define DS3231_ADDRESS 0xD0
#define DS3231_CONTROL 0x0E
#define DS3231_STATUS 0x0F
#define DS3231_Temp 0x0B
#define DS3231_Temp2 0x0C

#define Time_Year       (Time_Date.tm_year + 1900)
#define Time_Mon        (Time_Date.tm_mon + 1)
#define Time_Day        (Time_Date.tm_mday)
#define Time_Week       (Time_Date.tm_wday)
#define Time_Hour       (Time_Date.tm_hour)
#define Time_Min        (Time_Date.tm_min)
#define Time_Sec        (Time_Date.tm_sec)

extern int8_t TIME[7];
extern uint8_t AlarmStatus[2];
extern uint8_t Alarm_Set[8];
extern int8_t Alarm_Date[2];
extern int8_t Alarm_Date_Temp[2];
extern uint8_t Alarm_Enable;
extern int8_t PWM_Mod;

extern struct tm Time_Date;


void DS3231_WriteByte(uint8_t WordAddress, uint8_t Data);
uint8_t DS3231_ReadByte(uint8_t WordAddress);
void DS3231_ReadTime(void);
time_t DS3231_GetTimeStamp(void);
void DS3231_WriteTime(struct tm time_data);
void DS3231_WriteAlarm(void);
void DS3231_ResetAlarm(void);
void DS3231_Init(void);


#endif
