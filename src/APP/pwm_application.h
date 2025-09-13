#ifndef __PWM_APPLICATION_H__
#define __PWM_APPLICATION_H__

#include "BT24.h"
#include "DS3231.h"
#include "Key.h"
#include "PWM.h"
#include "sys.h"

// PWM退出时间设置，单位为小时
#define PWM_EXIT_HOUR          5

// PWM模式总数
#define PWM_NUM                7

// 各种PWM模式的时间参数（分钟）
#define PWM_MOD1               5
#define PWM_MOD2               10
#define PWM_MOD3               15
#define PWM_MOD4               20
#define PWM_MOD5               30
#define PWM_MOD6               40
#define PWM_MOD7               60
#define PWM_MOD_TEST           0

// 各种PWM模式的字符串表示
#define PWM_MOD1_STR           "5"
#define PWM_MOD2_STR           "10"
#define PWM_MOD3_STR           "15"
#define PWM_MOD4_STR           "20"
#define PWM_MOD5_STR           "30"
#define PWM_MOD6_STR           "40"
#define PWM_MOD7_STR           "60"
#define PWM_MOD_TEST_STR       "0"

// 各种PWM模式的亮度计算公式
// 模式1：5分钟渐亮模式，使用二次函数计算亮度
#define PWM_MOD1_CALC(sec)     (((sec / 1) * (sec / 1)) / 900 + 1)
// 模式2：10分钟渐亮模式，使用二次函数计算亮度
#define PWM_MOD2_CALC(sec)     (((sec / 2) * (sec / 2)) / 900 + 1)
// 模式3：15分钟渐亮模式，使用二次函数计算亮度
#define PWM_MOD3_CALC(sec)     (((sec / 3) * (sec / 3)) / 900 + 1)
// 模式4：20分钟渐亮模式，使用二次函数计算亮度
#define PWM_MOD4_CALC(sec)     (((sec / 4) * (sec / 4)) / 900 + 1)
// 模式5：30分钟渐亮模式，使用二次函数计算亮度
#define PWM_MOD5_CALC(sec)     (((sec / 6) * (sec / 6)) / 900 + 1)
// 模式6：40分钟渐亮模式，使用二次函数计算亮度
#define PWM_MOD6_CALC(sec)     (((sec / 8) * (sec / 8)) / 900 + 1)
// 模式7：60分钟渐亮模式，使用二次函数计算亮度
#define PWM_MOD7_CALC(sec)     (((sec / 12) * (sec / 12)) / 900 + 1)
// 测试模式：直接输出100%亮度
#define PWM_MOD_TEST_CALC(sec) (100)

void PWM_Run(uint8_t *Mod);
void PWM_AdjustAlarm(AlarmTypeDef *Alarm, uint8_t *Mod, int8_t Dir);
void PWM_ChartData(uint8_t *Data, uint8_t Num, uint8_t *Mod);
void PWM_Judge(uint8_t *Mod);
char *Get_PWM_Str(uint8_t *Mod);

#endif