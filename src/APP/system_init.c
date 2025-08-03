#include "system_init.h"

// ASCII艺术字Logo
const char startup_logo[] = "\r\n"
							"   _  __ __    ____________  ________\r\n"
							"  | |/ // /   /  _/ ____/ / / /_  __/\r\n"
							"  |   // /    / // / __/ /_/ / / /   \r\n"
							" /   |/ /____/ // /_/ / __  / / /    \r\n"
							"/_/|_/_____/___/\\____/_/ /_/ /_/    \r\n";

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
void System_Init()
{
	uint8_t MID = 0;
	uint16_t DID = 0;

	Uart_Init(DEBUG_BAUD);
	Debug_printf("%s", startup_logo);
	Delay_ms(10);
	Debug_printf("=============================================\r\n");
	Debug_printf("  Welcome to the Smart Wake-Up Light System\r\n");
	Debug_printf("  Firmware Version: %s\r\n", FIRMWARE_VERSION);
	Debug_printf("  Build Date: %s %s\r\n", FIRMWARE_BUILD_DATE, FIRMWARE_BUILD_TIME);
	Debug_printf("  MCU: STM32F103C8T6\r\n");
	Debug_printf("=============================================\r\n\r\n");

	Key_Init();
	AD_Init();
	Encoder_Init();
	OLED_Init();
	SHT30_Init();
	EXTI0_Init();
	EXTI9_Init();
	W25Q128_Init();
	ASRPRO_Init();
	Buzzer_Init();
	DS3231_Init(&Time, &Alarm);
	W25Q128_ReadID(&MID, &DID);
	if (MID)
	{
		W25Q128_ReadSetting(&Set);
		PWM_AdjustAlarm(&Alarm, &Set.PwmMod, 1);
		LOG_INFO("[INIT] Flash Memory Detected (MID=%d, DID=%d) \r\n", MID, DID);
	}
	else
	{
		LOG_WARN("[INIT] No Flash Memory Detected\r\n");
	}

	if (Set.DeviceName[0] != 0xff)
	{
		BT24_Init(Set.DeviceName);
	}
	else
	{
		BT24_Init(BT_DEVICE_NAME);
	}

	Battery_UpdateLevel(AD_GetValue());
	LOG_INFO("[INIT] Battery Level Checked: %d%%\r\n", Battery_GetLevel());

	if (!Battery_GetState())
	{
		PWR_Init();
		LowPowerON();
		EXTI5_Init();
		LOG_INFO("[INIT] System Starting in Low Power Mode\r\n");
		LowPower_Now = 1;
	}
	else
	{
		LED_Init();
		PWM_Init();
		LowPowerOFF();
		LOG_INFO("[INIT] System Starting in Normal Mode\r\n");
		LowPower_Now = 0;
		if (Set.VoiceEnable)
		{
			ASRPRO_Power_ON();
		}
	}

	Paint_NewImage(Image_BW, OLED_H, OLED_W, ROTATE_180, WHITE);
	EPD_WhiteScreen_White();
	LOG_INFO("[OK] System Initialized OK\r\n");
}
