#include "settings_handler.h"

/**
 * @brief 写入闹钟设置到DS3231
 * @param Alarm 指向闹钟结构体的指针
 * @param Mod 指向PWM模式的指针
 * @details 根据当前模式调整闹钟时间并写入DS3231芯片
 */
void WriteAlarm(AlarmTypeDef *Alarm, uint8_t *Mod)
{
    AlarmTypeDef alarm_temp;
    alarm_temp     = *Alarm;
    alarm_temp.Mod = Alarm_MatchHour;      // 设置闹钟模式为匹配小时
    PWM_AdjustAlarm(&alarm_temp, Mod, -1); // 根据PWM模式调整闹钟时间
    DS3231_WriteAlarm(&alarm_temp);        // 写入调整后的闹钟设置
}

/**
 * @brief 设置时钟时间
 * @details 通过编码器和按键设置年、月、日、时、分
 */
void KeyNumber_Set_Clock()
{
    uint8_t Time_Year_Str[5], Time_Mon_Str[3], Time_Day_Str[3];
    uint8_t Time_Choose     = 0;
    int8_t Time_Choose_Flag = 0;
    int8_t EncoderNum;
    uint8_t KeyNum;

    KeyNum = Key_GetEvent();   // 获取按键事件
    while (KeyNum != KEY_LONG) // 循环直到长按按键
    {
        KeyNum = Key_GetEvent();

        while (!KeyNum) // 没有按键时循环处理编码器输入
        {
            KeyNum = Key_GetEvent();

            OLED_Clear(WHITE);

            // 将时间数据转换为字符串
            sprintf((char *)Time_Year_Str, "%d", Time_Year);
            sprintf((char *)Time_Mon_Str, "%d", Time_Mon);
            sprintf((char *)Time_Day_Str, "%d", Time_Day);

            // 显示当前时间
            OLED_Printf(104 + 10, 0, OLED_52X104, BLACK, ":");
            OLED_Printf(Time_Hour < 10 ? 62 : 10, 4, OLED_52X104, BLACK, "%d", Time_Hour);
            OLED_Printf(104 + 10 + 20, 4, OLED_52X104, BLACK, "%02d", Time_Min);

            // 根据选择状态显示或清空字段
            if (Time_Choose_Flag < 0)
            {
                switch (Time_Choose)
                {
                case 0:
                    sprintf((char *)Time_Year_Str, "%s", "    "); // 清空年份显示
                    break;
                case 1:
                    sprintf((char *)Time_Mon_Str, "%s", Time_Mon > 9 ? "  " : " "); // 清空月份显示
                    break;
                case 2:
                    sprintf((char *)Time_Day_Str, "%s", Time_Day > 9 ? "  " : " "); // 清空日期显示
                    break;
                case 3:
                    OLED_Printf(10, 4, OLED_52X104, BLACK, ";;"); // 清空小时显示
                    break;
                case 4:
                    OLED_Printf(104 + 10 + 20, 4, OLED_52X104, BLACK, ";;"); // 清空分钟显示
                    break;
                }
            }
            // 显示日期信息
            OLED_Printf(16, 0, OLED_8X16, BLACK, "%s年%s月%s日  周%s  ", Time_Year_Str,
                        Time_Mon_Str, Time_Day_Str, Get_Week_Str(Time_Week));

            // 绘制分隔线
            OLED_DrawLine(0, 20, LINE_END, 20, BLACK);
            OLED_DrawLine(0, 110, LINE_END, 110, BLACK);
            OLED_DrawLine(LINE_END, 0, LINE_END, 110, BLACK);

            // 控制字段闪烁
            Time_Choose_Flag++;
            if (Time_Choose_Flag == 1)
                Time_Choose_Flag = -1;

            // 处理编码器输入
            EncoderNum = Encoder_GetDelta();
            if (EncoderNum)
            {
                // 根据当前选择的字段调整对应的值
                switch (Time_Choose)
                {
                case 0:
                    Time.tm_year += EncoderNum; // 调整年份
                    break;
                case 1:
                    Time.tm_mon += EncoderNum; // 调整月份
                    break;
                case 2:
                    Time.tm_mday += EncoderNum; // 调整日期
                    break;
                case 3:
                    Time.tm_hour += EncoderNum; // 调整小时
                    break;
                case 4:
                    Time.tm_min += EncoderNum; // 调整分钟
                    break;
                }
                TimeJudge(&Time); // 检查时间有效性
            }
            OLED_Display(Image_BW, Part); // 刷新显示
        }

        // 处理按键输入
        switch (KeyNum)
        {
        case KEY_CLICK:
            Time_Choose++;         // 切换到下一个字段
            Time_Choose %= 5;      // 循环选择
            Time_Choose_Flag = -1; // 重置光标状态
            break;
        case KEY_LONG:
            DS3231_WriteTime(&Time); // 保存时间设置
            EPD_WhiteScreen_White(); // 清屏
            break;
        }
    }
}

