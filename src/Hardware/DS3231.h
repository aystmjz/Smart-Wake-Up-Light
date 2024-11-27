#ifndef __DS3231_H__
#define __DS3231_H__

extern char TIME[7];
extern unsigned char AlarmStatus[2];
extern unsigned char Alarm_Set[8];
extern char Alarm_Date[2];
extern char Alarm_Date_Temp[2];
extern unsigned char Alarm_Enable;
extern char PWM_Mod;

void DS3231_WriteByte(unsigned char WordAddress,unsigned char Data);
unsigned char DS3231_ReadByte(unsigned char WordAddress);
void DS3231_ReadTime();
void DS3231_WriteTime();
void DS3231_WriteAlarm();
void DS3231_Init();

#endif
