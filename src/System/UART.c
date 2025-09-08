#include "UART.h"

// 加入以下代码,支持printf函数,而不需要选择use MicroLIB
#ifndef BUILD_BOOT_LOADER
#pragma import(__use_no_semihosting) // 标准库需要的支持函数
// 定义FILE结构体，用于重定向printf函数
struct __FILE
{
    int handle;
};
FILE __stdout; // 定义标准输出流

/**
 * @brief  系统退出函数重定义
 * @param  x 退出状态码
 * @details 重定义_sys_exit函数以避免使用半主机模式
 */
void _sys_exit(int x)
{
    x = x;
}

/**
 * @brief  字符输出函数重定义
 * @param  ch 要输出的字符
 * @param  f 文件流指针
 * @retval 输出的字符
 * @details 重定义fputc函数，将字符通过USART2发送，支持printf函数
 */
int fputc(int ch, FILE *f) // 重定义fputc函数
{
    while ((USART2->SR & 0X40) == 0)
    {
    } // 循环发送,直到发送完毕
    USART2->DR = (u8)ch;
    return ch;
}
#endif

volatile uint16_t ASRPRORxCounter;                    // ASRPRO模块接收计数器
volatile uint16_t BT24RxCounter;                      // BT24模块接收计数器
volatile uint8_t ASRPRORxBuffer[ASRPRO_UART_REC_LEN]; // ASRPRO接收缓冲区
volatile uint8_t BT24RxBuffer[BT24_UART_REC_LEN];     // BT24接收缓冲区

/**
 * @brief  初始化串口1(BT24)
 * @param  bound 波特率
 * @details 配置USART1用于BT24蓝牙模块通信，PA9为TX，PA10为RX
 */
void UART1_Init(uint32_t bound)
{
#if DEBUG_MODE == DEBUG_MODE_BT24
#else
    // 使能USART1和GPIOA时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);

    // 配置PA9为复用推挽输出(TX)
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 配置PA10为浮空输入(RX)
    GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 配置USART1中断
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel                   = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;      // 抢占优先级3
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 3;      // 子优先级3
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE; // IRQ通道使能
    NVIC_Init(&NVIC_InitStructure);

    // 配置USART1参数
    USART_DeInit(USART1);
    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate   = bound;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b; // 字长为8位数据格式
    USART_InitStructure.USART_StopBits   = USART_StopBits_1;    // 一个停止位
    USART_InitStructure.USART_Parity     = USART_Parity_No;     // 无奇偶校验位
    USART_InitStructure.USART_HardwareFlowControl =
        USART_HardwareFlowControl_None;                             // 无硬件数据流控制
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; // 收发模式
    USART_Init(USART1, &USART_InitStructure);

    // 使能USART1接收中断
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

    // 使能USART1
    USART_Cmd(USART1, ENABLE);
#endif
}

/**
 * @brief  初始化串口2(ASRPRO)
 * @param  bound 波特率
 * @details 配置USART2用于ASRPRO语音识别模块通信，PA2为TX，PA3为RX
 */
void UART2_Init(uint32_t bound)
{
    // 使能GPIOA和USART2时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

    // 配置PA2为复用推挽输出(TX)
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 配置PA3为浮空输入(RX)
    GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 配置USART2中断
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel                   = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;      // 抢占优先级0
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 1;      // 子优先级1
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE; // IRQ通道使能
    NVIC_Init(&NVIC_InitStructure);

    // 配置USART2参数
    USART_DeInit(USART2);
    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate   = bound;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b; // 字长为8位数据格式
    USART_InitStructure.USART_StopBits   = USART_StopBits_1;    // 一个停止位
    USART_InitStructure.USART_Parity     = USART_Parity_No;     // 无奇偶校验位
    USART_InitStructure.USART_HardwareFlowControl =
        USART_HardwareFlowControl_None;                             // 无硬件数据流控制
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; // 收发模式
    USART_Init(USART2, &USART_InitStructure);

    // 使能USART2接收中断
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);

    // 使能USART2
    USART_Cmd(USART2, ENABLE);
}

/**
 * @brief  初始化总串口
 * @param  bound 波特率
 * @details 同时初始化串口1和串口2
 */
void Uart_Init(uint32_t bound)
{
    UART1_Init(bound);
    UART2_Init(bound);
}

/*---------------------------------------------------------------*/
/* 串口1(BT24) */

/**
 * @brief  BT24串口发送字符串
 * @param  SendBuf 要发送的字符串指针
 * @details 通过USART1发送字符串数据
 */
