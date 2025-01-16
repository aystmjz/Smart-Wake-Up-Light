#include "DS3231.h"

const uint8_t RTC_CLOCK_ADDR[7] = {0x00, 0x01, 0x02, 0x04, 0x05, 0x03, 0x06}; // 秒分时日月周年
const uint8_t RTC_ALARM1_ADDR[4] = {0x07, 0x08, 0x09, 0x0A};
const uint8_t RTC_ALARM2_ADDR[3] = {0x0B, 0x0C, 0x0D};

void DS3231_WriteByte(uint8_t WordAddress, uint8_t Data)
{
	DS3231_I2C_Start();
	DS3231_I2C_SendByte(DS3231_ADDRESS);
	DS3231_I2C_ReceiveAck();
	DS3231_I2C_SendByte(WordAddress);
	DS3231_I2C_ReceiveAck();
	DS3231_I2C_SendByte(Data);
	DS3231_I2C_ReceiveAck();
	DS3231_I2C_Stop();
}

uint8_t DS3231_ReadByte(uint8_t WordAddress)
{
	uint8_t Data;
	DS3231_I2C_Start();
	DS3231_I2C_SendByte(DS3231_ADDRESS);
	DS3231_I2C_ReceiveAck();
	DS3231_I2C_SendByte(WordAddress);
	DS3231_I2C_ReceiveAck();
	DS3231_I2C_Stop();
	DS3231_I2C_Start();
	DS3231_I2C_SendByte(DS3231_ADDRESS | 0x01);
	DS3231_I2C_ReceiveAck();
	Data = DS3231_I2C_ReceiveByte();
	DS3231_I2C_SendAck(1);
	DS3231_I2C_Stop();
	return Data;
}

/// @brief 读取时钟数据
void DS3231_ReadTime(struct tm *Time)
{
	uint8_t temp, time_temp[7];
	for (uint8_t i = 0; i < 7; i++)
	{
		temp = DS3231_ReadByte(RTC_CLOCK_ADDR[i]);
		time_temp[i] = (temp / 16) * 10 + (temp % 16);
	}
	Time->tm_year = time_temp[6] + 2000 - 1900;
	Time->tm_mon = time_temp[4] - 1;
	Time->tm_mday = time_temp[3];
	Time->tm_hour = time_temp[2];
	Time->tm_min = time_temp[1];
	Time->tm_sec = time_temp[0];
	mktime(Time); // 计算星期
}

/// @brief 读取闹钟状态数据
uint8_t DS3231_ReadStatus(AlarmTypeDef *Alarm)
{
	uint8_t temp;
	temp=DS3231_ReadByte(DS3231_STATUS);
	if (Alarm->Num == Alarm_1)
		Alarm->Status = temp& 0x01;
	else if (Alarm->Num == Alarm_2)
		Alarm->Status = (temp & 0x02) >> 1;
	return Alarm->Status;
}

/// @brief 获取时间戳
/// @return 时间戳(北京时间)
time_t DS3231_GetTimeStamp(struct tm *Time)
{
	return (mktime(Time) - 8 * 60 * 60);
}

/// @brief 写入时钟数据
void DS3231_WriteTime(struct tm *Time)
{
	uint8_t time[7];
	mktime(Time); // 计算星期
	time[6] = Time->tm_year + 1900 - 2000;
	time[4] = Time->tm_mon + 1;
	time[5] = Time->tm_wday;
	time[3] = Time->tm_mday;
	time[2] = Time->tm_hour;
	time[1] = Time->tm_min;
	time[0] = Time->tm_sec;
	for (uint8_t i = 0; i < 7; i++)
	{
		DS3231_WriteByte(RTC_CLOCK_ADDR[i], (time[i] / 10) * 16 + time[i] % 10);
	}
}

