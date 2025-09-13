#ifndef __GLOBALS_H__
#define __GLOBALS_H__

#include "DS3231.h"
#include "SHT30.h"
#include "W25Q128.h"
#include "sys.h"

// 时间宏定义，用于方便访问时间结构体中的各个字段
#define Time_Year (Time.tm_year + 1900) // 当前年份
#define Time_Mon  (Time.tm_mon + 1)     // 当前月份
#define Time_Day  (Time.tm_mday)        // 当前日期
#define Time_Week (Time.tm_wday)        // 当前星期
#define Time_Hour (Time.tm_hour)        // 当前小时
#define Time_Min  (Time.tm_min)         // 当前分钟
#define Time_Sec  (Time.tm_sec)         // 当前秒

// 发布数据结构体定义,包含所有需要通过蓝牙发布的数据指针
typedef struct
{
    SHT30TypeDef *SHT;   // SHT30温湿度传感器数据指针
    AlarmTypeDef *Alarm; // 闹钟数据指针
    SettingTypeDef *Set; // 设置数据指针
    struct tm *Time;     // 时间数据指针
} PubDataTypeDef;

// 全局变量声明
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