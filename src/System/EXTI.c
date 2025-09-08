#include "EXTI.h"

static volatile uint8_t EXTI0_Flag, EXTI5_Flag, EXTI9_Flag;

/**
 * @brief  外部中断0初始化函数
 * @details 配置PA0引脚为外部中断输入，下降沿触发，抢占优先级2，子优先级2
 */
void EXTI0_Init(void)
{
    EXTI_InitTypeDef EXTI_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

    // 配置PA0为上拉输入模式
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
    GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 连接PA0到外部中断线0
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource0);

    // 配置外部中断线0
    EXTI_InitStructure.EXTI_Line    = EXTI_Line0;
    EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_Init(&EXTI_InitStructure);

    // 配置NVIC中断控制器
    NVIC_InitStructure.NVIC_IRQChannel                   = EXTI0_IRQn; // 使能外部中断通道
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;       // 抢占优先级 2，
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0x02;       // 子优先级 2
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;     // 使能外部中断通道
    NVIC_Init(&NVIC_InitStructure);                                    // 初始化 NVIC
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
}

/**
 * @brief  外部中断5初始化函数
 * @details 配置PB5引脚为外部中断输入，下降沿触发，抢占优先级3，子优先级3
 */
void EXTI5_Init(void)
{
    EXTI_InitTypeDef EXTI_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    // 连接PB5到外部中断线5
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource5);

    // 配置外部中断线5
    EXTI_InitStructure.EXTI_Line    = EXTI_Line5;
    EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_Init(&EXTI_InitStructure);

    // 配置NVIC中断控制器
    NVIC_InitStructure.NVIC_IRQChannel                   = EXTI9_5_IRQn; // 使能外部中断通道
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x03;         // 抢占优先级 3，
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0x03;         // 子优先级 3
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;       // 使能外部中断通道
    NVIC_Init(&NVIC_InitStructure);                                      // 初始化 NVIC
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
}

/**
 * @brief  外部中断9初始化函数
 * @details 配置PB9引脚为外部中断输入，上升沿触发，抢占优先级3，子优先级3
 */
void EXTI9_Init(void)
{
    EXTI_InitTypeDef EXTI_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    // 连接PB9到外部中断线9
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource9);

    // 配置外部中断线9
    EXTI_InitStructure.EXTI_Line    = EXTI_Line9;
    EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_Init(&EXTI_InitStructure);

    // 配置NVIC中断控制器
    NVIC_InitStructure.NVIC_IRQChannel                   = EXTI9_5_IRQn; // 使能外部中断通道
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x03;         // 抢占优先级 3，
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0x03;         // 子优先级 3
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;       // 使能外部中断通道
    NVIC_Init(&NVIC_InitStructure);                                      // 初始化 NVIC
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
}

/**
 * @brief  获取并清除外部中断0标志位
 * @retval 外部中断0标志位状态，1表示有中断触发，0表示无中断触发
 * @details 读取EXTI0_Flag的值并将其清零，实现标志位的一次性读取
 */
uint8_t EXTI0_Get_Flag(void)
{
    uint8_t Flag = EXTI0_Flag;
    EXTI0_Flag   = 0;
    return Flag;
}

/**
 * @brief  获取并清除外部中断5标志位
 * @retval 外部中断5标志位状态，1表示有中断触发，0表示无中断触发
 * @details 读取EXTI5_Flag的值并将其清零，实现标志位的一次性读取
 */
uint8_t EXTI5_Get_Flag(void)
{
    uint8_t Flag = EXTI5_Flag;
    EXTI5_Flag   = 0;
    return Flag;
}

/**
 * @brief  获取并清除外部中断9标志位
 * @retval 外部中断9标志位状态，1表示有中断触发，0表示无中断触发
 * @details 读取EXTI9_Flag的值并将其清零，实现标志位的一次性读取
 */
uint8_t EXTI9_Get_Flag(void)
{
    uint8_t Flag = EXTI9_Flag;
    EXTI9_Flag   = 0;
    return Flag;
}

#ifndef BUILD_BOOT_LOADER
/**
 * @brief  外部中断0服务函数
 * @details 处理PA0引脚的外部中断，当检测到下降沿时设置标志位
 */
void EXTI0_IRQHandler(void)
{
    // 判断外部中断线0是否有中断发生
    if (EXTI_GetITStatus(EXTI_Line0) != RESET) // 判断某个线上的中断是否发生
    {
        EXTI0_Flag = 1;                     // 设置外部中断0标志位
        EXTI_ClearITPendingBit(EXTI_Line0); // 清除外部中断线0上的中断标志位
    }
}

/**
 * @brief  外部中断9_5服务函数
 * @param  无
 * @retval 无
 * @details 处理PB5和PB9引脚的外部中断，当检测到相应边沿时设置对应标志位
 */
void EXTI9_5_IRQHandler(void)
{
    // 判断外部中断线5是否有中断发生
    if (EXTI_GetITStatus(EXTI_Line5) != RESET)
    {
        EXTI5_Flag = 1;                     // 设置外部中断5标志位
        EXTI_ClearITPendingBit(EXTI_Line5); // 清除外部中断线5上的中断标志位
    }

    // 判断外部中断线9是否有中断发生
    if (EXTI_GetITStatus(EXTI_Line9) != RESET)
    {
        EXTI9_Flag = 1;                     // 设置外部中断9标志位
        EXTI_ClearITPendingBit(EXTI_Line9); // 清除外部中断线9上的中断标志位
    }
}
#endif
