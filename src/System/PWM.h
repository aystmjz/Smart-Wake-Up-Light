#ifndef __PWM_H
#define __PWM_H

#include "sys.h"
#include "Key.h"
#include <time.h>
#include "DS3231.h"

#define PWM_EXIT_HOUR 5
#define PWM_NUM 7
#define PWM_MOD1 5
#define PWM_MOD2 10
#define PWM_MOD3 15
#define PWM_MOD4 20
#define PWM_MOD5 30
#define PWM_MOD6 40
#define PWM_MOD7 60
#define PWM_MOD_TEST 0

#define PWM_MOD1_STR "5"
#define PWM_MOD2_STR "10"
#define PWM_MOD3_STR "15"
#define PWM_MOD4_STR "20"
#define PWM_MOD5_STR "30"
#define PWM_MOD6_STR "40"
#define PWM_MOD7_STR "60"
#define PWM_MOD_TEST_STR "0"

#define PWM_MOD1_CALC(sec) ((sec * sec) / 900 + 1)
#define PWM_MOD2_CALC(sec) (((sec / 2) * (sec / 2)) / 900 + 1)
#define PWM_MOD3_CALC(sec) (((sec / 3) * (sec / 3)) / 900 + 1)
#define PWM_MOD4_CALC(sec) (((sec / 4) * (sec / 4)) / 900 + 1)
#define PWM_MOD5_CALC(sec) (((sec / 6) * (sec / 6)) / 900 + 1)
#define PWM_MOD6_CALC(sec) ((sec / 8) * (sec / 8) / 900 + 1)
#define PWM_MOD7_CALC(sec) (((sec / 12) * (sec / 12)) / 900 + 1)
#define PWM_MOD_TEST_CALC(sec) (100)

void PWM_Init(void);
void PWM_Enable(void);
void PWM_Disable(void);
void PWM_Set(uint16_t Percent);
void PWM_Run(uint8_t *Mod);
void PWM_AdjustAlarm(AlarmTypeDef *Alarm, uint8_t *Mod, int8_t Dir);
void PWM_ChartData(uint8_t *Data, uint8_t Num, uint8_t *Mod);
void PWM_Judge(uint8_t *Mod);
char *Get_PWM_Str(uint8_t *Mod);

#endif
