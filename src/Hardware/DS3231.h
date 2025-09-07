#ifndef __DS3231_H__
#define __DS3231_H__

#include "I2C.h"
#include "sys.h"
#include <stdio.h>
#include <time.h>

// DS3231寄存器地址定义
#define DS3231_ADDRESS  0xD0 // DS3231 I2C地址
#define DS3231_CONTROL  0x0E // 控制寄存器地址
#define DS3231_STATUS   0x0F // 状态寄存器地址

// 闹钟编号
#define Alarm_1         1 // 闹钟1编号
#define Alarm_2         2 // 闹钟2编号

// 闹钟匹配模式定义
#define Alarm_PerMin    0x0E // 每分钟匹配
#define Alarm_MatchMin  0x0C // 匹配分钟
#define Alarm_MatchHour 0x08 // 匹配小时
#define Alarm_MatchDay  0x00 // 匹配日期
#define Alarm_MatchWeek 0x10 // 匹配星期

// 闹钟参数结构体
typedef struct
{
    int8_t Day;     // 日期(1-31)
    int8_t Week;    // 星期(0-6, 0表示星期日)
    int8_t Hour;    // 小时(0-23)
    int8_t Min;     // 分钟(0-59)
    uint8_t Status; // 闹钟状态(1表示已触发，0表示未触发)
    uint8_t Enable; // 闹钟使能状态(1表示启用，0表示禁用)
    uint8_t Num;    // 闹钟编号(Alarm_1或Alarm_2)
    uint8_t Mod;    // 闹钟匹配模式
} AlarmTypeDef;

void DS3231_ReadTime(struct tm *Time);
void DS3231_ReadAlarm(AlarmTypeDef *Alarm);
uint8_t DS3231_ReadStatus(AlarmTypeDef *Alarm);
time_t DS3231_GetTimeStamp(struct tm *Time);
void DS3231_WriteTime(struct tm *Time);
void DS3231_InitAlarm(AlarmTypeDef *Alarm);
void DS3231_WriteAlarm(AlarmTypeDef *Alarm);
void DS3231_ResetAlarm(void);
uint8_t DS3231_Init(struct tm *Time, AlarmTypeDef *Alarm);
void TimeJudge(struct tm *Time);
void Alarm_Judge(AlarmTypeDef *Alarm);
char *Get_Week_Str(uint8_t time_Week);

#endif
