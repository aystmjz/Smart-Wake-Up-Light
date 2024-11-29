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

/// @brief 初始化串口1(Debug)
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
}

uint8_t RxCounter, RxBuffer[UART_REC_LEN]; // 接收缓冲

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

// DEBUG串口打印
void Uart1_SendStr(char *SendBuf)
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

// ASRPRO串口发送一个字节数据
void ASRPRO_Send_byte(uint8_t byte)
{
	while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET)
		; // 等待发送完成
	USART2->DR = byte;
}

// ASRPRO串口发送多字节数据
void ASRPRO_Send_bytes(uint8_t *bytes, uint8_t length)
{
	uint8_t i = 0;
	while (i < length)
	{
		ASRPRO_Send_byte(bytes[i++]);
	}
}

unsigned char UART_Check(unsigned char length, unsigned char *p)
{
	unsigned int Temp = 0;
	unsigned char i;
	for (i = 1; i <= length; i++)
	{
		Temp += p[i];
	}
	if ((((~Temp + 1) & 0x00ff) == p[length + 2]) && ((~Temp + 1) >> 8) == p[length + 1])
		return 1;
	else
		return 0;
}

void Debug_printf(char *SendBuf)
{
	Uart1_SendStr(SendBuf);
}
void ASRPRO_printf(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
}

// MC串口中断
void ASRPRO_IRQHandler(void)
{
	if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
	{
		RxBuffer[RxCounter++] = USART_ReceiveData(USART2);
	}
}