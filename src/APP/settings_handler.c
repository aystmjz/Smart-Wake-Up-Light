#include "settings_handler.h"


void WriteAlarm(AlarmTypeDef *Alarm, uint8_t *Mod)
{
	AlarmTypeDef alarm_temp;
	alarm_temp = *Alarm;
	alarm_temp.Mod = Alarm_MatchHour;
	PWM_AdjustAlarm(&alarm_temp, Mod, -1);
	DS3231_WriteAlarm(&alarm_temp);
}

// void Music_Time();
void KeyNumber_CTRL1()
{
}

void KeyNumber_CTRL2()
{
}

void KeyNumber_Set_Clock()
{
	uint8_t Time_Year_Str[5], Time_Mon_Str[3], Time_Day_Str[3];
	uint8_t Time_Choose = 0;
	int8_t Time_Choose_Flag = 0;
	int8_t EncoderNum;
	uint8_t KeyNum;

	KeyNum = Key_GetNumber();
	while (KeyNum != 2)
	{
		KeyNum = Key_GetNumber();

		while (!KeyNum)
		{
			KeyNum = Key_GetNumber();

			OLED_Clear(WHITE);

			sprintf((char *)Time_Year_Str, "%d", Time_Year);
			sprintf((char *)Time_Mon_Str, "%d", Time_Mon);
			sprintf((char *)Time_Day_Str, "%d", Time_Day);

			OLED_Printf(104 + 10, 0, OLED_52X104, BLACK, ":");
			OLED_Printf(Time_Hour < 10 ? 62 : 10, 4, OLED_52X104, BLACK, "%d", Time_Hour);
			OLED_Printf(104 + 10 + 20, 4, OLED_52X104, BLACK, "%02d", Time_Min);
			if (Time_Choose_Flag < 0)
			{
				switch (Time_Choose)
				{
				case 0:
					sprintf((char *)Time_Year_Str, "%s", "    ");
					break;
				case 1:
					sprintf((char *)Time_Mon_Str, "%s", Time_Mon > 9 ? "  " : " ");
					break;
				case 2:
					sprintf((char *)Time_Day_Str, "%s", Time_Day > 9 ? "  " : " ");
					break;
				case 3:
					OLED_Printf(10, 4, OLED_52X104, BLACK, ";;");
					break;
				case 4:
					OLED_Printf(104 + 10 + 20, 4, OLED_52X104, BLACK, ";;");
					break;
				}
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
					Time.tm_year += EncoderNum;
					break;
				case 1:
					Time.tm_mon += EncoderNum;
					break;
				case 2:
					Time.tm_mday += EncoderNum;
					break;
				case 3:
					Time.tm_hour += EncoderNum;
					break;
				case 4:
					Time.tm_min += EncoderNum;
					break;
				}
				TimeJudge(&Time);
			}
			OLED_Display(Image_BW, Part);
		}

		switch (KeyNum)
		{
		case 1:
			Time_Choose++;
			Time_Choose %= 5;
			Time_Choose_Flag = -1;
			break;
		case 2:
			DS3231_WriteTime(&Time);
			EPD_WhiteScreen_White();
			break;
		}
	}
}
void KeyNumber_Set_Alarm()
{
	uint8_t Alarm_Hour_Str[3], Alarm_Min_Str[3], Alarm_Flag, Light_Flag, Muzic_Flag, Buzzer_Flag;
	uint8_t Alarm_Choose = 0;
	int8_t Alarm_Choose_Flag = 0;
	int8_t EncoderNum;
	uint8_t KeyNum;
	AlarmTypeDef alarm_temp;
	uint8_t Data[200];

	KeyNum = Key_GetNumber();
	while (KeyNum != 2)
	{
		KeyNum = Key_GetNumber();

		while (!KeyNum)
		{

			KeyNum = Key_GetNumber();

			OLED_Clear(WHITE);

			sprintf((char *)Alarm_Hour_Str, "%d", Alarm.Hour);
			sprintf((char *)Alarm_Min_Str, "%02d", Alarm.Min);
			Alarm_Flag = 1;
			Light_Flag = 1;
			Muzic_Flag = 1;
			Buzzer_Flag = 1;

			OLED_Printf(6, OLED_8X16 * 0 + 1, OLED_8X16, BLACK, "周一 %s", Set.WeekEnable[0] ? "对" : "错");
			OLED_Printf(6, OLED_8X16 * 1 + 3, OLED_8X16, BLACK, "周二 %s", Set.WeekEnable[1] ? "对" : "错");
			OLED_Printf(6, OLED_8X16 * 2 + 5, OLED_8X16, BLACK, "周三 %s", Set.WeekEnable[2] ? "对" : "错");
			OLED_Printf(6, OLED_8X16 * 3 + 7, OLED_8X16, BLACK, "周四 %s", Set.WeekEnable[3] ? "对" : "错");
			OLED_Printf(6, OLED_8X16 * 4 + 9, OLED_8X16, BLACK, "周五 %s", Set.WeekEnable[4] ? "对" : "错");
			OLED_Printf(6, OLED_8X16 * 5 + 11, OLED_8X16, BLACK, "周六 %s", Set.WeekEnable[5] ? "对" : "错");
			OLED_Printf(6, OLED_8X16 * 6 + 13, OLED_8X16, BLACK, "周日 %s", Set.WeekEnable[6] ? "对" : "错");
			OLED_DrawLine(72, 0, 72, 128, BLACK);
			OLED_DrawLine(72, 110, OLED_W, 110, BLACK);

			PWM_ChartData(Data, 200, &Set.PwmMod);
			OLED_DrawChart(80, 0, OLED_W - 80, 88, Data, 200, BLACK);
			alarm_temp = Alarm;
			PWM_AdjustAlarm(&alarm_temp, &Set.PwmMod, -1);
			OLED_Printf(84, 88, OLED_8X16, BLACK, "%d:%02d                  %d:%02d", alarm_temp.Hour, alarm_temp.Min, Alarm.Hour, Alarm.Min);
			OLED_Printf(148, 0, OLED_8X16, BLACK, "光照曲线");

			if (Alarm_Choose_Flag < 0)
			{
				if (Alarm_Choose <= 6)
					OLED_Printf(6, (OLED_8X16 + 2) * Alarm_Choose + 1, OLED_8X16, BLACK, "    ");
				else
					switch (Alarm_Choose)
					{
					case 7:
						Alarm_Flag = 0;
						break;
					case 8:
						sprintf((char *)Alarm_Hour_Str, "%s", Alarm.Hour > 9 ? "  " : " ");
						break;
					case 9:
						sprintf((char *)Alarm_Min_Str, "%s", "  ");
						break;
					case 10:
						Light_Flag = 0;
						break;
					case 11:
						Muzic_Flag = 0;
						break;
					case 12:
						Buzzer_Flag = 0;
						break;
					}
			}

			OLED_Printf(80, 112, OLED_8X16, BLACK, "%s%s:%s 灯%smin  %s%s %s%s", Alarm_Flag ? (Alarm.Enable ? "铃" : "否") : "  ", Alarm_Hour_Str, Alarm_Min_Str, Light_Flag ? Get_PWM_Str(&Set.PwmMod) : (Set.PwmMod == 1 ? " " : "  "), Muzic_Flag ? "乐" : "  ", Set.MuzicEnable ? "对" : "错", Buzzer_Flag ? "符" : "  ", Set.BuzzerEnable ? "对" : "错");

			Alarm_Choose_Flag++;
			if (Alarm_Choose_Flag == 1)
				Alarm_Choose_Flag = -1;

			EncoderNum = Encoder_Get_Div4();
			if (EncoderNum)
			{
				if (Alarm_Choose < 7)
					Set.WeekEnable[Alarm_Choose] = !Set.WeekEnable[Alarm_Choose];
				else
				{
					switch (Alarm_Choose)
					{
					case 7:
						Alarm.Enable = !Alarm.Enable;
						break;
					case 8:
						Alarm.Hour += EncoderNum;
						break;
					case 9:
						Alarm.Min += EncoderNum;
						break;
					case 10:
						Set.PwmMod += EncoderNum;
						break;
					case 11:
						Set.MuzicEnable = !Set.MuzicEnable;
						break;
					case 12:
						Set.BuzzerEnable = !Set.BuzzerEnable;
						break;
					}
				}
				Alarm_Judge(&Alarm);
				PWM_Judge(&Set.PwmMod);
			}

			OLED_Display(Image_BW, Part);
		}

		switch (KeyNum)
		{
		case 1:
			Alarm_Choose++;
			Alarm_Choose %= 13;
			Alarm_Choose_Flag = -1;
			break;
		case 2:
			WriteAlarm(&Alarm, &Set.PwmMod);
			W25Q128_WriteSetting(&Set);
			EPD_WhiteScreen_White();
			break;
		}
	}
}

