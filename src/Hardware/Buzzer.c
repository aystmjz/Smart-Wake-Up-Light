#include "Buzzer.h"

uint8_t Buzzer_Flag;

/**
 * @brief  蜂鸣器初始化函数
 * @details 初始化TIM3的PWM输出功能，配置PB1为PWM输出引脚，用于驱动蜂鸣器
 */
void Buzzer_Init(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    // 配置GPIOB的PB1为复用推挽输出模式，用于PWM输出驱动蜂鸣器
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // 配置TIM3为内部时钟模式
    TIM_InternalClockConfig(TIM3);

    // 配置TIM3基本定时器参数
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode   = TIM_CounterMode_Up; // 向上计数模式
    TIM_TimeBaseInitStructure.TIM_Period = 100 - 1; // 自动重装载值，设置PWM周期为100，频率1KHz
    TIM_TimeBaseInitStructure.TIM_Prescaler = 720 - 1; // 预分频值，72MHz系统时钟分频为100KHz
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStructure);

    TIM_OCInitTypeDef TIM_OCInitStructure;
    TIM_OCStructInit(&TIM_OCInitStructure);
    TIM_OCInitStructure.TIM_OCMode      = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_OCPolarity  = TIM_OCPolarity_High;   // 输出有效电平为高电平
    TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset; // 输出空闲时为低电平
    TIM_OCInitStructure.TIM_Pulse       = 50;                    // 比较值为50，占空比50%
    TIM_OC4Init(TIM3, &TIM_OCInitStructure);

    // 使能TIM3通道4的预装载功能，在更新事件后才会重新装载数值
    TIM_OC4PreloadConfig(TIM3, TIM_OCPreload_Enable); /*开启预装载，在更新时间后才会重新装载数值*/

    // 使能PWM输出
    TIM_CtrlPWMOutputs(TIM3, ENABLE);

    // 初始禁用TIM3定时器
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
