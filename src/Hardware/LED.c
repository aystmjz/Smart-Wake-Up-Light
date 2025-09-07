#include "LED.h"

/**
 * @brief LED初始化函数
 * @details 配置PA1引脚为推挽输出模式
 */
void LED_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_ResetBits(GPIOA, GPIO_Pin_1);
}

/**
 * @brief 打开LED
 * @details 设置PA1引脚为高电平，点亮LED
 */
void LED_ON(void)
{
    GPIO_SetBits(GPIOA, GPIO_Pin_1);
}

/**
 * @brief 关闭LED
 * @details 设置PA1引脚为低电平，熄灭LED
 */
void LED_OFF(void)
{
    GPIO_ResetBits(GPIOA, GPIO_Pin_1);
}

/**
 * @brief 切换LED状态
 * @details 如果当前LED关闭则打开，如果当前LED打开则关闭
 */
void LED_Turn(void)
{
    if (GPIO_ReadOutputDataBit(GPIOA, GPIO_Pin_1) == 0)
        GPIO_SetBits(GPIOA, GPIO_Pin_1);
    else
        GPIO_ResetBits(GPIOA, GPIO_Pin_1);
}