void KeyNumber_Set_Other()
{
	uint8_t LowPower_Flag;
	uint8_t Other_Choose = 0;
	int8_t Other_Choose_Flag = 0;
	int8_t EncoderNum;
	uint8_t KeyNum;

	KeyNum = Key_GetNumber();
	while (KeyNum != 2)
	{
		KeyNum = Key_GetNumber();

		while (!KeyNum)
		{
			KeyNum = Key_GetNumber();

			OLED_Clear(WHITE);
			LowPower_Flag = 1;

			if (Other_Choose_Flag < 0)
			{
				switch (Other_Choose)
				{
				case 0:
					LowPower_Flag = 0;
					break;
				}
			}
			OLED_Printf(8, OLED_H / 2, OLED_8X16, BLACK, "%s麦%s", LowPower_Flag ? "语音助手常开" : "            ", Set.VoiceEnable ? "对" : "错");

			Other_Choose_Flag++;
			if (Other_Choose_Flag == 1)
				Other_Choose_Flag = -1;

			EncoderNum = Encoder_Get_Div4();
			if (EncoderNum)
			{
				switch (Other_Choose)
				{
				case 0:
					Set.VoiceEnable = !Set.VoiceEnable;
					break;
				}
			}
			OLED_Display(Image_BW, Part);
		}

		switch (KeyNum)
		{
		case 1:
			Other_Choose++;
			Other_Choose %= 1;
			Other_Choose_Flag = -1;
			break;
		case 2:
			W25Q128_WriteSetting(&Set);
			EPD_WhiteScreen_White();
			break;
		}
	}
}
void KeyNumber_Set()
{
	int8_t EncoderNum;
	uint8_t KeyNum;

	OLED_Clear(WHITE);
	OLED_Printf(132, 30, OLED_8X16, BLACK, "设置");
	OLED_Printf(84, 66, OLED_8X16, BLACK, "时间  其他  闹钟");
	OLED_DrawRectangle(76, 28, OLED_W - 76, OLED_H - 28, BLACK, 0);
	OLED_DrawLine(76, 50, OLED_W - 76, 50, BLACK);
	OLED_Display(Image_BW, Part);

	KeyNum = Key_Clear();
	EncoderNum = Encoder_Get_Div4();
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
		KeyNumber_Set_Other();
		break;
	case 2:

		break;
	}
}

