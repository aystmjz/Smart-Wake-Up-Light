#include "sys.h"
#include "UART.h"
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
#include "W25Q128.h"
#include "PWR.h"

uint8_t BUZ_Flag = 1;
AlarmTypeDef Alarm = {.Num = Alarm_1};
SettingTypeDef Set;
struct tm Time;
void Wait_Key()
{
	uint8_t KeyNum;
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
			OLED_Printf(Time_Hour ? 10 : 62, 4, OLED_52X104, BLACK, "%d", Time_Hour);
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
void WriteAlarm(AlarmTypeDef *Alarm, uint8_t *Mod)
{
	AlarmTypeDef alarm_temp;
	alarm_temp = *Alarm;
	alarm_temp.Mod = Alarm_MatchHour;
	PWM_AdjustAlarm(&alarm_temp, Mod, -1);
	DS3231_WriteAlarm(&alarm_temp);
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

			OLED_Printf(80, 112, OLED_8X16, BLACK, "%s%s:%s 灯%smin  %s%s %s%s", Alarm_Flag ? (Alarm.Enable ? "铃" : "否") : "  ", Alarm_Hour_Str, Alarm_Min_Str, Light_Flag ? Get_PWM_Str(&Set.PwmMod) : (Set.PwmMod == 1 ? " " : "  "), Muzic_Flag ? "音" : "  ", Set.MuzicEnable ? "对" : "错", Buzzer_Flag ? "符" : "  ", Set.BuzzerEnable ? "对" : "错");

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
			OLED_Printf(8, OLED_H / 2, OLED_8X16, BLACK, "%s叶%s", LowPower_Flag ? "低功耗模式" : "          ", Set.LowPowerEnable ? "对" : "错");

			Other_Choose_Flag++;
			if (Other_Choose_Flag == 1)
				Other_Choose_Flag = -1;

			EncoderNum = Encoder_Get_Div4();
			if (EncoderNum)
			{
				switch (Other_Choose)
				{
				case 0:
					Set.LowPowerEnable = !Set.LowPowerEnable;
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

	Delay_ms(1000);
	KeyNum = Key_Clear();
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
		KeyNumber_Set_Other();
		break;
	}
}

void LowPowerON(void)
{
	AlarmTypeDef alarm;
	DS3231_InitAlarm(&alarm);
	alarm.Enable = 1;
	alarm.Num = Alarm_2;
	alarm.Mod = Alarm_PerMin;
	DS3231_WriteAlarm(&alarm);
}

void LowPowerOFF(void)
{
	AlarmTypeDef alarm;
	DS3231_InitAlarm(&alarm);
	alarm.Enable = 0;
	alarm.Num = Alarm_2;
	DS3231_WriteAlarm(&alarm);
}

int main()
{
	uint8_t Refresh_Flag = 1, TIME_Judge = 0, ASRPRO_Status = 0;
	uint8_t KeyNum, CmdNum;
	uint8_t MID = 0;
	uint16_t DID = 0;
	char SendBuf[20];

	Key_Init();
	Encoder_Init();
	OLED_Init();
	SHT30_Init();
	EXTI0_Init();
	W25Q128_Init();
	ASRPRO_Init();
	Buzzer_Init();
	Uart_Init(115200);
	DS3231_Init(&Time, &Alarm);

	W25Q128_ReadID(&MID, &DID);
	sprintf(SendBuf, "MID=%d DID=%d\r\n", MID, DID);
	Debug_printf(SendBuf);
	if (MID)
	{
		W25Q128_ReadSetting(&Set);
		PWM_AdjustAlarm(&Alarm, &Set.PwmMod, 1);
	}

	if (!Set.LowPowerEnable)
	{
		LED_Init();
		PWM_Init();
		LowPowerOFF();
		Debug_printf("LowPower OFF\r\n");
	}
	else
	{
		PWR_Init();
		LowPowerON();
		EXTI5_Init();
		Debug_printf("LowPower ON\r\n");
	}

	Paint_NewImage(Image_BW, OLED_H, OLED_W, ROTATE_180, WHITE);
	EPD_WhiteScreen_White();
	Debug_printf("Init OK\r\n");

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
				ASRPRO_Status = ASRPRO_Power_Turn();
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
			OLED_Printf(16, 0, OLED_8X16, BLACK, "%d年%d月%d日  周%s  %s %s", Time_Year, Time_Mon, Time_Day, Get_Week_Str(Time_Week), Set.LowPowerEnable ? "叶" : "  ", ASRPRO_Status ? "助 " : "  ");
			OLED_Printf(Alarm.Hour > 9 ? 8 : 16, 112, OLED_8X16, BLACK, "%.2f℃ %.0f%% %s%d:%02d 灯%smin %s", SHT.Temp, SHT.Hum, (Alarm.Enable && !Set.LowPowerEnable) ? "铃" : "否", Alarm.Hour, Alarm.Min, Get_PWM_Str(&Set.PwmMod), (Set.MuzicEnable && !Set.BuzzerEnable) ? "音" : " ");
			OLED_DrawLine(0, 20, LINE_END, 20, BLACK);
			OLED_DrawLine(0, 110, LINE_END, 110, BLACK);
			OLED_DrawLine(LINE_END, 0, LINE_END, OLED_H, BLACK);
			if (Set.WeekEnable[0])
				OLED_Printf(LINE_END + 6, OLED_8X16 * 0 + 1, OLED_8X16, BLACK, "周一");
			if (Set.WeekEnable[1])
				OLED_Printf(LINE_END + 6, OLED_8X16 * 1 + 3, OLED_8X16, BLACK, "周二");
			if (Set.WeekEnable[2])
				OLED_Printf(LINE_END + 6, OLED_8X16 * 2 + 5, OLED_8X16, BLACK, "周三");
			if (Set.WeekEnable[3])
				OLED_Printf(LINE_END + 6, OLED_8X16 * 3 + 7, OLED_8X16, BLACK, "周四");
			if (Set.WeekEnable[4])
				OLED_Printf(LINE_END + 6, OLED_8X16 * 4 + 9, OLED_8X16, BLACK, "周五");
			if (Set.WeekEnable[5])
				OLED_Printf(LINE_END + 6, OLED_8X16 * 5 + 11, OLED_8X16, BLACK, "周六");
			if (Set.WeekEnable[6])
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

		if (Time_Min && Set.BuzzerEnable)
		{
			BUZ_Flag = 1;
		}

		KeyNum = Key_GetNumber();
		CmdNum = ASRPRO_Get_CMD();

		if (EXTI0_Get_Flag())
		{
			if (DS3231_ReadStatus(&Alarm) && !Set.LowPowerEnable)
			{
				EPD_WeakUp();
				EPD_WhiteScreen_White();
				OLED_ShowImage(0, 0, OLED_W, OLED_H, Image_1, BLACK);
				OLED_Display(Image_BW, Part);
				EPD_DeepSleep();
				PWM_Run(&Set.PwmMod);
				EPD_WeakUp();
				EPD_WhiteScreen_White();
			}
			DS3231_ResetAlarm();
		}

		if (Set.LowPowerEnable)
		{
			Debug_printf("Power OFF\r\n");
			Delay_ms(10);
			PWR_STOP();
			Debug_printf("Power ON\r\n");
		}
		else
		{
			Delay_ms(1000);
		}
		DS3231_ReadTime(&Time);

		if (EXTI5_Get_Flag())
		{
			Debug_printf("EXTI5\r\n");
			Delay_ms(3000);
		}

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
