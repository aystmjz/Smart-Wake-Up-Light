#include "I2C.h"

/* DS3231 I2C */

/**
 * @brief  DS3231 I2C接口初始化函数
 * @details 初始化GPIOA的PA11(SCL)和PA12(SDA)引脚为开漏输出模式，用于模拟I2C通信
 */
void DS3231_I2C_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    // 配置GPIOA的PA11和PA12为开漏输出模式
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_OD;
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_12 | GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

/**
 * @brief  DS3231 I2C延时函数
 * @details 提供约10微秒的延时，用于I2C时序控制
 */
static void DS3231_I2C_delay()
{
    Delay_us(10);
}

/**
 * @brief  DS3231 I2C起始信号函数
 * @details 产生I2C起始条件：SDA从高电平变为低电平，SCL保持高电平
 */
void DS3231_I2C_Start(void)
{
    DS3231_I2C_SDA_SET = 1;
    DS3231_I2C_SCL_SET = 1;
    DS3231_I2C_delay();
    DS3231_I2C_SDA_SET = 0;
    DS3231_I2C_delay();
    DS3231_I2C_SCL_SET = 0;
}

/**
 * @brief  DS3231 I2C停止信号函数
 * @details 产生I2C停止条件：SDA从低电平变为高电平，SCL保持高电平
 */
void DS3231_I2C_Stop(void)
{
    DS3231_I2C_SDA_SET = 0;
    DS3231_I2C_SCL_SET = 1;
    DS3231_I2C_delay();
    DS3231_I2C_SDA_SET = 1;
}

/**
 * @brief  DS3231 I2C发送一个字节函数
 * @param  Byte 要发送的字节数据
 * @details 通过SDA线逐位发送一个字节数据，高位先发
 */
void DS3231_I2C_SendByte(uint8_t Byte)
{
    // 循环发送8位数据
    for (uint8_t i = 0; i < 8; i++)
    {
        DS3231_I2C_SDA_SET = !(!(Byte & (0x80 >> i)));
        DS3231_I2C_delay();
        DS3231_I2C_SCL_SET = 1;
        DS3231_I2C_delay();
        DS3231_I2C_SCL_SET = 0;
        DS3231_I2C_delay();
    }
}

/**
 * @brief  DS3231 I2C接收一个字节函数
 * @retval 接收到的字节数据
 * @details 通过SDA线逐位接收一个字节数据，高位先收
 */
uint8_t DS3231_I2C_ReceiveByte()
{
    uint8_t Byte = 0x00;

    DS3231_I2C_SDA_SET = 1;
    DS3231_I2C_delay();

    // 循环接收8位数据
    for (uint8_t i = 0; i < 8; i++)
    {
        DS3231_I2C_SCL_SET = 1;
        DS3231_I2C_delay();

        // 读取SDA上的数据位
        if (DS3231_I2C_SDA_READ)
        {
            Byte |= (0x80 >> i);
        }

        DS3231_I2C_SCL_SET = 0;
        DS3231_I2C_delay();
    }

    return Byte;
}

/**
 * @brief  DS3231 I2C发送应答信号函数
 * @param  AckBit 应答位，0表示ACK，1表示NACK
 * @details 发送应答或非应答信号给从设备
 */
void DS3231_I2C_SendAck(uint8_t AckBit)
{
    DS3231_I2C_SDA_SET = AckBit;
    DS3231_I2C_delay();
    DS3231_I2C_SCL_SET = 1;
    DS3231_I2C_delay();
    DS3231_I2C_SCL_SET = 0;
    DS3231_I2C_delay();
}

/**
 * @brief  DS3231 I2C接收应答信号函数
 * @retval 接收到的应答位，0表示ACK，1表示NACK
 * @details 接收从设备发送的应答或非应答信号
 */
uint8_t DS3231_I2C_ReceiveAck()
{
    uint8_t AckBit;

    DS3231_I2C_SDA_SET = 1;
    DS3231_I2C_delay();
    DS3231_I2C_SCL_SET = 1;
    AckBit             = DS3231_I2C_SDA_READ;
    DS3231_I2C_delay();
    DS3231_I2C_SCL_SET = 0;
    DS3231_I2C_delay();

    return AckBit;
}