/**
 * @brief 设置闹钟参数
 * @details 设置闹钟开关、时间、星期、光照模式、音乐和蜂鸣器选项
 */
void KeyNumber_Set_Alarm()
{
    uint8_t Alarm_Hour_Str[3], Alarm_Min_Str[3], Alarm_Flag, Light_Flag, Muzic_Flag, Buzzer_Flag;
    uint8_t Alarm_Choose     = 0;
    int8_t Alarm_Choose_Flag = 0;
    int8_t EncoderNum;
    uint8_t KeyNum;
    AlarmTypeDef alarm_temp;
    uint8_t Data[200]; // 图表数据缓冲区

    KeyNum = Key_GetEvent();
    while (KeyNum != KEY_LONG) // 循环直到长按按键
    {
        KeyNum = Key_GetEvent(); // 清屏

        while (!KeyNum)
        {

            KeyNum = Key_GetEvent();

            OLED_Clear(WHITE);

            // 将闹钟时间转换为字符串
            sprintf((char *)Alarm_Hour_Str, "%d", Alarm.Hour);
            sprintf((char *)Alarm_Min_Str, "%02d", Alarm.Min);

            // 初始化标志变量
            Alarm_Flag  = 1;
            Light_Flag  = 1;
            Muzic_Flag  = 1;
            Buzzer_Flag = 1;

            // 显示星期设置
            OLED_Printf(6, OLED_8X16 * 0 + 1, OLED_8X16, BLACK, "周一 %s",
                        Set.WeekEnable[0] ? "*对" : "*错");
            OLED_Printf(6, OLED_8X16 * 1 + 3, OLED_8X16, BLACK, "周二 %s",
                        Set.WeekEnable[1] ? "*对" : "*错");
            OLED_Printf(6, OLED_8X16 * 2 + 5, OLED_8X16, BLACK, "周三 %s",
                        Set.WeekEnable[2] ? "*对" : "*错");
            OLED_Printf(6, OLED_8X16 * 3 + 7, OLED_8X16, BLACK, "周四 %s",
                        Set.WeekEnable[3] ? "*对" : "*错");
            OLED_Printf(6, OLED_8X16 * 4 + 9, OLED_8X16, BLACK, "周五 %s",
                        Set.WeekEnable[4] ? "*对" : "*错");
            OLED_Printf(6, OLED_8X16 * 5 + 11, OLED_8X16, BLACK, "周六 %s",
                        Set.WeekEnable[5] ? "*对" : "*错");
            OLED_Printf(6, OLED_8X16 * 6 + 13, OLED_8X16, BLACK, "周日 %s",
                        Set.WeekEnable[6] ? "*对" : "*错");

            // 绘制分隔线
            OLED_DrawLine(72, 0, 72, 128, BLACK);
            OLED_DrawLine(72, 110, OLED_W, 110, BLACK);

            // 绘制光照曲线图表
            PWM_ChartData(Data, 200, &Set.PwmMod);
            OLED_DrawChart(80, 0, OLED_W - 80, 88, Data, 200, BLACK);

            // 显示调整后的闹钟时间
            alarm_temp = Alarm;
            PWM_AdjustAlarm(&alarm_temp, &Set.PwmMod, -1);
            OLED_Printf(84, 88, OLED_8X16, BLACK, "%d:%02d                 %d:%02d",
                        alarm_temp.Hour, alarm_temp.Min, Alarm.Hour, Alarm.Min);
            OLED_Printf(148, 0, OLED_8X16, BLACK, "光照曲线");

            // 根据选择状态显示或清空字段
            if (Alarm_Choose_Flag < 0)
            {
                if (Alarm_Choose <= 6)
                    OLED_Printf(6, (OLED_8X16 + 2) * Alarm_Choose + 1, OLED_8X16, BLACK,
                                "    "); // 清空星期显示
                else
                    switch (Alarm_Choose)
                    {
                    case 7:
                        Alarm_Flag = 0; // 清空闹钟标志显示
                        break;
                    case 8:
                        sprintf((char *)Alarm_Hour_Str, "%s",
                                Alarm.Hour > 9 ? "  " : " "); // 清空小时显示
                        break;
                    case 9:
                        sprintf((char *)Alarm_Min_Str, "%s", "  "); // 清空分钟显示
                        break;
                    case 10:
                        Light_Flag = 0; // 清空光照模式标志显示
                        break;
                    case 11:
                        Muzic_Flag = 0; // 清空音乐标志显示
                        break;
                    case 12:
                        Buzzer_Flag = 0; // 清空蜂鸣器标志显示
                        break;
                    }
            }

            // 显示闹钟设置和其他信息
            OLED_Printf(80, 112, OLED_8X16, BLACK, "%s%s:%s *灯%smin  %s%s %s%s",
                        Alarm_Flag ? (Alarm.Enable ? "*铃" : "*否") : "  ", Alarm_Hour_Str,
                        Alarm_Min_Str,
                        Light_Flag ? Get_PWM_Str(&Set.PwmMod) : (Set.PwmMod == 1 ? " " : "  "),
                        Muzic_Flag ? "*乐" : "  ", Set.MuzicEnable ? "*对" : "*错",
                        Buzzer_Flag ? "*符" : "  ", Set.BuzzerEnable ? "*对" : "*错");

            // 控制字段闪烁
            Alarm_Choose_Flag++;
            if (Alarm_Choose_Flag == 1)
                Alarm_Choose_Flag = -1;

            // 处理编码器输入
            EncoderNum = Encoder_GetDelta();
            if (EncoderNum)
            {
                if (Alarm_Choose < 7)
                    Set.WeekEnable[Alarm_Choose] =
                        !Set.WeekEnable[Alarm_Choose]; // 切换星期使能状态
                else
                {
                    // 根据选择项调整对应设置
                    switch (Alarm_Choose)
                    {
                    case 7:
                        Alarm.Enable = !Alarm.Enable; // 切换闹钟使能
                        break;
                    case 8:
                        Alarm.Hour += EncoderNum; // 调整闹钟小时
                        break;
                    case 9:
                        Alarm.Min += EncoderNum; // 调整闹钟分钟
                        break;
                    case 10:
                        Set.PwmMod += EncoderNum; // 调整PWM模式
                        break;
                    case 11:
                        Set.MuzicEnable = !Set.MuzicEnable; // 切换音乐使能
                        break;
                    case 12:
                        Set.BuzzerEnable = !Set.BuzzerEnable; // 切换蜂鸣器使能
                        break;
                    }
                }
                Alarm_Judge(&Alarm);    // 检查闹钟时间有效性
                PWM_Judge(&Set.PwmMod); // 检查PWM模式有效性
            }

            OLED_Display(Image_BW, Part); // 刷新显示
        }

        // 处理按键输入
        switch (KeyNum)
        {
        case KEY_CLICK:
            Alarm_Choose++;         // 切换到下一个设置项
            Alarm_Choose %= 13;     // 循环选择
            Alarm_Choose_Flag = -1; // 重置光标状态
            break;
        case KEY_LONG:
            WriteAlarm(&Alarm, &Set.PwmMod); // 保存闹钟设置
            W25Q128_WriteSetting(&Set);      // 保存设置到闪存
            EPD_WhiteScreen_White();         // 清屏
            break;
        }
    }
}

