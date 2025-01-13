#include "sys.h"
#include "UART.h"
// #include "BlueTooth.h"
// #include "Timer0.h"
// #include "Int0.h"
#include "Key.h"
#include "Encoder.h"
#include "OLED.h"
#include "DS3231.h"
#include "I2C.h"
#include "PWM.h"
#include "Buzzer.h"
#include "ASRPRO.h"
#include "LED.h"
#include "SHT30.h"
#include "EXTI.h"
// #include "DHT11.h"
// #include "AT24C02.h"
// #include "Music.h"

#define Music_MAX_NUM 2
#define Music_Volume 2
#define DS3231_STATUS 0x0F

// sbit BUZ = P0 ^ 0;
// sbit RELAY = P2 ^ 0;

int8_t Alarm_Choose_Flag = 0;
int16_t Time_Choose_Flag = 0;
int8_t TIME_Judge;
uint8_t Alarm_Set_Judge[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

uint8_t BUZ_Flag = 1;

uint8_t KeyNum;
uint8_t CmdNum;
uint8_t EXTI_Flag;
int8_t EncoderNum;
uint8_t PWM_Couter = 0, f = 0;
unsigned int PWM_Timer = 0, PWM_Compare = 1;
uint8_t PWM_Timer_Sec = 0, PWM_Timer_Min = 0;
unsigned int PWM_Timer_Sec_Sum = 0;
uint8_t PWM_Run_Flag = 0;
uint8_t PWM_Auto_Flag = 0;
uint8_t Refresh_Flag = 0;
uint8_t Int1_Flag = 0;

unsigned int lx = 0;
uint8_t Music_NUM = 1;
uint8_t Time_Choose = 0;
uint8_t Alarm_Choose = 0;
uint8_t BlueTooth_Refresh_Flag = 0;
uint8_t Alarm_Reset_Flag;
uint8_t Voice_Flag_AZ = 0;
uint8_t Voice_Flag_AZ_Go = 0;
uint8_t Music_Time_Flag = 0;
uint8_t RT_Light_Flag = 0;
extern uint8_t Light_Date[181];

void Wait_Key()
{
	KeyNum = Key_GetNumber();

	while (!KeyNum)
	{
		KeyNum = Key_GetNumber();
	}
}
// void Music_Time();
void KeyNumber_CTRL1()
{
}

void KeyNumber_CTRL2()
{
}

void TimeJudge()
{
	if (Time_Date.tm_year < 120)
		Time_Date.tm_year = 150;
	if (Time_Date.tm_year > 150)
		Time_Date.tm_year = 120;
	if (Time_Date.tm_mon < 0)
		Time_Date.tm_mon = 11;
	if (Time_Date.tm_mon > 11)
		Time_Date.tm_mon = 0;
	if (Time_Date.tm_mday < 1)
		Time_Date.tm_mday = 31;
	if (Time_Date.tm_mday > 31)
		Time_Date.tm_mday = 1;
	if (Time_Date.tm_wday < 0)
		Time_Date.tm_wday = 6;
	if (Time_Date.tm_wday > 6)
		Time_Date.tm_wday = 0;
	if (Time_Date.tm_hour < 0)
		Time_Date.tm_hour = 23;
	if (Time_Date.tm_hour > 23)
		Time_Date.tm_hour = 0;
	if (Time_Date.tm_min < 0)
		Time_Date.tm_min = 59;
	if (Time_Date.tm_min > 59)
		Time_Date.tm_min = 0;
	if (Time_Date.tm_sec < 0)
		Time_Date.tm_sec = 59;
	if (Time_Date.tm_sec > 59)
		Time_Date.tm_sec = 0;
	mktime(&Time_Date);
}
uint8_t *Get_PWM_Str(uint8_t pwm_mod)
{
	switch (pwm_mod)
	{
	case 0:
		return "5";
	case 1:
		return "10";
	case 2:
		return "15";
	case 3:
		return "20";
	case 4:
		return "30";
	case 5:
		return "40";
	case 6:
		return "60";
	case 7:
		return "AT";
	} // 5,10,15,20,30,40,60
	return 0;
}
uint8_t *Get_Week_Str(uint8_t time_Week)
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
void KeyNumber_Set_Clock()
{
	uint8_t Time_Year_Str[5], Time_Mon_Str[3], Time_Day_Str[3];

	KeyNum = Key_GetNumber();
	while (KeyNum != 2)
	{
		KeyNum = Key_GetNumber();

		while (!KeyNum)
		{
			KeyNum = Key_GetNumber();
			if (KeyNum)
				Time_Choose_Flag = 0;

			OLED_Clear(WHITE);

			sprintf((char *)Time_Year_Str, "%d", Time_Year);
			sprintf((char *)Time_Mon_Str, "%d", Time_Mon);
			sprintf((char *)Time_Day_Str, "%d", Time_Day);

			OLED_Printf(104 + 10, 0, OLED_52X104, BLACK, ":");
			OLED_Printf(Time_Hour ? 10 : 62, 4, OLED_52X104, BLACK, "%d", Time_Hour);
			OLED_Printf(104 + 10 + 20, 4, OLED_52X104, BLACK, "%02d", Time_Min);
			switch (Time_Choose)
			{
			case 0:
				if (Time_Choose_Flag < 0)
					sprintf((char *)Time_Year_Str, "%s", "    ");
				break;
			case 1:
				if (Time_Choose_Flag < 0)
					sprintf((char *)Time_Mon_Str, "%s", Time_Mon > 9 ? "  " : " ");
				break;
			case 2:
				if (Time_Choose_Flag < 0)
					sprintf((char *)Time_Day_Str, "%s", Time_Day > 9 ? "  " : " ");
				break;
			case 3:
				if (Time_Choose_Flag < 0)
					OLED_Printf(10, 4, OLED_52X104, BLACK, ";;");
				break;
			case 4:
				if (Time_Choose_Flag < 0)
					OLED_Printf(104 + 10 + 20, 4, OLED_52X104, BLACK, ";;");
				break;
			}
			OLED_Printf(16, 0, OLED_8X16, BLACK, "%s年%s月%s日  周%s  ", Time_Year_Str, Time_Mon_Str, Time_Day_Str, Get_Week_Str(Time_Week));
			OLED_DrawLine(0, 20, LINE_END, 20, BLACK);
			OLED_DrawLine(0, 110, LINE_END, 110, BLACK);
			OLED_DrawLine(LINE_END, 0, LINE_END, 110, BLACK);

			Time_Choose_Flag++;
			if (Time_Choose_Flag == 1)
				Time_Choose_Flag = -1;

			EncoderNum = Encoder_Get_Div4();
			if (EncoderNum)
			{
				switch (Time_Choose)
				{
				case 0:
					Time_Date.tm_year += EncoderNum;
					break;
				case 1:
					Time_Date.tm_mon += EncoderNum;
					break;
				case 2:
					Time_Date.tm_mday += EncoderNum;
					break;
				case 3:
					Time_Date.tm_hour += EncoderNum;
					break;
				case 4:
					Time_Date.tm_min += EncoderNum;
					break;
				}
				TimeJudge();
			}
			OLED_Display(Image_BW, Part);
		}

		switch (KeyNum)
		{
		case 1:
			Time_Choose++;
			Time_Choose %= 5;
			Time_Choose_Flag = -5;
			break;
		case 2:
			DS3231_WriteTime(Time_Date);
			EPD_WhiteScreen_White();
			break;
		}
	}
}
void WriteAlarm()
{
	if (PWM_Mod == 7)
	{
		Alarm_Date[0] = 6;
		Alarm_Date[1] = 0;
		Alarm_Date_Temp[0] = 6;
		Alarm_Date_Temp[1] = 0;
	}
	else
	{
		switch (PWM_Mod)
		{
		case 0:
			Alarm_Date_Temp[0] = (Alarm_Date[0] * 60 + Alarm_Date[1] - 5) / 60;
			Alarm_Date_Temp[1] = (Alarm_Date[0] * 60 + Alarm_Date[1] - 5) % 60;
			break;
		case 1:
			Alarm_Date_Temp[0] = (Alarm_Date[0] * 60 + Alarm_Date[1] - 10) / 60;
			Alarm_Date_Temp[1] = (Alarm_Date[0] * 60 + Alarm_Date[1] - 10) % 60;
			break;
		case 2:
			Alarm_Date_Temp[0] = (Alarm_Date[0] * 60 + Alarm_Date[1] - 15) / 60;
			Alarm_Date_Temp[1] = (Alarm_Date[0] * 60 + Alarm_Date[1] - 15) % 60;
			break;
		case 3:
			Alarm_Date_Temp[0] = (Alarm_Date[0] * 60 + Alarm_Date[1] - 20) / 60;
			Alarm_Date_Temp[1] = (Alarm_Date[0] * 60 + Alarm_Date[1] - 20) % 60;
			break;
		case 4:
			Alarm_Date_Temp[0] = (Alarm_Date[0] * 60 + Alarm_Date[1] - 30) / 60;
			Alarm_Date_Temp[1] = (Alarm_Date[0] * 60 + Alarm_Date[1] - 30) % 60;
			break;
		case 5:
			Alarm_Date_Temp[0] = (Alarm_Date[0] * 60 + Alarm_Date[1] - 40) / 60;
			Alarm_Date_Temp[1] = (Alarm_Date[0] * 60 + Alarm_Date[1] - 40) % 60;
			break;
		case 6:
			Alarm_Date_Temp[0] = (Alarm_Date[0] * 60 + Alarm_Date[1] - 60) / 60;
			Alarm_Date_Temp[1] = (Alarm_Date[0] * 60 + Alarm_Date[1] - 60) % 60;
			break;
		}

		if (Alarm_Date_Temp[0] < 0 || Alarm_Date_Temp[1] < 0)
		{
			Alarm_Date_Temp[0] = (1440 + Alarm_Date_Temp[0] * 60 + Alarm_Date_Temp[1]) / 60;
			Alarm_Date_Temp[1] = (1440 + Alarm_Date_Temp[0] * 60 + Alarm_Date_Temp[1]) % 60;
		}
	}
	DS3231_WriteAlarm();
}

void Alarm_Judge()
{
	if (Alarm_Date[0] < 0)
		Alarm_Date[0] = 23;
	if (Alarm_Date[0] > 23)
		Alarm_Date[0] = 0;
	if (Alarm_Date[1] < 0)
		Alarm_Date[1] = 59;
	if (Alarm_Date[1] > 59)
		Alarm_Date[1] = 0;
	if (PWM_Mod < 0)
		PWM_Mod = 7;
	if (PWM_Mod > 7)
		PWM_Mod = 0;
}

void KeyNumber_Set_Alarm()
{ // 5,10,15,20,30,40,60
	// EPD_WhiteScreen_White();
	OLED_Clear(WHITE);

	OLED_ShowChinese(0, 0 * 16, "周", 16, BLACK);
	if (Alarm_Set[1])
		OLED_ShowChinese(16, 2 * 16, "√", 16, BLACK);
	else
		OLED_ShowChinese(16, 2 * 16, "x", 16, BLACK);
	OLED_ShowChinese(16, 0 * 16, "一", 16, BLACK);
	if (Alarm_Set[2])
		OLED_ShowChinese(32, 2 * 16, "√", 16, BLACK);
	else
		OLED_ShowChinese(32, 2 * 16, "x", 16, BLACK);
	OLED_ShowChinese(32, 0 * 16, "二", 16, BLACK);
	if (Alarm_Set[3])
		OLED_ShowChinese(48, 2 * 16, "√", 16, BLACK);
	else
		OLED_ShowChinese(48, 2 * 16, "x", 16, BLACK);
	OLED_ShowChinese(48, 0 * 16, "三", 16, BLACK);
	if (Alarm_Set[4])
		OLED_ShowChinese(64, 2 * 16, "√", 16, BLACK);
	else
		OLED_ShowChinese(64, 2 * 16, "x", 16, BLACK);
	OLED_ShowChinese(64, 0 * 16, "四", 16, BLACK);
	if (Alarm_Set[5])
		OLED_ShowChinese(80, 2 * 16, "√", 16, BLACK);
	else
		OLED_ShowChinese(80, 2 * 16, "x", 16, BLACK);
	OLED_ShowChinese(80, 0 * 16, "五", 16, BLACK);
	if (Alarm_Set[6])
		OLED_ShowChinese(96, 2 * 16, "√", 16, BLACK);
	else
		OLED_ShowChinese(96, 2 * 16, "x", 16, BLACK);
	OLED_ShowChinese(96, 0 * 16, "六", 16, BLACK);
	if (Alarm_Set[7])
		OLED_ShowChinese(112, 2 * 16, "√", 16, BLACK);
	else
		OLED_ShowChinese(112, 2 * 16, "x", 16, BLACK);
	OLED_ShowChinese(112, 0 * 16, "日", 16, BLACK);
	// if (Alarm_Date[0] / 10)
	//	OLED_ShowChinese(16, 4*16, Alarm_Date[0] / 10 + 10, 16, BLACK);
	// OLED_ShowChinese(32, 4*16, Alarm_Date[0] % 10 + 10, 16, BLACK); // ʱ
	OLED_ShowChinese(48, 4 * 16, ":", 16, BLACK);
	// OLED_ShowChinese(64, 4*16, Alarm_Date[1] / 10 + 10, 16, BLACK); // ؖ
	// OLED_ShowChinese(80, 4*16, Alarm_Date[1] % 10 + 10, 16, BLACK);

	OLED_ShowChinese(0, 6, "闹", 16, BLACK);
	OLED_ShowChinese(16, 6, "钟", 16, BLACK);
	if (Alarm_Enable)
		OLED_ShowChinese(32, 6, "√", 16, BLACK);
	else
		OLED_ShowChinese(32, 6, "x", 16, BLACK);
	OLED_ShowChinese(48, 6, "整", 16, BLACK);
	OLED_ShowChinese(64, 6, "点", 16, BLACK);
	OLED_ShowChinese(80, 6, "报", 16, BLACK);
	OLED_ShowChinese(96, 6, "时", 16, BLACK);
	if (Alarm_Set[0])
		OLED_ShowChinese(112, 6, "√", 16, BLACK);
	else
		OLED_ShowChinese(112, 6, "x", 16, BLACK);

	switch (PWM_Mod) // 5,10,15,20,30,40,60
	{
	case 0:
		OLED_ShowNum(104, 4 * 16, 5, 2, 16, BLACK);
		break;
	case 1:
		OLED_ShowNum(104, 4 * 16, 10, 2, 16, BLACK);
		break;
	case 2:
		OLED_ShowNum(104, 4 * 16, 15, 2, 16, BLACK);
		break;
	case 3:
		OLED_ShowNum(104, 4 * 16, 20, 2, 16, BLACK);
		break;
	case 4:
		OLED_ShowNum(104, 4 * 16, 30, 2, 16, BLACK);
		break;
	case 5:
		OLED_ShowNum(104, 4 * 16, 40, 2, 16, BLACK);
		break;
	case 6:
		OLED_ShowNum(104, 4 * 16, 60, 2, 16, BLACK);
		break;
	case 7:
		OLED_ShowString(104, 4 * 16, "A", 16, BLACK);
		OLED_ShowString(112, 4 * 16, "T", 16, BLACK);
		break;
	} // 5,10,15,20,30,40,60,Auto
	OLED_ShowChar(120, 4 * 16, 'm', 16, BLACK);

	OLED_Display(Image_BW, Part);

	KeyNum = Key_GetNumber();
	while (KeyNum != 2)
	{
		KeyNum = Key_GetNumber();

		while (!KeyNum)
		{

			KeyNum = Key_GetNumber();
			if (KeyNum)
				Alarm_Choose_Flag = 0;
			if (Alarm_Choose == 0)
			{
				if (Alarm_Set[1])
					OLED_ShowChinese(16, 2 * 16, "√", 16, BLACK);
				else
					OLED_ShowChinese(16, 2 * 16, "x", 16, BLACK);
				if (Alarm_Choose_Flag >= 0)
					OLED_ShowChinese(16, 0, "一", 16, BLACK);
				else
					OLED_ShowString(16, 0, "  ", 16, BLACK);
			}
			if (Alarm_Choose == 1)
			{
				if (Alarm_Set[2])
					OLED_ShowChinese(32, 2 * 16, "√", 16, BLACK);
				else
					OLED_ShowChinese(32, 2 * 16, "x", 16, BLACK);
				if (Alarm_Choose_Flag >= 0)
					OLED_ShowChinese(32, 0 * 16, "二", 16, BLACK);
				else
					OLED_ShowString(32, 0, "  ", 16, BLACK);
			}
			if (Alarm_Choose == 2)
			{
				if (Alarm_Set[3])
					OLED_ShowChinese(48, 2 * 16, "√", 16, BLACK);
				else
					OLED_ShowChinese(48, 2 * 16, "x", 16, BLACK);
				if (Alarm_Choose_Flag >= 0)
					OLED_ShowChinese(48, 0 * 16, "三", 16, BLACK);
				else
					OLED_ShowString(48, 0, "  ", 16, BLACK);
			}
			if (Alarm_Choose == 3)
			{
				if (Alarm_Set[4])
					OLED_ShowChinese(64, 2 * 16, "√", 16, BLACK);
				else
					OLED_ShowChinese(64, 2 * 16, "x", 16, BLACK);
				if (Alarm_Choose_Flag >= 0)
					OLED_ShowChinese(64, 0 * 16, "四", 16, BLACK);
				else
					OLED_ShowString(64, 0, "  ", 16, BLACK);
			}
			if (Alarm_Choose == 4)
			{
				if (Alarm_Set[5])
					OLED_ShowChinese(80, 2 * 16, "√", 16, BLACK);
				else
					OLED_ShowChinese(80, 2 * 16, "x", 16, BLACK);
				if (Alarm_Choose_Flag >= 0)
					OLED_ShowChinese(80, 0, "五", 16, BLACK);
				else
					OLED_ShowString(80, 0, "  ", 16, BLACK);
			}
			if (Alarm_Choose == 5)
			{
				if (Alarm_Set[6])
					OLED_ShowChinese(96, 2 * 16, "√", 16, BLACK);
				else
					OLED_ShowChinese(96, 2 * 16, "x", 16, BLACK);
				if (Alarm_Choose_Flag >= 0)
					OLED_ShowChinese(96, 0, "六", 16, BLACK);
				else
					OLED_ShowString(96, 0, "  ", 16, BLACK);
			}
			if (Alarm_Choose == 6)
			{
				if (Alarm_Set[7])
					OLED_ShowChinese(112, 2 * 16, "√", 16, BLACK);
				else
					OLED_ShowChinese(112, 2 * 16, "x", 16, BLACK);
				if (Alarm_Choose_Flag >= 0)
					OLED_ShowChinese(112, 0, "周", 16, BLACK);
				else
					OLED_ShowString(112, 0, "  ", 16, BLACK);
			}

			if (Alarm_Choose == 7)
			{
				if (Alarm_Choose_Flag >= 0)
				{
					// if (Alarm_Date[0] / 10)
					// 	OLED_ShowChinese(16, 4*16, Alarm_Date[0] / 10 + 10, 16, BLACK);
					// OLED_ShowChinese(32, 4*16, Alarm_Date[0] % 10 + 10, 16, BLACK);
				}
				else
					OLED_ShowString(16, 4 * 16, "    ", 16, BLACK);
			}

			if (Alarm_Choose == 8)
			{
				if (Alarm_Choose_Flag >= 0)
				{
					// OLED_ShowChinese(64, 4*16, Alarm_Date[1] / 10 + 10, 16, BLACK); // ؖ
					// OLED_ShowChinese(80, 4*16, Alarm_Date[1] % 10 + 10, 16, BLACK);
				}
				else
					OLED_ShowString(64, 4 * 16, "    ", 16, BLACK);
			}

			if (Alarm_Choose == 9)
			{
				if (Alarm_Enable)
					OLED_ShowChinese(32, 6, "√", 16, BLACK);
				else
					OLED_ShowChinese(32, 6, "x", 16, BLACK);
				if (Alarm_Choose_Flag >= 0)
				{
					OLED_ShowChinese(0, 6, "闹", 16, BLACK);
					OLED_ShowChinese(16, 6, "钟", 16, BLACK);
				}
				else
					OLED_ShowString(0, 6, "    ", 16, BLACK);
			}

			if (Alarm_Choose == 10)
			{
				if (Alarm_Set[0])
					OLED_ShowChinese(112, 6, "√", 16, BLACK);
				else
					OLED_ShowChinese(112, 6, "x", 16, BLACK);
				if (Alarm_Choose_Flag >= 0)
				{
					OLED_ShowChinese(48, 6, "整", 16, BLACK);
					OLED_ShowChinese(64, 6, "点", 16, BLACK);
					OLED_ShowChinese(80, 6, "报", 16, BLACK);
					OLED_ShowChinese(96, 6, "时", 16, BLACK);
				}
				else
					OLED_ShowString(48, 6, "        ", 16, BLACK);
			}

			if (Alarm_Choose == 11)
			{
				switch (PWM_Mod)
				{
				case 0:
					OLED_ShowNum(104, 4 * 16, 5, 2, 16, BLACK);
					break;
				case 1:
					OLED_ShowNum(104, 4 * 16, 10, 2, 16, BLACK);
					break;
				case 2:
					OLED_ShowNum(104, 4 * 16, 15, 2, 16, BLACK);
					break;
				case 3:
					OLED_ShowNum(104, 4 * 16, 20, 2, 16, BLACK);
					break;
				case 4:
					OLED_ShowNum(104, 4 * 16, 30, 2, 16, BLACK);
					break;
				case 5:
					OLED_ShowNum(104, 4 * 16, 40, 2, 16, BLACK);
					break;
				case 6:
					OLED_ShowNum(104, 4 * 16, 60, 2, 16, BLACK);
					break;
				case 7:
					OLED_ShowString(104, 4 * 16, "A", 16, BLACK);
					OLED_ShowString(112, 4 * 16, "T", 16, BLACK);
					break;
				} // 5,10,15,20,30,40,60
				if (Alarm_Choose_Flag >= 0)
					OLED_ShowChar(120, 4 * 16, 'm', 16, BLACK);
				else
					OLED_ShowString(120, 4 * 16, " ", 16, BLACK);
			}

			Alarm_Choose_Flag++;
			if (Alarm_Choose_Flag == 1)
				Alarm_Choose_Flag = -1;

			EncoderNum = Encoder_Get_Div4();
			if (EncoderNum)
			{
				if (Alarm_Choose < 7)
					Alarm_Set[Alarm_Choose + 1] = !Alarm_Set[Alarm_Choose + 1];
				else if (Alarm_Choose < 9)
					Alarm_Date[Alarm_Choose - 7] += EncoderNum;
				else if (Alarm_Choose == 9)
					Alarm_Enable = !Alarm_Enable;
				else if (Alarm_Choose == 11)
					PWM_Mod++;
				else
					Alarm_Set[0] = !Alarm_Set[0];
				Alarm_Judge();
			}

			OLED_Display(Image_BW, Part);
		}

		switch (KeyNum)
		{
		case 1:
			Alarm_Choose++;
			Alarm_Choose %= 12;
			Alarm_Choose_Flag = -2;
			break;
		case 2:
			WriteAlarm();
			EPD_WhiteScreen_White();
			break;
		}
	}
}

// void KeyNumber_Set_Other()
// {
// 	OLED_Clear();
// 	OLED_DrawLight();
// 	Wait_Key();
// 	switch (KeyNum)
// 	{
// 	case 1:
// 	{
// 		uint8_t i, Temp = 0x00;
// 		for (i = 0; i <= 180; i++)
// 		{
// 			AT24C02_WriteByte(Temp, (i * i) / 140);
// 			Temp++;
// 		}
// 		break;
// 	}
// 	case 2:
// 		OLED_Clear();
// 		break;
// 	case 3:
// 		OLED_Clear();
// 		break;
// 	case 4:
// 		OLED_Clear();
// 		break;
// 	}
// }
void KeyNumber_Set()
{
	// EPD_WhiteScreen_White();
	OLED_Clear(WHITE);

	OLED_ShowChinese(48, 0, "设", 16, BLACK);
	OLED_ShowChinese(64, 0, "置", 16, BLACK);

	OLED_ShowChinese(0, 3 * 16, "时", 16, BLACK);
	OLED_ShowChinese(16, 3 * 16, "间", 16, BLACK);

	OLED_ShowChinese(48, 3 * 16, "其", 16, BLACK);
	OLED_ShowChinese(64, 3 * 16, "他", 16, BLACK);

	OLED_ShowChinese(96, 3 * 16, "闹", 16, BLACK);
	OLED_ShowChinese(112, 3 * 16, "钟", 16, BLACK);

	OLED_Display(Image_BW, Part);

	KeyNum = Key_Clear();
	while (KeyNum != 2)
	{
		KeyNum = Key_GetNumber();
		while (!KeyNum)
		{
			KeyNum = Key_GetNumber();
			EncoderNum = Encoder_Get_Div4();

			if (EncoderNum < 0)
			{
				KeyNumber_Set_Clock();
				return;
			}
			else if (EncoderNum > 0)
			{
				KeyNumber_Set_Alarm();
				return;
			}
		}

		switch (KeyNum)
		{
		case 1:
			EPD_WhiteScreen_White();
			KeyNum = 2;
			break;
		case 2:
			EPD_WhiteScreen_White();
			break;
		}
	}
}

void KeyNumber_CTRL4()
{
	// Music_CMD(0, 0, Sotp);
	// UART_SendByte(0xFE);
	// UART_SendByte(0xFE);
	// UART_SendByte(0xFE);

	// DHT11_Read_RH_C();
}

int main()
{
	// Timer0_Init();
	// BlueTooth_Init();
	// Int0_Init();
	// Int1_Init();
	// Timer_Init();
	Key_Init();
	ASRPRO_Init();
	Uart_Init(115200);
	Encoder_Init();
	DS3231_Init();
	OLED_Init();
	Buzzer_Init();
	PWM_Init();
	LED_Init();
	SHT30_Init();
	EXTI0_Init();
	// AT24C02_Init();
	// EX0 = 1;
	// PX1 = 1;
	// IPH = 0x04;
	Refresh_Flag = 1;
	// Music_CMD(0, Music_Volume, Volume);
	Paint_NewImage(Image_BW, OLED_H, OLED_W, ROTATE_180, WHITE);
	EPD_WhiteScreen_White();

	Debug_printf("Init OK\r\n");

	// DS3231_ReadTime();

	// while (1)
	// 	{DS3231_ReadTime();
	// 	OLED_ShowNum(0, 2, Time_Sec, 2, 16, BLACK);
	// 		OLED_Display(Image_BW, Part);
	// 		OLED_Clear(WHITE);
	// 		OLED_Display(Image_BW, Part);
	// 	}

	// OLED_ShowChinese(270, 50, "音", OLED_8X16, BLACK);
	// OLED_ShowImage(0, 0, OLED_W, OLED_H, Image_1, BLACK); // 显示图片

	// while (1)
	// {
	// 	/* code */
	// }

	while (1)
	{
		KeyNum = Key_GetNumber();
		if (KeyNum)
		{
			BUZ_Flag = 0;
			Encoder_Clear();
			switch (KeyNum)
			{
			case 1:
				ASRPRO_Power_Turn();
				break;
			case 2:
				EPD_WeakUp();
				KeyNumber_Set();
				break;
			}
			Refresh_Flag = 1;
			BUZ_Flag = 1;
		}

		if (Refresh_Flag)
		{
			SHT30_GetData();

			OLED_Printf(Time_Hour ? 10 : 62, 4, OLED_52X104, BLACK, "%d", Time_Hour);
			OLED_Printf(104 + 10, 0, OLED_52X104, BLACK, ":");
			OLED_Printf(104 + 10 + 20, 4, OLED_52X104, BLACK, "%02d", Time_Min);
			OLED_Printf(16, 0, OLED_8X16, BLACK, "%d年%d月%d日  周%s", Time_Year, Time_Mon, Time_Day, Get_Week_Str(Time_Week));
			OLED_Printf(16, 112, OLED_8X16, BLACK, "%.2f℃ %.0f%% %s%d:%02d 光%smin 音", SHT.Temp, SHT.Hum, Alarm_Enable ? "铃" : " ", Alarm_Date[0], Alarm_Date[1], Get_PWM_Str(PWM_Mod));
			OLED_DrawLine(0, 20, LINE_END, 20, BLACK);
			OLED_DrawLine(0, 110, LINE_END, 110, BLACK);
			OLED_DrawLine(LINE_END, 0, LINE_END, OLED_H, BLACK);
			if (Alarm_Set[1])
				OLED_Printf(LINE_END + 6, OLED_8X16 * 0 + 1, OLED_8X16, BLACK, "周一");
			if (Alarm_Set[2])
				OLED_Printf(LINE_END + 6, OLED_8X16 * 1 + 3, OLED_8X16, BLACK, "周二");
			if (Alarm_Set[3])
				OLED_Printf(LINE_END + 6, OLED_8X16 * 2 + 5, OLED_8X16, BLACK, "周三");
			if (Alarm_Set[4])
				OLED_Printf(LINE_END + 6, OLED_8X16 * 3 + 7, OLED_8X16, BLACK, "周四");
			if (Alarm_Set[5])
				OLED_Printf(LINE_END + 6, OLED_8X16 * 4 + 9, OLED_8X16, BLACK, "周五");
			if (Alarm_Set[6])
				OLED_Printf(LINE_END + 6, OLED_8X16 * 5 + 11, OLED_8X16, BLACK, "周六");
			if (Alarm_Set[7])
				OLED_Printf(LINE_END + 6, OLED_8X16 * 6 + 13, OLED_8X16, BLACK, "周日");
		}

		if (Refresh_Flag)
		{
			OLED_Display(Image_BW, Part);
			EPD_DeepSleep();
		}
		Refresh_Flag = 0;

		if (TIME_Judge != Time_Min)
		{
			Refresh_Flag = 1;
			TIME_Judge = Time_Min;
		}

		if (Time_Min)
		{
			BUZ_Flag = 1;
		}

		KeyNum = Key_GetNumber();
		CmdNum = ASRPRO_Get_CMD();
		EXTI_Flag = EXTI0_Get_Flag();

		if (EXTI_Flag)
		{
			PWM_Run();
			DS3231_ResetAlarm();
		}

		Delay_ms(1000);
		DS3231_ReadTime();

		if (CmdNum)
		{
			switch (CmdNum)
			{
			case 1:
				ASRPRO_printf("%c%c%c%c", 0xaa, 0x00, 0x01, (uint8_t)SHT.Temp);
				break;
			case 2:
				ASRPRO_printf("%c%c%c%c", 0xaa, 0x00, 0x02, (uint8_t)SHT.Hum);
				break;
			case 3:
				ASRPRO_printf("%c%c%c%c%c%c", 0xaa, 0x00, 0x03, (uint8_t)Time_Hour, (uint8_t)Time_Min, (uint8_t)Time_Sec);
				break;
			default:
				break;
			}
		}
	}
}

// #define CMD_Get 0x01
// #define CMD_Switch 0x02
// #define CMD_Set 0x03
// #define CMD_AlarmReset 0x04
// #define CMD_OVER 0xFF
// #define LF 0x0A
// uint8_t UART_RX_BUF[4];

// void BlueTooth_CMD_Get()
// {
// 	uint8_t i;
// 	if (UART_RX_BUF[1] == 1)
// 	{
// 		BlueTooth_SendString("当前时间为：");
// 		BlueTooth_SendByte(LF);
// 		BlueTooth_SendString("20");
// 		BlueTooth_SendNum(Time_Year);
// 		BlueTooth_SendString("年");
// 		BlueTooth_SendNum(Time_Mon);
// 		BlueTooth_SendString("月");
// 		BlueTooth_SendNum(Time_Day);
// 		BlueTooth_SendString("日");

// 		BlueTooth_SendString("周");
// 		switch (Time_Week)
// 		{
// 		case 1:
// 			BlueTooth_SendString("一");
// 			break;
// 		case 2:
// 			BlueTooth_SendString("二");
// 			break;
// 		case 3:
// 			BlueTooth_SendString("三");
// 			break;
// 		case 4:
// 			BlueTooth_SendString("四");
// 			break;
// 		case 5:
// 			BlueTooth_SendString("五");
// 			break;
// 		case 6:
// 			BlueTooth_SendString("六");
// 			break;
// 		case 7:
// 			BlueTooth_SendString("日");
// 			break;
// 		}

// 		BlueTooth_SendString(" ");
// 		BlueTooth_SendNum(Time_Hour);
// 		BlueTooth_SendString(":");
// 		BlueTooth_SendNum(Time_Min);

// 		BlueTooth_SendString(" ");
// 		BlueTooth_SendNum(Time_Sec / 10);
// 		BlueTooth_SendNum(Time_Sec % 10);
// 		BlueTooth_SendString("s");
// 		BlueTooth_SendByte(LF);
// 	}
// 	else if (UART_RX_BUF[1] == 2)
// 	{
// 		BlueTooth_SendString("当前温度:");
// 		BlueTooth_SendNum(DHT11_RH_C[1]);
// 		BlueTooth_SendString(".");
// 		BlueTooth_SendNum(DHT11_RH_C[2]);
// 		BlueTooth_SendString("℃ ");
// 		BlueTooth_SendString("湿度:");
// 		BlueTooth_SendNum(DHT11_RH_C[0]);
// 		BlueTooth_SendString("RH ");
// 		BlueTooth_SendString("光照:");
// 		BlueTooth_SendNum(lx);
// 		BlueTooth_SendString("lx");
// 		BlueTooth_SendByte(LF);
// 	}
// 	else if (UART_RX_BUF[1] == 3)
// 	{
// 		BlueTooth_SendString("当前闹钟信息：");
// 		BlueTooth_SendByte(LF);
// 		BlueTooth_SendString("闹钟:");
// 		if (Alarm_Enable)
// 			BlueTooth_SendString("开 ");
// 		else
// 			BlueTooth_SendString("关 ");

// 		BlueTooth_SendString("整点报时:");
// 		if (Alarm_Set[0])
// 			BlueTooth_SendString("开");
// 		else
// 			BlueTooth_SendString("关");

// 		BlueTooth_SendByte(LF);
// 		BlueTooth_SendString("设置时间：");
// 		BlueTooth_SendNum(Alarm_Date[0]);
// 		BlueTooth_SendString(":");
// 		BlueTooth_SendNum(Alarm_Date[1] / 10);
// 		BlueTooth_SendNum(Alarm_Date[1] % 10);

// 		BlueTooth_SendByte(LF);
// 		BlueTooth_SendString("开始时间：");
// 		BlueTooth_SendNum(Alarm_Date_Temp[0]);
// 		BlueTooth_SendString(":");
// 		BlueTooth_SendNum(Alarm_Date_Temp[1] / 10);
// 		BlueTooth_SendNum(Alarm_Date_Temp[1] % 10);

// 		BlueTooth_SendByte(LF);
// 		BlueTooth_SendString("         唤醒周期：");
// 		switch (PWM_Mod)
// 		{
// 		case 0:
// 			BlueTooth_SendString("5");
// 			break;
// 		case 1:
// 			BlueTooth_SendString("10");
// 			break;
// 		case 2:
// 			BlueTooth_SendString("15");
// 			break;
// 		case 3:
// 			BlueTooth_SendString("20");
// 			break;
// 		case 4:
// 			BlueTooth_SendString("30");
// 			break;
// 		case 5:
// 			BlueTooth_SendString("40");
// 			break;
// 		case 6:
// 			BlueTooth_SendString("60");
// 			break;
// 		}
// 		BlueTooth_SendString("分钟");

// 		BlueTooth_SendByte(LF);
// 		BlueTooth_SendString("闹钟周期:周");
// 		for (i = 1; i <= 7; i++)
// 		{
// 			if (Alarm_Set[i])
// 			{
// 				switch (i)
// 				{
// 				case 1:
// 					BlueTooth_SendString("一 ");
// 					break;
// 				case 2:
// 					BlueTooth_SendString("二 ");
// 					break;
// 				case 3:
// 					BlueTooth_SendString("三 ");
// 					break;
// 				case 4:
// 					BlueTooth_SendString("四 ");
// 					break;
// 				case 5:
// 					BlueTooth_SendString("五 ");
// 					break;
// 				case 6:
// 					BlueTooth_SendString("六 ");
// 					break;
// 				case 7:
// 					BlueTooth_SendString("日 ");
// 					break;
// 				}
// 			}
// 		}
// 		BlueTooth_SendByte(LF);
// 	}
// 	else
// 		BlueTooth_SendString("未知指令");
// }

// void BlueTooth_CMD_Switch()
// {
// 	switch (UART_RX_BUF[1])
// 	{
// 	case 1:
// 		Alarm_Set[1] = UART_RX_BUF[2];
// 		BlueTooth_SendString("星期一闹钟已");
// 		if (Alarm_Enable)
// 			BlueTooth_SendString("开启");
// 		else
// 			BlueTooth_SendString("关闭");
// 		BlueTooth_SendByte(LF);
// 		break;
// 	case 2:
// 		Alarm_Set[2] = UART_RX_BUF[2];
// 		BlueTooth_SendString("星期二闹钟已");
// 		if (Alarm_Enable)
// 			BlueTooth_SendString("开启");
// 		else
// 			BlueTooth_SendString("关闭");
// 		BlueTooth_SendByte(LF);
// 		break;
// 	case 3:
// 		Alarm_Set[3] = UART_RX_BUF[2];
// 		BlueTooth_SendString("星期三闹钟已");
// 		if (Alarm_Enable)
// 			BlueTooth_SendString("开启");
// 		else
// 			BlueTooth_SendString("关闭");
// 		BlueTooth_SendByte(LF);
// 		break;
// 	case 4:
// 		Alarm_Set[4] = UART_RX_BUF[2];
// 		BlueTooth_SendString("星期四闹钟已");
// 		if (Alarm_Enable)
// 			BlueTooth_SendString("开启");
// 		else
// 			BlueTooth_SendString("关闭");
// 		BlueTooth_SendByte(LF);
// 		break;
// 	case 5:
// 		Alarm_Set[5] = UART_RX_BUF[2];
// 		BlueTooth_SendString("星期五闹钟已");
// 		if (Alarm_Enable)
// 			BlueTooth_SendString("开启");
// 		else
// 			BlueTooth_SendString("关闭");
// 		BlueTooth_SendByte(LF);
// 		break;
// 	case 6:
// 		Alarm_Set[6] = UART_RX_BUF[2];
// 		BlueTooth_SendString("星期六闹钟已");
// 		if (Alarm_Enable)
// 			BlueTooth_SendString("开启");
// 		else
// 			BlueTooth_SendString("关闭");
// 		BlueTooth_SendByte(LF);
// 		break;
// 	case 7:
// 		Alarm_Set[7] = UART_RX_BUF[2];
// 		BlueTooth_SendString("星期日闹钟已");
// 		if (Alarm_Enable)
// 			BlueTooth_SendString("开启");
// 		else
// 			BlueTooth_SendString("关闭");
// 		BlueTooth_SendByte(LF);
// 		break;

// 	case 8:
// 		Alarm_Enable = UART_RX_BUF[2];
// 		BlueTooth_SendString("闹钟已");
// 		if (Alarm_Enable)
// 			BlueTooth_SendString("开启");
// 		else
// 			BlueTooth_SendString("关闭");
// 		BlueTooth_SendByte(LF);
// 		break;
// 	case 9:
// 		Alarm_Set[0] = UART_RX_BUF[2];
// 		BlueTooth_SendString("整点报时已");
// 		if (Alarm_Set[0])
// 			BlueTooth_SendString("开启");
// 		else
// 			BlueTooth_SendString("关闭");
// 		BlueTooth_SendByte(LF);
// 		break;
// 	default:
// 		BlueTooth_SendString("未知指令");
// 		break;
// 	}
// 	BlueTooth_Refresh_Flag = 1;
// }

// void BlueTooth_CMD_Set()
// {
// 	if (UART_RX_BUF[1] == 1)
// 	{
// 		Time_Hour = (UART_RX_BUF[2] >> 4) * 10 + (UART_RX_BUF[2] & 0x0F);
// 		Time_Min = (UART_RX_BUF[3] >> 4) * 10 + (UART_RX_BUF[3] & 0x0F);
// 		BlueTooth_SendString("时间 ");
// 		BlueTooth_SendNum(Time_Hour);
// 		BlueTooth_SendString(":");
// 		BlueTooth_SendNum(Time_Min / 10);
// 		BlueTooth_SendNum(Time_Min % 10);
// 		BlueTooth_SendString(" 已设置完成");
// 		BlueTooth_SendByte(LF);
// 	}
// 	else if (UART_RX_BUF[1] == 2)
// 	{
// 		Alarm_Date[0] = (UART_RX_BUF[2] >> 4) * 10 + (UART_RX_BUF[2] & 0x0F);
// 		Alarm_Date[1] = (UART_RX_BUF[3] >> 4) * 10 + (UART_RX_BUF[3] & 0x0F);
// 		BlueTooth_SendString("闹钟 ");
// 		BlueTooth_SendNum(Alarm_Date[0]);
// 		BlueTooth_SendString(":");
// 		BlueTooth_SendNum(Alarm_Date[1] / 10);
// 		BlueTooth_SendNum(Alarm_Date[1] % 10);
// 		BlueTooth_SendString(" 已设置完成");
// 		BlueTooth_SendByte(LF);
// 		BlueTooth_SendString("唤醒周期：");
// 		switch (PWM_Mod)
// 		{
// 		case 0:
// 			BlueTooth_SendString("5");
// 			break;
// 		case 1:
// 			BlueTooth_SendString("10");
// 			break;
// 		case 2:
// 			BlueTooth_SendString("15");
// 			break;
// 		case 3:
// 			BlueTooth_SendString("20");
// 			break;
// 		case 4:
// 			BlueTooth_SendString("30");
// 			break;
// 		case 5:
// 			BlueTooth_SendString("40");
// 			break;
// 		case 6:
// 			BlueTooth_SendString("60");
// 			break;
// 		}
// 		BlueTooth_SendString("分钟");
// 		BlueTooth_SendByte(LF);
// 	}
// 	else if (UART_RX_BUF[1] == 3)
// 	{
// 		switch (UART_RX_BUF[2])
// 		{
// 		case 0:
// 			PWM_Mod = 0;
// 			break;
// 		case 1:
// 			PWM_Mod = 1;
// 			break;
// 		case 2:
// 			PWM_Mod = 2;
// 			break;
// 		case 3:
// 			PWM_Mod = 3;
// 			break;
// 		case 4:
// 			PWM_Mod = 4;
// 			break;
// 		case 5:
// 			PWM_Mod = 5;
// 			break;
// 		case 6:
// 			PWM_Mod = 6;
// 			break;
// 		}
// 		BlueTooth_SendString("唤醒周期 ");
// 		switch (PWM_Mod)
// 		{
// 		case 0:
// 			BlueTooth_SendString("5");
// 			break;
// 		case 1:
// 			BlueTooth_SendString("10");
// 			break;
// 		case 2:
// 			BlueTooth_SendString("15");
// 			break;
// 		case 3:
// 			BlueTooth_SendString("20");
// 			break;
// 		case 4:
// 			BlueTooth_SendString("30");
// 			break;
// 		case 5:
// 			BlueTooth_SendString("40");
// 			break;
// 		case 6:
// 			BlueTooth_SendString("60");
// 			break;
// 		}

// 		BlueTooth_SendString("分钟 已设置完成");
// 		BlueTooth_SendByte(LF);
// 	}
// 	else
// 		BlueTooth_SendString("未知指令");

// 	BlueTooth_Refresh_Flag = 1;
// }

// void BlueTooth_CMD_AlarmReset()
// {
// 	Int1_Flag = 1;
// }

// void BlueTooth_CMD_Default()
// {
// 	BlueTooth_SendString("未知指令");
// }

// uint8_t UART_State = 0;

// // TIME[7] = {0, 0, 0x16, 0x1C, 0x06, 0x01, 0x17};//秒分时日月周年
// uint8_t Music_Time_Wait_Flag = 0;

// void Music_Time_Wait()
// {
// 	Music_Time_Wait_Flag = 0;
// 	while (!Music_Time_Wait_Flag)
// 	{
// 	}

// 	Music_Time_Wait_Flag = 0;
// }
// void Music_Time_Hour()
// {
// 	switch (Time_Hour)
// 	{
// 	case 1:
// 		Music_CMD(4, 1, Play_Folder);
// 		break;
// 	case 2:
// 		Music_CMD(4, 2, Play_Folder);
// 		break;
// 	case 3:
// 		Music_CMD(4, 3, Play_Folder);
// 		break;
// 	case 4:
// 		Music_CMD(4, 4, Play_Folder);
// 		break;
// 	case 5:
// 		Music_CMD(4, 5, Play_Folder);
// 		break;
// 	case 6:
// 		Music_CMD(4, 6, Play_Folder);
// 		break;
// 	case 7:
// 		Music_CMD(4, 7, Play_Folder);
// 		break;
// 	case 8:
// 		Music_CMD(4, 8, Play_Folder);
// 		break;
// 	case 9:
// 		Music_CMD(4, 9, Play_Folder);
// 		break;
// 	case 10:
// 		Music_CMD(4, 10, Play_Folder);
// 		break;
// 	case 11:
// 		Music_CMD(4, 11, Play_Folder);
// 		break;
// 	case 12:
// 		Music_CMD(4, 12, Play_Folder);
// 		break;
// 	case 13:
// 		Music_CMD(4, 1, Play_Folder);
// 		break;
// 	case 14:
// 		Music_CMD(4, 2, Play_Folder);
// 		break;
// 	case 15:
// 		Music_CMD(4, 3, Play_Folder);
// 		break;
// 	case 16:
// 		Music_CMD(4, 4, Play_Folder);
// 		break;
// 	case 17:
// 		Music_CMD(4, 5, Play_Folder);
// 		break;
// 	case 18:
// 		Music_CMD(4, 6, Play_Folder);
// 		break;
// 	case 19:
// 		Music_CMD(4, 7, Play_Folder);
// 		break;
// 	case 20:
// 		Music_CMD(4, 8, Play_Folder);
// 		break;
// 	case 21:
// 		Music_CMD(4, 9, Play_Folder);
// 		break;
// 	case 22:
// 		Music_CMD(4, 10, Play_Folder);
// 		break;
// 	case 23:
// 		Music_CMD(4, 11, Play_Folder);
// 		break;
// 	case 0:
// 		Music_CMD(4, 12, Play_Folder);
// 		break;
// 	}
// }

// void UART_BlueTooth(uint8_t Date)
// {
// 	static uint8_t UART_BlueTooth_NUM = 0;
// 	Date = 0;
// 	UART_BlueTooth_NUM++;
// 	UART_State = 0;
// }

// void UART_Light(uint8_t Date) // 4
// {
// 	static uint8_t UART_Light_NUM = 0, UART_Light_BUF[5];
// 	UART_Light_NUM++;
// 	if (UART_Light_NUM <= 4)
// 	{
// 		UART_Light_BUF[UART_Light_NUM] = Date;
// 		if (UART_Light_NUM == 4)
// 		{
// 			if (UART_Check(2, UART_Light_BUF))
// 			{
// 				lx = UART_Light_BUF[1] * 256 + UART_Light_BUF[2];
// 			}
// 		}
// 	}
// 	else if (UART_Light_NUM > 4 && Date == 0xEF)
// 	{
// 		UART_Light_NUM = 0;
// 		UART_State = 0;
// 	}
// }

// void UART_Voice(uint8_t Date)//4
// {
// 	static uint8_t UART_Voice_NUM=0,UART_Voice_BUF[5];
// 	UART_Voice_NUM++;
// 	if(UART_Voice_NUM<=4)
// 	{
// 		UART_Voice_BUF[UART_Voice_NUM]=Date;
// 		if(UART_Voice_NUM==4)
// 		{
// 			if(UART_Check(2,UART_Voice_BUF))
// 			{
// 				if(UART_Voice_BUF[2]==0x01||UART_Voice_BUF[2]==0x62||UART_Voice_BUF[2]==0x64)
// 				{
// 					Voice_Flag_AZ=1;
// 					Voice_Flag_AZ_Go=1;
// 					return;
// 				}
// 				if(Voice_Flag_AZ)
// 				{
// 					if(UART_Voice_BUF[1])
// 					{
// 						if(UART_Voice_BUF[2]==0xEF)
// 						{
// 							Voice_NUM=0;
// 							Voice_Flag_AZ=0;
// 							return;
// 						}
// 						Voice_Flag=1;
// 						Voice_NUM++;
// 						Voice_BUF[Voice_NUM]=UART_Voice_BUF[2];

// 					}
// 					else
// 					{
// 						switch(UART_Voice_BUF[2])
// 						{
// 							case 0xF0: Voice_Flag_AZ=0;break;
// 							case 0xF1: Voice_Flag_AZ=0;break;
// 							case 0xF2: Voice_Flag_AZ=0;break;
// 							case 0xF3: Music_CMD(0,Music_Volume,Volume);Music_Time_Flag=1;Voice_Flag_AZ=0;break;
// 							case 0xF4: Music_CMD(0,Music_Volume,Volume);Music_CMD(0,Music_NUM+1,Play_Music);Voice_Flag_AZ=0;break;
// 							case 0xF5: Music_NUM++;Music_NUM%=Music_MAX_NUM;Music_CMD(0,1,Volume);Music_CMD(0,Music_NUM+1,Play_Music);Voice_Flag_AZ=0;break;
// 							case 0xF6: Music_CMD(0,Music_Volume,Volume);Music_CMD(2,5,Play_Folder);Voice_Flag_AZ=0;break;
// 							case 0xF7:
// 							{
// 								Refresh_Flag=1;
// 								Alarm_Date[0]=6;
// 								Alarm_Date[1]=0;
// 								Alarm_Date_Temp[0]=6;
// 								Alarm_Date_Temp[1]=0;
// 								DS3231_WriteAlarm();
// 								RT_Light_Flag=1;
// 								Music_CMD(0,Music_Volume,Volume);
// 								Music_CMD(2,4,Play_Folder);
// 								Voice_Flag_AZ=0;
// 								break;
// 							}
// 						}
// 					}
// 				}
// 			}
// 		}
// 	}

// 	else if(UART_Voice_NUM>4&&Date==0xEF)
// 	{
// 		UART_Voice_NUM=0;
// 		UART_State=0;
// 	}
// }
// uint8_t SBUF_Temp,SBUF_Flag=0;
// uint8_t SWITCH=1;

// void UART_Routine() interrupt 4
// {
// 	uint8_t Temp;
// 	if(RI==1)
// 	{
// 		RI=0;
// 		if(SWITCH)
// 		{
// 			if(!PWM_Run_Flag)
// 			{
// 				Temp=SBUF;
// 				if(!UART_State)
// 				{
// 					switch(Temp)
// 					{
// 						case 0xEE: UART_State=1;break;
// 						case 0xFE: UART_State=2;break;
// 						case 0x7E: UART_State=3;break;
// 						case 0xDE: UART_State=4;break;
// 					}
// 				}
// 				else
// 				{
// 					switch(UART_State)
// 					{
// 						case 1: UART_BlueTooth(Temp);break;
// 						case 2: UART_Light(Temp);break;
// 						case 3: UART_Music(Temp);break;
// 						case 4: UART_Voice(Temp);break;
// 					}
// 				}
// 			}
// 		}
// 		else
// 		{
// 			static uint8_t BUF_NUM=0;
// 			Temp=SBUF;
// 			if(Temp==CMD_OVER)
// 			{
// 				switch(UART_RX_BUF[0])
// 				{
// 					case CMD_Get: BlueTooth_CMD_Get();break;
// 					case CMD_Switch: BlueTooth_CMD_Switch();break;
// 					case CMD_Set: BlueTooth_CMD_Set();break;
// 					case CMD_AlarmReset: BlueTooth_CMD_AlarmReset();break;
// 					default: BlueTooth_CMD_Default();break;
// 				}
// 				BUF_NUM=0;
// 			}
// 			else
// 			{
// 				UART_RX_BUF[BUF_NUM++]=Temp;
// 				BUF_NUM%=4;
// 			}
// 		}
// 	}
// }

void TIM2_IRQHandler(void) // 1ms
{
	static uint16_t Key_Counter = 0, Buzzer_Counter = 0;
	static int16_t Encoder_Last = 0;
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)
	{
		Key_Counter++;
		if (Key_Counter >= 40)
		{
			Key_Counter = 0;
			Key_Entry();
			if (Encoder_Last != Encoder_GetCounter())
			{
				Encoder_Last = Encoder_GetCounter();
				Buzzer_Flag = 1;
			}
		}
		if (Buzzer_Flag)
		{
			Buzzer_Flag = 0;
			Buzzer_Counter = 20;
			Buzzer_ON();
		}
		if (Buzzer_Counter)
		{
			if (Buzzer_Counter == 1)
				Buzzer_OFF();
			Buzzer_Counter--;
		}
		if ((!Time_Min) && BUZ_Flag)
		{
			BUZ_Flag = 0;
			Buzzer_Counter = 200;
			Buzzer_ON();
		}
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	}
}
