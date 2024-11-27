#ifndef __UART_H__
#define __UART_H__

void UART_Init(void);
void UART_SendByte(unsigned char Byte);
char UART_Check(unsigned char length,unsigned char *p);

#endif
