#include "pwm_application.h"
void PWM_Run(uint8_t *Mod)
{
    time_t stamp_last, stamp;
    uint8_t KeyNum;
    uint16_t Timer_Sec;
    struct tm Time_Temp;
    DS3231_ReadTime(&Time_Temp);
    stamp_last = DS3231_GetTimeStamp(&Time_Temp);
    KeyNum = Key_GetNumber();
    PWM_Set(1);
    PWM_Enable();
    while (!KeyNum&&!BT24_GetStatus())
    {
        KeyNum = Key_GetNumber();
        DS3231_ReadTime(&Time_Temp);
        stamp = DS3231_GetTimeStamp(&Time_Temp);
        Timer_Sec = stamp > stamp_last ? stamp - stamp_last : stamp_last - stamp;
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
        if (Timer_Sec > PWM_EXIT_HOUR * 60 * 60)
        {
            PWM_Disable();
            return;
        }
    }
    PWM_Disable();
}

void PWM_AdjustAlarm(AlarmTypeDef *Alarm, uint8_t *Mod, int8_t Dir)
{
    uint16_t sum;
    sum = Alarm->Hour * 60 + Alarm->Min;
    switch (*Mod)
    {
    case 1:
        Alarm->Hour = (sum + PWM_MOD1 * Dir) / 60;
        Alarm->Min = (sum + PWM_MOD1 * Dir) % 60;
        break;
    case 2:
        Alarm->Hour = (sum + PWM_MOD2 * Dir) / 60;
        Alarm->Min = (sum + PWM_MOD2 * Dir) % 60;
        break;
    case 3:
        Alarm->Hour = (sum + PWM_MOD3 * Dir) / 60;
        Alarm->Min = (sum + PWM_MOD3 * Dir) % 60;
        break;
    case 4:
        Alarm->Hour = (sum + PWM_MOD4 * Dir) / 60;
        Alarm->Min = (sum + PWM_MOD4 * Dir) % 60;
        break;
    case 5:
        Alarm->Hour = (sum + PWM_MOD5 * Dir) / 60;
        Alarm->Min = (sum + PWM_MOD5 * Dir) % 60;
        break;
    case 6:
        Alarm->Hour = (sum + PWM_MOD6 * Dir) / 60;
        Alarm->Min = (sum + PWM_MOD6 * Dir) % 60;
        break;
    case 7:
        Alarm->Hour = (sum + PWM_MOD7 * Dir) / 60;
        Alarm->Min = (sum + PWM_MOD7 * Dir) % 60;
        break;
    default:
        Alarm->Hour = sum / 60;
        Alarm->Min = sum % 60;
        break;
    }
}

void PWM_ChartData(uint8_t *Data, uint8_t Num, uint8_t *Mod)
{
    uint16_t temp;
    for (uint8_t i = 0; i < Num; i++)
    {
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
        if (temp > 100)
            Data[i] = 100;
        else
            Data[i] = temp;
    }
}

void PWM_Judge(uint8_t *Mod)
{
    if (*Mod < 1)
        *Mod = PWM_NUM;
    if (*Mod > PWM_NUM)
        *Mod = 1;
}

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