/*--------------------------------------------------------------------*/
/* SHT30 I2C */

/**
 * @brief  SHT30 I2C接口初始化函数
 * @details 初始化GPIOC的PC14(SCL)和PC15(SDA)引脚为开漏输出模式，用于模拟I2C通信
 */
void SHT30_I2C_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

    // 配置GPIOC的PC14和PC15为开漏输出模式
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_OD;
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
}

/**
 * @brief  SHT30 I2C延时函数
 * @details 提供约10微秒的延时，用于I2C时序控制
 */
static void SHT30_I2C_delay(void)
{
    Delay_us(10);
}

/**
 * @brief  SHT30 I2C起始信号函数
 * @details 产生I2C起始条件：SDA从高电平变为低电平，SCL保持高电平
 */
void SHT30_I2C_Start(void)
{
    SHT30_I2C_SDA_SET = 1;
    SHT30_I2C_SCL_SET = 1;
    SHT30_I2C_delay();
    SHT30_I2C_SDA_SET = 0;
    SHT30_I2C_delay();
    SHT30_I2C_SCL_SET = 0;
}

/**
 * @brief  SHT30 I2C停止信号函数
 * @details 产生I2C停止条件：SDA从低电平变为高电平，SCL保持高电平
 */
void SHT30_I2C_Stop(void)
{
    SHT30_I2C_SDA_SET = 0;
    SHT30_I2C_SCL_SET = 1;
    SHT30_I2C_delay();
    SHT30_I2C_SDA_SET = 1;
}

/**
 * @brief  SHT30 I2C发送一个字节函数
 * @param  Byte 要发送的字节数据
 * @details 通过SDA线逐位发送一个字节数据，高位先发
 */
void SHT30_I2C_SendByte(uint8_t Byte)
{
    // 循环发送8位数据
    for (uint8_t i = 0; i < 8; i++)
    {
        SHT30_I2C_SDA_SET = !(!(Byte & (0x80 >> i)));
        SHT30_I2C_delay();
        SHT30_I2C_SCL_SET = 1;
        SHT30_I2C_delay();
        SHT30_I2C_SCL_SET = 0;
        SHT30_I2C_delay();
    }
}

/**
 * @brief  SHT30 I2C接收一个字节函数
 * @retval 接收到的字节数据
 * @details 通过SDA线逐位接收一个字节数据，高位先收
 */
uint8_t SHT30_I2C_ReceiveByte(void)
{
    uint8_t Byte = 0x00;

    SHT30_I2C_SDA_SET = 1;
    SHT30_I2C_delay();

    // 循环接收8位数据
    for (uint8_t i = 0; i < 8; i++)
    {
        SHT30_I2C_SCL_SET = 1;
        SHT30_I2C_delay();

        // 读取SDA上的数据位
        if (SHT30_I2C_SDA_READ)
        {
            Byte |= (0x80 >> i);
        }

        SHT30_I2C_SCL_SET = 0;
        SHT30_I2C_delay();
    }

    return Byte;
}

/**
 * @brief  SHT30 I2C发送应答信号函数
 * @param  AckBit 应答位，0表示ACK，1表示NACK
 * @details 发送应答或非应答信号给从设备
 */
void SHT30_I2C_SendAck(uint8_t AckBit)
{
    SHT30_I2C_SDA_SET = AckBit;
    SHT30_I2C_delay();
    SHT30_I2C_SCL_SET = 1;
    SHT30_I2C_delay();
    SHT30_I2C_SCL_SET = 0;
    SHT30_I2C_delay();
}

/**
 * @brief  SHT30 I2C接收应答信号函数
 * @retval 接收到的应答位，0表示ACK，1表示NACK
 * @details 接收从设备发送的应答或非应答信号
 */
uint8_t SHT30_I2C_ReceiveAck(void)
{
    uint8_t AckBit;

    SHT30_I2C_SDA_SET = 1;
    SHT30_I2C_delay();
    SHT30_I2C_SCL_SET = 1;
    AckBit            = SHT30_I2C_SDA_READ;
    SHT30_I2C_delay();
    SHT30_I2C_SCL_SET = 0;
    SHT30_I2C_delay();

    return AckBit;
}
