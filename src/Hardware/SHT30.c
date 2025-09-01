#include "SHT30.h"
void SHT30_WriteCommand(uint16_t Command)
{
    SHT30_I2C_Start();
    SHT30_I2C_SendByte(SHT30_ADDRESS);
    SHT30_I2C_ReceiveAck();
    SHT30_I2C_SendByte((Command >> 8) & 0xff);
    SHT30_I2C_ReceiveAck();
    SHT30_I2C_SendByte(Command & 0xff);
    SHT30_I2C_ReceiveAck();
    SHT30_I2C_Stop();
}

void SHT30_MultiReadByte(uint8_t *Data_p, uint8_t Length)
{
    SHT30_I2C_Start();
    SHT30_I2C_SendByte(SHT30_ADDRESS | 0x01);
    SHT30_I2C_ReceiveAck();
    Length--;
    while (Length--)
    {
        *Data_p++ = SHT30_I2C_ReceiveByte();
        SHT30_I2C_SendAck(0);
    }
    *Data_p = SHT30_I2C_ReceiveByte();
    SHT30_I2C_SendAck(1);
    SHT30_I2C_Stop();
}

void SHT30_Init(void)
{
    SHT30_I2C_Init();

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    GPIO_ResetBits(GPIOC, GPIO_Pin_13);
    Delay_ms(20);
    GPIO_SetBits(GPIOC, GPIO_Pin_13);
    SHT30_WriteCommand(SHT30_MEAS_CLOCKNOSTR_H_CMD);
}

void SHT30_Reset(void)
{
    SHT30_WriteCommand(SHT30_SOFT_RESET_CMD);
}

/**
 * @brief 读取SHT30温湿度传感器数据
 * @param SHT 指向SHT30设备结构体的指针，用于存储读取的原始数据和转换后的温湿度值
 * @return 返回操作状态，0表示成功，1表示失败
 */
uint8_t SHT30_GetData(SHT30TypeDef *SHT)
{
    uint16_t t_ticks, rh_ticks;
    SHT30_WriteCommand(SHT30_MEAS_CLOCKNOSTR_H_CMD);
    Delay_ms(40);
    SHT30_MultiReadByte(SHT->raw_data, 6);
    t_ticks = ((uint16_t)SHT->raw_data[0] << 8) | SHT->raw_data[1];
    rh_ticks = ((uint16_t)SHT->raw_data[3] << 8) | SHT->raw_data[4];
    if (!t_ticks && !rh_ticks)
        return 1;
    SHT->Temp = -45 + 175 * (float)t_ticks / 65535;
    SHT->Hum = 100 * (float)rh_ticks / 65535;
    if (SHT->Hum > 100)
        SHT->Hum = 100;
    if (SHT->Hum < 0)
        SHT->Hum = 0;

    return 0;
}
