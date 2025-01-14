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
int8_t EncoderNum;


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

void KeyNumber_Set_Clock()
{
	uint8_t Time_Year_Str[5], Time_Mon_Str[3], Time_Day_Str[3];
	uint8_t Time_Choose = 0;

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
			Time_Choose_Flag = -1;
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

void KeyNumber_Set_Alarm()
{
	uint8_t Alarm_Hour_Str[3], Alarm_Min_Str[3], Alarm_Flag, Light_Flag, Muzic_Flag, Buzzer_Flag;
	uint8_t Alarm_Choose = 0;
	KeyNum = Key_GetNumber();
	while (KeyNum != 2)
	{
		KeyNum = Key_GetNumber();

		while (!KeyNum)
		{

			KeyNum = Key_GetNumber();

			OLED_Clear(WHITE);

			sprintf((char *)Alarm_Hour_Str, "%d", Alarm_Hour);
			sprintf((char *)Alarm_Min_Str, "%02d", Alarm_Min);
			Alarm_Flag = 1;
			Light_Flag = 1;
			Muzic_Flag = 1;
			Buzzer_Flag = 1;

			OLED_Printf(6, OLED_8X16 * 0 + 1, OLED_8X16, BLACK, "周一 %s", Alarm_Set[1] ? "对" : "错");
			OLED_Printf(6, OLED_8X16 * 1 + 3, OLED_8X16, BLACK, "周二 %s", Alarm_Set[2] ? "对" : "错");
			OLED_Printf(6, OLED_8X16 * 2 + 5, OLED_8X16, BLACK, "周三 %s", Alarm_Set[3] ? "对" : "错");
			OLED_Printf(6, OLED_8X16 * 3 + 7, OLED_8X16, BLACK, "周四 %s", Alarm_Set[4] ? "对" : "错");
			OLED_Printf(6, OLED_8X16 * 4 + 9, OLED_8X16, BLACK, "周五 %s", Alarm_Set[5] ? "对" : "错");
			OLED_Printf(6, OLED_8X16 * 5 + 11, OLED_8X16, BLACK, "周六 %s", Alarm_Set[6] ? "对" : "错");
			OLED_Printf(6, OLED_8X16 * 6 + 13, OLED_8X16, BLACK, "周日 %s", Alarm_Set[7] ? "对" : "错");
			OLED_DrawLine(72, 0, 72, 128, BLACK);
			OLED_DrawLine(72, 110, OLED_W, 110, BLACK);

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
						sprintf((char *)Alarm_Hour_Str, "%s", Alarm_Hour > 9 ? "  " : " ");
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

			OLED_Printf(80, 112, OLED_8X16, BLACK, "%s%s:%s 光%smin  %s%s %s%s", Alarm_Flag ? (Alarm_Enable ? "铃" : "否") : "  ", Alarm_Hour_Str, Alarm_Min_Str, Light_Flag ? Get_PWM_Str(PWM_Mod) : (PWM_Mod ? "  " : " "), Muzic_Flag ? "音" : "  ", Alarm_Set[0] ? "对" : "错", Buzzer_Flag ? "符" : "  ", Alarm_Set[0] ? "对" : "错");

			Alarm_Choose_Flag++;
			if (Alarm_Choose_Flag == 1)
				Alarm_Choose_Flag = -1;

			EncoderNum = Encoder_Get_Div4();
			if (EncoderNum)
			{
				if (Alarm_Choose < 7)
					Alarm_Set[Alarm_Choose + 1] = !Alarm_Set[Alarm_Choose + 1];
				else
				{
					switch (Alarm_Choose)
					{
					case 7:
						Alarm_Enable = !Alarm_Enable;
						break;
					case 8:
						Alarm_Date[0] += EncoderNum;
						break;
					case 9:
						Alarm_Date[1] += EncoderNum;
						break;
					case 10:
						PWM_Mod += EncoderNum;
						break;
					case 11:
						Alarm_Set[0] = !Alarm_Set[0];
						break;
					case 12:
						Alarm_Set[0] = !Alarm_Set[0];
						break;
					}
				}
				Alarm_Judge();
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

	Delay_ms(1000);
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
	uint8_t Refresh_Flag = 0;
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
			OLED_Printf(16, 112, OLED_8X16, BLACK, "%.2f℃ %.0f%% %s%d:%02d 光%smin 音", SHT.Temp, SHT.Hum, Alarm_Enable ? "铃" : "否", Alarm_Date[0], Alarm_Date[1], Get_PWM_Str(PWM_Mod));
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

		if (EXTI0_Get_Flag())
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
