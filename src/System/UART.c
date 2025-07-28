#include "UART.h"

// 加入以下代码,支持printf函数,而不需要选择use MicroLIB

#if 1

#pragma import(__use_no_semihosting) // 标准库需要的支持函数
struct __FILE
{
	int handle;
};
FILE __stdout;
void _sys_exit(int x) // 定义_sys_exit()以避免使用半主机模式
{
	x = x;
}
int fputc(int ch, FILE *f) // 重定义fputc函数
{
	while ((USART2->SR & 0X40) == 0)
	{
	} // 循环发送,直到发送完毕
	USART2->DR = (u8)ch;
	return ch;
}

#endif

volatile uint16_t ASRPRORxCounter;
volatile uint16_t BT24RxCounter;
volatile uint8_t ASRPRORxBuffer[ASRPRO_UART_REC_LEN]; // 接收缓冲
volatile uint8_t BT24RxBuffer[BT24_UART_REC_LEN];	  // 接收缓冲

/// @brief 初始化串口1(BT24)
/// @param bound 波特率
void uart1_init(uint32_t bound)
{
#if DEBUG_MODE == DEBUG_MODE_BT24T
#else
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
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		  // 子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  // IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);

	USART_InitStructure.USART_BaudRate = bound;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;						// 字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;							// 一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;								// 无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // 无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;					// 收发模式
	USART_Init(USART1, &USART_InitStructure);
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	USART_Cmd(USART1, ENABLE);
#endif
}

/// @brief 初始化串口2(ASRPRO)
/// @param bound 波特率
void uart2_init(uint32_t bound)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	USART_DeInit(USART2); // 复位串口2
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; // 复用推挽输出
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; // 浮空输入
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; // 抢占优先级0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;		  // 子优先级1
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  // IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);

	USART_InitStructure.USART_BaudRate = bound;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;						// 字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;							// 一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;								// 无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // 无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;					// 收发模式
	USART_Init(USART2, &USART_InitStructure);
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
	USART_Cmd(USART2, ENABLE);
}

/// @brief 初始化总串口
/// @param bound 波特率
void Uart_Init(uint32_t bound)
{
	uart1_init(bound);
	uart2_init(bound);
}

// 串口1(BT24)-----------------------------------------------------

// BT24串口打印
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
	while ((USART1->SR & 0X40) == 0)
	{
	} // 确保发送完成
}

void BT24_printf(char *SendBuf)
{
#if DEBUG_MODE == DEBUG_MODE_BT24
#else
	BT24_SendStr(SendBuf);
#endif
}

void BT24_Clear_Buff(void)
{
	BT24RxCounter = 0;
	for (uint16_t i = 0; i < BT24_UART_REC_LEN; i++)
	{
		BT24RxBuffer[i] = 0;
	}
}

void Debug_printf(const char *format, ...)
{
#if DEBUG_MODE == DEBUG_MODE_NORMAL || DEBUG_MODE == DEBUG_MODE_STM32
	static char Debug_buffer[DEBUG_BUFF_LEN];
	va_list args;
	va_start(args, format);
	vsnprintf(Debug_buffer, sizeof(Debug_buffer), format, args);
	va_end(args);
	BT24_SendStr(Debug_buffer);
#endif
}

// BT24串口中断
void USART1_IRQHandler(void)
{
	if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
	{
		BT24RxBuffer[BT24RxCounter++] = USART_ReceiveData(USART1);
		if (BT24RxBuffer[BT24RxCounter - 1] == '}')
		{
			if (BT24RxBuffer[BT24RxCounter - 2] == '"')
			{
				BT24RxCounter = 0;
			}
		}
#if DEBUG_MODE == DEBUG_MODE_ASRPRO
		while ((USART2->SR & 0X40) == 0)
		{
		} // 等待发送完成
		USART2->DR = (uint8_t)BT24RxBuffer[BT24RxCounter - 1];
#endif
		BT24RxCounter %= BT24_UART_REC_LEN;
	}
}

// 串口2(ASRPRO)-----------------------------------------------------

void ASRPRO_Clear_Buff(void)
{
	ASRPRORxCounter = 0;
	for (uint16_t i = 0; i < ASRPRO_UART_REC_LEN; i++)
	{
		ASRPRORxBuffer[i] = 0;
	}
}

void ASRPRO_printf(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
}

#if !BOOT_LOADER
// ASRPRO串口中断
void USART2_IRQHandler(void)
{
	if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
	{
		ASRPRORxBuffer[ASRPRORxCounter++] = USART_ReceiveData(USART2);
#if DEBUG_MODE == DEBUG_MODE_NORMAL || DEBUG_MODE == DEBUG_MODE_ASRPRO
		while ((USART1->SR & 0X40) == 0)
		{
		} // 等待发送完成
		USART1->DR = (uint8_t)ASRPRORxBuffer[ASRPRORxCounter - 1];
#endif
		ASRPRORxCounter %= ASRPRO_UART_REC_LEN;
	}
}
#endif