/**
 * @brief 设置其他参数
 * @details 设置语音助手等其他系统参数
 */
void KeyNumber_Set_Other()
{
    uint8_t LowPower_Flag;
    uint8_t Other_Choose     = 0;
    int8_t Other_Choose_Flag = 0;
    int8_t EncoderNum;
    uint8_t KeyNum;

    KeyNum = Key_GetEvent();
    while (KeyNum != KEY_LONG) // 循环直到长按按键
    {
        KeyNum = Key_GetEvent();

        while (!KeyNum)
        {
            KeyNum = Key_GetEvent();

            OLED_Clear(WHITE); // 清屏
            LowPower_Flag = 1; // 初始化标志

            // 根据选择状态显示或清空字段
            if (Other_Choose_Flag < 0)
            {
                switch (Other_Choose)
                {
                case 0:
                    LowPower_Flag = 0; // 清空选择项显示
                    break;
                }
            }

            // 显示设置信息
            OLED_Printf(8, OLED_H / 2, OLED_8X16, BLACK, "%s*麦%s",
                        LowPower_Flag ? "语音助手常开" : "            ",
                        Set.VoiceEnable ? "*对" : "*错");

            // 控制字段闪烁
            Other_Choose_Flag++;
            if (Other_Choose_Flag == 1)
                Other_Choose_Flag = -1;

            // 显示其他设置选项
            EncoderNum = Encoder_GetDelta();
            if (EncoderNum)
            {
                switch (Other_Choose)
                {
                case 0:
                    Set.VoiceEnable = !Set.VoiceEnable; // 切换语音助手使能
                    break;
                }
            }
            OLED_Display(Image_BW, Part); // 刷新显示
        }

        // 处理按键输入
        switch (KeyNum)
        {
        case 1:
            Other_Choose++;         // 切换到下一个选项
            Other_Choose %= 1;      // 循环选择
            Other_Choose_Flag = -1; // 重置光标状态
            break;
        case 2:
            W25Q128_WriteSetting(&Set); // 保存设置到闪存
            EPD_WhiteScreen_White();    // 清屏
            break;
        }
    }
}

