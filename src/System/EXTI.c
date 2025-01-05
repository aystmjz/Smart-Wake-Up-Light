#include "EXTI.h"

static uint8_t EXTI_Flag;

// �ⲿ�ж�0����
void EXTI0_Init(void)
{
    EXTI_InitTypeDef EXTI_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE); // ʹ�� PORTA,PORTE ʱ�Ӻ�ʹ�� AFIO ʱ��
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; // PA0 ���ó����룬����
    GPIO_Init(GPIOA, &GPIO_InitStructure);        // ��ʼ��PA0

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource0);

    EXTI_InitStructure.EXTI_Line = EXTI_Line0;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_Init(&EXTI_InitStructure); // ��ʼ�� EXTI �Ĵ���

    NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;             // ʹ���ⲿ�ж�ͨ��
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02; // ��ռ���ȼ� 2��
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;        // �����ȼ� 2
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;              // ʹ���ⲿ�ж�ͨ��
    NVIC_Init(&NVIC_InitStructure);                              // ��ʼ�� NVIC

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); // ���� NVIC �жϷ��� 2
}

uint8_t EXTI0_Get_Flag(void)
{
    uint8_t Flag = EXTI_Flag;
    EXTI_Flag = 0;
    return Flag;
}

void EXTI0_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line0) != RESET) // �ж�ĳ�����ϵ��ж��Ƿ���
    {
        EXTI_Flag = 1;
        EXTI_ClearITPendingBit(EXTI_Line0); // ��� LINE0 �ϵ��жϱ�־λ
    }
}