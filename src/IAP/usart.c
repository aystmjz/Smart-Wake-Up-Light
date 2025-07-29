#include "usart.h"

uint8_t UART1_RX_BUFF[UART1_RX_SIZE];
UCB_DATE UCB_DATA;

/**
 * @brief 初始化USART1接收缓冲区指针
 * @param 无
 * @retval 无
 */
void uart1_rx_ptr_init(void)
{
    // 初始化控制块指针
    UCB_DATA.RxDataIN = &UCB_DATA.RxDataPtr[0];
    UCB_DATA.RxDataOUT = &UCB_DATA.RxDataPtr[0];
    UCB_DATA.RxDataEND = &UCB_DATA.RxDataPtr[BLOCK_NUM - 1];

    // 初始化接收计数器
    UCB_DATA.RxCounter = 0;

    UCB_DATA.RxDataIN->start = UART1_RX_BUFF;
}

/// @brief 串口1
/// @param bound 波特率
void uart1_init(uint32_t bound)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);
    USART_DeInit(USART1);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; // 复用推挽输出
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; // 浮空输入
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3; // 抢占优先级3
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;        // 子优先级3
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;           // IRQ通道使能
    NVIC_Init(&NVIC_InitStructure);

    USART_InitStructure.USART_BaudRate = bound;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;                     // 字长为8位数据格式
    USART_InitStructure.USART_StopBits = USART_StopBits_1;                          // 一个停止位
    USART_InitStructure.USART_Parity = USART_Parity_No;                             // 无奇偶校验位
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // 无硬件数据流控制
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;                 // 收发模式
    USART_Init(USART1, &USART_InitStructure);

    USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE);

    USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);
    USART_Cmd(USART1, ENABLE);

    uart1_rx_ptr_init();
    dma_init(DMA1_Channel5, (u32)&USART1->DR, (u32)UART1_RX_BUFF, UART1_RX_MAX + 1);
}

/**
 * @brief  向串口发送单个字节
 * @param  byte : 要发送的字节
 * @retval None
 */
void uart1_send_byte(uint8_t byte)
{
    while ((USART1->SR & 0x40) == 0); // 等待发送完成
    USART1->DR = byte;
}

// 串口1打印
void uart1_send_str(char *send_buf)
{
    while (*send_buf)
    {
        while ((USART1->SR & 0X40) == 0)
        {
        } // 等待发送完成
        USART1->DR = (u8)*send_buf;
        send_buf++;
    }
    while ((USART1->SR & 0X40) == 0)
    {
    } // 确保发送完成
}

void uart1_printf(const char *format, ...)
{
    static char send_buffer[UART1_TX_SIZE];
    va_list args;
    va_start(args, format);
    vsnprintf(send_buffer, sizeof(send_buffer), format, args);
    va_end(args);
    uart1_send_str(send_buffer);
}

/**
 * @brief  从串口接收缓冲区读取数据并更新指针
 * @param  buffer: 数据存储缓冲区指针
 * @param  max_len: 最大读取长度
 * @retval 实际读取的数据长度
 */
uint16_t uart1_read_data(uint8_t *buffer, uint16_t max_len)
{
    uint16_t data_len = 0;
    
    if (UART_DATA_AVAILABLE && buffer != NULL)
    {
        data_len = UCB_DATA.RxDataOUT->end - UCB_DATA.RxDataOUT->start + 1;
        
        // 限制读取长度不超过最大长度
        if (data_len > max_len)
        {
            data_len = max_len;
        }
        
        // 复制数据到目标缓冲区
        for (uint16_t i = 0; i < data_len; i++)
        {
            buffer[i] = UCB_DATA.RxDataOUT->start[i];
        }
        
        uart1_update_endptr();
    }
    
    return data_len;
}
/**
 * @brief  更新串口接收数据指针
 * @param  None
 * @retval None
 */
void uart1_update_endptr(void)
{
    UCB_DATA.RxDataOUT++;
    if (UCB_DATA.RxDataOUT == UCB_DATA.RxDataEND)
    {
        UCB_DATA.RxDataOUT = &UCB_DATA.RxDataPtr[0];
    }
}


// 串口1中断
#ifdef BUILD_BOOT_LOADER
void USART1_IRQHandler()
{
    uint8_t clear = clear;
    uint16_t dma_remaining;
    uint16_t received_bytes;
    if (USART_GetITStatus(USART1, USART_IT_IDLE) != RESET)
    {
        clear = USART1->SR;
        clear = USART1->DR; // 清除空闲中断标志

        // 获取DMA剩余传输数量
        dma_remaining = DMA_GetCurrDataCounter(DMA1_Channel5);

        // 计算实际接收到的字节数
        received_bytes = (UART1_RX_MAX + 1) - dma_remaining;

        // 更新接收计数器
        UCB_DATA.RxCounter += received_bytes;

        // 设置当前缓冲区的结束位置
        UCB_DATA.RxDataIN->end = &UART1_RX_BUFF[UCB_DATA.RxCounter - 1];

        // 移动到下一个缓冲区
        UCB_DATA.RxDataIN++;

        // 检查是否到达缓冲区数组末尾
        if (UCB_DATA.RxDataIN == UCB_DATA.RxDataEND)
        {
            UCB_DATA.RxDataIN = &UCB_DATA.RxDataPtr[0]; // 回到起始位置
        }

        // 检查剩余空间是否足够进行下一次接收
        if (UART1_RX_SIZE - UCB_DATA.RxCounter > UART1_RX_MAX)
        {
            UCB_DATA.RxDataIN->start = &UART1_RX_BUFF[UCB_DATA.RxCounter];
        }
        else
        {
            UCB_DATA.RxDataIN->start = UART1_RX_BUFF; // 回到缓冲区起始位置
            UCB_DATA.RxCounter = 0;                   // 重置计数器
        }
        DMA_Cmd(DMA1_Channel5, DISABLE);
        // 重新配置DMA
        dma_init(DMA1_Channel5, (u32)&USART1->DR, (u32)UCB_DATA.RxDataIN->start, UART1_RX_MAX + 1);
    }
}
#endif
