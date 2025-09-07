#include "bt24_application.h"
#include "globals.h"
#include "log.h"
#include "pwm_application.h"
#include "settings_handler.h"
#include "system_init.h"
#include "ui_display.h"

#ifndef BUILD_BOOT_LOADER
int main()
{
    // 设置中断向量表偏移地址，跳过引导程序区域
    SCB->VTOR = FLASH_BASE | BOOTLOADER_SIZE_SYS;

    uint8_t time_last = 0;
    uint8_t key_num = 0, cmd_num = 0;

    // 初始化系统各模块
    System_Init();

    // 获取按键编号
    key_num = Key_GetEvent();

    // OLED_Printf(0, 0, OLED_6X12, BLACK, "中文测试：你好，世界！这是一个用于 Unicode
    // 字库显示测试的示例文本。"); OLED_Display(Image_BW, Part); Delay_ms(2000);
    // OLED_RLE_ShowImage(0, 0, OLED_W, OLED_H, Image_RLE, Image_RLE_index, 0, BLACK);
    // OLED_Display(Image_BW, Part);
    // while (1)
    // {
    // 	key_num = Key_GetEvent();
    // 	if (key_num)
    // 	{
    // 		switch (key_num)
    // 		{
    // 		case KEY_CLICK:
    // 			LOG_INFO("[EVENT] Key Short Pressed\r\n");
    // 			break;
    // 		case KEY_LONG:
    // 			LOG_INFO("[EVENT] Key Long Pressed\r\n");
    // 			break;
    // 		case KEY_DOUBLE:
    // 			LOG_INFO("[EVENT] Key Double Clicked\r\n");
    // 			break;
    // 		}
    // 	}
    // 	Delay_ms(100);
    // }

    while (1)
    {
        // 处理按键事件
        if (key_num)
        {
            BUZ_Flag = 0;
            switch (key_num)
            {
            case KEY_CLICK:
                LOG_INFO("[EVENT] Key Short Pressed\r\n");
                break;
            case KEY_LONG:
                LOG_INFO("[EVENT] Key Long Pressed\r\n");
                EPD_WeakUp();    // 唤醒电子纸显示
                KeyNumber_Set(); // 进入设置界面
                Key_GetEvent();  // 清除按键编号
                break;
            case KEY_DOUBLE:
                LOG_INFO("[EVENT] Key Double Clicked\r\n");
                WakeUp_Flag = 1;
                break;
            }
            Refresh_Flag = 1;
            BUZ_Flag     = 1;
        }

        // 处理主屏幕刷新
        if (Refresh_Flag)
        {
            // 获取温湿度数据
            SHT30_GetData(&SHT);

            // 更新电池电量和状态
            Battery_UpdateLevel(AD_GetValue());

            // 检查低功耗模式变化，如有变化则重启系统
            if ((!LowPower_Now && !Battery_GetState()) || (LowPower_Now && Battery_GetState()))
            {
                LOG_INFO("[POWER] Change LowPowerMode,SystemReset\r\n");
                Delay_ms(100);
                NVIC_SystemReset();
            }

            // 如果蓝牙已连接，则发布数据
            if (BT24_GetStatus() == BT24_CONNECTED)
            {
                Delay_ms(100);
                BT24_PubData(&PubData);
            }

            // 显示主界面并进入深度睡眠
            UI_Display_MainScreen();
            EPD_DeepSleep();
            Refresh_Flag = 0; // 清除刷新标志
        }

        // 检查时间变化，每分钟刷新一次显示
        if (time_last != Time_Min)
        {
            Refresh_Flag = 1;
            time_last    = Time_Min;
        }

        // 蜂鸣器整点报时
        if (Time_Min && Set.BuzzerEnable)
        {
            BUZ_Flag = 1;
        }

        // 获取按键和语音命令
        key_num = Key_GetEvent();
        cmd_num = ASRPRO_Get_CMD();

        // 根据条件控制语音模块开关
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

        // 低功耗模式下进入睡眠
        if (LowPower_Now && !BT24_GetStatus() && !WakeUp_Flag)
        {
            LOG_INFO("[POWER] Entering Sleep Mode\r\n");
            Delay_ms(10);
            PWR_STOP();
            LOG_INFO("[POWER] Wake from Sleep Mode\r\n");
        }
        else
        {
            Delay_ms(1000); // 正常模式下延时1秒
        }

        // 读取实时时钟
        DS3231_ReadTime(&Time);

        // 处理蓝牙连接中断
        if (EXTI9_Get_Flag())
        {
            LOG_INFO("[EVENT] Bluetooth Connected(EXTI9)\r\n"); // 蓝牙连接
            Delay_ms(1000);
            BT24_PubData(&PubData);
        }

        // 处理闹钟中断
        if (EXTI0_Get_Flag())
        {
            if (DS3231_ReadStatus(&Alarm) && !LowPower_Now)
            {
                EPD_WeakUp();
                EPD_WhiteScreen_White();
                OLED_RLE_ShowImage(0, 0, OLED_W, OLED_H, Image_RLE, Image_RLE_index, 0, BLACK);
                OLED_Display(Image_BW, Part); // 显示图片
                EPD_DeepSleep();
                PWM_Run(&Set.PwmMod); // 运行PWM控制
                EPD_WeakUp();
                EPD_WhiteScreen_White();
            }
            DS3231_ResetAlarm(); // 重置闹钟
        }

        // 处理唤醒按键中断
        if (EXTI5_Get_Flag())
        {
            LOG_INFO("[EVENT] Wake-up Key Pressed (EXTI5)\r\n");
            Delay_ms(100);
            Key_Clear();
            Delay_ms(2000);
            key_num = Key_GetEvent();
            if (key_num == KEY_CLICK)
                WakeUp_Flag = 1; // 唤醒计时器工作
        }

        // 处理蓝牙命令和语音命令
        if (BT24_GetStatus() == BT24_CONNECTED)
        {
            BT24_ProcessCommand();
        }
        if (cmd_num)
        {
            ASRPRO_ProcessCommand(cmd_num);
        }
    }
}

