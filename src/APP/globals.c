#include "globals.h"

uint8_t Refresh_Flag = 1;
uint8_t BUZ_Flag = 1;
uint8_t WakeUp_Flag = 0;
uint8_t LowPower_Now = 0;
SHT30TypeDef SHT;
AlarmTypeDef Alarm = {.Num = Alarm_1};
SettingTypeDef Set;
struct tm Time;
PubDataTypeDef PubData = {.SHT = &SHT, .Alarm = &Alarm, .Set = &Set, .Time = &Time};
