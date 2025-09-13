#include "pwm_application.h"

/**
 * @brief PWM运行主函数，根据不同的模式控制PWM输出亮度渐变
 * @param Mod 指向当前模式的指针，决定使用哪种亮度变化模式
 * @details 该函数实现了一个基于时间的PWM亮度控制机制，根据当前模式选择不同的亮度计算方式。
 *          函数会持续运行直到检测到按键输入或蓝牙信号，同时会根据运行时间自动退出。
 */
void PWM_Run(uint8_t *Mod)
{
    time_t stamp_last, stamp; // 时间戳变量，用于计算时间差
    uint8_t KeyNum;
    uint16_t Timer_Sec;
    struct tm Time_Temp;

    // 读取当前时间并获取时间戳
    DS3231_ReadTime(&Time_Temp);
    stamp_last = DS3231_GetTimeStamp(&Time_Temp);

    // 获取按键事件
    KeyNum = Key_GetEvent();

    // 初始化PWM输出为1%并启用PWM
    PWM_Set(1);
    PWM_Enable();

    while (!KeyNum && !BT24_GetStatus())
    {
        KeyNum = Key_GetEvent();
        DS3231_ReadTime(&Time_Temp);
        stamp = DS3231_GetTimeStamp(&Time_Temp);
        // 计算经过的时间（秒）
        Timer_Sec = stamp > stamp_last ? stamp - stamp_last : stamp_last - stamp;

        // 根据当前模式设置PWM亮度
        switch (*Mod)
        {
        case 1:
            PWM_Set(PWM_MOD1_CALC(Timer_Sec));
            break;
        case 2:
            PWM_Set(PWM_MOD2_CALC(Timer_Sec));
            break;
        case 3:
            PWM_Set(PWM_MOD3_CALC(Timer_Sec));
            break;
        case 4:
            PWM_Set(PWM_MOD4_CALC(Timer_Sec));
            break;
        case 5:
            PWM_Set(PWM_MOD5_CALC(Timer_Sec));
            break;
        case 6:
            PWM_Set(PWM_MOD6_CALC(Timer_Sec));
            break;
        case 7:
            PWM_Set(PWM_MOD7_CALC(Timer_Sec));
            break;
        default:
            PWM_Set(PWM_MOD_TEST_CALC(Timer_Sec));
            break;
        }

        Delay_ms(1000);

        // 如果运行时间超过设定的退出时间，则退出PWM模式
        if (Timer_Sec > PWM_EXIT_HOUR * 60 * 60)
        {
            PWM_Disable();
            return;
        }
    }
    // 检测到按键或蓝牙信号时禁用PWM并退出
    PWM_Disable();
}

/**
 * @brief 调整闹钟时间，根据PWM模式进行时间增减
 * @param Alarm 指向闹钟结构体的指针
 * @param Mod 指向当前模式的指针
 * @param Dir 调整方向，1表示增加，-1表示减少
 * @details 根据当前PWM模式和方向参数，调整闹钟的小时和分钟设置
 */
void PWM_AdjustAlarm(AlarmTypeDef *Alarm, uint8_t *Mod, int8_t Dir)
{
    uint16_t sum;

    // 将当前时间转换为总分钟数
    sum = Alarm->Hour * 60 + Alarm->Min;

    // 根据不同模式调整时间
    switch (*Mod)
    {
    case 1:
        Alarm->Hour = (sum + PWM_MOD1 * Dir) / 60;
        Alarm->Min  = (sum + PWM_MOD1 * Dir) % 60;
        break;
    case 2:
        Alarm->Hour = (sum + PWM_MOD2 * Dir) / 60;
        Alarm->Min  = (sum + PWM_MOD2 * Dir) % 60;
        break;
    case 3:
        Alarm->Hour = (sum + PWM_MOD3 * Dir) / 60;
        Alarm->Min  = (sum + PWM_MOD3 * Dir) % 60;
        break;
    case 4:
        Alarm->Hour = (sum + PWM_MOD4 * Dir) / 60;
        Alarm->Min  = (sum + PWM_MOD4 * Dir) % 60;
        break;
    case 5:
        Alarm->Hour = (sum + PWM_MOD5 * Dir) / 60;
        Alarm->Min  = (sum + PWM_MOD5 * Dir) % 60;
        break;
    case 6:
        Alarm->Hour = (sum + PWM_MOD6 * Dir) / 60;
        Alarm->Min  = (sum + PWM_MOD6 * Dir) % 60;
        break;
    case 7:
        Alarm->Hour = (sum + PWM_MOD7 * Dir) / 60;
        Alarm->Min  = (sum + PWM_MOD7 * Dir) % 60;
        break;
    default:
        Alarm->Hour = sum / 60;
        Alarm->Min  = sum % 60;
        break;
    }
}

/**
 * @brief 生成PWM图表数据，用于显示亮度变化曲线
 * @param Data 存储PWM数据的数组指针
 * @param Num 数据点的数量
 * @param Mod 指向当前模式的指针
 * @details 根据当前模式和数据点数量，计算每个点的PWM值并存储在数组中
 */
void PWM_ChartData(uint8_t *Data, uint8_t Num, uint8_t *Mod)
{
    uint16_t temp;

    // 遍历每个数据点
    for (uint8_t i = 0; i < Num; i++)
    {
        // 根据不同模式计算对应的PWM值
        switch (*Mod)
        {
        case 1:
            temp = PWM_MOD1_CALC(PWM_MOD1 * 60 / (float)Num * i);
            break;
        case 2:
            temp = PWM_MOD2_CALC(PWM_MOD2 * 60 / (float)Num * i);
            break;
        case 3:
            temp = PWM_MOD3_CALC(PWM_MOD3 * 60 / (float)Num * i);
            break;
        case 4:
            temp = PWM_MOD4_CALC(PWM_MOD4 * 60 / (float)Num * i);
            break;
        case 5:
            temp = PWM_MOD5_CALC(PWM_MOD5 * 60 / (float)Num * i);
            break;
        case 6:
            temp = PWM_MOD6_CALC(PWM_MOD6 * 60 / (float)Num * i);
            break;
        case 7:
            temp = PWM_MOD7_CALC(PWM_MOD7 * 60 / (float)Num * i);
            break;
        default:
            temp = PWM_MOD_TEST_CALC(PWM_MOD_TEST * 60 / (float)Num * i);
            break;
        }

        // 限制PWM值在0-100范围内
        if (temp > 100)
            Data[i] = 100;
        else
            Data[i] = temp;
    }
}

/**
 * @brief PWM模式判断，确保模式值在有效范围内
 * @param Mod 指向当前模式的指针
 * @details 如果模式值小于1则设置为最大模式值，如果大于最大模式值则设置为1
 */
void PWM_Judge(uint8_t *Mod)
{
    if (*Mod < 1)
        *Mod = PWM_NUM;
    if (*Mod > PWM_NUM)
        *Mod = 1;
}

/**
 * @brief 获取PWM模式对应的字符串描述
 * @param Mod 指向当前模式的指针
 * @return 返回对应模式的字符串描述
 * @details 根据当前模式返回对应的字符串标识
 */
char *Get_PWM_Str(uint8_t *Mod)
{
    switch (*Mod)
    {
    case 1:
        return PWM_MOD1_STR;
    case 2:
        return PWM_MOD2_STR;
    case 3:
        return PWM_MOD3_STR;
    case 4:
        return PWM_MOD4_STR;
    case 5:
        return PWM_MOD5_STR;
    case 6:
        return PWM_MOD6_STR;
    case 7:
        return PWM_MOD7_STR;
    default:
        return PWM_MOD_TEST_STR;
    }
}
