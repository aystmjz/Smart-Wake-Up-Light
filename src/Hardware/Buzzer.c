#include "Buzzer.h"

uint8_t Buzzer_Flag;
void Buzzer_Init(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    TIM_InternalClockConfig(TIM3);

    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode   = TIM_CounterMode_Up;
    TIM_TimeBaseInitStructure.TIM_Period        = 100 - 1; // ARR 1Khz
    TIM_TimeBaseInitStructure.TIM_Prescaler     = 720 - 1; // PSC
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStructure);

    TIM_OCInitTypeDef TIM_OCInitStructure;
    TIM_OCStructInit(&TIM_OCInitStructure);
    TIM_OCInitStructure.TIM_OCMode      = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; /*输出使能*/
    TIM_OCInitStructure.TIM_OCPolarity  = TIM_OCPolarity_High;    /*输出有效电平为高电平*/
    TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset;  /*输出空闲时为低电平*/
    TIM_OCInitStructure.TIM_Pulse       = 50;                     // CCR

    TIM_OC4Init(TIM3, &TIM_OCInitStructure);
    TIM_OC4PreloadConfig(TIM3, TIM_OCPreload_Enable); /*开启预装载，在更新时间后才会重新装载数值*/

    TIM_Cmd(TIM3, DISABLE);
}

/**
 * @brief 打开蜂鸣器
 * @details 使能TIM3定时器，开始PWM输出驱动蜂鸣器发声
 */
void Buzzer_ON(void)
{
    TIM_Cmd(TIM3, ENABLE);
}

/**
 * @brief 关闭蜂鸣器
 * @details 禁用TIM3定时器，停止PWM输出，蜂鸣器停止发声
 */
void Buzzer_OFF(void)
{
    TIM_Cmd(TIM3, DISABLE);
}
