#include "SHT30.h"

/**
 * @brief 向SHT30写入命令
 * @param Command 要写入的16位命令
 * @details 通过I2C接口向SHT30发送命令，先发送高字节再发送低字节
 */
static void SHT30_WriteCommand(uint16_t Command)
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

/**
 * @brief 从SHT30读取多个字节数据
 * @param Data_p 数据存储缓冲区指针
 * @param Length 要读取的数据长度
 * @details 通过I2C接口从SHT30连续读取指定长度的数据
 */
static void SHT30_MultiReadByte(uint8_t *Data_p, uint8_t Length)
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

/**
 * @brief SHT30软件复位
 * @details 向SHT30发送软件复位命令，使传感器重新初始化
 */
void SHT30_SoftReset(void)
{
    SHT30_WriteCommand(SHT30_SOFT_RESET_CMD);
}

/**
 * @brief SHT30硬件复位
 * @details 通过控制传感器的复位引脚执行硬件复位操作
 */
void SHT30_HardReset(void)
{
    // 拉低20ms后拉高
    GPIO_ResetBits(GPIOC, GPIO_Pin_13);
    Delay_ms(20);
    GPIO_SetBits(GPIOC, GPIO_Pin_13);
    Delay_ms(20);
}

/**
 * @brief SHT30传感器初始化
 * @details 初始化SHT30传感器，包括I2C接口和硬件复位引脚
 */
void SHT30_Init(void)
{
    SHT30_I2C_Init();

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    // 执行硬件复位
    SHT30_HardReset();

    // 发送测量命令
    SHT30_WriteCommand(SHT30_MEAS_CLOCKNOSTR_H_CMD);
}

/**
 * @brief 计算CRC8校验值
 * @param data 数据指针
 * @param len 数据长度
 * @return CRC8校验值
 * @details 使用SHT30 CRC8算法计算校验值
 */
static uint8_t SHT30_CRC8(uint8_t *data, uint8_t len)
{
    uint8_t crc        = 0xFF;
    uint8_t polynomial = 0x31;

    for (uint8_t i = 0; i < len; i++)
    {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++)
        {
            if (crc & 0x80)
            {
                crc = (crc << 1) ^ polynomial;
            }
            else
            {
                crc <<= 1;
            }
        }
    }
    return crc;
}

/**
 * @brief 读取SHT30温湿度传感器数据
 * @param SHT 指向SHT30设备结构体的指针，用于存储读取的原始数据和转换后的温湿度值
 * @return 返回操作状态，0表示成功，1表示失败
 * @details 从SHT30读取温湿度原始数据，并转换为实际的温度和湿度值，增加CRC校验
 */
uint8_t SHT30_GetData(SHT30TypeDef *SHT)
{
    uint16_t t_ticks, rh_ticks;

    // 发送测量命令并等待数据准备完成
    SHT30_WriteCommand(SHT30_MEAS_CLOCKNOSTR_H_CMD);
    Delay_ms(40);

    // 读取6字节原始数据(温度2字节+CRC+湿度2字节+CRC)
    SHT30_MultiReadByte(SHT->raw_data, 6);

    // CRC校验检查
    uint8_t temp_data[2] = {SHT->raw_data[0], SHT->raw_data[1]};
    uint8_t hum_data[2]  = {SHT->raw_data[3], SHT->raw_data[4]};

    if (SHT30_CRC8(temp_data, 2) != SHT->raw_data[2])
    {
        return 1; // 温度数据CRC校验失败
    }

    if (SHT30_CRC8(hum_data, 2) != SHT->raw_data[5])
    {
        return 1; // 湿度数据CRC校验失败
    }

    // 提取温度和湿度的原始数据
    t_ticks  = ((uint16_t)SHT->raw_data[0] << 8) | SHT->raw_data[1]; // 温度原始数据
    rh_ticks = ((uint16_t)SHT->raw_data[3] << 8) | SHT->raw_data[4]; // 湿度原始数据

    // 检查数据有效性
    if (!t_ticks && !rh_ticks)
        return 1; // 数据无效返回失败

    // 将原始数据转换为实际温湿度值
    SHT->Temp = -45 + 175 * (float)t_ticks / 65535; // 温度转换公式:-45+175*(原始值/65535)
    SHT->Hum  = 100 * (float)rh_ticks / 65535;      // 湿度转换公式:100*(原始值/65535)

    // 限制湿度值在合理范围内
    if (SHT->Hum > 100)
        SHT->Hum = 100; // 湿度上限100%
    if (SHT->Hum < 0)
        SHT->Hum = 0; // 湿度下限0%

    return 0; // 返回成功
}