/**
 * @brief 主设置菜单
 * @details 提供时间、其他、闹钟三个设置选项的导航菜单
 */
void KeyNumber_Set()
{
    int8_t EncoderNum;
    uint8_t KeyNum;

    // 显示设置菜单
    OLED_Clear(WHITE);
    OLED_Printf(132, 30, OLED_8X16, BLACK, "设置");
    OLED_Printf(84, 66, OLED_8X16, BLACK, "时间  其他  闹钟");
    OLED_DrawRectangle(76, 28, OLED_W - 76, OLED_H - 28, BLACK, 0);
    OLED_DrawLine(76, 50, OLED_W - 76, 50, BLACK);
    OLED_Display(Image_BW, Part);

    KeyNum     = Key_Clear();        // 清除按键状态
    EncoderNum = Encoder_GetDelta(); // 获取编码器状态
    while (!KeyNum)                  // 循环直到有按键输入
    {
        KeyNum     = Key_GetEvent();
        EncoderNum = Encoder_GetDelta();

        if (EncoderNum < 0)
        {
            KeyNumber_Set_Clock(); // 向左拨进入时间设置
            return;
        }
        else if (EncoderNum > 0)
        {
            KeyNumber_Set_Alarm(); // 向右拨进入闹钟设置
            return;
        }
    }

    // 处理按键输入
    switch (KeyNum)
    {
    case KEY_CLICK:
        KeyNumber_Set_Other(); // 单击按键进入其他设置
        break;
    case KEY_LONG:
        // 长按按键不执行任何操作，退出设置界面
        break;
    }
}
