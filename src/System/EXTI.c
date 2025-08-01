#include "EXTI.h"

static volatile uint8_t EXTI0_Flag, EXTI5_Flag, EXTI9_Flag;

// 外部中断0配置
void EXTI0_Init(void)
{
    EXTI_InitTypeDef EXTI_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE); // 使能 PORTA,PORTE 时钟和使能 AFIO 时钟
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; // PA0 设置成输入，上拉
    GPIO_Init(GPIOA, &GPIO_InitStructure);        // 初始化PA0

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource0);

    EXTI_InitStructure.EXTI_Line = EXTI_Line0;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_Init(&EXTI_InitStructure); // 初始化 EXTI 寄存器

    NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;             // 使能外部中断通道
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02; // 抢占优先级 2，
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;        // 子优先级 2
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;              // 使能外部中断通道
    NVIC_Init(&NVIC_InitStructure);                              // 初始化 NVIC
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
}

uint8_t EXTI0_Get_Flag(void)
{
    uint8_t Flag = EXTI0_Flag;
    EXTI0_Flag = 0;
    return Flag;
}

uint8_t EXTI5_Get_Flag(void)
{
    uint8_t Flag = EXTI5_Flag;
    EXTI5_Flag = 0;
    return Flag;
}

uint8_t EXTI9_Get_Flag(void)
{
    uint8_t Flag = EXTI9_Flag;
    EXTI9_Flag = 0;
    return Flag;
}

#ifndef BUILD_BOOT_LOADER
void EXTI0_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line0) != RESET) // 判断某个线上的中断是否发生
    {
        EXTI0_Flag = 1;
        EXTI_ClearITPendingBit(EXTI_Line0); // 清除 LINE0 上的中断标志位
    }
}
#endif

// 外部中断5配置
void EXTI5_Init(void)
{
    EXTI_InitTypeDef EXTI_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource5);

    EXTI_InitStructure.EXTI_Line = EXTI_Line5;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_Init(&EXTI_InitStructure); // 初始化 EXTI 寄存器

    NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;           // 使能外部中断通道
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x03; // 抢占优先级 3，
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x03;        // 子优先级 3
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;              // 使能外部中断通道
    NVIC_Init(&NVIC_InitStructure);                              // 初始化 NVIC
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
}

// 外部中断9配置
void EXTI9_Init(void)
{
    EXTI_InitTypeDef EXTI_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource9);

    EXTI_InitStructure.EXTI_Line = EXTI_Line9;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_Init(&EXTI_InitStructure); // 初始化 EXTI 寄存器

    NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;           // 使能外部中断通道
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x03; // 抢占优先级 3，
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x03;        // 子优先级 3
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;              // 使能外部中断通道
    NVIC_Init(&NVIC_InitStructure);                              // 初始化 NVIC
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
}

#ifndef BUILD_BOOT_LOADER
void EXTI9_5_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line5) != RESET) // 判断某个线上的中断是否发生
    {
        EXTI5_Flag = 1;
        EXTI_ClearITPendingBit(EXTI_Line5); // 清除 LINE5 上的中断标志位
    }
    if (EXTI_GetITStatus(EXTI_Line9) != RESET) // 判断某个线上的中断是否发生
    {
        EXTI9_Flag = 1;
        EXTI_ClearITPendingBit(EXTI_Line9); // 清除 LINE9 上的中断标志位
    }
}
#endif