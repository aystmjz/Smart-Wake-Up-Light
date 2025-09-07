#ifndef __BATTERY_H
#define __BATTERY_H

#include "sys.h"

#define BATTERY_FULL   5174 // 电池满电时的ADC参考值(对应实际电压约4.2V)
#define BATTERY_CHARGE 5200 // 充电检测阈值ADC值(高于此值认为正在充电)
#define BATTERY_DC     5450 // 直流电源接入检测阈值ADC值(高于此值认为接入了直流电源)
#define BATTERY_MULT   2 // ADC值倍数修正系数(根据实际硬件配置来确定)

// 电池状态枚举（0=正常，1=充电中，2=充满）
typedef enum
{
    BATTERY_STATE_NORMAL   = 0, // 正常状态
    BATTERY_STATE_CHARGING = 1, // 充电中
    BATTERY_STATE_FULL     = 2  // 已充满
} BatteryState;

void Battery_UpdateLevel(uint16_t battery_ADC);
uint8_t Battery_GetLevel(void);
BatteryState Battery_GetState(void);

#endif