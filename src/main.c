#include "system_init.h"
#include "globals.h"
#include "log.h"
#include "settings_handler.h"
#include "pwm_application.h"
#include "bt24_application.h"
#include "ui_display.h"
int main()
{
	uint8_t time_last = 0;
	uint8_t key_num = 0, cmd_num = 0;

	System_Init();
	key_num = Key_GetNumber();

	while (1)
	{

		if (key_num)
		{
			BUZ_Flag = 0;
			switch (key_num)
			{
			case 1:
				LOG_INFO("[EVENT] Key Short Pressed\r\n");
				break;
			case 2:
				LOG_INFO("[EVENT] Key Long Pressed\r\n");
				EPD_WeakUp();
				KeyNumber_Set();
				Key_GetNumber();
				break;
			case 3:
				LOG_INFO("[EVENT] Key Double Clicked\r\n");
				WakeUp_Flag = 1;
				break;
			}
			Refresh_Flag = 1;
			BUZ_Flag = 1;
		}

		if (Refresh_Flag)
		{
			SHT30_GetData(&SHT);
			Battery_UpdateLevel(AD_GetValue());
			if ((!LowPower_Now && !Battery_GetState()) || (LowPower_Now && Battery_GetState()))
			{
				LOG_INFO("[POWER] Change LowPowerMode,SystemReset\r\n");
				Delay_ms(100);
				NVIC_SystemReset();
			}

			if (BT24_GetStatus())
			{
				Delay_ms(100);
				BT24_PubData(&PubData);
			}

			UI_Display_MainScreen();
		}

		if (Refresh_Flag)
		{
			OLED_Display(Image_BW, Part);
			EPD_DeepSleep();
		}
		Refresh_Flag = 0;

		if (time_last != Time_Min)
		{
			Refresh_Flag = 1;
			time_last = Time_Min;
		}

		if (Time_Min && Set.BuzzerEnable)
		{
			BUZ_Flag = 1;
		}

		key_num = Key_GetNumber();
		cmd_num = ASRPRO_Get_CMD();

		if (((LowPower_Now || !Set.VoiceEnable) && !WakeUp_Flag) && ASRPRO_Get_State())
		{
			ASRPRO_Power_OFF();
			Refresh_Flag = 1;
			LOG_INFO("[ASRPRO] Power OFF\r\n");
		}

		if ((WakeUp_Flag || (!LowPower_Now && Set.VoiceEnable)) && !ASRPRO_Get_State())
		{
			ASRPRO_Power_ON();
			Refresh_Flag = 1;
			LOG_INFO("[ASRPRO] Power ON\r\n");
		}

		if (LowPower_Now && !BT24_GetStatus() && !WakeUp_Flag)
		{
			LOG_INFO("[POWER] Entering Sleep Mode\r\n");
			Delay_ms(10);
			PWR_STOP();
			LOG_INFO("[POWER] Wake from Sleep Mode\r\n");
		}
		else
		{
			Delay_ms(1000);
		}
		DS3231_ReadTime(&Time);

		if (EXTI9_Get_Flag())
		{
			LOG_INFO("[EVENT] Bluetooth Connected(EXTI9)\r\n"); // 蓝牙连接
			Delay_ms(1000);
			BT24_PubData(&PubData);
		}

		if (EXTI0_Get_Flag())
		{
			if (DS3231_ReadStatus(&Alarm) && !LowPower_Now)
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

		if (EXTI5_Get_Flag())
		{
			LOG_INFO("[EVENT] Wake-up Key Pressed (EXTI5)\r\n");
			Delay_ms(100);
			Key_Clear();
			Delay_ms(2000);
			key_num = Key_GetNumber();
			if (key_num == 1)
				WakeUp_Flag = 1;
		}

		if (BT24_GetStatus())
		{
			BT24_ProcessCommand();
		}

		if (cmd_num)
		{
			ASRPRO_ProcessCommand(cmd_num);
		}
	}
}

void TIM2_IRQHandler(void) // 1ms
{
	static uint16_t Key_Counter = 0, Buzzer_Counter = 0, WakeUp_Counter = 0;
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
		if (WakeUp_Flag == 1)
		{
			WakeUp_Counter++;
		}
		if (WakeUp_Counter >= 60000)
		{
			WakeUp_Counter = 0;
			WakeUp_Flag = 0;
		}
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	}
}