void DS3231_InitAlarm(AlarmTypeDef *Alarm)
{
	Alarm->Day = 0;
	Alarm->Week = 0;
	Alarm->Hour = 0;
	Alarm->Min = 0;
	Alarm->Enable = 1;
	Alarm->Num = Alarm_1;
	Alarm->Mod = Alarm_MatchHour;
}
void DS3231_WriteAlarm(AlarmTypeDef *Alarm)
{
	uint8_t temp;
	if (Alarm->Num == Alarm_1)
	{
		DS3231_WriteByte(RTC_ALARM1_ADDR[0], 0);
		DS3231_WriteByte(RTC_ALARM1_ADDR[1], ((Alarm->Mod & 0x02) << 6) | ((Alarm->Min / 10) << 4) | (Alarm->Min % 10));
		DS3231_WriteByte(RTC_ALARM1_ADDR[2], ((Alarm->Mod & 0x04) << 5) | ((Alarm->Hour / 10) << 4) | (Alarm->Hour % 10));
		if (Alarm->Mod == Alarm_MatchWeek)
			DS3231_WriteByte(RTC_ALARM1_ADDR[3], ((Alarm->Mod & 0x08) << 4) | 0x40 | Alarm->Week);
		else
			DS3231_WriteByte(RTC_ALARM1_ADDR[3], ((Alarm->Mod & 0x08) << 4) | ((Alarm->Day / 10) << 4) | (Alarm->Day % 10));
		temp = DS3231_ReadByte(DS3231_CONTROL);
		DS3231_WriteByte(DS3231_CONTROL, 0x04 | temp & 0x02 | Alarm->Enable);
	}
	else if (Alarm->Num == Alarm_2)
	{
		DS3231_WriteByte(RTC_ALARM2_ADDR[0], ((Alarm->Mod & 0x02) << 6) | ((Alarm->Min / 10) << 4) | (Alarm->Min % 10));
		DS3231_WriteByte(RTC_ALARM2_ADDR[1], ((Alarm->Mod & 0x04) << 5) | ((Alarm->Hour / 10) << 4) | (Alarm->Hour % 10));
		if (Alarm->Mod == Alarm_MatchWeek)
			DS3231_WriteByte(RTC_ALARM2_ADDR[2], ((Alarm->Mod & 0x08) << 4) | 0x40 | Alarm->Week);
		else
			DS3231_WriteByte(RTC_ALARM2_ADDR[2], ((Alarm->Mod & 0x08) << 4) | ((Alarm->Day / 10) << 4) | (Alarm->Day % 10));
		temp = DS3231_ReadByte(DS3231_CONTROL);
		DS3231_WriteByte(DS3231_CONTROL, 0x04 | temp & 0x01 | Alarm->Enable << 1);
	}
}

/// @brief 复位闹钟
void DS3231_ResetAlarm(void)
{
	DS3231_WriteByte(DS3231_STATUS, 0x00);
}

void DS3231_Init(struct tm *Time, AlarmTypeDef *Alarm)
{
	uint8_t Temp[3];
	DS3231_I2C_Init();
	DS3231_ResetAlarm();
	DS3231_ReadTime(Time);

	Temp[0] = DS3231_ReadByte(DS3231_CONTROL);
	if (Alarm->Num == Alarm_1)
	{
		Alarm->Enable = Temp[0] & 0x01;
		for (uint8_t i = 0; i < 3; i++)
		{
			Temp[i] = DS3231_ReadByte(RTC_ALARM1_ADDR[i + 1]);
		}
	}
	else if (Alarm->Num == Alarm_2)
	{
		Alarm->Enable = (Temp[0] & 0x02) >> 1;
		for (uint8_t i = 0; i < 3; i++)
		{
			Temp[i] = DS3231_ReadByte(RTC_ALARM2_ADDR[i]);
		}
	}
	Alarm->Mod = ((Temp[0] & 0x80) >> 6) | ((Temp[1] & 0x80) >> 5) | ((Temp[2] & 0x80) >> 4) | ((Temp[2] & 0x40) >> 2);
	Alarm->Min = ((Temp[0] & 0x70) >> 4) * 10 + (Temp[0] & 0x0F);
	Alarm->Hour = ((Temp[1] & 0x30) >> 4) * 10 + (Temp[1] & 0x0F);
	if (Temp[2] & 0x40)
		Alarm->Week = Temp[2] & 0x0F;
	else
		Alarm->Day = ((Temp[2] & 0x30) >> 4) * 10 + (Temp[2] & 0x0F);
}

void TimeJudge(struct tm *Time)
{
	if (Time->tm_year < 120)
		Time->tm_year = 150;
	if (Time->tm_year > 150)
		Time->tm_year = 120;
	if (Time->tm_mon < 0)
		Time->tm_mon = 11;
	if (Time->tm_mon > 11)
		Time->tm_mon = 0;
	if (Time->tm_mday < 1)
		Time->tm_mday = 31;
	if (Time->tm_mday > 31)
		Time->tm_mday = 1;
	if (Time->tm_wday < 0)
		Time->tm_wday = 6;
	if (Time->tm_wday > 6)
		Time->tm_wday = 0;
	if (Time->tm_hour < 0)
		Time->tm_hour = 23;
	if (Time->tm_hour > 23)
		Time->tm_hour = 0;
	if (Time->tm_min < 0)
		Time->tm_min = 59;
	if (Time->tm_min > 59)
		Time->tm_min = 0;
	if (Time->tm_sec < 0)
		Time->tm_sec = 59;
	if (Time->tm_sec > 59)
		Time->tm_sec = 0;
	mktime(Time);
}

void Alarm_Judge(AlarmTypeDef *Alarm)
{
	if (Alarm->Hour < 0)
		Alarm->Hour = 23;
	if (Alarm->Hour > 23)
		Alarm->Hour = 0;
	if (Alarm->Min < 0)
		Alarm->Min = 59;
	if (Alarm->Min > 59)
		Alarm->Min = 0;
}

char *Get_Week_Str(uint8_t time_Week)
{
	switch (time_Week)
	{
	case 0:
		return "日";
	case 1:
		return "一";
	case 2:
		return "二";
	case 3:
		return "三";
	case 4:
		return "四";
	case 5:
		return "五";
	case 6:
		return "六";
	}
	return 0;
}
