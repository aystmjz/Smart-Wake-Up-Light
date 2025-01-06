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

void PWM_Run(void)
{
    time_t stamp_last, stamp;
    uint8_t KeyNum;
    uint16_t Timer_Sec;
    stamp_last = DS3231_GetTimeStamp();
    KeyNum = Key_GetNumber();
    PWM_Set(1);
    PWM_Enable();
    while (!KeyNum)
    {
        KeyNum = Key_GetNumber();
        DS3231_ReadTime();
        stamp = DS3231_GetTimeStamp();
        Timer_Sec = stamp > stamp_last ? stamp - stamp_last : stamp_last - stamp;
        switch (PWM_Mod)
        {
        case 0:
            PWM_Set((Timer_Sec * Timer_Sec) / 900 + 1);
            break;
        case 1:
            PWM_Set(((Timer_Sec / 2) * (Timer_Sec / 2)) / 900 + 1);
            break;
        case 2:
            PWM_Set(((Timer_Sec / 3) * (Timer_Sec / 3)) / 900 + 1);
            break;
        case 3:
            PWM_Set(((Timer_Sec / 4) * (Timer_Sec / 4)) / 900 + 1);
            break;
        case 4:
            PWM_Set(((Timer_Sec / 6) * (Timer_Sec / 6)) / 900 + 1);
            break;
        case 5:
            PWM_Set(((Timer_Sec / 8) * (Timer_Sec / 8)) / 900 + 1);
            break;
        case 6:
            PWM_Set(((Timer_Sec / 12) * (Timer_Sec / 12)) / 900 + 1);
            break;
        case 7:
            // PWM_Set(Light_Date[PWM_Timer_Min + 1]);
            break;
        } // 5,10,15,20,30,40,60

        Delay_ms(1000);
        if (Timer_Sec > 5 * 60 * 60)
        {
            PWM_Disable();
            return;
        }
    }
    PWM_Disable();
}