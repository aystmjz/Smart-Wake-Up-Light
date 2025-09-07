#include "Encoder.h"

static int16_t lastNum;

void Encoder_Init(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    TIM_TimeBaseInitStructure.TIM_ClockDivision     = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode       = TIM_CounterMode_Up;
    TIM_TimeBaseInitStructure.TIM_Period            = 65536 - 1; // ARR
    TIM_TimeBaseInitStructure.TIM_Prescaler         = 1 - 1;     // PSC
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseInitStructure);

    TIM_ICInitTypeDef TIM_ICInitStructure;
    TIM_ICStructInit(&TIM_ICInitStructure);
    TIM_ICInitStructure.TIM_Channel  = TIM_Channel_1;
    TIM_ICInitStructure.TIM_ICFilter = 0xf;
    TIM_ICInit(TIM4, &TIM_ICInitStructure);
    TIM_ICInitStructure.TIM_Channel  = TIM_Channel_2;
    TIM_ICInitStructure.TIM_ICFilter = 0xf;
    TIM_ICInit(TIM4, &TIM_ICInitStructure);
    TIM_EncoderInterfaceConfig(TIM4, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising,
                               TIM_ICPolarity_Rising);

    TIM_Cmd(TIM4, ENABLE);
}

/**
 * @brief 获取编码器计数值
 * @return 编码器当前计数值(经过4分频处理)
 * @details 读取TIM4计数器值并进行4分频处理，消除编码器抖动影响
 */
int16_t Encoder_GetCounter(void)
{
    return (TIM_GetCounter(TIM4) + 1) / 4;
}

/**
 * @brief 设置编码器计数值
 * @param Num 要设置的计数值
 * @details 设置TIM4计数器的值，用于初始化或重置编码器计数
 */
void Encoder_Set(int16_t Num)
{
    lastNum = Num;
    TIM_SetCounter(TIM4, Num * 4 - 1);
}

/**
 * @brief 清零编码器计数
 * @details 将编码器计数器和上次计数缓存清零
 */
void Encoder_Clear(void)
{
    lastNum = 0;
    TIM_SetCounter(TIM4, 0);
}

/**
 * @brief 获取编码器增量值(4分频)
 * @return 编码器增量值，无变化时返回0
 * @details 计算自上次读取以来编码器的增量值，用于检测编码器转动
 */
int16_t Encoder_GetDelta(void)
{
    int16_t temp = Encoder_GetCounter() - lastNum;
    if (temp)
    {
        lastNum = Encoder_GetCounter();
        return temp;
    }
    else
        return 0;
}
