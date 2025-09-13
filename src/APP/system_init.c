#include "system_init.h"

// ASCII艺术字Logo
const char startup_logo[] = "\r\n"
                            "   _  __ __    ____________  ________\r\n"
                            "  | |/ // /   /  _/ ____/ / / /_  __/\r\n"
                            "  |   // /    / // / __/ /_/ / / /   \r\n"
                            " /   |/ /____/ // /_/ / __  / / /    \r\n"
                            "/_/|_/_____/___/\\____/_/ /_/ /_/    \r\n";

/**
 * @brief 启用低功耗模式
 * @details 通过配置DS3231的闹钟2为每分钟触发一次来实现低功耗唤醒
 */
void LowPowerON(void)
{
    AlarmTypeDef alarm;
    DS3231_InitAlarm(&alarm);
    alarm.Enable = 1;            // 使能闹钟
    alarm.Num    = Alarm_2;      // 选择闹钟2
    alarm.Mod    = Alarm_PerMin; // 设置为每分钟触发模式
    DS3231_WriteAlarm(&alarm);   // 写入闹钟配置
}

/**
 * @brief 禁用低功耗模式
 * @details 禁用DS3231的闹钟2，退出低功耗模式
 */
void LowPowerOFF(void)
{
    AlarmTypeDef alarm;
    DS3231_InitAlarm(&alarm);
    alarm.Enable = 0;          // 禁用闹钟
    alarm.Num    = Alarm_2;    // 选择闹钟2
    DS3231_WriteAlarm(&alarm); // 写入闹钟配置
}

/**
 * @brief 系统初始化函数
 * @details 初始化所有硬件模块和系统组件，包括外设、传感器、存储器等
 */
void System_Init()
{
    uint8_t MID  = 0;
    uint16_t DID = 0;

    // 初始化调试串口并输出启动信息
    Uart_Init(DEBUG_BAUD);
    Debug_printf("%s", startup_logo);
    Delay_ms(10);
    Debug_printf("=============================================\r\n");
    Debug_printf("  Welcome to the Smart Wake-Up Light System\r\n");
    Debug_printf("  Firmware Version: %s\r\n", FIRMWARE_VERSION);
    Debug_printf("  Build Date: %s %s\r\n", FIRMWARE_BUILD_DATE, FIRMWARE_BUILD_TIME);
    Debug_printf("  MCU: STM32F103C8T6\r\n");
    Debug_printf("=============================================\r\n\r\n");

    // 初始化OLED显示屏
    if (OLED_Init())
    {
        LOG_ERROR("[INIT] OLED Display Initialization Timeout\r\n");
    }
    else
    {
        LOG_INFO("[INIT] OLED Display Initialized Successfully\r\n");
    }

    // 初始化各种外设和模块
    Key_Init();
    AD_Init();
    Encoder_Init();
    SHT30_Init();
    EXTI0_Init();
    EXTI9_Init();
    W25Q128_Init();
    ASRPRO_Init();
    Buzzer_Init();

    // 读取SHT30传感器数据
    if (SHT30_GetData(&SHT))
    {
        LOG_ERROR("[INIT] SHT30 sensor not responding or data invalid\r\n");
    }
    else
    {
        LOG_INFO("[INIT] Temperature: %.2f°C, Humidity: %.2f%%\r\n", SHT.Temp, SHT.Hum);
    }

    // 初始化DS3231实时时钟
    if (DS3231_Init(&Time, &Alarm))
    {
        LOG_ERROR("[INIT] DS3231 time may be invalid or hardware not connected. Consider setting "
                  "time.\r\n");
    }
    else
    {
        LOG_INFO("[INIT] Time: %04d-%02d-%02d %02d:%02d:%02d\r\n", Time.tm_year + 1900,
                 Time.tm_mon + 1, Time.tm_mday, Time.tm_hour, Time.tm_min, Time.tm_sec);
    }

    // 读取W25Q128闪存ID和设置
    W25Q128_ReadID(&MID, &DID);
    if (MID)
    {
        W25Q128_ReadSetting(&Set);
        PWM_AdjustAlarm(&Alarm, &Set.PwmMod, 1);
        LOG_INFO("[INIT] Flash Memory Detected (MID=%d, DID=%d) \r\n", MID, DID);
    }
    else
    {
        LOG_ERROR("[INIT] No Flash Memory Detected\r\n");
    }

    // 初始化蓝牙模块
    BT24_GPIO_Init();
    if (Set.DeviceName[0] != 0xff)
    {
        BT24_AT_Init(Set.DeviceName);
    }
    else
    {
        BT24_AT_Init(BT_DEVICE_NAME);
    }

    // 检查电池电量
    Battery_UpdateLevel(AD_GetValue());
    LOG_INFO("[INIT] Battery Level Checked: %d%%\r\n", Battery_GetLevel());

    // 根据电池状态决定系统运行模式
    if (!Battery_GetState())
    {
        // 低电量模式
        PWR_Init();
        LowPowerON();
        EXTI5_Init();
        LOG_INFO("[INIT] System Starting in Low Power Mode\r\n");
        LowPower_Now = 1;
    }
    else
    {
        // 正常模式
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

    // 初始化显示屏并清屏
    Paint_NewImage(Image_BW, OLED_H, OLED_W, ROTATE_180, WHITE);
    EPD_WhiteScreen_White();
    LOG_INFO("[OK] System Initialized OK\r\n");
}
