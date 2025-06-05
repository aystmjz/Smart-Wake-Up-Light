#include "ASRPRO.h"

void ASRPRO_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_SetBits(GPIOB, GPIO_Pin_8);
}

void ASRPRO_Mute_ON(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_SetBits(GPIOB, GPIO_Pin_11);
}

void ASRPRO_Mute_OFF(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

void ASRPRO_Power_ON(void)
{
    GPIO_ResetBits(GPIOB, GPIO_Pin_8);
}

void ASRPRO_Power_OFF(void)
{
    GPIO_SetBits(GPIOB, GPIO_Pin_8);
}

void ASRPRO_Power_Control(uint8_t state)
{
    if (state)
    {
        ASRPRO_Power_ON();
    }
    else
    {
        ASRPRO_Power_OFF();
    }
}

uint8_t ASRPRO_Power_Turn(void)
{
    if (GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_8) == 0)
    {
        ASRPRO_Power_OFF();
        return 0;
    }
    else
    {
        ASRPRO_Power_ON();
        return 1;
    }
}

uint8_t ASRPRO_Get_State(void)
{
    return !GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_8);
}

uint8_t ASRPRO_Get_CMD(void)
{
    uint8_t temp;
    for (uint8_t i = 0; i < ASRPRO_UART_REC_LEN; i++)
    {
        if (ASRPRORxBuffer[i] == 0xaa && ASRPRORxBuffer[(i + 1) % ASRPRO_UART_REC_LEN] == 0x00)
        {
            temp = ASRPRORxBuffer[(i + 2) % ASRPRO_UART_REC_LEN];
            ASRPRO_Clear_Buff();
            return temp;
        }
    }
    return 0;
}
