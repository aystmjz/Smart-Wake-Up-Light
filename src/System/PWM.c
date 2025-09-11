#include "PWM.h"

/**
 * @brief  PWM初始化函数
 * @details 初始化TIM1的PWM输出功能，配置PA8为PWM输出引脚，PWM频率为20KHz
 */
void PWM_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    // 配置GPIOA的PA8为复用推挽输出模式，用于PWM输出
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 配置TIM1为内部时钟模式
    TIM_InternalClockConfig(TIM1);

    // 配置TIM1基本定时器参数
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode   = TIM_CounterMode_Up; // 向上计数模式
    TIM_TimeBaseInitStructure.TIM_Period = 100 - 1; // 自动重装载值，设置PWM周期为100，频率20KHz
    TIM_TimeBaseInitStructure.TIM_Prescaler = 72 / 2 - 1; // 预分频值，72MHz系统时钟分频为2MHz
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseInitStructure);

    // 配置TIM1通道1的PWM输出参数
    TIM_OCInitTypeDef TIM_OCInitStructure;
    TIM_OCStructInit(&TIM_OCInitStructure);
    TIM_OCInitStructure.TIM_OCMode      = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_OCPolarity  = TIM_OCPolarity_High;   // 输出有效电平为高电平
    TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset; // 输出空闲时为低电平
    TIM_OCInitStructure.TIM_Pulse       = 0; // 初始比较值为0，即占空比为0%
    TIM_OC1Init(TIM1, &TIM_OCInitStructure);

    // 使能TIM1通道1的预装载功能，在更新事件后才会重新装载数值
    TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);

    // 使能PWM输出
    TIM_CtrlPWMOutputs(TIM1, ENABLE);

    // 初始禁用TIM1定时器
    TIM_Cmd(TIM1, DISABLE);
}

/**
 * @brief  使能PWM输出
 * @details 使能TIM1定时器，开始PWM输出
 */
void PWM_Enable(void)
{
    TIM_CtrlPWMOutputs(TIM1, ENABLE);
    TIM_Cmd(TIM1, ENABLE);
}

/**
 * @brief  禁用PWM输出
 * @details 禁用TIM1定时器，停止PWM输出
 */
void PWM_Disable(void)
{
    TIM_CtrlPWMOutputs(TIM1, DISABLE);
    TIM_Cmd(TIM1, DISABLE);
}

/**
 * @brief  设置PWM占空比
 * @param  Percent PWM占空比百分比(0-100)
 * @details 设置TIM1通道1的比较值，控制PWM输出占空比
 */
void PWM_Set(uint16_t Percent)
{
    // 限制占空比范围在0-100之间
    if (Percent > 100)
        Percent = 100;

    // 设置TIM1通道1的比较值，控制PWM占空比
    TIM_SetCompare1(TIM1, Percent);
}
