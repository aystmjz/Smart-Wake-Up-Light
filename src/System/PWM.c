#include "PWM.h"
void PWM_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    TIM_InternalClockConfig(TIM1);

    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStructure.TIM_Period = 100 - 1;       // ARR 20K Hz
    TIM_TimeBaseInitStructure.TIM_Prescaler = 72 / 2 - 1; // PSC
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseInitStructure);

    TIM_OCInitTypeDef TIM_OCInitStructure;
    TIM_OCStructInit(&TIM_OCInitStructure);
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; /*输出使能*/
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;     /*输出有效电平为高电平*/
    TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset;  /*输出空闲时为低电平*/
    TIM_OCInitStructure.TIM_Pulse = 0;                            // CCR
    TIM_OC1Init(TIM1, &TIM_OCInitStructure);
    TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable); /*开启预装载，在更新时间后才会重新装载数值*/

    TIM_Cmd(TIM1, ENABLE);
    TIM_CtrlPWMOutputs(TIM1, DISABLE);
}

void PWM_Enable(void)
{
    TIM_CtrlPWMOutputs(TIM1, ENABLE);
}

void PWM_Disable(void)
{
    TIM_CtrlPWMOutputs(TIM1, DISABLE);
}

void PWM_Set(uint16_t Percent)
{
    if (Percent > 100)
        Percent = 100;
    TIM_SetCompare1(TIM1, Percent);
}

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
