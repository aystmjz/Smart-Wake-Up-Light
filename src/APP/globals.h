#ifndef __GLOBALS_H__
#define __GLOBALS_H__

#include "sys.h"
#include "W25Q128.h"
#include "DS3231.h"
#include "SHT30.h"

#define Time_Year (Time.tm_year + 1900)
#define Time_Mon (Time.tm_mon + 1)
#define Time_Day (Time.tm_mday)
#define Time_Week (Time.tm_wday)
#define Time_Hour (Time.tm_hour)
#define Time_Min (Time.tm_min)
#define Time_Sec (Time.tm_sec)

typedef struct
{
    SHT30TypeDef *SHT;
    AlarmTypeDef *Alarm;
    SettingTypeDef *Set;
    struct tm *Time;
} PubDataTypeDef;

extern uint8_t Refresh_Flag;
extern uint8_t BUZ_Flag;
extern uint8_t WakeUp_Flag;
extern uint8_t LowPower_Now;
extern SHT30TypeDef SHT;
extern AlarmTypeDef Alarm;
extern SettingTypeDef Set;
extern struct tm Time;
extern PubDataTypeDef PubData;

#endif