void TIM2_IRQHandler(void) // 1ms定时器中断
{
    static uint16_t Key_Counter = 0, Buzzer_Counter = 0, WakeUp_Counter = 0;
    static int16_t Encoder_Last = 0;
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)
    {
        Key_Counter++;
        // 每40ms扫描一次按键
        if (Key_Counter >= 40)
        {
            Key_Counter = 0;
            Key_Entry(); // 按键处理入口

            // 检测编码器变化
            if (Encoder_Last != Encoder_GetCounter())
            {
                Encoder_Last = Encoder_GetCounter();
                Buzzer_Flag  = 1; // 触发蜂鸣器
            }
        }

        // 蜂鸣器按键触发
        if (Buzzer_Flag)
        {
            Buzzer_Flag    = 0;
            Buzzer_Counter = 20; // 设置蜂鸣器响20ms
            Buzzer_ON();
        }
        // 蜂鸣器整点报时
        if ((!Time_Min) && BUZ_Flag)
        {
            BUZ_Flag       = 0;
            Buzzer_Counter = 200; // 设置蜂鸣器响200ms
            Buzzer_ON();
        }

        // 蜂鸣器计时器
        if (Buzzer_Counter)
        {
            if (Buzzer_Counter == 1)
                Buzzer_OFF();
            Buzzer_Counter--;
        }

        // 唤醒计时器，60秒后自动清除唤醒标志
        if (WakeUp_Flag == 1)
        {
            WakeUp_Counter++;
        }
        if (WakeUp_Counter >= 60000)
        {
            WakeUp_Counter = 0;
            WakeUp_Flag    = 0;
        }
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    }
}
#endif