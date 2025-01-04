#include "ASRPRO.h"

void ASRPRO_Init(void)
{
    Uart_Init(115200);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_SetBits(GPIOB, GPIO_Pin_8);
    GPIO_SetBits(GPIOB, GPIO_Pin_11);
}

void ASRPRO_Mute_ON(void)
{
    GPIO_ResetBits(GPIOB, GPIO_Pin_11);
}

void ASRPRO_Mute_OFF(void)
{
    GPIO_SetBits(GPIOB, GPIO_Pin_11);
}

void ASRPRO_Power_ON(void)
{
    GPIO_ResetBits(GPIOB, GPIO_Pin_8);
    Delay_ms(1000);
    ASRPRO_Mute_ON();
}

void ASRPRO_Power_OFF(void)
{
    GPIO_SetBits(GPIOB, GPIO_Pin_8);
    ASRPRO_Mute_OFF();
}

void ASRPRO_Power_Turn(void)
{
    if (GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_8) == 0)
        ASRPRO_Power_OFF();
    else
        ASRPRO_Power_ON();
}
