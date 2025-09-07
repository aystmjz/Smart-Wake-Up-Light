#include "ASRPRO.h"

void ASRPRO_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_SetBits(GPIOB, GPIO_Pin_8);

    UART2_Init(ASRPRO_DEVICE_BAUD);
}

/**
 * @brief 开启ASRPRO模块静音功能
 * @details 配置GPIOB Pin11为推挽输出高电平，使ASRPRO模块进入静音状态
 */
void ASRPRO_Mute_ON(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_SetBits(GPIOB, GPIO_Pin_11);
}

/**
 * @brief 关闭ASRPRO模块静音功能
 * @details 配置GPIOB Pin11为浮空输入模式，关闭ASRPRO模块静音状态
 */
void ASRPRO_Mute_OFF(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

/**
 * @brief 开启ASRPRO模块电源
 * @details 将GPIOB Pin8设置为低电平，开启ASRPRO模块电源
 */
void ASRPRO_Power_ON(void)
{
    GPIO_ResetBits(GPIOB, GPIO_Pin_8);
}

/**
 * @brief 关闭ASRPRO模块电源
 * @details 将GPIOB Pin8设置为高电平，关闭ASRPRO模块电源
 */
void ASRPRO_Power_OFF(void)
{
    GPIO_SetBits(GPIOB, GPIO_Pin_8);
}

/**
 * @brief 切换ASRPRO模块电源状态
 * @return 返回切换后的电源状态，1表示开启，0表示关闭
 * @details 切换ASRPRO模块的电源状态，如果当前为开启则关闭，如果当前为关闭则开启
 */
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

/**
 * @brief 获取ASRPRO模块电源状态
 * @return 返回当前电源状态，1表示开启，0表示关闭
 * @details 读取GPIOB Pin8引脚的输出状态，返回ASRPRO模块的电源状态
 */
uint8_t ASRPRO_Get_State(void)
{
    return !GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_8);
}

/**
 * @brief 获取ASRPRO模块识别到的命令
 * @return 返回识别到的命令码，0表示无有效命令
 * @details 从接收缓冲区中查找有效的命令帧(0xaa 0x00 xx)，提取命令码并清空缓冲区
 */
uint8_t ASRPRO_Get_CMD(void)
{
    uint8_t temp;

    // 遍历接收缓冲区查找有效的命令帧
    for (uint8_t i = 0; i < ASRPRO_UART_REC_LEN; i++)
    {
        // 查找命令帧头(0xaa 0x00)
        if (ASRPRORxBuffer[i] == 0xaa && ASRPRORxBuffer[(i + 1) % ASRPRO_UART_REC_LEN] == 0x00)
        {
            // 提取命令码(第三个字节)
            temp = ASRPRORxBuffer[(i + 2) % ASRPRO_UART_REC_LEN];
            ASRPRO_Clear_Buff(); // 清空接收缓冲区
            return temp;         // 返回命令码
        }
    }
    return 0; // 未找到有效命令帧，返回0
}
