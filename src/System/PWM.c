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
    TIM_TimeBaseInitStructure.TIM_Period = 100 - 1;   // ARR 10K Hz
    TIM_TimeBaseInitStructure.TIM_Prescaler = 72 - 1; // PSC
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

void PWM_Set(uint8_t Spead)
{
    if (Spead > 100)
        Spead = 100;
    TIM_SetCompare1(TIM1, Spead);
}
