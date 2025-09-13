#include "globals.h"

// 刷新标志位，用于指示是否需要刷新显示界面
uint8_t Refresh_Flag = 1;

// 蜂鸣器整点报时控制位，置1后且到达整点时蜂鸣器响200ms
uint8_t BUZ_Flag = 0;

// 唤醒控制位，置1后唤醒语音助手且不会进入睡眠状态，60秒后自动清除唤醒标志
uint8_t WakeUp_Flag = 0;

// 当前低功耗状态标志，0表示正常模式，1表示低功耗模式
uint8_t LowPower_Now = 0;

// SHT30温湿度传感器数据结构体
SHT30TypeDef SHT;

// 闹钟数据结构体，初始化为闹钟1
AlarmTypeDef Alarm = {.Num = Alarm_1};

// 设置数据结构体
SettingTypeDef Set;

// 时间结构体
struct tm Time;

// 发布数据结构体，包含所有需要发布的数据指针
PubDataTypeDef PubData = {.SHT = &SHT, .Alarm = &Alarm, .Set = &Set, .Time = &Time};