void BT24_SendStr(char *SendBuf)
{
    while (*SendBuf)
    {
        while ((USART1->SR & 0X40) == 0)
        {
        } // 等待发送完成
        USART1->DR = (u8)*SendBuf;
        SendBuf++;
    }
}

/**
 * @brief  BT24串口格式化打印
 * @param  SendBuf 要发送的字符串指针
 * @details 根据调试模式决定是否通过BT24串口发送数据
 */
void BT24_printf(char *SendBuf)
{
#if DEBUG_MODE == DEBUG_MODE_BT24
#else
    BT24_SendStr(SendBuf);
#endif
}

/**
 * @brief  清空BT24接收缓冲区
 * @details 将BT24接收计数器清零，并将接收缓冲区全部置0
 */
void BT24_Clear_Buff(void)
{
    BT24RxCounter = 0;
    for (uint16_t i = 0; i < BT24_UART_REC_LEN; i++)
    {
        BT24RxBuffer[i] = 0;
    }
}

/**
 * @brief  调试信息打印函数
 * @param  format 格式化字符串
 * @param  ... 可变参数
 * @details 根据调试模式将格式化字符串通过BT24串口发送
 */
void Debug_printf(const char *format, ...)
{
#if DEBUG_MODE == DEBUG_MODE_NORMAL || DEBUG_MODE == DEBUG_MODE_STM32
    static char Debug_buffer[DEBUG_BUFF_LEN]; // 调试缓冲区
    va_list args;
    va_start(args, format);
    vsnprintf(Debug_buffer, sizeof(Debug_buffer), format, args);
    va_end(args);
    BT24_SendStr(Debug_buffer);
#endif
}

#ifndef BUILD_BOOT_LOADER
/**
 * @brief  USART1中断服务函数
 * @details 处理USART1接收中断，接收BT24蓝牙模块数据
 */
void USART1_IRQHandler(void)
{
    // 判断是否为接收中断
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
        // 读取接收到的数据并存入缓冲区
        BT24RxBuffer[BT24RxCounter++] = USART_ReceiveData(USART1);

        // 检查是否接收到结束符'}'和'"'
        if (BT24RxBuffer[BT24RxCounter - 1] == '}')
        {
            if (BT24RxBuffer[BT24RxCounter - 2] == '"')
            {
                BT24RxCounter = 0; // 接收到完整数据包，清零计数器
            }
        }

#if DEBUG_MODE == DEBUG_MODE_ASRPRO
        // 如果调试模式为ASRPRO，则将接收到的数据转发到USART2
        while ((USART2->SR & 0X40) == 0)
        {
        } // 等待发送完成
        USART2->DR = (uint8_t)BT24RxBuffer[BT24RxCounter - 1];
#endif

        // 接收计数器循环使用缓冲区
        BT24RxCounter %= BT24_UART_REC_LEN;
    }
}
#endif

/*---------------------------------------------------------------*/
/* 串口2(ASRPRO) */

/**
 * @brief  清空ASRPRO接收缓冲区
 * @details 将ASRPRO接收计数器清零，并将接收缓冲区全部置0
 */
void ASRPRO_Clear_Buff(void)
{
    ASRPRORxCounter = 0;
    for (uint16_t i = 0; i < ASRPRO_UART_REC_LEN; i++)
    {
        ASRPRORxBuffer[i] = 0;
    }
}

/**
 * @brief  ASRPRO串口格式化打印函数
 * @param  format 格式化字符串
 * @param  ... 可变参数
 * @details 通过标准printf函数将格式化字符串通过USART2发送给ASRPRO模块
 */
void ASRPRO_printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vprintf(format, args); // 使用重定向的printf函数
    va_end(args);
}

#ifndef BUILD_BOOT_LOADER
/**
 * @brief  USART2中断服务函数
 * @details 处理USART2接收中断，接收ASRPRO语音识别模块数据
 */
void USART2_IRQHandler(void)
{
    // 判断是否为接收中断
    if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
    {
        // 读取接收到的数据并存入缓冲区
        ASRPRORxBuffer[ASRPRORxCounter++] = USART_ReceiveData(USART2);

#if DEBUG_MODE == DEBUG_MODE_NORMAL || DEBUG_MODE == DEBUG_MODE_ASRPRO
        // 如果调试模式为NORMAL或ASRPRO，则将接收到的数据转发到USART1
        while ((USART1->SR & 0X40) == 0)
        {
        } // 等待发送完成
        USART1->DR = (uint8_t)ASRPRORxBuffer[ASRPRORxCounter - 1];
#endif

        // 接收计数器循环使用缓冲区
        ASRPRORxCounter %= ASRPRO_UART_REC_LEN;
    }
}
#endif
